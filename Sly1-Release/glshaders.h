#pragma once
#ifdef CANE_GL41
#include <glad/glad.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// GL-free helpers come first so tests can use them with CANE_SHADER_TOOLS_ONLY
// defined, without the game headers below.

// GLSL version used on the OpenGL 4.1 core path (macOS caps GL at 4.1).
inline constexpr const char kShaderVersionGL41[] = "410 core";

// Replaces a leading "#version 430 core" directive with "#version <targetVersion>".
// A BOM or whitespace before the directive is kept; any other source is returned unchanged.
inline std::string RewriteShaderVersion(std::string_view source, std::string_view targetVersion)
{
	constexpr std::string_view bom = "\xEF\xBB\xBF";
	constexpr std::string_view directive = "#version 430 core";

	size_t start = source.substr(0, bom.size()) == bom ? bom.size() : 0;
	while (start < source.size() &&
		(source[start] == ' ' || source[start] == '\t' || source[start] == '\r' || source[start] == '\n'))
		++start;

	if (source.compare(start, directive.size(), directive) != 0)
		return std::string(source);

	// Only the whole directive counts: "#version 430 core_x" is left alone.
	const size_t end = start + directive.size();
	if (end < source.size() && source[end] != '\n' && source[end] != '\r' &&
		source[end] != ' ' && source[end] != '\t')
		return std::string(source);

	std::string rewritten;
	rewritten.reserve(source.size() + targetVersion.size());
	rewritten.append(source.substr(0, start));
	rewritten.append("#version ");
	rewritten.append(targetVersion);
	rewritten.append(source.substr(end));
	return rewritten;
}

// Uniform block binding points, set from C++ because layout(binding = N) on
// uniform blocks needs GL 4.2 / ARB_shading_language_420pack, which macOS lacks.
// Matches the stream bind indices: CMGL at 0, per-draw blocks at 1.
struct UniformBlockBinding
{
	const char* name;
	GLuint binding;
};

inline constexpr UniformBlockBinding kUniformBlockBindings[] =
{
	{ "CMGL", 0 },
	{ "RO", 1 },
	{ "ROCEL", 1 },
	{ "BlipGroup", 1 },
};

// Texture unit for dysh.vert's bone matrices (a GL_TEXTURE_BUFFER over GLOBSET's
// boneMatrixSSBO). Unit 0 holds material textures and SHADOW_TEXTURE_UNIT is 3.
inline constexpr GLint kBoneMatrixTextureUnit = 15;

// Texture units for celborder.vert's buffer textures; it reads bones from
// kBoneMatrixTextureUnit as well.
inline constexpr GLint kEdgeTextureUnit = 12;
inline constexpr GLint kEdgePoseDposTextureUnit = 13;
inline constexpr GLint kPoseWeightTextureUnit = 14;

// Texture units for glob's shadow buffer textures (shadow.cpp's shadowSsbo
// and activeShadowsSsbo).
inline constexpr GLint kShadowTextureUnit = 4;
inline constexpr GLint kActiveShadowTextureUnit = 5;

// Texture units for glob.vert's light buffer textures (light.cpp's
// g_lightSsbo, g_activeLightsSsbo and g_tvLightSsbo).
inline constexpr GLint kLightTextureUnit = 6;
inline constexpr GLint kActiveLightTextureUnit = 7;
inline constexpr GLint kTvLightTextureUnit = 8;

// Texture units for glob.vert's per-object buffer textures: WRBGLOB_GL's
// ssboState and GLOB's poseDposSSBO and poseDnormalSSBO. glob.vert also reads
// bones from kBoneMatrixTextureUnit and pose weights from kPoseWeightTextureUnit.
inline constexpr GLint kWarpStateTextureUnit = 9;
inline constexpr GLint kPoseDposTextureUnit = 10;
inline constexpr GLint kPoseDnormalTextureUnit = 11;

// Texture unit for glob.vert's cached three-way material (GLOB's
// ssboCachedMaterial). Units 0-15 are taken. GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS
// (16 here) caps how many samplers a vertex shader uses, not the unit numbers,
// which only have to stay below GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS (80 here).
inline constexpr GLint kCachedMaterialTextureUnit = 16;

