#include "shd.h"
#include "upscale.h"
#include "sensor.h"
#include "runtime_error.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace
{
constexpr GLenum kTextureMaxAnisotropy = 0x84FE;
constexpr GLenum kMaxTextureMaxAnisotropy = 0x84FF;

bool FSupportsAnisotropicFiltering()
{
	GLint extensionCount = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
	for (GLint i = 0; i < extensionCount; ++i)
	{
		const char* extension = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
		if (extension != nullptr &&
			(std::strcmp(extension, "GL_EXT_texture_filter_anisotropic") == 0 ||
			 std::strcmp(extension, "GL_ARB_texture_filter_anisotropic") == 0))
		{
			return true;
		}
	}
	return false;
}

void ApplyAnisotropyToBoundTexture()
{
	if (!FSupportsAnisotropicFiltering())
		return;

	const float level = static_cast<float>((std::clamp)(
		g_anisotropicFiltering, 1, MaxSupportedAnisotropicFiltering()));
	glTexParameterf(GL_TEXTURE_2D, kTextureMaxAnisotropy, level);
}

bool ReportTextureGlErrors(const char* operation, std::size_t imageIndex,
	const BMP& image, int mipLevel = -1)
{
	bool succeeded = true;
	for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError())
	{
		succeeded = false;
		ReportRuntimeError("Graphics",
			"OpenGL error 0x%X during PCBRX texture %s: image=%zu, GL texture=%u, "
			"format=%u, size=%ux%u, mip=%d/%u, bytes=%zu.",
			error, operation, imageIndex, image.glDiffuseMap, image.textureFormat,
			image.bmpWidth, image.bmpHeight, mipLevel,
			image.mipCount, static_cast<std::size_t>(image.cbPixels));
	}
	return succeeded;
}
}

void UnloadShaders()
{
    // World material samplers use texture units 0-2. Detach them before the
    // backing TEX objects are destroyed so a deleted world cannot leave stale
    // bindings in the next one.
    for (int unit = 0; unit < 3; ++unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);

    for (int i = 0; i < g_cbmp; i++)
    {
        if (g_abmp[i].glShadowMap != 0)
        {
            glDeleteTextures(1, &g_abmp[i].glShadowMap);
            g_abmp[i].glShadowMap = 0;
        }

        if (g_abmp[i].glDiffuseMap != 0)
        {
            glDeleteTextures(1, &g_abmp[i].glDiffuseMap);
            g_abmp[i].glDiffuseMap = 0;
        }

        if (g_abmp[i].glSaturateMap != 0)
        {
            glDeleteTextures(1, &g_abmp[i].glSaturateMap);
            g_abmp[i].glSaturateMap = 0;
        }
    }

    g_cshd = 0;
    g_ashd.clear();
    g_ashd.shrink_to_fit();
    g_cbmp = 0;
    g_abmp.clear();
    g_abmp.shrink_to_fit();
    g_cfontBrx = 0;
    g_afontBrx.clear();
    g_afontBrx.shrink_to_fit();
    g_pfont = nullptr;
    g_pfontScreenCounters = nullptr;
    g_pfontJoy = nullptr;
    g_grfzonShaders = 0;

    g_cpsaa = 0;
    g_apsaa.clear();
    g_apsaa.shrink_to_fit();
    g_apsaaSw.clear();
    g_apsaaSw.shrink_to_fit();
    g_testFontBrx.clear();
    g_testFontBrx.shrink_to_fit();
}

void LoadBitmapsFromBrx(CBinaryInputStream* pbis)
{
    g_cbmp = static_cast<int>(pbis->U32Read());
    g_abmp.resize(g_cbmp);

    for (int i = 0; i < g_cbmp; i++)
    {
        g_abmp[i].bmpWidth   = pbis->U32Read();
        g_abmp[i].bmpHeight  = pbis->U32Read();
        g_abmp[i].grfzon     = pbis->U32Read();
        g_abmp[i].mipCount   = pbis->U32Read();
        g_abmp[i].cbPixels   = pbis->U32Read();
        g_abmp[i].baseOffset = pbis->U32Read();
        g_abmp[i].textureFlags = pbis->U32Read();
        g_abmp[i].textureFormat = pbis->U32Read();
    }
}

