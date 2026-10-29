#include "render.h"
#include "emitter.h"
#include "main.h"
#include "blip.h"
#include "emitter.h"
#include "gl.h"
#include "tv.h"
#include "thndflash.h"
#include "sensor.h"
#include "button.h"
#include "wm.h"
#include <algorithm>
#ifdef CANE_GL41
#include <cassert>
#endif
#include <cmath>
#include <cstring>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	struct RENDERSTATECACHE
	{
		GLuint vao = (std::numeric_limits<GLuint>::max)();
		GLenum activeTexture = (std::numeric_limits<GLenum>::max)();
		GLuint textures2D[32]{};
		GLuint textures2DArray[32]{};
#ifndef CANE_GL41
		GLuint ssboBindings[16]{};
#endif
		GLint blendEnabled = -1;
		GLint depthEnabled = -1;
		GLint cullEnabled = -1;
		GLint depthMask = -1;
		GLenum depthFunc = (std::numeric_limits<GLenum>::max)();
		GLenum blendEquationRgb = (std::numeric_limits<GLenum>::max)();
		GLenum blendEquationAlpha = (std::numeric_limits<GLenum>::max)();
		GLenum blendSrcRgb = (std::numeric_limits<GLenum>::max)();
		GLenum blendDstRgb = (std::numeric_limits<GLenum>::max)();
		GLenum blendSrcAlpha = (std::numeric_limits<GLenum>::max)();
		GLenum blendDstAlpha = (std::numeric_limits<GLenum>::max)();

		void Invalidate()
		{
			vao = (std::numeric_limits<GLuint>::max)();
			activeTexture = (std::numeric_limits<GLenum>::max)();
			std::fill(std::begin(textures2D), std::end(textures2D),
				(std::numeric_limits<GLuint>::max)());
			std::fill(std::begin(textures2DArray), std::end(textures2DArray),
				(std::numeric_limits<GLuint>::max)());
#ifndef CANE_GL41
			std::fill(std::begin(ssboBindings), std::end(ssboBindings),
				(std::numeric_limits<GLuint>::max)());
#endif
			blendEnabled = depthEnabled = cullEnabled = depthMask = -1;
			depthFunc = blendEquationRgb = blendEquationAlpha =
				blendSrcRgb = blendDstRgb = blendSrcAlpha = blendDstAlpha =
				(std::numeric_limits<GLenum>::max)();
		}
	};

	RENDERSTATECACHE s_renderState;

	void UseShader(GLSHADER& shader)
	{
		shader.Use();
	}

	void InvalidateCachedRenderState()
	{
		s_renderState.Invalidate();
		InvalidateShaderProgramCache();
	}

	GLint& CapabilityState(GLenum capability)
	{
		switch (capability)
		{
			case GL_BLEND: return s_renderState.blendEnabled;
			case GL_DEPTH_TEST: return s_renderState.depthEnabled;
			case GL_CULL_FACE: return s_renderState.cullEnabled;
			default: throw std::runtime_error("unsupported cached OpenGL capability");
		}
	}

	void SetCapability(GLenum capability, bool enabled)
	{
		if (capability != GL_BLEND && capability != GL_DEPTH_TEST && capability != GL_CULL_FACE)
		{
			if (enabled)
				glEnable(capability);
			else
				glDisable(capability);
			return;
		}
		GLint& current = CapabilityState(capability);
		const GLint requested = enabled ? 1 : 0;
		if (current == requested)
			return;
		if (enabled)
			glEnable(capability);
		else
			glDisable(capability);
		current = requested;
	}

	GLboolean GetCapability(GLenum capability)
	{
		GLint& current = CapabilityState(capability);
		if (current < 0)
			current = glIsEnabled(capability) ? 1 : 0;
		return current ? GL_TRUE : GL_FALSE;
	}

	void SetDepthMask(GLboolean enabled)
	{
		const GLint requested = enabled ? 1 : 0;
		if (s_renderState.depthMask != requested)
		{
			glDepthMask(enabled);
			s_renderState.depthMask = requested;
		}
	}

	void SetDepthFunc(GLenum function)
	{
		if (s_renderState.depthFunc != function)
		{
			glDepthFunc(function);
			s_renderState.depthFunc = function;
		}
	}

	void SetBlendEquationSeparate(GLenum rgb, GLenum alpha)
	{
		if (s_renderState.blendEquationRgb != rgb || s_renderState.blendEquationAlpha != alpha)
		{
			glBlendEquationSeparate(rgb, alpha);
			s_renderState.blendEquationRgb = rgb;
			s_renderState.blendEquationAlpha = alpha;
		}
	}

	void SetBlendEquation(GLenum equation)
	{
		SetBlendEquationSeparate(equation, equation);
	}

	void SetBlendFuncSeparate(GLenum srcRgb, GLenum dstRgb, GLenum srcAlpha, GLenum dstAlpha)
	{
		if (s_renderState.blendSrcRgb != srcRgb || s_renderState.blendDstRgb != dstRgb ||
			s_renderState.blendSrcAlpha != srcAlpha || s_renderState.blendDstAlpha != dstAlpha)
		{
			glBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha);
			s_renderState.blendSrcRgb = srcRgb;
			s_renderState.blendDstRgb = dstRgb;
			s_renderState.blendSrcAlpha = srcAlpha;
			s_renderState.blendDstAlpha = dstAlpha;
		}
	}

	void SetBlendFunc(GLenum src, GLenum dst)
	{
		SetBlendFuncSeparate(src, dst, src, dst);
	}

	template <typename T>
	T& AppendRenderQueue(std::vector<T>& queue, int& count, const T& value, const char* name)
	{
		if (count < 0 || static_cast<size_t>(count) >= queue.size())
			throw std::runtime_error(std::string("render queue overflow: ") + name);
		T& entry = queue[static_cast<size_t>(count++)];
		entry = value;
		return entry;
	}

	void EnsureBlendStateKnown()
	{
		if (s_renderState.blendEquationRgb == (std::numeric_limits<GLenum>::max)())
		{
			GLint value = 0;
			glGetIntegerv(GL_BLEND_EQUATION_RGB, &value);
			s_renderState.blendEquationRgb = static_cast<GLenum>(value);
			glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &value);
			s_renderState.blendEquationAlpha = static_cast<GLenum>(value);
			glGetIntegerv(GL_BLEND_SRC_RGB, &value);
			s_renderState.blendSrcRgb = static_cast<GLenum>(value);
			glGetIntegerv(GL_BLEND_DST_RGB, &value);
			s_renderState.blendDstRgb = static_cast<GLenum>(value);
			glGetIntegerv(GL_BLEND_SRC_ALPHA, &value);
			s_renderState.blendSrcAlpha = static_cast<GLenum>(value);
			glGetIntegerv(GL_BLEND_DST_ALPHA, &value);
			s_renderState.blendDstAlpha = static_cast<GLenum>(value);
		}
	}

#undef glEnable
#undef glDisable
#undef glDepthMask
#undef glDepthFunc
#undef glBlendEquation
#undef glBlendEquationSeparate
#undef glBlendFunc
#undef glBlendFuncSeparate
#define glEnable(capability) SetCapability((capability), true)
#define glDisable(capability) SetCapability((capability), false)
#define glDepthMask(enabled) SetDepthMask((enabled))
#define glDepthFunc(function) SetDepthFunc((function))
#define glBlendEquation(equation) SetBlendEquation((equation))
#define glBlendEquationSeparate(rgb, alpha) SetBlendEquationSeparate((rgb), (alpha))
#define glBlendFunc(src, dst) SetBlendFunc((src), (dst))
#define glBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha) \
	SetBlendFuncSeparate((srcRgb), (dstRgb), (srcAlpha), (dstAlpha))

	void SetActiveTextureUnit(GLuint unit)
	{
		const GLenum textureUnit = GL_TEXTURE0 + unit;
		if (s_renderState.activeTexture != textureUnit)
		{
			glActiveTexture(textureUnit);
			s_renderState.activeTexture = textureUnit;
		}
	}

	void BindTexture2D(GLuint unit, GLuint texture)
	{
		SetActiveTextureUnit(unit);
		if (unit >= std::size(s_renderState.textures2D) ||
			s_renderState.textures2D[unit] != texture)
		{
			glBindTexture(GL_TEXTURE_2D, texture);
			if (unit < std::size(s_renderState.textures2D))
				s_renderState.textures2D[unit] = texture;
		}
	}

	void BindTexture2DArray(GLuint unit, GLuint texture)
	{
		SetActiveTextureUnit(unit);
		if (unit >= std::size(s_renderState.textures2DArray) ||
			s_renderState.textures2DArray[unit] != texture)
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
			if (unit < std::size(s_renderState.textures2DArray))
				s_renderState.textures2DArray[unit] = texture;
		}
	}

	void BindVertexArray(GLuint vao)
	{
		if (s_renderState.vao != vao)
		{
			glBindVertexArray(vao);
			s_renderState.vao = vao;
		}
	}

#ifdef CANE_GL41
	// Leaves `unit` active; draws switch back to unit 0 when done. Texture 0
	// binds the unit's placeholder (glshaders.h) so no buffer sampler's unit
	// is ever empty.
	void BindBufferTexture(GLuint unit, GLuint texture)
	{
		SetActiveTextureUnit(unit);
		glBindTexture(GL_TEXTURE_BUFFER, BufferTextureOrPlaceholder(static_cast<GLint>(unit), texture));
	}

	// dysh.vert, celborder.vert and glob.vert read bones from this buffer texture.
	void BindBoneMatrixTexture(const GLOBSET* pglobset)
	{
		BindBufferTexture(kBoneMatrixTextureUnit, pglobset->boneMatrixTexture);
	}
#else
	void BindShaderStorageBuffer(GLuint index, GLuint buffer)
	{
		if (index >= std::size(s_renderState.ssboBindings) ||
			s_renderState.ssboBindings[index] != buffer)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer);
			if (index < std::size(s_renderState.ssboBindings))
				s_renderState.ssboBindings[index] = buffer;
		}
	}