// Buffer texture formats. A CELPOINT (vec4 position, uvec4 boneIndices,
// vec4 boneWeights; 48 bytes) is three RGBA32UI texels: the shader turns the
// float parts back with uintBitsToFloat. Never view it as float, since small
// bone indices read as float bits are denormals a GPU may flush to zero.
inline constexpr GLenum kEdgeTextureFormat = GL_RGBA32UI;
// Bone matrices (GLOBSET::boneMatrixSSBO): a column-major mat4 per bone, one
// column per RGBA32F texel (glob.cpp, PostGlobsetLoad).
inline constexpr GLenum kBoneMatrixTextureFormat = GL_RGBA32F;
inline constexpr GLenum kEdgePoseDposTextureFormat = GL_RGBA32F;
inline constexpr GLenum kPoseWeightTextureFormat = GL_R32F;
// The shadow buffer is a 16-byte header then 224-byte SHADOWBLKs: one texel,
// then 14 per record, read as RGBA32UI for the same reason as CELPOINT. The
// active list is ACTIVESHADOWS as-is, one int per texel.
inline constexpr GLenum kShadowTextureFormat = GL_RGBA32UI;
inline constexpr GLenum kActiveShadowTextureFormat = GL_R32I;
// The light buffer is a 16-byte header then 208-byte LIGHTBLKs: one texel,
// then 13 per record, read as RGBA32UI like the shadows. The TV light buffer
// is two LIGHTBLKs with no header. The active list is ACTIVELIGHTS as-is, one
// int per texel: the static count at texel 0, the dynamic count at 257.
inline constexpr GLenum kLightTextureFormat = GL_RGBA32UI;
inline constexpr GLenum kActiveLightTextureFormat = GL_R32I;
inline constexpr GLenum kTvLightTextureFormat = GL_RGBA32UI;
// Warp state and pose deltas are plain vec4 arrays, one RGBA32F texel each.
inline constexpr GLenum kWarpStateTextureFormat = GL_RGBA32F;
inline constexpr GLenum kPoseDposTextureFormat = GL_RGBA32F;
inline constexpr GLenum kPoseDnormalTextureFormat = GL_RGBA32F;
// The material cache is one 48-byte MATERIAL per vertex (float ambient, 12
// bytes of padding, vec4 midtone, vec4 light): three RGBA32F texels.
inline constexpr GLenum kCachedMaterialTextureFormat = GL_RGBA32F;
inline constexpr GLsizeiptr kCachedMaterialStride = 48;

// glob's transform feedback varyings, captured interleaved into the material
// cache: ambient at byte 0, the skip leaves bytes 4-15 alone, midtone at 16,
// light at 32. Set on the glob program before it links.
inline constexpr const char* kGlobFeedbackVaryings[] =
{
	"material.ambient",
	"gl_SkipComponents3",
	"material.midtone",
	"material.light",
};
inline constexpr GLsizei kGlobFeedbackVaryingCount = 4;

// Sampler texture units, set from C++ for the same reason as the blocks above:
// layout(binding = N) on samplers also needs GL 4.2.
struct SamplerUnit
{
	const char* name;
	GLint unit;
};

inline constexpr SamplerUnit kSamplerUnits[] =
{
	{ "boneMatrixTex", kBoneMatrixTextureUnit },
	{ "edgeTex", kEdgeTextureUnit },
	{ "edgePoseDposTex", kEdgePoseDposTextureUnit },
	{ "poseWeightTex", kPoseWeightTextureUnit },
	{ "shadowTex", kShadowTextureUnit },
	{ "activeShadowTex", kActiveShadowTextureUnit },
	{ "lightTex", kLightTextureUnit },
	{ "activeLightTex", kActiveLightTextureUnit },
	{ "tvLightTex", kTvLightTextureUnit },
	{ "warpStateTex", kWarpStateTextureUnit },
	{ "poseDposTex", kPoseDposTextureUnit },
	{ "poseDnormalTex", kPoseDnormalTextureUnit },
	{ "cachedMaterialTex", kCachedMaterialTextureUnit },
};

// Call after a successful link; blocks and samplers the program doesn't declare
// are skipped. glProgramUniform1i leaves the current program alone.
inline void ApplyUniformBlockBindings(GLuint program)
{
	for (const UniformBlockBinding& entry : kUniformBlockBindings)
	{
		const GLuint index = glGetUniformBlockIndex(program, entry.name);
		if (index != GL_INVALID_INDEX)
			glUniformBlockBinding(program, index, entry.binding);
	}

	for (const SamplerUnit& entry : kSamplerUnits)
	{
		const GLint location = glGetUniformLocation(program, entry.name);
		if (location >= 0)
			glProgramUniform1i(program, location, entry.unit);
	}
}