void LoadFontsFromBrx(CBinaryInputStream* pbis)
{
    // Loading number of fonts from file
    g_cfontBrx = pbis->U16Read();
    g_afontBrx.resize(g_cfontBrx);

    // Loading font property's from binary file
    for (int i = 0; i < g_cfontBrx; i++)
    {
        g_afontBrx[i].LoadFromBrx(pbis);
        g_afontBrx[i].m_grffont = 1 << (i & 0x1f);
    }

    g_pfont = PfontFromFont(0);
}

void LoadTexFromBrx(TEX* ptex, CBinaryInputStream* pbis)
{
    ptex->oid    = (OID)pbis->S16Read();
    ptex->grftex = pbis->S16Read();
    ptex->cibmp = pbis->U16Read();
    ptex->ambientImageIndex.resize(ptex->cibmp);
    ptex->diffuseImageIndex.resize(ptex->cibmp);
    ptex->saturateImageIndex.resize(ptex->cibmp);
    ptex->abmp.resize(ptex->cibmp, nullptr);
    for (int i = 0; i < ptex->cibmp; ++i)
    {
        ptex->ambientImageIndex[i] = pbis->S32Read();
        ptex->diffuseImageIndex[i] = pbis->S32Read();
        ptex->saturateImageIndex[i] = pbis->S32Read();
        const int32_t displayIndex = ptex->diffuseImageIndex[i] >= 0
            ? ptex->diffuseImageIndex[i] : ptex->ambientImageIndex[i];
        if (displayIndex >= 0 && displayIndex < g_cbmp)
            ptex->abmp[i] = &g_abmp[displayIndex];
    }
}

void ConvertUserHsvToUserRgb(glm::vec3& pvecHSV, glm::vec3& pvecRGB)
{
    float h = pvecHSV.x;
    float s = pvecHSV.y * (1.0f / 255.0f);  // scale from 0�255
    float v = pvecHSV.z * (1.0f / 255.0f);  // scale from 0�255

    // Clamp hue to [0, 360], or set to -1 for undefined
    float hue = (h >= 0.0f) ? glm::clamp(h, 0.0f, 360.0f) : -1.0f;

    // Clamp saturation and value to [0, 1]
    s = glm::clamp(s, 0.0f, 1.0f);
    v = glm::clamp(v, 0.0f, 1.0f);

    glm::vec3 rgb(0.0f);

    if (s == 0.0f || hue < 0.0f)
        rgb = glm::vec3(v);
    else
    {
        hue = (hue == 360.0f) ? 0.0f : hue;
        float hueSegment = hue / 60.0f;
        int i = static_cast<int>(std::floor(hueSegment));
        float f = hueSegment - i;

        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));

        switch (i)
        {
            case 0:  rgb = glm::vec3(v, t, p); break;
            case 1:  rgb = glm::vec3(q, v, p); break;
            case 2:  rgb = glm::vec3(p, v, t); break;
            case 3:  rgb = glm::vec3(p, q, v); break;
            case 4:  rgb = glm::vec3(t, p, v); break;
            case 5:  rgb = glm::vec3(v, p, q); break;
            default: rgb = glm::vec3(0.0f);    break;
        }
    }

    pvecRGB = rgb * 255.0f;
}

SHD* PshdFindShader(OID oid)
{
    if (oid == -1)
        return nullptr;

    for (int i = 0; i < g_cshd; i++)
    {
        if (g_ashd[i].oid == oid)
            return &g_ashd[i];
    }

    return nullptr;
}