#endif

	struct alignas(16) BLIPGPU
	{
		glm::vec4 posCreated;
		glm::vec4 velocity;
		glm::vec4 color;
		glm::vec4 roll;
	};

	struct BLIPDRAWRANGE
	{
		GLuint texture = 0;
		GLuint baseInstance = 0;
		GLsizei instanceCount = 0;
	};

	struct BLIPDRAWGROUP
	{
		bool additive = false;
		GLsizeiptr streamOffset = -1;
		size_t firstRange = 0;
		size_t rangeCount = 0;
	};

	std::vector<BLIPGPU> s_blipFrameInstances;
	std::vector<BLIPDRAWRANGE> s_blipFrameRanges;
	std::vector<BLIPDRAWGROUP> s_blipFrameGroups;

	void BeginBlipFrame()
	{
		s_blipFrameInstances.clear();
		s_blipFrameRanges.clear();
		s_blipFrameGroups.clear();
	}

	void DrawStagedBlips()
	{
		if (s_blipFrameInstances.empty() || s_blipFrameGroups.empty())
			return;

		UploadFrameStream(&blipStream);

		const GLsizeiptr uploadSize = static_cast<GLsizeiptr>(
			s_blipFrameInstances.size() * sizeof(BLIPGPU));
		if (uploadSize > g_gl.blipInstanceCapacity)
			g_gl.blipInstanceCapacity = uploadSize;

		// Orphan and upload once for the complete frame instead of once per
		// BLIP group. Retaining capacity avoids shrinking the store when particle
		// counts fluctuate between frames.
		glBufferData(GL_ARRAY_BUFFER, g_gl.blipInstanceCapacity, nullptr, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, uploadSize, s_blipFrameInstances.data());

		for (const BLIPDRAWGROUP& group : s_blipFrameGroups)
		{
			BindFrameStreamRange(&blipStream, group.streamOffset, sizeof(BLIPGROUPGPU));
			if (group.additive)
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
			else
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

			auto drawRanges = [&]()
			{
				for (size_t i = 0; i < group.rangeCount; ++i)
				{
					const BLIPDRAWRANGE& range = s_blipFrameRanges[group.firstRange + i];
					BindTexture2D(0, range.texture);
#ifdef CANE_GL41
					DrawBlipInstances(static_cast<GLsizei>(sizeof(BLIPGPU)), range.baseInstance,
						range.instanceCount);
#else
					glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT,
						nullptr, range.instanceCount, range.baseInstance);
#endif
				}
			};

			if (group.additive)
			{
				glUniform1i(glslBlipAlphaPass, 0);
				glDepthMask(GL_FALSE);
				drawRanges();
			}
			else
			{
				glUniform1i(glslBlipAlphaPass, 1);
				glDepthMask(GL_TRUE);
				drawRanges();

				glUniform1i(glslBlipAlphaPass, 2);
				glDepthMask(GL_FALSE);
				drawRanges();
			}
		}
#ifdef CANE_GL41

		// Put the instance attributes back at instance 0, as gl.cpp set them up.
		PointBlipInstanceAttributes(static_cast<GLsizei>(sizeof(BLIPGPU)), 0);
#endif
	}
}

void SetGlobDraw(GLOB* pglob)
{
	switch (pglob->rp)
	{
		case RP_DynamicTexture:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_Background:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_BlotContext:
		pglob->PFNDRAW = DrawTranslucent;
		break;

		case RP_Opaque:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_Cutout:
		pglob->PFNDRAW = DrawTranslucent;
		break;

		case RP_ProjVolume:
		pglob->PFNDRAW = DrawProjVolume;
		break;

		case RP_OpaqueAfterProjVolume:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_CutoutAfterProjVolume:
		pglob->PFNDRAW = DrawTranslucent;
		break;

		case RP_MurkClear:
		pglob->PFNDRAW = DrawMurkClear;
		break;

		case RP_MurkOpaque:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_MurkFill:
		pglob->PFNDRAW = DrawMurkFill;
		break;

		case RP_Translucent:
		pglob->PFNDRAW = DrawTranslucent;
		break;

		/*case RP_Blip:
		pglob->PFNDRAW = Draw;
		break;*/

		case RP_Foreground:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_WorldMap:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_Max:
		pglob->PFNDRAW = DrawSubGlob;
		break;
	}
}

void AllocateRpl()
{
	g_dynamicTexturePrpl.resize(2048);
	g_backGroundPrpl.resize(2048);
	g_backGroundBlendPrpl.resize(2048);
	g_blotContextPrpl.resize(2048);
	g_opaquePrpl.resize(2048);
	g_cutOutBlendAddPrpl.resize(2048);
	g_cutOutPrpl.resize(2048);
	g_celBorderPrpl.resize(2048);
	g_projVolumePrpl.resize(6144);
	g_opaqueAfterProjVolumePrpl.resize(2048);
	g_cutOutAfterProjVolumeAddPrpl.resize(2048);
	g_cutOutAfterProjVolumePrpl.resize(2048);
	g_celBorderAfterProjVolumePrpl.resize(2048);
	g_murkClearPrpl.resize(2048);
	g_murkOpaquePrpl.resize(2048);
	g_murkFillPrpl.resize(2048);
	g_translucentAddPrpl.resize(2048);
	g_translucentPrpl.resize(2048);
	g_translucentCelBorderPrpl.resize(2048);
	g_blipPrpl.resize(2048);
	g_foreGroundPrpl.resize(2048);
	g_worldMapPrpl.resize(2048);
	g_maxPrpl.resize(2048);
	
	g_dynamicTextureCount = 0;
	g_backGroundCount = 0;
	g_backGroundBlendCount = 0;
	g_blotContextCount = 0;
	g_opaqueCount = 0;
	g_cutOutBlendAddCount = 0;
	g_cutOutCount = 0;
	g_celBorderCount = 0;
	g_projVolumeCount = 0;
	g_opaqueAfterProjVolumeCount = 0;
	g_cutOutAfterProjVolumeAddCount = 0;
	g_cutOutAfterProjVolumeCount = 0;
	g_celBorderAfterProjVolumeCount = 0;
	g_murkClearCount = 0;
	g_murkOpaqueCount = 0;
	g_murkFillCount = 0;
	g_translucentAddCount = 0;
	g_translucentCount = 0;
	g_translucentCelBorderCount = 0;
	g_blipCount = 0;
	g_foreGroundCount = 0;
	g_worldMapCount = 0;
}

void RenderSw(SW* psw, CM* pcm)
{
	DLI dlBusyWalker;
	dlBusyWalker.m_pdl = &psw->dlBusy;
	dlBusyWalker.m_ibDle = psw->dlBusy.ibDle;
	dlBusyWalker.m_pdliNext = s_pdliFirst;

	LO* currentObject = psw->dlBusy.ploFirst;

	dlBusyWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle) : nullptr;

	s_pdliFirst = &dlBusyWalker;

	while (currentObject != nullptr)
	{
		currentObject->pvtalo->pfnRenderAloAll((ALO*)currentObject, pcm, nullptr);

		currentObject = reinterpret_cast<LO*>(*dlBusyWalker.m_ppv);
		dlBusyWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle) : nullptr;
	}

	s_pdliFirst = dlBusyWalker.m_pdliNext;

	DLI dlRealClockWalker;
	dlRealClockWalker.m_pdl = &psw->dlMRDRealClock;
	dlRealClockWalker.m_ibDle = psw->dlMRDRealClock.ibDle;
	dlRealClockWalker.m_pdliNext = s_pdliFirst;

	currentObject = psw->dlMRDRealClock.ploFirst;

	dlRealClockWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlRealClockWalker.m_ibDle) : nullptr;

	s_pdliFirst = &dlRealClockWalker;

	while (currentObject != nullptr)
	{
		currentObject->pvtalo->pfnRenderAloAll((ALO*)currentObject, pcm, nullptr);

		currentObject = reinterpret_cast<LO*>(*dlRealClockWalker.m_ppv);
		dlRealClockWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlRealClockWalker.m_ibDle) : nullptr;
	}

	s_pdliFirst = dlRealClockWalker.m_pdliNext;

	DLI dlThndFlashWalker;
	dlThndFlashWalker.m_pdl = &psw->dlThndFlash;
	dlThndFlashWalker.m_ibDle = psw->dlThndFlash.ibDle;
	dlThndFlashWalker.m_pdliNext = s_pdliFirst;

	THNDFLASH* pthndflash = static_cast<THNDFLASH*>(psw->dlThndFlash.pvFirst);
	dlThndFlashWalker.m_ppv = pthndflash != nullptr
		? reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pthndflash) + dlThndFlashWalker.m_ibDle)
		: nullptr;
	s_pdliFirst = &dlThndFlashWalker;

	while (pthndflash != nullptr)
	{
		RenderThndFlash(pthndflash);

		pthndflash = static_cast<THNDFLASH*>(*dlThndFlashWalker.m_ppv);
		dlThndFlashWalker.m_ppv = pthndflash != nullptr
			? reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pthndflash) + dlThndFlashWalker.m_ibDle)
			: nullptr;
	}

	s_pdliFirst = dlThndFlashWalker.m_pdliNext;

}

void RenderSwAloAll(SW* psw, CM* pcm)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
	{
		CID cid = allSWAloObjs[i]->pvtalo->cid;

		allSWAloObjs[i]->pvtalo->pfnRenderAloAll(allSWAloObjs[i], pcm, nullptr);
	}
}

void RenderSwGlobset(SW* psw, CM* pcm)
{
	for (int i = 0; i < 40; i++)
		allSWAloObjs[i]->pvtalo->pfnRenderAloGlobset(allSWAloObjs[i], pcm, nullptr);
}