// glad only loads glTexStorage2D/3D for GL 4.2+, but the macOS 4.1 driver exports
// them (GL_ARB_texture_storage). Fills whichever pointer glad left null from
// `load`; returns false if either is still missing.
inline bool LoadTexStorageFallback(GLADloadproc load)
{
	if (glad_glTexStorage2D == nullptr)
		glad_glTexStorage2D = reinterpret_cast<PFNGLTEXSTORAGE2DPROC>(load("glTexStorage2D"));
	if (glad_glTexStorage3D == nullptr)
		glad_glTexStorage3D = reinterpret_cast<PFNGLTEXSTORAGE3DPROC>(load("glTexStorage3D"));
	return glad_glTexStorage2D != nullptr && glad_glTexStorage3D != nullptr;
}

// Buffer textures over GL buffers, keyed by buffer name. They live in this
// sidecar instead of in the game structs so the reconstructed layouts stay
// as they are. GL reuses buffer names, so deleting a buffer must go through
// DeleteBufferTexture first.
struct BufferTexture
{
	GLuint texture;
	GLenum internalFormat;
};

inline std::unordered_map<GLuint, BufferTexture>& BufferTextures()
{
	static std::unordered_map<GLuint, BufferTexture> s_bufferTextures;
	return s_bufferTextures;
}

// Call after each glBufferData that creates or respecifies `buffer`. Reuses
// the buffer's texture if it has one and reattaches it with `internalFormat`.
// Binds GL_TEXTURE_BUFFER on the active unit and leaves it unbound.
inline GLuint CreateBufferTexture(GLuint buffer, GLenum internalFormat)
{
	if (buffer == 0)
		return 0;

	BufferTexture& entry = BufferTextures().try_emplace(buffer, BufferTexture{ 0, internalFormat }).first->second;
	if (entry.texture == 0)
		glGenTextures(1, &entry.texture);
	entry.internalFormat = internalFormat;

	glBindTexture(GL_TEXTURE_BUFFER, entry.texture);
	glTexBuffer(GL_TEXTURE_BUFFER, internalFormat, buffer);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	return entry.texture;
}

// Returns 0 when `buffer` has no buffer texture.
inline GLuint GetBufferTexture(GLuint buffer)
{
	const auto it = BufferTextures().find(buffer);
	return it != BufferTextures().end() ? it->second.texture : 0;
}

// Deletes `buffer`'s texture, if any, and forgets the buffer name.
inline void DeleteBufferTexture(GLuint buffer)
{
	const auto it = BufferTextures().find(buffer);
	if (it == BufferTextures().end())
		return;

	if (it->second.texture != 0)
		glDeleteTextures(1, &it->second.texture);
	BufferTextures().erase(it);
}

// The buffer texture format each buffer-sampler unit reads. Placeholders take
// the same format so a fetch sees its sampler's own class: float, unsigned or
// signed.
struct BufferSamplerFormat
{
	GLint unit;
	GLenum format;
};

inline constexpr BufferSamplerFormat kBufferSamplerFormats[] =
{
	{ kBoneMatrixTextureUnit, kBoneMatrixTextureFormat },
	{ kEdgeTextureUnit, kEdgeTextureFormat },
	{ kEdgePoseDposTextureUnit, kEdgePoseDposTextureFormat },
	{ kPoseWeightTextureUnit, kPoseWeightTextureFormat },
	{ kShadowTextureUnit, kShadowTextureFormat },
	{ kActiveShadowTextureUnit, kActiveShadowTextureFormat },
	{ kLightTextureUnit, kLightTextureFormat },
	{ kActiveLightTextureUnit, kActiveLightTextureFormat },
	{ kTvLightTextureUnit, kTvLightTextureFormat },
	{ kWarpStateTextureUnit, kWarpStateTextureFormat },
	{ kPoseDposTextureUnit, kPoseDposTextureFormat },
	{ kPoseDnormalTextureUnit, kPoseDnormalTextureFormat },
	{ kCachedMaterialTextureUnit, kCachedMaterialTextureFormat },
};

// 0 when `unit` is not a buffer-sampler unit.
inline GLenum BufferSamplerFormatForUnit(GLint unit)
{
	for (const BufferSamplerFormat& entry : kBufferSamplerFormats)
	{
		if (entry.unit == unit)
			return entry.format;
	}
	return 0;
}