void LoadShadersFromBrx(CBinaryInputStream* pbis)
{
    LoadBitmapsFromBrx(pbis);

    g_cshd = pbis->U16Read();
    g_ashd.resize(g_cshd);

    g_cpsaa = pbis->U16Read();
    g_apsaa.reserve(g_cpsaa);

    for (int i = 0; i < g_cshd; i++)
    {
        SHD& shd = g_ashd[i];
        
        shd.shdk = (SHDK)pbis->U8Read();

        shd.grfshd = pbis->U8Read();
        shd.oid = (OID)pbis->S16Read();

        shd.rgba = pbis->ReadVector4();
        shd.rgbaVolume = pbis->ReadVector4();

        shd.grfzon = pbis->U32Read();
        shd.oidAltSat = (OID)pbis->U16Read();
        shd.rp = (RP)pbis->U8Read();
        shd.ctex = pbis->U8Read();

        shd.atex.resize(shd.ctex);

        std::shared_ptr<SAA> psaaOwner = PsaaLoadFromBrx(pbis);
        SAA* psaa = psaaOwner.get();
        shd.psaa = psaa;

        if (psaa != nullptr)
        {
            psaa->sai.pshd = &shd;
            g_apsaa.push_back(psaa);
        }

        for (int a = 0; a < shd.ctex; a++)
        {
            LoadTexFromBrx(&shd.atex[a], pbis);
            shd.atex[a].pshd = &shd;
        }

        if (!shd.atex.empty())
            shd.cframe = shd.atex[0].cibmp;
        else
            shd.cframe = 0;

        // ------------------------------------------------------------
        // Original Sly-style SHDP allocation.
        // ------------------------------------------------------------
        switch (shd.shdk)
        {
            case SHDK_ThreeWay:
            shd.cshdp = 3;
            break;

            case SHDK_ProjectedVolume:
            shd.cshdp = 4;
            break;

            default:
            shd.cshdp = 1;
            break;
        }

        shd.ashdp.clear();
        shd.ashdp.resize(shd.cshdp);

        auto AllocShdp = [&](int index, int cqwRegs)
        {
            SHDP& shdp = shd.ashdp[index];

            shdp.cqwRegs = cqwRegs;

            // vector<uint16_t>
            // 1 QW = 16 bytes = 8 uint16_t
            shdp.aaqwRegs.resize(shd.cframe * cqwRegs * 8);
        };

        switch (shd.shdk)
        {
            case SHDK_ThreeWay:
            {
                AllocShdp(0, 8);
                AllocShdp(1, 8);
                AllocShdp(2, 8);
                break;
            }

            case SHDK_ProjectedVolume:
            {
                int extra = 0;

                if ((shd.grfshd & 2) != 0)
                    extra = 3;

                AllocShdp(0, extra + 5);
                AllocShdp(1, 5);
                AllocShdp(2, 7);
                AllocShdp(3, 4);
                break;
            }

            case SHDK_Shadow:
            case SHDK_SpotLight:
            {
                AllocShdp(0, 6);
                break;
            }

            case SHDK_CreateTexture:
            {
                AllocShdp(0, 7);
                break;
            }

            case SHDK_MurkClear:
            {
                AllocShdp(0, 4);
                break;
            }

            default:
            {
                AllocShdp(0, 8);
                break;
            }
        }
    }

    LoadFontsFromBrx(pbis);

    for (int i = 0; i < g_apsaa.size(); i++)
    {
        if (g_apsaa[i]->pvtsaa != nullptr)
            g_apsaa[i]->pvtsaa->pfnPostSaaLoad(g_apsaa[i]);
    }

    PostBlotsLoad();
}

void SetSaiIframe(SAI* psai, int iframe)
{
    if (!psai || !psai->pshd || psai->pshd->cframe <= 0)
        return;

    int maxFrame = psai->pshd->cframe - 1;

    if (iframe < 0)
        iframe = 0;
    else if (iframe > maxFrame)
        iframe = maxFrame;

    if (psai->iframe == iframe)
        return;

    psai->iframe = iframe;
}