void SubmitRpl(RPL* prpl)
{
	if (prpl == nullptr || FIsCelBorderRenderPhase(prpl->rp))
		return;
	switch (prpl->rp)
	{
		case RP_DynamicTexture:
		AppendRenderQueue(g_dynamicTexturePrpl, g_dynamicTextureCount, *prpl,
			"dynamic texture").PFNDRAWRPL = DrawDysh;
		break;

		case RP_Background:
		if (prpl->fTransluscentSort == 1)
		{
			AppendRenderQueue(g_backGroundBlendPrpl, g_backGroundBlendCount, *prpl,
				"background blend").PFNDRAWRPL = DrawGlob;
		}
		else
		{
			AppendRenderQueue(g_backGroundPrpl, g_backGroundCount, *prpl,
				"background").PFNDRAWRPL = DrawGlob;
		}
		break;

		case RP_BlotContext:
		{
		RPL& entry = AppendRenderQueue(g_blotContextPrpl, g_blotContextCount, *prpl,
			"blot context");
		// Blot-context entries are an ordered stream.  Ordinary ALO globs do
		// not supply a callback here and use DrawGlob, while TV context entries
		// supply PreTvContext/PostTvContext and must retain those callbacks.
		if (entry.PFNDRAWRPL == nullptr)
			entry.PFNDRAWRPL = DrawGlob;
		}
		break;

		case RP_Opaque:
		AppendRenderQueue(g_opaquePrpl, g_opaqueCount, *prpl,
			"opaque").PFNDRAWRPL = DrawGlob;
		break;

		case RP_Cutout:
		if (prpl->fTransluscentSort == 1)
		{
			AppendRenderQueue(g_cutOutPrpl, g_cutOutCount, *prpl,
				"cutout").PFNDRAWRPL = DrawGlob;
		}
		else
		{
			AppendRenderQueue(g_cutOutBlendAddPrpl, g_cutOutBlendAddCount, *prpl,
				"cutout additive").PFNDRAWRPL = DrawGlob;
		}
		break;

		case RP_ProjVolume:
		AppendRenderQueue(g_projVolumePrpl, g_projVolumeCount, *prpl,
			"projected volume").PFNDRAWRPL = DrawGlob;
		break;

		case RP_OpaqueAfterProjVolume:
		AppendRenderQueue(g_opaqueAfterProjVolumePrpl, g_opaqueAfterProjVolumeCount, *prpl,
			"opaque after projected volume").PFNDRAWRPL = DrawGlob;
		break;

		case RP_CutoutAfterProjVolume:
		if (prpl->fTransluscentSort == 1)
		{
			AppendRenderQueue(g_cutOutAfterProjVolumePrpl, g_cutOutAfterProjVolumeCount, *prpl,
				"cutout after projected volume").PFNDRAWRPL = DrawGlob;
		}
		else
		{
			AppendRenderQueue(g_cutOutAfterProjVolumeAddPrpl, g_cutOutAfterProjVolumeAddCount, *prpl,
				"cutout additive after projected volume").PFNDRAWRPL = DrawGlob;
		}
		break;

		case RP_MurkClear:
		AppendRenderQueue(g_murkClearPrpl, g_murkClearCount, *prpl,
			"murk clear").PFNDRAWRPL = DrawGlob;
		break;

		case RP_MurkOpaque:
		AppendRenderQueue(g_murkOpaquePrpl, g_murkOpaqueCount, *prpl,
			"murk opaque").PFNDRAWRPL = DrawGlob;
		break;

		case RP_MurkFill:
		AppendRenderQueue(g_murkFillPrpl, g_murkFillCount, *prpl,
			"murk fill").PFNDRAWRPL = DrawGlob;
		break;

		case RP_Translucent:
		// Retail partitions every RP_Translucent RPL into one bucket and sorts
		// that bucket by the owning glob's Z.  Blend state is material state and
		// is selected per subglob in DrawGlob; it is not a second RPL partition.
		AppendRenderQueue(g_translucentPrpl, g_translucentCount, *prpl,
			"translucent").PFNDRAWRPL = DrawGlob;
		break;

		case RP_Blip:
		AppendRenderQueue(g_blipPrpl, g_blipCount, *prpl,
			"blip").PFNDRAWRPL = DrawBlip;
		break;

		case RP_Foreground:
		{
		RPL& entry = AppendRenderQueue(g_foreGroundPrpl, g_foreGroundCount, *prpl,
			"foreground");
		if (entry.PFNDRAWRPL == nullptr)
			entry.PFNDRAWRPL = DrawGlob;
		}
		break;

		case RP_WorldMap:
		AppendRenderQueue(g_worldMapPrpl, g_worldMapCount, *prpl,
			"world map").PFNDRAWRPL = DrawGlob;
		break;

		case RP_Max:
		AppendRenderQueue(g_maxPrpl, g_maxCount, *prpl,
			"max").PFNDRAWRPL = DrawGlob;
		break;
	}

	g_cFrameGlobs++;
	//numFrameObjs += prpl->pglob->asubglob.size();
}

void SubmitRplCel(RPLCEL* prplcel)
{
	if (prplcel == nullptr)
		return;
	switch (prplcel->rp)
	{
		case RP_CelBorder:
		AppendRenderQueue(g_celBorderPrpl, g_celBorderCount, *prplcel, "cel border");
		break;

		case RP_CelBorderAfterProjVolume:
		AppendRenderQueue(g_celBorderAfterProjVolumePrpl,
			g_celBorderAfterProjVolumeCount, *prplcel,
			"cel border after projected volume");
		break;

		case RP_TranslucentCelBorder:
		AppendRenderQueue(g_translucentCelBorderPrpl, g_translucentCelBorderCount,
			*prplcel, "translucent cel border");
		break;
	}

	g_cFrameCelGlobs++;
	//numFrameObjs++;
}

RPL& QueueTranslucentRpl(const RPL& rpl)
{
	return AppendRenderQueue(g_translucentPrpl, g_translucentCount, rpl,
		"translucent");
}

RPL& QueueTranslucentAddRpl(const RPL& rpl)
{
	return AppendRenderQueue(g_translucentAddPrpl, g_translucentAddCount, rpl,
		"translucent additive");
}

RPL& QueueBlotContextRpl(const RPL& rpl)
{
	return AppendRenderQueue(g_blotContextPrpl, g_blotContextCount, rpl,
		"blot context");
}

void SortRenderRpl()
{
	if (g_backGroundBlendCount > 1)
		std::stable_sort(g_backGroundBlendPrpl.begin(), g_backGroundBlendPrpl.begin() + g_backGroundBlendCount, compareZ);

	if (g_cutOutCount > 1)
		std::stable_sort(g_cutOutPrpl.begin(), g_cutOutPrpl.begin() + g_cutOutCount, compareZ);

	if (g_cutOutAfterProjVolumeCount > 1)
		std::stable_sort(g_cutOutAfterProjVolumePrpl.begin(), g_cutOutAfterProjVolumePrpl.begin() + g_cutOutAfterProjVolumeCount, compareZ);

	if (g_translucentCount > 1)
		std::stable_sort(g_translucentPrpl.begin(), g_translucentPrpl.begin() + g_translucentCount, compareZ);
}

inline bool compareZ(const RPL& prpl0, const RPL& prpl1)
{
	return prpl0.z > prpl1.z;
}

static bool FStageTvContext(const TV* ptv)
{
	if (ptv == nullptr)
		return false;

	int itvbMin = 0;
	int itvbMax = 0;
	GetTvItvbMinMax(const_cast<TV*>(ptv), &itvbMin, &itvbMax);
	if (itvbMin == itvbMax)
		return false;

	const float x0 = glm::clamp(ptv->x, 0.0f, static_cast<float>(g_gl.width));
	const float x1 = glm::clamp(ptv->x + ptv->dx, 0.0f, static_cast<float>(g_gl.width));
	const float y0 = glm::clamp(ptv->y, 0.0f, static_cast<float>(g_gl.height));
	const float y1 = glm::clamp(ptv->y + ptv->dy, 0.0f, static_cast<float>(g_gl.height));
	return x1 > x0 && y1 > y0;
}

static void StageGlobRecord(RPL* prpl, TV* ptvContext)
{
	prpl->fQueuedCacheWrite = prpl->ro.trlk == TRLK_Relight;
	prpl->ro.blotTvLight = 0;

	GLOB* pglob = prpl->pglob;
	if (ptvContext != nullptr && pglob != nullptr &&
		pglob->fThreeWay == 1 &&
		pglob->pblot == static_cast<BLOT*>(ptvContext))
	{
		if (ptvContext == &g_tvLeft)
			prpl->ro.blotTvLight = 1;
		else if (ptvContext == &g_tvRight)
			prpl->ro.blotTvLight = 2;

		if (prpl->ro.blotTvLight != 0)
		{
			prpl->ro.trlk = TRLK_Quick;
			prpl->ro.fDynamicLight = 0;
		}
	}

	prpl->streamOffset = StageFrameStream(&ropStream, &prpl->ro, sizeof(ROGL));
}

static void StageDrawStreams()
{
	auto StageList = [](std::vector<RPL>& list, int count)
	{
		for (int i = 0; i < count; ++i)
		{
			RPL& rpl = list[i];
			if (rpl.PFNDRAWRPL == DrawGlob)
				StageGlobRecord(&rpl, nullptr);
		}
	};

	StageList(g_backGroundPrpl, g_backGroundCount);
	StageList(g_backGroundBlendPrpl, g_backGroundBlendCount);

	TV* ptvContext = nullptr;
	for (int i = 0; i < g_blotContextCount; ++i)
	{
		RPL& rpl = g_blotContextPrpl[i];
		if (rpl.PFNDRAWRPL == PreTvContext)
			ptvContext = FStageTvContext(rpl.ptv) ? rpl.ptv : nullptr;
		else if (rpl.PFNDRAWRPL == PostTvContext)
			ptvContext = nullptr;
		else if (rpl.PFNDRAWRPL == DrawGlob)
			StageGlobRecord(&rpl, ptvContext);
	}

	StageList(g_opaquePrpl, g_opaqueCount);
	StageList(g_cutOutBlendAddPrpl, g_cutOutBlendAddCount);
	StageList(g_cutOutPrpl, g_cutOutCount);
	StageList(g_projVolumePrpl, g_projVolumeCount);
	StageList(g_opaqueAfterProjVolumePrpl, g_opaqueAfterProjVolumeCount);
	StageList(g_cutOutAfterProjVolumeAddPrpl, g_cutOutAfterProjVolumeAddCount);
	StageList(g_cutOutAfterProjVolumePrpl, g_cutOutAfterProjVolumeCount);
	StageList(g_murkClearPrpl, g_murkClearCount);
	StageList(g_murkOpaquePrpl, g_murkOpaqueCount);
	StageList(g_murkFillPrpl, g_murkFillCount);
	StageList(g_translucentAddPrpl, g_translucentAddCount);
	StageList(g_translucentPrpl, g_translucentCount);
	StageList(g_foreGroundPrpl, g_foreGroundCount);
	StageList(g_worldMapPrpl, g_worldMapCount);
	StageList(g_maxPrpl, g_maxCount);

	auto StageCelList = [](std::vector<RPLCEL>& list, int count)
	{
		for (int i = 0; i < count; ++i)
			list[i].streamOffset = StageFrameStream(
				&rcbStream, &list[i].rocel, sizeof(ROCEL));
	};

	const int celRecordCount = g_celBorderCount +
		g_celBorderAfterProjVolumeCount + g_translucentCelBorderCount;
	if (celRecordCount > 0)
	{
		StageCelList(g_celBorderPrpl, g_celBorderCount);
		StageCelList(g_celBorderAfterProjVolumePrpl, g_celBorderAfterProjVolumeCount);
		StageCelList(g_translucentCelBorderPrpl, g_translucentCelBorderCount);
	}

	UploadFrameStream(&ropStream);
	if (celRecordCount > 0)
		UploadFrameStream(&rcbStream);
}

void DrawSw(SW* psw, CM* pcm)
{
	// Other renderers (notably ImGui) may have changed OpenGL state since the
	// previous world frame. Re-learn it lazily instead of querying the driver.
	InvalidateCachedRenderState();
#ifdef CANE_GL41
	// Every buffer-sampler unit starts the frame with its placeholder; the
	// draws below bind the textures they use over it. A unit can also lose
	// its texture between frames when a world is deleted.
	for (const BufferSamplerFormat& entry : kBufferSamplerFormats)
		BindBufferTexture(static_cast<GLuint>(entry.unit), 0);
	// The same for glob's material samplers (units 0-2): one-way draws only
	// bind unit 1, and the driver flags any empty unit a program's samplers
	// name. whiteTex is what BindGlobThreeWayTextures binds for a missing map.
	for (GLuint unit = 0; unit < 3; ++unit)
		BindTexture2D(unit, whiteTex);
	SetActiveTextureUnit(0);
#endif
	/*std::cout << numFrameObjs << "\n";*/
	//numFrameObjs = 0;

	UseShader(glGlobShader);
	glUniform1i(glslProjectedVolumeFinalPass, 0);

	//std::cout << g_cFrameGlobs << "\n";
	BeginFrameStream(&ropStream);

	bool anyCel = g_cFrameCelGlobs > 0;

	if (anyCel == true)
		BeginFrameStream(&rcbStream);
	
	SortRenderRpl();
	StageDrawStreams();

	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CMGL), &pcm->matWorldToClip);

	PrepareSwLights(psw, pcm);
	PrepareSwShadows(psw, pcm);

	// OpenGL line widths are measured in render-target pixels. Scale the
	// authored cel-border width with the internal resolution so resolving the
	// scene back to the window does not make outlines appear thinner.
	const float celBorderRasterScale = static_cast<float>(g_gl.renderHeight) / static_cast<float>((std::max)(g_gl.presentHeight, 1));