// One 16-byte, zero-filled buffer texture per format. Apple's 4.1 driver
// logs "GLD_TEXTURE_INDEX_BUFFER is unloadable" for any buffer sampler a
// program declares whose unit has no buffer texture, even when the shader's
// own flags skip the fetch, and samples zero instead. A placeholder keeps
// every unit bound and reads the same zero.
struct PlaceholderBufferTextureEntry
{
	GLenum format;
	GLuint buffer;
	GLuint texture;
};

inline std::vector<PlaceholderBufferTextureEntry>& PlaceholderBufferTextures()
{
	static std::vector<PlaceholderBufferTextureEntry> s_placeholders;
	return s_placeholders;
}

// Created on first use; binds GL_TEXTURE_BUFFER on the active unit and
// leaves it unbound.
inline GLuint PlaceholderBufferTexture(GLenum format)
{
	for (const PlaceholderBufferTextureEntry& entry : PlaceholderBufferTextures())
	{
		if (entry.format == format)
			return entry.texture;
	}
	PlaceholderBufferTextureEntry entry{ format, 0, 0 };
	const unsigned char zeros[16] = {};
	glGenBuffers(1, &entry.buffer);
	glBindBuffer(GL_TEXTURE_BUFFER, entry.buffer);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(zeros), zeros, GL_STATIC_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glGenTextures(1, &entry.texture);
	glBindTexture(GL_TEXTURE_BUFFER, entry.texture);
	glTexBuffer(GL_TEXTURE_BUFFER, format, entry.buffer);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	PlaceholderBufferTextures().push_back(entry);
	return entry.texture;
}

// `texture`, or the unit's placeholder when it is 0. A placeholder is never
// a buffer anything writes, so binding it in place of a cache being baked
// keeps that cache unsampled.
inline GLuint BufferTextureOrPlaceholder(GLint unit, GLuint texture)
{
	if (texture != 0)
		return texture;
	const GLenum format = BufferSamplerFormatForUnit(unit);
	return format != 0 ? PlaceholderBufferTexture(format) : 0;
}

inline GLuint& PlaceholderTexture2DArrayName()
{
	static GLuint s_texture = 0;
	return s_texture;
}

// A 1x1, one-layer, zero RGBA8 2D array for glob's shadowTextureArray while
// a level has no shadow map, for the same driver check. Created on first use;
// binds GL_TEXTURE_2D_ARRAY on the active unit and leaves it unbound. Its
// alpha must stay 0: glob.frag reads a shadow's coverage from alpha, so any
// other value would darken whatever a shadow record points at layer 0.
inline GLuint PlaceholderTexture2DArray()
{
	GLuint& texture = PlaceholderTexture2DArrayName();
	if (texture != 0)
		return texture;
	const unsigned char zeros[4] = {};
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, zeros);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	return texture;
}

inline void DeletePlaceholderBufferTextures()
{
	for (PlaceholderBufferTextureEntry& entry : PlaceholderBufferTextures())
	{
		glDeleteTextures(1, &entry.texture);
		glDeleteBuffers(1, &entry.buffer);
	}
	PlaceholderBufferTextures().clear();
	if (PlaceholderTexture2DArrayName() != 0)
	{
		glDeleteTextures(1, &PlaceholderTexture2DArrayName());
		PlaceholderTexture2DArrayName() = 0;
	}
}

// Bakes glob.vert's static three-way lighting for vertices [baseVertex,
// baseVertex + count) of the bound vertex array into the same records of
// `cache` with transform feedback; nothing is rasterised. The glob program
// (linked with kGlobFeedbackVaryings) must be current with the subglob's
// uniforms set. fCacheBake keeps the dynamic lights out of the cache.
inline void BakeCachedMaterial(GLuint cache, GLint baseVertex, GLsizei count, GLint fCacheBakeLocation)
{
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, cache,
		static_cast<GLintptr>(baseVertex) * kCachedMaterialStride,
		static_cast<GLsizeiptr>(count) * kCachedMaterialStride);
	glUniform1i(fCacheBakeLocation, 1);
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, baseVertex, count);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);
	glUniform1i(fCacheBakeLocation, 0);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
}

// The blip VAO's per-instance attributes (gl.cpp): one vec4 each at locations
// 1-4, consecutive within an instance.
inline constexpr GLuint kBlipInstanceAttributeFirst = 1;
inline constexpr GLuint kBlipInstanceAttributeCount = 4;