void LoadTexturesFromBrx(CBinaryInputStream* pbis)
{
    const size_t dataStart = pbis->GetOffset();
    const size_t dataSize = pbis->Remaining();
    size_t dataEnd = dataStart;

    // Attribute any old error to work performed before this loader. Every GL
    // operation below is then checked immediately, so a later framebuffer
    // resize cannot accidentally receive and mislabel the texture error.
    for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError())
    {
        ReportRuntimeError("Graphics",
            "OpenGL error 0x%X was pending before PCBRX texture loading began.", error);
    }

    for (std::size_t imageIndex = 0; imageIndex < g_abmp.size(); ++imageIndex)
    {
        BMP& image = g_abmp[imageIndex];
        if (image.bmpWidth == 0 || image.bmpHeight == 0 ||
            image.bmpWidth > 32768 || image.bmpHeight > 32768 ||
            image.mipCount == 0 || image.mipCount > 32 ||
            image.baseOffset > dataSize || image.cbPixels > dataSize - image.baseOffset)
            throw std::runtime_error("invalid PCBRX texture descriptor");

        constexpr uint32_t kTextureRgba8 = 0;
        constexpr uint32_t kTextureBc1 = 1;
        constexpr uint32_t kTextureBc3 = 2;
        constexpr uint32_t kGenerateMipmaps = 1u << 2;
        if (image.textureFormat > kTextureBc3)
            throw std::runtime_error("unsupported PCBRX texture format");

        size_t expectedBytes = 0;
        uint32_t expectedWidth = image.bmpWidth;
        uint32_t expectedHeight = image.bmpHeight;
        for (uint32_t level = 0; level < image.mipCount; ++level)
        {
            const size_t levelBytes = image.textureFormat == kTextureRgba8
                ? static_cast<size_t>(expectedWidth) * expectedHeight * 4
                : static_cast<size_t>((expectedWidth + 3) / 4) * ((expectedHeight + 3) / 4) *
                    (image.textureFormat == kTextureBc1 ? 8 : 16);
            if (expectedBytes > std::numeric_limits<size_t>::max() - levelBytes)
                throw std::runtime_error("PCBRX texture dimensions overflow host size_t");
            expectedBytes += levelBytes;
            expectedWidth = std::max(1u, expectedWidth >> 1);
            expectedHeight = std::max(1u, expectedHeight >> 1);
        }
        if (expectedBytes != image.cbPixels)
            throw std::runtime_error("PCBRX texture payload size does not match its mip chain");

        image.diffuseTexture.resize(image.cbPixels);
        pbis->Seek(dataStart + image.baseOffset);
        pbis->ReadBytes(image.diffuseTexture.data(), image.diffuseTexture.size());

        dataEnd = std::max(dataEnd, dataStart + image.baseOffset + image.cbPixels);

        glGenTextures(1, &image.glDiffuseMap);
        glBindTexture(GL_TEXTURE_2D, image.glDiffuseMap);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
            (image.textureFlags & 1) ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
            (image.textureFlags & 2) ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        const bool generateMipmaps = (image.textureFlags & kGenerateMipmaps) != 0;
        uint32_t storageMipCount = image.mipCount;
        if (generateMipmaps)
        {
            storageMipCount = 1;
            for (uint32_t dimension = std::max(image.bmpWidth, image.bmpHeight);
                dimension > 1; dimension >>= 1)
                ++storageMipCount;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            storageMipCount > 1
				? (g_textureFiltering != 0 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST)
				: GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        ApplyAnisotropyToBoundTexture();
        ReportTextureGlErrors("setup", imageIndex, image);
        constexpr GLenum kGlBc1 = 0x83F0; // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
        constexpr GLenum kGlBc3 = 0x83F3; // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
        const GLenum internalFormat = image.textureFormat == kTextureBc1
            ? kGlBc1 : image.textureFormat == kTextureBc3 ? kGlBc3 : GL_RGBA8;
        glTexStorage2D(GL_TEXTURE_2D, storageMipCount, internalFormat,
            image.bmpWidth, image.bmpHeight);
        ReportTextureGlErrors("glTexStorage2D", imageIndex, image);

        size_t levelOffset = 0;
        uint32_t width = image.bmpWidth;
        uint32_t height = image.bmpHeight;
        for (uint32_t level = 0; level < image.mipCount; ++level)
        {
            const size_t levelBytes = image.textureFormat == kTextureRgba8
                ? static_cast<size_t>(width) * height * 4
                : static_cast<size_t>((width + 3) / 4) * ((height + 3) / 4) *
                    (image.textureFormat == kTextureBc1 ? 8 : 16);
            if (levelOffset + levelBytes > image.diffuseTexture.size())
                throw std::runtime_error("truncated PCBRX texture mip chain");
            if (image.textureFormat == kTextureRgba8)
                glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, width, height,
                    GL_RGBA, GL_UNSIGNED_BYTE, image.diffuseTexture.data() + levelOffset);
            else
                glCompressedTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, width, height,
                    internalFormat, static_cast<GLsizei>(levelBytes),
                    image.diffuseTexture.data() + levelOffset);
            ReportTextureGlErrors(image.textureFormat == kTextureRgba8
                ? "glTexSubImage2D" : "glCompressedTexSubImage2D",
                imageIndex, image, static_cast<int>(level));
            levelOffset += levelBytes;
            width = std::max(1u, width >> 1);
            height = std::max(1u, height >> 1);
        }

        if (levelOffset != image.diffuseTexture.size())
            throw std::runtime_error("PCBRX texture payload size does not match its mip chain");

        if (generateMipmaps)
        {
            if (image.mipCount != 1)
                throw std::runtime_error("runtime mip generation requires one source level");
            glGenerateMipmap(GL_TEXTURE_2D);
            ReportTextureGlErrors("glGenerateMipmap", imageIndex, image);
        }

        if (!g_fDeveloperDebugMode)
        {
            image.diffuseTexture.clear();
            image.diffuseTexture.shrink_to_fit();
        }
    }

    auto ImageHandle = [](int32_t index) -> GLuint
    {
        return index >= 0 && static_cast<size_t>(index) < g_abmp.size()
            ? g_abmp[static_cast<size_t>(index)].glDiffuseMap : 0;
    };

    for (SHD& shd : g_ashd)
    {
        for (TEX& tex : shd.atex)
        {
            tex.glShadowMap.resize(tex.cibmp);
            tex.glDiffuseMap.resize(tex.cibmp);
            tex.glSaturateMap.resize(tex.cibmp);
            for (uint32_t iframe = 0; iframe < tex.cibmp; ++iframe)
            {
                tex.glShadowMap[iframe] = ImageHandle(tex.ambientImageIndex[iframe]);
                tex.glDiffuseMap[iframe] = ImageHandle(tex.diffuseImageIndex[iframe]);
                tex.glSaturateMap[iframe] = ImageHandle(tex.saturateImageIndex[iframe]);
                if (tex.glDiffuseMap[iframe] == 0)
                    tex.glDiffuseMap[iframe] = tex.glShadowMap[iframe];
            }
        }
    }

    pbis->Seek(dataEnd);
}