#ifdef CANE_GL41
	// GL 4.1 core rejects widths above 1 (GL_INVALID_VALUE); celborder's
	// geometry stage widens the outlines in the scene target instead.
	SetCelBorderLineWidth(glCelBorderShader.ID, glslCelLineWidthPx, glslCelViewportPx,
		4.125f * celBorderRasterScale, g_gl.renderWidth, g_gl.renderHeight);
	// Wide lines take another shape with MSAA. Set every frame, after any
	// ApplyMsaaSettings this frame, from the target the outlines draw into.
	glProgramUniform1i(glCelBorderShader.ID, glslCelMultisampled,
		g_gl.fboMSAA != 0 && g_sceneFbo == g_gl.fboMSAA);
#else
	glLineWidth(4.125f * celBorderRasterScale);
#endif

	glEnable(GL_CULL_FACE);

	if (g_dynamicTextureCount > 0)
	{
		// Do not leave the array simultaneously attached and bound for sampling.
		BindTexture2DArray(SHADOW_TEXTURE_UNIT, 0);
		SetActiveTextureUnit(0);

		glBindFramebuffer(GL_FRAMEBUFFER, g_gl.dyshFbo);
		glViewport(0, 0, g_gl.dyshWidth, g_gl.dyshHeight);

		/*glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		UseShader(glDyshadow);

		for (int i = 0; i < g_dynamicTextureCount; i++)
			g_dynamicTexturePrpl[i].PFNDRAWRPL(&g_dynamicTexturePrpl[i]);

		glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);
		glViewport(0, 0, g_gl.renderWidth, g_gl.renderHeight);

		//glDisable(GL_BLEND);

		glDepthFunc(GL_LESS);
		glDepthMask(true);

		UseShader(glGlobShader);

		g_dynamicTextureCount = 0;
	}

	// Dynamic maps have now been rendered. Bind the shared projected-shadow
	// array once before regular geometry is submitted.
	BindSwShadowTextures(psw);
#ifdef CANE_GL41
	BindSwLightTextures();
#endif

	if (g_backGroundCount > 0)
	{
		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		for (int i = 0; i < g_backGroundCount; i++)
			g_backGroundPrpl[i].PFNDRAWRPL(&g_backGroundPrpl[i]);

		glDepthFunc(GL_LESS);
		glDepthMask(true);

		g_backGroundCount = 0;
	}

	if (g_backGroundBlendCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		for (int i = 0; i < g_backGroundBlendCount; i++)
			g_backGroundBlendPrpl[i].PFNDRAWRPL(&g_backGroundBlendPrpl[i]);

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glDepthMask(true);

		g_backGroundBlendCount = 0;
	}

	if (g_blotContextCount > 0)
	{
		// TV lighting is scoped by PreTvContext/PostTvContext entries inside
		// this ordered list. Never carry an unfinished context across frames.
		g_ptvDrawContext = nullptr;

		for (int i = 0; i < g_blotContextCount; i++)
		{
			RPL& rpl = g_blotContextPrpl[i];
			rpl.PFNDRAWRPL(&rpl);
			if (rpl.PFNDRAWRPL != DrawGlob)
				InvalidateCachedRenderState();
		}

		g_ptvDrawContext = nullptr;
		g_blotContextCount = 0;

	}

	if (g_opaqueCount > 0)
	{
		for (int i = 0; i < g_opaqueCount; i++)
			g_opaquePrpl[i].PFNDRAWRPL(&g_opaquePrpl[i]);

		g_opaqueCount = 0;
	}

	if (g_cutOutBlendAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutBlendAddCount; i++)
			g_cutOutBlendAddPrpl[i].PFNDRAWRPL(&g_cutOutBlendAddPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutBlendAddCount = 0;
	}

	if (g_cutOutCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutCount; i++)
			g_cutOutPrpl[i].PFNDRAWRPL(&g_cutOutPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutCount = 0;
	}

	if (g_celBorderCount > 0)
	{
		UseShader(glCelBorderShader);

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glEnable(GL_STENCIL_TEST);
		glStencilMask(0x40);
		glStencilFunc(GL_ALWAYS, 0x40, 0x40);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);

		for (int i = 0; i < g_celBorderCount; i++)
			DrawCelBorder(&g_celBorderPrpl[i]);

		glStencilMask(0xFF);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		UseShader(glGlobShader);

		g_celBorderCount = 0;
	}

	if (g_projVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LESS);
		glStencilMask(128);

		for (int i = 0; i < g_projVolumeCount; i++)
		{
			g_grfshd = g_projVolumePrpl[i].pglob->grfshd;
			g_projVolumePrpl[i].PFNDRAWRPL(&g_projVolumePrpl[i]);
		}

		glStencilMask(0xFF);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

		g_projVolumeCount = 0;
	}

	if (g_opaqueAfterProjVolumeCount > 0)
	{
		for (int i = 0; i < g_opaqueAfterProjVolumeCount; i++)
			g_opaqueAfterProjVolumePrpl[i].PFNDRAWRPL(&g_opaqueAfterProjVolumePrpl[i]);

		g_opaqueAfterProjVolumeCount = 0;
	}

	if (g_cutOutAfterProjVolumeAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutAfterProjVolumeAddCount; i++)
			g_cutOutAfterProjVolumeAddPrpl[i].PFNDRAWRPL(&g_cutOutAfterProjVolumeAddPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutAfterProjVolumeAddCount = 0;
	}

	if (g_cutOutAfterProjVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutAfterProjVolumeCount; i++)
			g_cutOutAfterProjVolumePrpl[i].PFNDRAWRPL(&g_cutOutAfterProjVolumePrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutAfterProjVolumeCount = 0;
	}

	if (g_celBorderAfterProjVolumeCount > 0)
	{
		UseShader(glCelBorderShader);

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_celBorderAfterProjVolumeCount; i++)
			DrawCelBorder(&g_celBorderAfterProjVolumePrpl[i]);

		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		UseShader(glGlobShader);

		g_celBorderAfterProjVolumeCount = 0;
	}

	if (g_murkClearCount > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_murkClearCount; i++)
			g_murkClearPrpl[i].PFNDRAWRPL(&g_murkClearPrpl[i]);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		g_murkClearCount = 0;
	}

	if (g_murkOpaqueCount > 0)
	{
		glUniform1i(glslfAlphaTest, 1);
		glUniform1f(glslAlphaCutOff, 0.1);

		for (int i = 0; i < g_murkOpaqueCount; i++)
			g_murkOpaquePrpl[i].PFNDRAWRPL(&g_murkOpaquePrpl[i]);

		glUniform1i(glslfAlphaTest, 0);

		g_murkOpaqueCount = 0;
	}

	if (g_murkFillCount > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(false);
		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_murkFillCount; i++)
			g_murkFillPrpl[i].PFNDRAWRPL(&g_murkFillPrpl[i]);

		glDisable(GL_BLEND);
		glDepthMask(true);
		glDepthFunc(GL_LESS);

		g_murkFillCount = 0;
	}

	if (g_translucentAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_translucentAddCount; i++)
		{
			g_translucentAddPrpl[i].PFNDRAWRPL(&g_translucentAddPrpl[i]);
			InvalidateCachedRenderState();
		}

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_translucentAddCount = 0;
	}

	if (g_translucentCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_translucentCount; i++)
			g_translucentPrpl[i].PFNDRAWRPL(&g_translucentPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_translucentCount = 0;
	}

	if (g_translucentCelBorderCount > 0)
	{
		UseShader(glCelBorderShader);

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_translucentCelBorderCount; i++)
			DrawCelBorder(&g_translucentCelBorderPrpl[i]);

		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		UseShader(glGlobShader);

		g_translucentCelBorderCount = 0;
	}

	if (g_blipCount > 0)
	{
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(false);
		glDisable(GL_CULL_FACE);

		UseShader(glBlipShader);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, cmUBO);
		SetActiveTextureUnit(0);
		BindVertexArray(g_gl.blipVao);
		glBindBuffer(GL_ARRAY_BUFFER, g_gl.blipInstanceVbo);
		BeginFrameStream(&blipStream);
		BeginBlipFrame();

		// DrawBlipg loads g_clock.t for curve age and g_clock.dtReal for the
		// flying-strip displacement (retail instructions 0x0013B0E8 and
		// 0x0013B140). Thus pause freezes lifetime without collapsing streaks.
		glUniform1f(glslBlipCurrentTime, g_clock.t);
		glUniform1f(glslBlipDtFrame, g_clock.dtReal);
		glUniformMatrix3fv(glslBlipCameraMat, 1, GL_FALSE, glm::value_ptr(g_pcm->mat));

		for (int i = 0; i < g_blipCount; i++)
			g_blipPrpl[i].PFNDRAWRPL(&g_blipPrpl[i]);

		DrawStagedBlips();

		EndFrameStream(&blipStream);
		BindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
#ifdef CANE_GL41
		// One-way globs drawn after this pass (the hub tour saw the foreground
		// TV speaker) only bind unit 1, and glob's ambientMap reads unit 0:
		// the 4.1 driver flags an empty unit ("unit 3 GLD_TEXTURE_INDEX_2D is
		// unloadable"). whiteTex is what DrawSw starts units 0-2 with.
		BindTexture2D(0, whiteTex);
#else
		BindTexture2D(0, 0);
#endif
		glDepthMask(true);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		UseShader(glGlobShader);

		g_blipCount = 0;
	}

	if (g_foreGroundCount > 0)
	{
		// Foreground objects are camera/UI composites (including confrontation
		// TV speakers), not members of the level depth hierarchy.
		glDepthFunc(GL_ALWAYS);
		glDepthMask(GL_FALSE);

		for (int i = 0; i < g_foreGroundCount; i++)
			g_foreGroundPrpl[i].PFNDRAWRPL(&g_foreGroundPrpl[i]);

		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		g_foreGroundCount = 0;
	}

	if (g_worldMapCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_ALWAYS);

		// The world map is authored and projected as a 4:3 presentation.  On a
		// wider framebuffer, cover the unused sides instead of exposing the
		// paused 3D world behind the map.
		constexpr float mapAspect = 4.0f / 3.0f;
		const float screenWidth = static_cast<float>(g_gl.width);
		const float screenHeight = static_cast<float>(g_gl.height);
		const float mapWidth = screenHeight * mapAspect;
		float pillarboxOpacity = 1.0f;
		WM* const pwm = g_wmc.pwmCurrent;

		if (pwm != nullptr)
		{
			if (pwm->wms == WMS_Appearing && pwm->pasegOpen != nullptr && pwm->pasegOpen->tMax > 0.0f)
				pillarboxOpacity = std::clamp((g_clock.tReal - pwm->tWms) / pwm->pasegOpen->tMax, 0.0f, 1.0f);
			else if (pwm->wms == WMS_Disappearing && pwm->pasegClose != nullptr && pwm->pasegClose->tMax > 0.0f)
				pillarboxOpacity = 1.0f - std::clamp((g_clock.tReal - pwm->tWms) / pwm->pasegClose->tMax, 0.0f, 1.0f);
		}

		if (screenWidth > mapWidth)
		{
			const float mapLeft = (screenWidth - mapWidth) * 0.5f;
			const float mapRight = mapLeft + mapWidth;
			const int pillarboxAlpha = static_cast<int>(pillarboxOpacity * 255.0f);
			FillScreenRect(0, 0, 0, pillarboxAlpha, 0.0f, 0.0f, mapLeft, screenHeight);
			FillScreenRect(0, 0, 0, pillarboxAlpha, mapRight, 0.0f, screenWidth, screenHeight);
			UseShader(glGlobShader);
		}

		for (int i = 0; i < g_worldMapCount; i++)
			g_worldMapPrpl[i].PFNDRAWRPL(&g_worldMapPrpl[i]);

		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		g_worldMapCount = 0;
	}

	if (g_maxCount > 0)
	{
		for (int i = 0; i < g_maxCount; i++)
			g_maxPrpl[i].PFNDRAWRPL(&g_maxPrpl[i]);

		g_maxCount = 0;
	}

	EndFrameStream(&ropStream);

	if (anyCel == true)
		EndFrameStream(&rcbStream);

	g_cFrameGlobs = 0;
	g_cFrameCelGlobs = 0;

	glDisable(GL_CULL_FACE);
	glClearStencil(0x00);
}

void FillScreenRect(int r, int g, int b, int alpha, float xLeft, float yTop, float xRight, float yBottom, bool fAdditive)
{
	const float width = xRight - xLeft;
	const float height = yBottom - yTop;

	if (width <= 0.0f || height <= 0.0f || alpha <= 0)
		return;

	const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(xLeft, yTop, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
	const glm::vec4 color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(alpha) / 255.0f);
	const GLboolean fCullEnabled = GetCapability(GL_CULL_FACE);
	const GLboolean fDepthEnabled = GetCapability(GL_DEPTH_TEST);
	const GLboolean fBlendEnabled = GetCapability(GL_BLEND);
	EnsureBlendStateKnown();
	const GLenum blendEquationRgb = s_renderState.blendEquationRgb;
	const GLenum blendEquationAlpha = s_renderState.blendEquationAlpha;
	const GLenum blendSrcRgb = s_renderState.blendSrcRgb;
	const GLenum blendDstRgb = s_renderState.blendDstRgb;
	const GLenum blendSrcAlpha = s_renderState.blendSrcAlpha;
	const GLenum blendDstAlpha = s_renderState.blendDstAlpha;

	UseShader(glBlotShader);

	glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
	glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
	glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));

	// Screen-space overlays may be submitted from a 3D render pass.  Do not let
	// that pass's culling, depth, or subtractive blend state discard the quad.
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, fAdditive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);

	BindTexture2D(0, whiteTex);
	glUniform1i(u_fontTexLoc, 0);

	BindVertexArray(g_gl.gao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	BindVertexArray(0);

	glBlendFuncSeparate(blendSrcRgb, blendDstRgb, blendSrcAlpha, blendDstAlpha);
	glBlendEquationSeparate(blendEquationRgb, blendEquationAlpha);
	if (!fBlendEnabled)
		glDisable(GL_BLEND);
	if (fDepthEnabled)
		glEnable(GL_DEPTH_TEST);
	if (fCullEnabled)
		glEnable(GL_CULL_FACE);
}

void DrawDysh(RPL *prpl)
{
	DYSH *pdysh = prpl->pdysh;

	if (pdysh == nullptr || pdysh->shadowTex == 0 || pdysh->shadowLayer < 0)
		return;

	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		pdysh->shadowTex, 0, pdysh->shadowLayer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 matWorldToClip = g_uvToClip * pdysh->pshadowGen->matWorldToUv;

	glUniformMatrix4fv(glslDyshMatWorldClip, 1, GL_FALSE, glm::value_ptr(matWorldToClip));
	glUniformMatrix4fv(glslDyshModel, 1, GL_FALSE, glm::value_ptr(prpl->ro.model));

	GLOBSET* pglobset = &pdysh->globset;
#ifdef CANE_GL41
	const bool fHasBones = pglobset->cbnd > 0 && pglobset->boneMatrixSSBO != 0 &&
		pglobset->boneMatrixTexture != 0;

	// dysh.vert reads bones from a buffer texture (no SSBOs on GL 4.1). Material
	// rendering expects unit 0 to be active, so switch back afterwards.
	if (fHasBones)
	{
		BindBoneMatrixTexture(pglobset);
		SetActiveTextureUnit(0);
	}
#else
	const bool fHasBones = pglobset->cbnd > 0 && pglobset->boneMatrixSSBO != 0;

	if (fHasBones)
		BindShaderStorageBuffer(9, pglobset->boneMatrixSSBO);
#endif

	for (int i = 0; i < pglobset->aglob.size(); i++)
	{
		GLOB& glob = pglobset->aglob[i];
		BindVertexArray(glob.VAO);

		for (int a = 0; a < glob.asubglob.size(); a++)
		{
			SUBGLOB& sg = glob.asubglob[a];
			glUniform1i(glslDyshfSkin, fHasBones && sg.cibnd > 0 ? 1 : 0);
			glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)sg.indexCount, GL_UNSIGNED_SHORT, (void*)(uintptr_t)(sg.firstIndex * sizeof(uint16_t)), (GLint)sg.baseVertex);
		}
	}

	glUniform1i(glslDyshfSkin, 0);
}

void BindGlobOneWayTexture(GLuint diffuseTexture)
{
	BindTexture2D(1, diffuseTexture != 0 ? diffuseTexture : whiteTex);
	SetActiveTextureUnit(0);
}

void BindGlobThreeWayTextures(GLuint ambientTexture, GLuint diffuseTexture, GLuint saturateTexture)
{
	const GLuint textures[3] = { ambientTexture, diffuseTexture, saturateTexture };
	for (int unit = 0; unit < 3; ++unit)
		BindTexture2D(unit, textures[unit] != 0 ? textures[unit] : whiteTex);
	SetActiveTextureUnit(0);
}

void DrawGlob(RPL* prpl)
{
	GLOB* pglob = prpl->pglob;
	const bool fDebugWireframe = g_fDebugObjectInspector &&
		g_paloDebugSelected != nullptr && prpl->palo == g_paloDebugSelected;
	const bool fQueuedCacheWrite = prpl->fQueuedCacheWrite;
	BindFrameStreamRange(&ropStream, prpl->streamOffset, sizeof(ROGL));

	GLOBSET* pglobset = prpl->palo != nullptr ? &prpl->palo->globset : nullptr;
#ifdef CANE_GL41
	// glob.vert reads bones, pose deltas, pose weights and warp state through
	// buffer textures (no SSBOs on GL 4.1).
	const GLuint poseDposTexture = GetBufferTexture(pglob->poseDposSSBO);
	const GLuint poseDnormalTexture = GetBufferTexture(pglob->poseDnormalSSBO);
	const GLuint poseWeightTexture = GetBufferTexture(GetGlobsetPoseWeightBuffer(pglobset));

	const bool fHasBones = pglobset != nullptr && pglobset->cbnd > 0 && pglobset->boneMatrixSSBO != 0 &&
		pglobset->boneMatrixTexture != 0;
	const bool fHasPoses = pglobset != nullptr && pglob->poseCount > 0 &&
		poseDposTexture != 0 && poseDnormalTexture != 0 &&
		poseWeightTexture != 0 &&
		pglobset->agPoses.size() >= static_cast<size_t>(pglob->poseCount);

	glUniform1i(glslfPose, fHasPoses ? 1 : 0);

	if (fHasBones)
		BindBoneMatrixTexture(pglobset);

	if (fHasPoses)
	{
		BindBufferTexture(kPoseDposTextureUnit, poseDposTexture);
		BindBufferTexture(kPoseDnormalTextureUnit, poseDnormalTexture);
		BindBufferTexture(kPoseWeightTextureUnit, poseWeightTexture);
		glUniform1i(glslPoseCount, pglob->poseCount);
	}

	if (prpl->ro.warpType != WARP_NONE)
		BindBufferTexture(kWarpStateTextureUnit, GetBufferTexture(pglob->pwarpGlob->ssboState));

	// A relight draw bakes the glob's static three-way lighting into its
	// cache by transform feedback, subglob by subglob; a baked draw reads the
	// cache through its buffer texture. The cache is never bound as a texture
	// while it is being baked; the unit holds its placeholder instead.
	const bool fBakeCache = pglob->fThreeWay == 1 && prpl->ro.trlk == TRLK_Relight &&
		pglob->ssboCachedMaterial != 0;
	if (pglob->fThreeWay == 1)
	{
		if (prpl->ro.trlk == TRLK_Relight)
			BindBufferTexture(kCachedMaterialTextureUnit, 0);
		else if (prpl->ro.trlk == TRLK_Baked && prpl->ro.fDynamic == 0)
			BindBufferTexture(kCachedMaterialTextureUnit, GetBufferTexture(pglob->ssboCachedMaterial));
	}

	// glob.cpp packs the subglobs back to back from vertex 0 and sizes the
	// cache to their total, one record per vertex.
	GLint cacheVertices = 0;
	GLint bakedEnd = 0;
	if (fBakeCache)
	{
		for (const SUBGLOB& sub : pglob->asubglob)
			cacheVertices += static_cast<GLint>(sub.vertices.size());
	}

	// Material rendering expects unit 0 to be active.
	SetActiveTextureUnit(0);
#else
	const GLuint poseWeightSSBO = GetGlobsetPoseWeightBuffer(pglobset);

	const bool fHasBones = pglobset != nullptr && pglobset->cbnd > 0 && pglobset->boneMatrixSSBO != 0;
	const bool fHasPoses = pglobset != nullptr && pglob->poseCount > 0 &&
		pglob->poseDposSSBO != 0 && pglob->poseDnormalSSBO != 0 &&
		poseWeightSSBO != 0 &&
		pglobset->agPoses.size() >= static_cast<size_t>(pglob->poseCount);

	glUniform1i(glslfPose, fHasPoses ? 1 : 0);

	if (fHasBones)
		BindShaderStorageBuffer(9, pglobset->boneMatrixSSBO);

	if (fHasPoses)
	{
		BindShaderStorageBuffer(10, pglob->poseDposSSBO);
		BindShaderStorageBuffer(11, pglob->poseDnormalSSBO);
		BindShaderStorageBuffer(14, poseWeightSSBO);
		glUniform1i(glslPoseCount, pglob->poseCount);
	}

	if (pglob->fThreeWay == 1)
	{
		if (prpl->ro.trlk == TRLK_Relight ||
			(prpl->ro.trlk == TRLK_Baked && prpl->ro.fDynamic == 0))
			BindShaderStorageBuffer(7, pglob->ssboCachedMaterial);
	}
#endif

	BindVertexArray(pglob->VAO);
	if (fDebugWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#ifdef CANE_GL41
		// GL 4.1 core only allows 1 px lines: the debug wireframe stays 1 px there.
#else
		glLineWidth(2.0f);
#endif
	}

#ifndef CANE_GL41
	if (prpl->ro.warpType != WARP_NONE)
		BindShaderStorageBuffer(6, pglob->pwarpGlob->ssboState);

#endif
	for (int i = 0; i < static_cast<int>(pglob->asubglob.size()); ++i)
	{
		SUBGLOB& sub = pglob->asubglob[i];
		SAI* sai = sub.uvSai;
		int iframe = 0;

		if (sai != nullptr && sai->pshd != nullptr && sai->pshd->cframe > 0)
		{
			iframe = sai->iframe;

			if (iframe < 0)
				iframe = 0;
			else if (iframe >= sai->pshd->cframe)
				iframe = sai->pshd->cframe - 1;
		}

		const int fSkin = fHasBones && sub.cibnd > 0 ? 1 : 0;
		glUniform1i(glslfSkin, fSkin);

		const int animate = sai != nullptr && (sai->grfsai & 0x2) != 0 ? 1 : 0;

		glUniform1i(glslfAnimateUv, animate);

		if (animate)
			glUniform2f(glsluvOffsets, sai->tcx.du, sai->tcx.dv);

		SHD* pshd = sub.pshd;

		if (prpl->rp == RP_Translucent && pshd != nullptr)
		{
			// FillShaders emits the GS ALPHA state with each material packet.
			// grfshd 2/6 use ordinary source-alpha blending; the remaining
			// translucent material variants use additive destination blending.
			if (pshd->grfshd == 2 || pshd->grfshd == 6)
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
			else
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		}

		// The visible projected-color packet uses PRIM 0x6c (TME=0), so its
		// PackRGBA bytes are ordinary 0..255 framebuffer color. The earlier
		// textured packet only participates in constructing the volume mask.
		// GS alpha uses 0x80 as full strength and is quantized again after the
		// VU applies the object's fade.
		if (pshd != nullptr && pshd->shdk == SHDK_ProjectedVolume)
		{
			auto SourceByte = [](float channel)
			{
				return std::clamp(static_cast<int>(channel * 255.0f + 0.5f), 0, 255);
			};

			// PackRGBA stores (source + 1) >> 1. RGB remains ordinary 8-bit GS
			// color and therefore normalizes against 255. Only GS alpha uses 0x80
			// as full strength. Expanding RGB against 128 saturates additive CAMSEN
			// footprints into opaque-looking primary/secondary colors.
			constexpr float colorDivisor = 255.0f;
			const float packedRed = static_cast<float>((SourceByte(pshd->rgbaVolume.r) + 1) >> 1);
			const float packedGreen = static_cast<float>((SourceByte(pshd->rgbaVolume.g) + 1) >> 1);
			const float packedBlue = static_cast<float>((SourceByte(pshd->rgbaVolume.b) + 1) >> 1);

			// DrawVolume scales the packed alpha byte in-place and truncates it.
			// Compensate for DrawOneWay's later op.uAlpha multiply so the final
			// source alpha equals that exact GS byte divided by 0x80.
			const int packedAlpha = (SourceByte(pshd->rgbaVolume.a) + 1) >> 1;
			const float objectAlpha = std::clamp(prpl->ro.uAlpha, 0.0f, 1.0f);
			const int fadedAlpha = static_cast<int>(packedAlpha * objectAlpha);
			const float projectedAlpha = objectAlpha > 0.0f
				? (static_cast<float>(fadedAlpha) / 128.0f) / objectAlpha
				: 0.0f;

			glUniform4f(glslProjectedVolumeColor,
				packedRed / colorDivisor,
				packedGreen / colorDivisor,
				packedBlue / colorDivisor,
				projectedAlpha);
		}
		else
		{
			glUniform4f(glslProjectedVolumeColor, 1.0f, 1.0f, 1.0f, 1.0f);
		}

		if (pshd != nullptr && !pshd->atex.empty() && !pshd->atex[0].abmp.empty())
		{
			if (iframe >= static_cast<int>(pshd->atex[0].abmp.size()))
				iframe = static_cast<int>(pshd->atex[0].abmp.size()) - 1;

			BMP* pbmp = pshd->atex[0].abmp[iframe];

			if (pbmp != nullptr)
			{
				if (pshd->shdk != SHDK_ThreeWay)
				{
					glUniform1i(glslRko, 0);
					const TEX& tex = pshd->atex[0];
					GLuint diffuseTexture =
						iframe < static_cast<int>(tex.glDiffuseMap.size())
						? tex.glDiffuseMap[iframe]
						: pbmp->glDiffuseMap;

					// The retail projected-volume packet binds a texture only when
					// grfshd bit 1 is set. Untextured volumes use their packed
					// volume RGBA directly.
					if (pshd->shdk == SHDK_ProjectedVolume && (pshd->grfshd & 2) == 0)
						diffuseTexture = whiteTex;

					BindGlobOneWayTexture(diffuseTexture);
				}
				else
				{
					glUniform1i(glslRko, 1);
					const TEX& tex = pshd->atex[0];
					const GLuint ambientTexture =
						iframe < static_cast<int>(tex.glShadowMap.size()) ? tex.glShadowMap[iframe] : 0;
					const GLuint diffuseTexture =
						iframe < static_cast<int>(tex.glDiffuseMap.size()) ? tex.glDiffuseMap[iframe] : 0;
					const GLuint saturateTexture =
						iframe < static_cast<int>(tex.glSaturateMap.size()) ? tex.glSaturateMap[iframe] : 0;
					BindGlobThreeWayTextures(ambientTexture, diffuseTexture, saturateTexture);
				}
			}
		}

		if (activeShadows.numShadows > 0)
		{
			const glm::vec4 subGlobPosCenter = prpl->ro.model * glm::vec4(sub.posCenter, 1.0f);
			const float subGlobRadius = pglob->rSubglobRadius * sub.sRadius;

			glUniform3f(glslSubGlobPosCenter, subGlobPosCenter.x, subGlobPosCenter.y, subGlobPosCenter.z);
			glUniform1f(glslSubGlobRadius, subGlobRadius);
		}

		glUniform1f(glslUnSelfIllum, sub.unSelfIllum);
#ifdef CANE_GL41

		// Bake with this subglob's uniforms, before its draw. The draw
		// itself still lights live, dynamic lights included.
		if (fBakeCache)
		{
			const GLint count = static_cast<GLint>(sub.vertices.size());
			const bool fInside = sub.baseVertex >= bakedEnd && sub.baseVertex + count <= cacheVertices;
			assert(fInside && "subglob cache ranges must lie inside the cache and not overlap");
			if (fInside && count > 0)
				BakeCachedMaterial(pglob->ssboCachedMaterial, sub.baseVertex, count, glslfCacheBake);
			bakedEnd = (std::max)(bakedEnd, sub.baseVertex + count);
		}

#endif
		pglob->PFNDRAW(sub.baseVertex, sub.firstIndex, sub.indexCount);
	}

	if (fDebugWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glLineWidth(1.0f);
	}

	glUniform1i(glslfSkin, 0);
	glUniform1i(glslfPose, 0);

	if (prpl->ro.trlk == TRLK_Relight)
	{
#ifdef CANE_GL41
		// The subglob loop baked the cache. Transform feedback writes are
		// visible to later texel fetches without a barrier (glMemoryBarrier
		// is GL 4.2; the 4.1 driver doesn't export it).
		if (pglob->trlk == TRLK_BakePending)
			pglob->trlk = TRLK_Baked;
#else
		// Make vertex-shader SSBO writes visible before any later packet reads
		// this glob's cached three-way material.
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		if (pglob->trlk == TRLK_BakePending)
			pglob->trlk = TRLK_Baked;

#endif
	}
	else if (fQueuedCacheWrite && pglob->trlk == TRLK_BakePending)
	{
		// A TV draw can redirect a queued relight packet through TRLK_Quick.
		// No cache was written, so make the normal world draw try again.
		pglob->trlk = TRLK_Relight;
	}
}

void DrawCelBorder(RPLCEL* prplcel)
{
	BindFrameStreamRange(&rcbStream, prplcel->streamOffset, sizeof(ROCEL));

	GLOB* pglob = prplcel->pglob;
	GLOBSET* pglobset = prplcel->palo != nullptr ? &prplcel->palo->globset : nullptr;
#ifdef CANE_GL41
	// celborder.vert reads buffer textures (no SSBOs on GL 4.1).
	const GLuint edgePoseDposTexture = GetBufferTexture(pglob->edgePoseDposSSBO);
	const GLuint poseWeightTexture = GetBufferTexture(GetGlobsetPoseWeightBuffer(pglobset));

	const bool fSkin = pglobset != nullptr && pglobset->cbnd > 0 && pglobset->boneMatrixSSBO != 0 &&
		pglobset->boneMatrixTexture != 0;
	const bool fPose = pglobset != nullptr && pglob->poseCount > 0 &&
		edgePoseDposTexture != 0 && poseWeightTexture != 0 &&
		pglobset->agPoses.size() >= static_cast<size_t>(pglob->poseCount);

	BindBufferTexture(kEdgeTextureUnit, GetBufferTexture(pglob->edgeSSBO));

	if (fSkin)
	{
		BindBoneMatrixTexture(pglobset);
	}

	if (fPose)
	{
		BindBufferTexture(kEdgePoseDposTextureUnit, edgePoseDposTexture);
		BindBufferTexture(kPoseWeightTextureUnit, poseWeightTexture);
		glUniform1i(glslCelPoseCount, pglob->poseCount);
	}

	// Material rendering expects unit 0 to be active.
	SetActiveTextureUnit(0);
#else
	const GLuint poseWeightSSBO = GetGlobsetPoseWeightBuffer(pglobset);

	const bool fSkin = pglobset != nullptr && pglobset->cbnd > 0 && pglobset->boneMatrixSSBO != 0;
	const bool fPose = pglobset != nullptr && pglob->poseCount > 0 &&
		pglob->edgePoseDposSSBO != 0 && poseWeightSSBO != 0 &&
		pglobset->agPoses.size() >= static_cast<size_t>(pglob->poseCount);

	BindShaderStorageBuffer(8, pglob->edgeSSBO);

	if (fSkin)
	{
		BindShaderStorageBuffer(9, pglobset->boneMatrixSSBO);
	}

	if (fPose)
	{
		BindShaderStorageBuffer(12, pglob->edgePoseDposSSBO);
		BindShaderStorageBuffer(14, poseWeightSSBO);
		glUniform1i(glslCelPoseCount, pglob->poseCount);
	}
#endif

	glUniform1i(glslCelSkin, fSkin ? 1 : 0);
	glUniform1i(glslCelPose, fPose ? 1 : 0);
#ifdef CANE_GL41
	// The 4.1 core driver rejects draws with no vertex array bound
	// (GL_INVALID_OPERATION), and DrawSqtr ends with a raw
	// glBindVertexArray(0) the cache does not see. celborder.vert reads no
	// attributes, so any vertex array will do; bind it raw so a stale cache
	// entry cannot skip the bind, and keep the cache in step.
	glBindVertexArray(g_gl.gao);
	s_renderState.vao = g_gl.gao;

	// celborder.geom's quads are counter-clockwise on screen, which
	// GL_CULL_FACE keeps only with back faces culled and CCW front faces
	// (the shared build's GL_LINES are never culled). Draw with that state
	// whatever the caller left, then give the caller its own back.
	GLint frontFace = GL_CCW;
	GLint cullFace = GL_BACK;
	glGetIntegerv(GL_FRONT_FACE, &frontFace);
	glGetIntegerv(GL_CULL_FACE_MODE, &cullFace);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
#endif
	glDrawArrays(GL_LINES, 0, pglob->edgeCount * 2);
#ifdef CANE_GL41
	glFrontFace(static_cast<GLenum>(frontFace));
	glCullFace(static_cast<GLenum>(cullFace));
#endif
	glUniform1i(glslCelSkin, 0);
	glUniform1i(glslCelPose, 0);
}

void DrawSubGlob(int baseVertex, int firstIndex, int indexCount)
{
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_SHORT, (void*)(uintptr_t)(firstIndex * sizeof(uint16_t)), (GLint)baseVertex);
}

void DrawProjVolume(int baseVertex, int firstIndex, int indexCount)
{
	const auto DrawVolumeMesh = [baseVertex, firstIndex, indexCount]()
	{
		glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_SHORT,
			(void*)(uintptr_t)(firstIndex * sizeof(uint16_t)), (GLint)baseVertex);
	};

	if (g_grfshd == 0)
	{
		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
		glDepthFunc(GL_LESS);
		glStencilFunc(GL_ALWAYS, 128, 128);
		glStencilOp(GL_ZERO, GL_REPLACE, GL_ZERO);
		glColorMask(0, 0, 0, 0);
		glFrontFace(GL_CW);
		DrawVolumeMesh();

		glColorMask(1, 1, 1, 1);
		glStencilOp(GL_KEEP, GL_ZERO, GL_KEEP);
		glFrontFace(GL_CCW);
		DrawVolumeMesh();

		glDepthFunc(GL_ALWAYS);
		// Bit 6 protects cel-border pixels; bit 7 contains the volume mask.
		// Testing both bits prevents projected volumes from drawing over borders.
		glStencilFunc(GL_EQUAL, 128, 0xC0);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glFrontFace(GL_CW);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1i(glslProjectedVolumeFinalPass, 1);
		DrawVolumeMesh();
		glUniform1i(glslProjectedVolumeFinalPass, 0);
		return;
	}

	if (g_grfshd == 2)
	{
		glDepthFunc(GL_LESS);
		glStencilFunc(GL_ALWAYS, 128, 128);
		glStencilOp(GL_NONE, GL_REPLACE, GL_NONE);
		glColorMask(0, 0, 0, 0);
		glFrontFace(GL_CW);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		DrawVolumeMesh();

		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
		glColorMask(1, 1, 1, 1);
		glStencilOp(GL_KEEP, GL_NONE, GL_KEEP);
		glFrontFace(GL_CCW);
		DrawVolumeMesh();

		glDepthFunc(GL_ALWAYS);
		glStencilFunc(GL_EQUAL, 128, 0xC0);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glFrontFace(GL_CW);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1i(glslProjectedVolumeFinalPass, 1);
		DrawVolumeMesh();
		glUniform1i(glslProjectedVolumeFinalPass, 0);
		return;
	}

	// grfshd 1 and 3 use the additive projected-volume packet path.
	glUniform1i(glslProjectedVolumeFinalPass, 2);
	glBlendFuncSeparate(g_grfshd == 3 ? GL_SRC_ALPHA : GL_ZERO,
		GL_ONE, GL_ONE, GL_ZERO);
	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_ZERO, GL_REPLACE, GL_ZERO);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	DrawVolumeMesh();

	if ((g_grfshd & 2) != 0)
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
	else
		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_ZERO, GL_KEEP);
	glFrontFace(GL_CCW);
	DrawVolumeMesh();

	glDepthFunc(GL_ALWAYS);
	glStencilFunc(GL_EQUAL, 128, 0xC0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CW);
	if ((g_grfshd & 2) != 0)
		BindGlobOneWayTexture(whiteTex);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
	glUniform1i(glslProjectedVolumeFinalPass, 1);
	DrawVolumeMesh();
	glUniform1i(glslProjectedVolumeFinalPass, 0);
}

void DrawMurkClear(int baseVertex, int firstIndex, int indexCount)
{
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
	glFrontFace(GL_CW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_SHORT, (void*)(uintptr_t)(firstIndex * sizeof(uint16_t)), (GLint)baseVertex);

	glFrontFace(GL_CCW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_SHORT, (void*)(uintptr_t)(firstIndex * sizeof(uint16_t)), (GLint)baseVertex);
}

void DrawMurkFill(int baseVertex, int firstIndex, int indexCount)
{
	glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA, GL_ONE, GL_NONE);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_SHORT, (void*)(uintptr_t)(firstIndex * sizeof(uint16_t)), (GLint)baseVertex);

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_SHORT, (void*)(uintptr_t)(firstIndex * sizeof(uint16_t)), (GLint)baseVertex);
}

void DrawTranslucent(int baseVertex, int firstIndex, int indexCount)
{
	glUniform1i(glslfAlphaTest, 1);
	glDepthMask(GL_TRUE);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_SHORT, (void*)(uintptr_t)(firstIndex * sizeof(uint16_t)), (GLint)baseVertex);

	glUniform1i(glslfAlphaTest, 0);
	glDepthMask(GL_FALSE);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_SHORT, (void*)(uintptr_t)(firstIndex * sizeof(uint16_t)), (GLint)baseVertex);

	glDepthMask(GL_TRUE);
}

void DrawBlip(RPL* prplblip)
{
	if (prplblip == nullptr || prplblip->pblipg == nullptr || g_pcm == nullptr)
		return;

	BLIPG* pblipg = prplblip->pblipg;
	if (pblipg->pshd == nullptr) {
		return;
	}
	if (pblipg->pshd->atex.empty()) {
		return;
	}
	if (pblipg->pshd->atex[0].abmp.empty()) {
		return;
	}
	if (pblipg->cblipe <= 0) {
		return;
	}

	// DrawBlipg selected between the PS2's normal and clamped-add GIF
	// packets from shader flag bit 0.  Applying additive blending to every
	// BLIP makes effects such as jump/landing dust glow and disappear into
	// the scene, even though additive rain still happens to look plausible.
	const bool additiveBlip = (pblipg->pshd->grfshd & 1u) != 0;
	struct BLIPBATCH
	{
		GLuint texture = 0;
		std::vector<BLIPGPU> instances;
		GLuint baseInstance = 0;
	};

	// A BLIP texture table is capped at 32 entries by PropagateBlipgShader.
	// Retain these buffers between calls so particle rendering does not create
	// and destroy several vectors every frame.
	static BLIPBATCH batches[32];
	static std::vector<BLIPGPU> orderedInstances;
	static std::vector<GLuint> orderedTextures;
	static std::vector<BLIPBATCH> orderedBatches;
	for (BLIPBATCH& batch : batches)
	{
		batch.texture = 0;
		batch.instances.clear();
		batch.baseInstance = 0;
	}
	orderedInstances.clear();
	orderedTextures.clear();
	orderedBatches.clear();

	const TEX& tex = pblipg->pshd->atex[0];
	const int textureTableCount = (std::min)(pblipg->cqwTexture, 32);
	if (textureTableCount <= 0) {
		return;
	}

	GLuint tableTextures[32]{};
	int tableBatches[32]{};
	int batchCount = 0;
	for (int tableIndex = 0; tableIndex < textureTableCount; ++tableIndex)
	{
		tableBatches[tableIndex] = -1;
		const int iframe = pblipg->aiframeTexture[tableIndex];
		if (iframe < 0 || iframe >= static_cast<int>(tex.abmp.size()) || tex.abmp[iframe] == nullptr)
			continue;

		BMP* const pframeBmp = tex.abmp[iframe];
		// The original BLIP texture packet is built with CTK_Ambient. For a
		// three-way shader that is the ambient/shadow texture, not diffuse.
		const bool threeWay = pblipg->pshd->shdk == SHDK_ThreeWay;
		// PCBRX stores each material role as an already-resolved RGBA image.
		const GLuint textureId = threeWay
			? (iframe < static_cast<int>(tex.glShadowMap.size())
				? tex.glShadowMap[iframe] : 0)
			: (iframe < static_cast<int>(tex.glDiffuseMap.size())
				? tex.glDiffuseMap[iframe] : 0);
		if (textureId == 0)
			continue;

		tableTextures[tableIndex] = textureId;
		int ibatch = 0;
		while (ibatch < batchCount && batches[ibatch].texture != textureId)
			++ibatch;
		if (ibatch == batchCount)
			batches[batchCount++].texture = textureId;
		tableBatches[tableIndex] = ibatch;
	}

	auto evaluateClq = [](const CLQ& clq, float t)
	{
		return clq.g0 + t * (clq.g1 + t * clq.g2);
	};

	for (BLIP* pblip = pblipg->dlBlip.pblipFirst; pblip != nullptr; pblip = pblip->dle.pblipNext)
	{
		const int iblipf = pblip->iblipfLatest & 1;
		const BLIPF& blipf = pblip->ablipf[iblipf];
		const int particleCount = pblip->cblipe;

		for (int i = 0; i < particleCount; ++i)
		{
			const BLIPP& pos = blipf.ablipp[i];
			const BLIPV& velocity = blipf.ablipv[i];
			const BLIPX& extra = blipf.ablipx[i];
			// VU1 evaluates every BLIP curve from the raw creation-time delta.
			// Emitters may place a creation time slightly ahead of the current
			// tick, so clamping here changes the first texture/color frame.
			const float age = g_clock.t - pos.tCreated;

			const int frameOffset = static_cast<int>(evaluateClq(pblipg->clqTexture, age));
			int tableIndex = static_cast<int>(extra.itex0) + frameOffset;
			if ((textureTableCount & (textureTableCount - 1)) == 0)
				tableIndex &= textureTableCount - 1;
			else
				tableIndex = ((tableIndex % textureTableCount) + textureTableCount) % textureTableCount;

			const int ibatch = tableBatches[tableIndex];
			if (ibatch < 0 || tableTextures[tableIndex] == 0)
				continue;

			glm::vec4 color(1.0f);
			if (pblipg->crgba > 0)
			{
				const int colorIndex = (std::min)(static_cast<int>(extra.irgba), pblipg->crgba - 1);
				color = pblipg->argba[colorIndex];
				if (pblipg->fColorRanges != 0 && colorIndex + 1 < pblipg->crgba)
				{
					const float u = glm::clamp(evaluateClq(pblipg->clqColor, age), 0.0f, 1.0f);
					color = glm::mix(color, pblipg->argba[colorIndex + 1], u);
				}
			}

			BLIPGPU instance{};
			instance.posCreated = glm::vec4(pos.x, pos.y, pos.z, pos.tCreated);
			instance.velocity = glm::vec4(velocity.x, velocity.y, velocity.z, 0.0f);
			instance.color = color;
			instance.roll = glm::vec4(extra.radRoll, extra.swRoll, 0.0f, 0.0f);
			if (additiveBlip)
				batches[ibatch].instances.push_back(instance);
			else
			{
				orderedInstances.push_back(instance);
				orderedTextures.push_back(tableTextures[tableIndex]);
			}
		}
	}

	const size_t groupInstanceBegin = s_blipFrameInstances.size();
	if (additiveBlip)
	{
		for (int ibatch = 0; ibatch < batchCount; ++ibatch)
		{
			BLIPBATCH& batch = batches[ibatch];
			batch.baseInstance = static_cast<GLuint>(s_blipFrameInstances.size());
			s_blipFrameInstances.insert(s_blipFrameInstances.end(), batch.instances.begin(), batch.instances.end());
		}
	}
	else
	{
		// Alpha blending is order-dependent. Preserve the BLIP list order and
		// merge only adjacent particles which already reference the same frame.
		for (size_t i = 0; i < orderedInstances.size(); ++i)
		{
			if (orderedBatches.empty() || orderedBatches.back().texture != orderedTextures[i])
			{
				orderedBatches.emplace_back();
				orderedBatches.back().texture = orderedTextures[i];
				orderedBatches.back().baseInstance = static_cast<GLuint>(s_blipFrameInstances.size());
			}

			orderedBatches.back().instances.push_back(orderedInstances[i]);
			s_blipFrameInstances.push_back(orderedInstances[i]);
		}
	}

	if (groupInstanceBegin == s_blipFrameInstances.size())
	{
		return;
	}

	BLIPGROUPGPU groupGpu{};
	groupGpu.model = prplblip->ro.model;
	groupGpu.params = glm::vec4(prplblip->ro.uAlpha, pblipg->rSFlying, static_cast<float>(pblipg->blipok), 0.0f);
	groupGpu.clqScale = glm::vec4(pblipg->clqScale.g0, pblipg->clqScale.g1, pblipg->clqScale.g2, 0.0f);
	groupGpu.clqAlpha = glm::vec4(pblipg->clqAlpha.g0, pblipg->clqAlpha.g1, pblipg->clqAlpha.g2, 0.0f);
	BLIPDRAWGROUP group{};
	group.additive = additiveBlip;
	group.streamOffset = StageFrameStream(&blipStream, &groupGpu, sizeof(groupGpu));
	group.firstRange = s_blipFrameRanges.size();

	if (additiveBlip)
	{
		for (int ibatch = 0; ibatch < batchCount; ++ibatch)
		{
			const BLIPBATCH& batch = batches[ibatch];
			if (!batch.instances.empty())
				s_blipFrameRanges.push_back({ batch.texture, batch.baseInstance, static_cast<GLsizei>(batch.instances.size()) });
		}
	}
	else
	{
		for (const BLIPBATCH& batch : orderedBatches)
			s_blipFrameRanges.push_back({ batch.texture, batch.baseInstance, static_cast<GLsizei>(batch.instances.size()) });
	}

	group.rangeCount = s_blipFrameRanges.size() - group.firstRange;
	if (group.streamOffset >= 0 && group.rangeCount > 0)
		s_blipFrameGroups.push_back(group);

}

void DrawSwCollisionAll(CM* pcm)
{
	s_renderState.Invalidate();
	UseShader(glGeomShader);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cmUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CMGL), &pcm->matWorldToClip);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, geomUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, geomUBO);

#ifdef CANE_GL41
	// GL 4.1 core only allows 1 px lines: the debug view stays 1 px there.
#else
	glLineWidth(2.0);
#endif
	glUniform4f(glslGeomDebugColor, 0.76f, 0.76f, 0.76f, 1.0f);

	glm::mat4 model{};

	for (int i = 0; i < allSWSoObjs.size(); i++)
	{
		SO* pso = allSWSoObjs[i];

		if (!SphereInFrustum(pcm->frustum, pso->xf.posWorld, pso->geomLocal.sRadius))
			continue;

		LoadMatrixFromPosRot(&pso->xf.posWorld, &pso->xf.matWorld, &model);

		glm::mat4 modelToClip = pcm->matWorldToClip * model;
		glUniformMatrix4fv(glslGeomModelToClip, 1, GL_FALSE, glm::value_ptr(modelToClip));

		if (pso->geomLocal.VAO != 0) 
		{
			BindVertexArray(pso->geomLocal.VAO);
			glDrawElements(GL_LINES, (GLsizei)pso->geomLocal.indices.size(), GL_UNSIGNED_SHORT, 0);
		}

		if (pso->geomCameraLocal.VAO != 0) 
		{
			BindVertexArray(pso->geomCameraLocal.VAO);
			glDrawElements(GL_LINES, (GLsizei)pso->geomCameraLocal.indices.size(), GL_UNSIGNED_SHORT, 0);
		}
	}

	BindVertexArray(0);
}

void DrawSwDialogTriggers(CM* pcm)
{
	s_renderState.Invalidate();
	UseShader(glGeomShader);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cmUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CMGL), &pcm->matWorldToClip);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, geomUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, geomUBO);
#ifdef CANE_GL41
	// GL 4.1 core only allows 1 px lines: the debug view stays 1 px there.
#else
	glLineWidth(4.0f);
#endif

	glm::mat4 model{};
	for (SO* pso : allSWSoObjs)
	{
		if (pso == nullptr || !FIsBasicDerivedFrom(pso, CID_VOLBTN))
			continue;

		VOLBTN* pvolbtn = static_cast<VOLBTN*>(pso);
		if (pvolbtn->btn.oidDialog == OID_Nil)
			continue;

		if (!SphereInFrustum(pcm->frustum, pso->xf.posWorld, pso->geomLocal.sRadius))
			continue;

		// Yellow is an available dialogue volume, green means Sly is currently
		// inside it, and red means the volume is disabled or already pushed.
		if (!pvolbtn->btn.fEnabled || pvolbtn->btn.buttons == BUTTONS_Disabled ||
			pvolbtn->btn.buttons == BUTTONS_Pushed)
			glUniform4f(glslGeomDebugColor, 1.0f, 0.20f, 0.16f, 1.0f);
		else if (pvolbtn->btn.buttons == BUTTONS_Contact || pvolbtn->cploPushWithin > 0)
			glUniform4f(glslGeomDebugColor, 0.20f, 1.0f, 0.30f, 1.0f);
		else
			glUniform4f(glslGeomDebugColor, 1.0f, 0.78f, 0.08f, 1.0f);

		LoadMatrixFromPosRot(&pso->xf.posWorld, &pso->xf.matWorld, &model);
		const glm::mat4 modelToClip = pcm->matWorldToClip * model;
		glUniformMatrix4fv(glslGeomModelToClip, 1, GL_FALSE, glm::value_ptr(modelToClip));

		if (pso->geomLocal.VAO != 0)
		{
			BindVertexArray(pso->geomLocal.VAO);
			glDrawElements(GL_LINES, static_cast<GLsizei>(pso->geomLocal.indices.size()),
				GL_UNSIGNED_SHORT, nullptr);
		}

		if (pso->geomCameraLocal.VAO != 0)
		{
			BindVertexArray(pso->geomCameraLocal.VAO);
			glDrawElements(GL_LINES, static_cast<GLsizei>(pso->geomCameraLocal.indices.size()),
				GL_UNSIGNED_SHORT, nullptr);
		}
	}

	glLineWidth(1.0f);
	BindVertexArray(0);
}

int numRo = 0;
int numRoCel = 0;

int g_cFrameGlobs = 0;
int g_cFrameCelGlobs = 0;

int numFrameObjs = 0;

int g_cframe = 0;
int g_cframeStaticLightsInvalid = 0;

int g_boundVAO = 0;

int g_shdIDBound = 0;
int g_lastAnimateUv = -1;

int g_grfshd = 0;

int g_dynamicTextureCount = 0;
std::vector <RPL> g_dynamicTexturePrpl;

int g_backGroundCount = 0;
std::vector <RPL> g_backGroundPrpl;

int g_backGroundBlendCount = 0;
std::vector <RPL> g_backGroundBlendPrpl;

int g_blotContextCount = 0;
std::vector <RPL> g_blotContextPrpl;

int g_opaqueCount = 0;
std::vector <RPL> g_opaquePrpl;

int g_cutOutCount = 0;
std::vector <RPL> g_cutOutPrpl;

int g_cutOutBlendAddCount = 0;
std::vector <RPL> g_cutOutBlendAddPrpl;

int g_celBorderCount = 0;
std::vector <RPLCEL> g_celBorderPrpl;


int g_projVolumeCount = 0;
std::vector <RPL> g_projVolumePrpl;

int g_opaqueAfterProjVolumeCount = 0;
std::vector <RPL> g_opaqueAfterProjVolumePrpl;

int g_cutOutAfterProjVolumeCount = 0;
std::vector <RPL> g_cutOutAfterProjVolumePrpl;

int g_cutOutAfterProjVolumeAddCount = 0;
std::vector <RPL> g_cutOutAfterProjVolumeAddPrpl;

int g_celBorderAfterProjVolumeCount = 0;
std::vector <RPLCEL> g_celBorderAfterProjVolumePrpl;

int g_murkClearCount = 0;
std::vector <RPL> g_murkClearPrpl;

int g_murkOpaqueCount = 0;
std::vector <RPL> g_murkOpaquePrpl;

int g_murkFillCount = 0;
std::vector <RPL> g_murkFillPrpl;

int g_translucentCount = 0;
std::vector <RPL> g_translucentPrpl;

int g_translucentAddCount = 0;
std::vector <RPL> g_translucentAddPrpl;

int g_translucentCelBorderCount = 0;
std::vector <RPLCEL> g_translucentCelBorderPrpl;

int g_blipCount = 0;
std::vector <RPL> g_blipPrpl;

int g_foreGroundCount = 0;
std::vector <RPL> g_foreGroundPrpl;

int g_worldMapCount = 0;
std::vector <RPL> g_worldMapPrpl;

int g_maxCount = 0;
std::vector <RPL> g_maxPrpl;

bool g_fVsync = false;

int g_fDisableSkin = 0;
int g_fDisablePoses = 0;