// Points the blip instance attributes of the bound VAO at `baseInstance` in the
// buffer bound to GL_ARRAY_BUFFER, `stride` bytes per instance.
inline void PointBlipInstanceAttributes(GLsizei stride, GLuint baseInstance)
{
	for (GLuint i = 0; i < kBlipInstanceAttributeCount; ++i)
	{
		const uintptr_t offset = static_cast<uintptr_t>(baseInstance) * static_cast<uintptr_t>(stride) +
			i * 4 * sizeof(float);
		glVertexAttribPointer(kBlipInstanceAttributeFirst + i, 4, GL_FLOAT, GL_FALSE, stride,
			reinterpret_cast<void*>(offset));
	}
}

// glDrawElementsInstancedBaseInstance is GL 4.2, so glad leaves it null on the
// 4.1 context. Draws the blip quad's `instanceCount` instances from
// `baseInstance` by moving the instance attributes there instead; blip.vert
// doesn't read gl_InstanceID. Needs the blip VAO and its instance buffer bound;
// PointBlipInstanceAttributes(stride, 0) puts the attributes back afterwards.
inline void DrawBlipInstances(GLsizei stride, GLuint baseInstance, GLsizei instanceCount)
{
	PointBlipInstanceAttributes(stride, baseInstance);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, instanceCount);
}

// GL 4.1 core forward-compatible contexts reject glLineWidth above 1, so the
// celborder program gets a Mac-only geometry stage that widens each outline
// edge into a quad of the given width in pixels of the target it draws into.
inline constexpr const char kCelBorderGeometryShader[] = "celborder.geom";
inline constexpr const char kCelLineWidthUniform[] = "celLineWidthPx";
inline constexpr const char kCelViewportUniform[] = "celViewportPx";
// True while the outlines draw into the multisampled scene target: GL draws
// wide lines differently with multisampling, and the geometry stage follows.
inline constexpr const char kCelMultisampledUniform[] = "celMultisampled";

// Sets the outline width (what glLineWidth got) and the size of the viewport
// the outlines draw into. glProgramUniform* leaves the current program alone.
inline void SetCelBorderLineWidth(GLuint program, GLint widthLocation, GLint viewportLocation,
	float widthPx, int viewportWidth, int viewportHeight)
{
	glProgramUniform1f(program, widthLocation, widthPx);
	glProgramUniform2f(program, viewportLocation,
		static_cast<float>(viewportWidth > 0 ? viewportWidth : 1),
		static_cast<float>(viewportHeight > 0 ? viewportHeight : 1));
}

#ifndef CANE_SHADER_TOOLS_ONLY
#include <iostream>
#include "shd.h"

class GLSHADER
{
    public:

    GLuint ID;

    // `feedbackVaryings`, when given, are captured interleaved (set before the link).
    void Init(const char* vertexFile, const char* geometryFile, const char* fragmentFile,
        const char* const* feedbackVaryings = nullptr, GLsizei feedbackVaryingCount = 0);

    void Use();
    void Delete();

    private:
    bool compileErrors(unsigned int shader, const char* type,
        const char* vertexFile = nullptr,
        const char* geometryFile = nullptr,
        const char* fragmentFile = nullptr);
};

void InvalidateShaderProgramCache();

extern GLSHADER glScreenShader;
extern GLSHADER glDyshadow;
extern GLSHADER glGlobShader;
extern GLSHADER glCelBorderShader;
extern GLSHADER glGeomShader;
extern GLSHADER glBlotShader;
extern GLSHADER glBlipShader;

#endif // CANE_SHADER_TOOLS_ONLY
#else
#include <iostream>
#include <glad/glad.h>
#include <string>
#include "shd.h"

class GLSHADER
{
    public:

    GLuint ID;

    void Init(const char* vertexFile, const char* geometryFile, const char* fragmentFile);

    void Use();
    void Delete();

    private:
    bool compileErrors(unsigned int shader, const char* type,
        const char* vertexFile = nullptr,
        const char* geometryFile = nullptr,
        const char* fragmentFile = nullptr);
};

void InvalidateShaderProgramCache();

extern GLSHADER glScreenShader;
extern GLSHADER glDyshadow;
extern GLSHADER glGlobShader;
extern GLSHADER glCelBorderShader;
extern GLSHADER glGeomShader;
extern GLSHADER glBlotShader;
extern GLSHADER glBlipShader;
#endif // CANE_GL41