int MaxSupportedAnisotropicFiltering()
{
	if (!FSupportsAnisotropicFiltering())
		return 1;

	GLfloat maximum = 1.0f;
	glGetFloatv(kMaxTextureMaxAnisotropy, &maximum);
	return (std::max)(1, static_cast<int>(maximum));
}

void ApplyAnisotropicFilteringSettings(int level)
{
	const int maximum = MaxSupportedAnisotropicFiltering();
	g_anisotropicFiltering = (std::clamp)(level, 1, maximum);

	GLint previousTexture = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
	for (const BMP& image : g_abmp)
	{
		if (image.glDiffuseMap != 0)
		{
			glBindTexture(GL_TEXTURE_2D, image.glDiffuseMap);
			ApplyAnisotropyToBoundTexture();
		}
	}
	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
}

void ApplyTextureFilteringSettings(int mode)
{
	g_textureFiltering = mode != 0 ? 1 : 0;
	GLint previousTexture = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
	for (const BMP& image : g_abmp)
	{
		if (image.glDiffuseMap == 0)
			continue;
		glBindTexture(GL_TEXTURE_2D, image.glDiffuseMap);
		const bool hasMipmaps = image.mipCount > 1 || (image.textureFlags & (1u << 2)) != 0;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, hasMipmaps
			? (g_textureFiltering != 0 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST)
			: GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
}

void UpdateShaders(float dt)
{
    for (int i = 0; i < g_apsaa.size(); ++i)
    {
        SAA* saa = g_apsaa[i];
        if (!saa)
            continue;

        if (!FUpdatableSaa(saa))
            continue;

        auto* vtsaa = saa->pvtscroller;
        if (!vtsaa || !vtsaa->pfnUpdateScroller)
            continue;

        vtsaa->pfnUpdateScroller((SCROLLER*)saa, dt);

    }
}

int g_grfzonShaders;
int g_cbmp;
std::vector <BMP> g_abmp;
int g_cshd;
std::vector <SHD> g_ashd;
std::vector <TEX> g_atex;
int g_cpsaa;
std::vector <SAA*> g_apsaa;
std::vector<std::shared_ptr<SAA>> g_apsaaSw;
SAI* g_psaiUpdate = nullptr;
SAI* g_psaiUpdateTail = nullptr;
int g_anisotropicFiltering = 1;
