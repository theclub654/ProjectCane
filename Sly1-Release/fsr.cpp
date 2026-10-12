// AMD FidelityFX Super Resolution for ProjectCane. See fsr.h.
//
// Backend 1: AMD FSR SDK (FidelityFX SDK 2.x "ffx-api", D3D12). The loader
// amd_fidelityfx_loader_dx12.dll picks FSR 4 (ML) on Radeon RX 7000/9000 and
// FSR 3.1 everywhere else. The DLLs are AMD's signed binaries, redistributed
// under the FidelityFX SDK license (Libraries\fidelityfx\license.md).
//
// Backend 2 (fallback): FSR 1. The GLSL below is a port of the 32-bit EASU
// and RCAS paths of AMD's ffx_fsr1.h, FidelityFX-FSR 1.0.2:
//
//   Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved.
//
//   Permission is hereby granted, free of charge, to any person obtaining a copy
//   of this software and associated documentation files (the "Software"), to deal
//   in the Software without restriction, including without limitation the rights
//   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//   copies of the Software, and to permit persons to whom the Software is
//   furnished to do so, subject to the following conditions:
//
//   The above copyright notice and this permission notice shall be included in
//   all copies or substantial portions of the Software.
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//   THE SOFTWARE.

#include "fsr.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#include <api/include/ffx_api.h>
#include <api/include/ffx_api_types.h>
#include <api/include/dx12/ffx_api_dx12.h>
#include <upscalers/include/ffx_upscale.h>

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>

// gl.cpp: re-runs the framebuffer-size logic (render size negotiation).
bool ApplyInternalResolutionSettings();

namespace
{
void FsrLog(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	std::vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	std::cout << "[FSR] " << buffer << std::endl;
}

// =============================================================================
// OpenGL <-> D3D12 interop entry points (glad was generated without them).
// =============================================================================
constexpr GLenum kGL_DEDICATED_MEMORY_OBJECT_EXT = 0x9581;
constexpr GLenum kGL_LAYOUT_GENERAL_EXT = 0x958D;
constexpr GLenum kGL_DEVICE_LUID_EXT = 0x9599;
constexpr GLenum kGL_HANDLE_TYPE_D3D12_RESOURCE_EXT = 0x958A;
constexpr GLenum kGL_HANDLE_TYPE_D3D12_FENCE_EXT = 0x9594;
constexpr GLenum kGL_D3D12_FENCE_VALUE_EXT = 0x9595;

typedef void (APIENTRY* PFN_fsr_glGetUnsignedBytevEXT)(GLenum pname, GLubyte* data);
typedef void (APIENTRY* PFN_fsr_glSemaphoreParameterui64vEXT)(GLuint semaphore, GLenum pname, const GLuint64* params);
typedef void (APIENTRY* PFN_fsr_glCreateMemoryObjectsEXT)(GLsizei n, GLuint* memoryObjects);
typedef void (APIENTRY* PFN_fsr_glDeleteMemoryObjectsEXT)(GLsizei n, const GLuint* memoryObjects);
typedef void (APIENTRY* PFN_fsr_glMemoryObjectParameterivEXT)(GLuint memoryObject, GLenum pname, const GLint* params);
typedef void (APIENTRY* PFN_fsr_glTexStorageMem2DEXT)(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLuint memory, GLuint64 offset);
typedef void (APIENTRY* PFN_fsr_glImportMemoryWin32HandleEXT)(GLuint memory, GLuint64 size, GLenum handleType, void* handle);
typedef void (APIENTRY* PFN_fsr_glGenSemaphoresEXT)(GLsizei n, GLuint* semaphores);
typedef void (APIENTRY* PFN_fsr_glDeleteSemaphoresEXT)(GLsizei n, const GLuint* semaphores);
typedef void (APIENTRY* PFN_fsr_glImportSemaphoreWin32HandleEXT)(GLuint semaphore, GLenum handleType, void* handle);
typedef void (APIENTRY* PFN_fsr_glSignalSemaphoreEXT)(GLuint semaphore, GLuint numBufferBarriers, const GLuint* buffers, GLuint numTextureBarriers, const GLuint* textures, const GLenum* dstLayouts);
typedef void (APIENTRY* PFN_fsr_glWaitSemaphoreEXT)(GLuint semaphore, GLuint numBufferBarriers, const GLuint* buffers, GLuint numTextureBarriers, const GLuint* textures, const GLenum* srcLayouts);

struct FSRGLINTEROP
{
	PFN_fsr_glGetUnsignedBytevEXT GetUnsignedBytev = nullptr;
	PFN_fsr_glSemaphoreParameterui64vEXT SemaphoreParameterui64v = nullptr;
	PFN_fsr_glCreateMemoryObjectsEXT CreateMemoryObjects = nullptr;
	PFN_fsr_glDeleteMemoryObjectsEXT DeleteMemoryObjects = nullptr;
	PFN_fsr_glMemoryObjectParameterivEXT MemoryObjectParameteriv = nullptr;
	PFN_fsr_glTexStorageMem2DEXT TexStorageMem2D = nullptr;
	PFN_fsr_glImportMemoryWin32HandleEXT ImportMemoryWin32Handle = nullptr;
	PFN_fsr_glGenSemaphoresEXT GenSemaphores = nullptr;
	PFN_fsr_glDeleteSemaphoresEXT DeleteSemaphores = nullptr;
	PFN_fsr_glImportSemaphoreWin32HandleEXT ImportSemaphoreWin32Handle = nullptr;
	PFN_fsr_glSignalSemaphoreEXT SignalSemaphore = nullptr;
	PFN_fsr_glWaitSemaphoreEXT WaitSemaphore = nullptr;
};
FSRGLINTEROP s_glx;

// =============================================================================
// State
// =============================================================================
enum FSRBACKEND
{
	FSRB_None = 0,   // not started yet
	FSRB_Sdk,        // FSR 4 / FSR 3.1 through the AMD FSR SDK (D3D12)
	FSRB_Fsr1        // OpenGL FSR 1 fallback
};

struct SHAREDTEX
{
	ID3D12Resource* res = nullptr;
	HANDLE handle = nullptr;
	GLuint glMemory = 0;
	GLuint glTexture = 0;
	int width = 0;
	int height = 0;
};

constexpr int kDxSlots = 3;

struct DXSLOT
{
	ID3D12CommandAllocator* alloc = nullptr;
	ID3D12GraphicsCommandList* list = nullptr;
	uint64_t fenceValue = 0;
};

struct FSRSTATE
{
	FSRQ fsrq = FSRQ_Off;
	int sharpness = 2;          // index into s_aSharpness

	FSRBACKEND backend = FSRB_None;
	std::string sdkStatus;      // why the SDK backend is not used (log/menu)
	char version[32] = "FSR";

	// ---- Final output (both backends): present size, with depth/stencil ----
	int width = 0;
	int height = 0;
	GLuint outTex = 0;
	GLuint outFbo = 0;
	GLuint outDepthStencil = 0;

	// ---- FSR 1 ----
	bool fFsr1Ready = false;
	bool fFsr1Failed = false;
	GLuint vao = 0;
	GLuint sampler = 0;
	GLuint easuProgram = 0;
	GLuint rcasProgram = 0;
	GLint easuCon[4] = { -1, -1, -1, -1 };
	GLint rcasCon = -1;
	GLuint easuTex = 0;
	GLuint easuFbo = 0;

	// ---- SDK backend ----
	HMODULE hmodLoader = nullptr;
	HMODULE hmodD3D12 = nullptr;
	HMODULE hmodDxgi = nullptr;
	PfnFfxCreateContext CreateContext = nullptr;
	PfnFfxDestroyContext DestroyContext = nullptr;
	PfnFfxConfigure Configure = nullptr;
	PfnFfxQuery Query = nullptr;
	PfnFfxDispatch Dispatch = nullptr;

	ID3D12Device* device = nullptr;
	ID3D12CommandQueue* queue = nullptr;
	ID3D12Fence* fence = nullptr;
	HANDLE hFence = nullptr;
	HANDLE hFenceEvent = nullptr;
	GLuint glFence = 0;
	uint64_t fenceValue = 0;
	DXSLOT slots[kDxSlots];
	uint32_t frameIndex = 0;

	ffxContext context = nullptr;
	int contextWidth = 0;
	int contextHeight = 0;
	bool fContextFailed = false;

	SHAREDTEX color;     // render size, RGBA8, top-down
	SHAREDTEX depth;     // render size, R32F device depth, top-down
	SHAREDTEX motion;    // render size, RG16F UV motion (current -> previous), top-down
	SHAREDTEX output;    // present size, RGBA8, top-down
	int sizedRenderWidth = 0;
	int sizedRenderHeight = 0;
	int sizedOutputWidth = 0;
	int sizedOutputHeight = 0;

	GLuint prepVao = 0;
	GLuint prepProgram = 0;
	GLuint prepFbo = 0;
	GLuint blitReadFbo = 0;
	GLuint blitDrawFbo = 0;
	GLint uClipToPrevClip = -1;
	GLint uJitterNdc = -1;
	GLint uRenderSize = -1;

	// Jitter (FSR pixel convention: +y = down) and the matching GL NDC offset.
	bool fJitterThisFrame = false;
	uint32_t jitterIndex = 0;
	float jitterX = 0.0f;
	float jitterY = 0.0f;
	glm::vec2 jitterNdc = glm::vec2(0.0f);

	bool fReset = true;
	bool fHavePrevCamera = false;
	glm::mat4 matPrevWorldToClip = glm::mat4(1.0f);
	glm::vec3 dirPrevForward = glm::vec3(0.0f, 0.0f, -1.0f);
	LARGE_INTEGER tPrev{};
};
FSRSTATE s_fsr;

const char* const s_apchzQuality[FSRQ_Max] = { "Off", "Native AA", "Quality", "Balanced", "Performance", "Ultra Performance" };
const wchar_t* const s_apwchzQualityKey[FSRQ_Max] = { L"Off", L"NativeAA", L"Quality", L"Balanced", L"Performance", L"UltraPerformance" };
const float s_aScale[FSRQ_Max] = { 1.0f, 1.0f, 1.5f, 1.7f, 2.0f, 3.0f };

struct SHARPNESS
{
	const char* name;
	const wchar_t* key;
	float sdk;      // FSR 4 / 3.1: 0..1
	float stops;    // FSR 1 RCAS: 0 = max, larger = softer
};
const SHARPNESS s_aSharpness[] =
{
	{ "Off",    L"Off",    0.0f,  4.0f },
	{ "Low",    L"Low",    0.25f, 1.0f },
	{ "Medium", L"Medium", 0.5f,  0.5f },
	{ "High",   L"High",   0.8f,  0.2f },
	{ "Max",    L"Max",    1.0f,  0.0f },
};
constexpr int kSharpnessCount = static_cast<int>(std::size(s_aSharpness));

std::filesystem::path ExeDirectory()
{
	wchar_t path[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, path, MAX_PATH);
	return std::filesystem::path(path).parent_path();
}

std::wstring IniPath()
{
	return (ExeDirectory() / L"FSR.ini").wstring();
}

void LoadFsrSettings()
{
	const std::wstring ini = IniPath();
	wchar_t value[64] = {};
	GetPrivateProfileStringW(L"FSR", L"Mode", L"Off", value, 64, ini.c_str());
	s_fsr.fsrq = FSRQ_Off;
	for (int i = 0; i < FSRQ_Max; ++i)
	{
		if (_wcsicmp(value, s_apwchzQualityKey[i]) == 0)
			s_fsr.fsrq = static_cast<FSRQ>(i);
	}
	GetPrivateProfileStringW(L"FSR", L"Sharpness", L"Medium", value, 64, ini.c_str());
	s_fsr.sharpness = 2;
	for (int i = 0; i < kSharpnessCount; ++i)
	{
		if (_wcsicmp(value, s_aSharpness[i].key) == 0)
			s_fsr.sharpness = i;
	}
}

void SaveFsrSettings()
{
	const std::wstring ini = IniPath();
	WritePrivateProfileStringW(L"FSR", L"Mode", s_apwchzQualityKey[s_fsr.fsrq], ini.c_str());
	WritePrivateProfileStringW(L"FSR", L"Sharpness", s_aSharpness[s_fsr.sharpness].key, ini.c_str());
}

template <typename T>
void SafeRelease(T*& p)
{
	if (p != nullptr)
	{
		p->Release();
		p = nullptr;
	}
}

float Halton(uint32_t index, uint32_t base)
{
	float f = 1.0f;
	float result = 0.0f;
	while (index > 0)
	{
		f /= static_cast<float>(base);
		result += f * static_cast<float>(index % base);
		index /= base;
	}
	return result;
}

// =============================================================================
// Shaders
// =============================================================================
const char* const s_szVert = R"(#version 430 core
void main()
{
	// Fullscreen triangle.
	vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)";

const char* const s_szCommon = R"(#version 430 core
// Approximations from ffx_a.h (exactly as FSR uses them; they also keep
// zero inputs finite, which the exact versions would not).
float APrxLoRcpF1(float a)  { return uintBitsToFloat(0x7ef07ebbu - floatBitsToUint(a)); }
float APrxMedRcpF1(float a) { float b = uintBitsToFloat(0x7ef19fffu - floatBitsToUint(a)); return b * (-b * a + 2.0); }
float APrxLoRsqF1(float a)  { return uintBitsToFloat(0x5f347d74u - (floatBitsToUint(a) >> 1u)); }
float ASatF1(float a)       { return clamp(a, 0.0, 1.0); }
float AMin3F1(float a, float b, float c) { return min(a, min(b, c)); }
float AMax3F1(float a, float b, float c) { return max(a, max(b, c)); }
vec3  AMin3F3(vec3 a, vec3 b, vec3 c)    { return min(a, min(b, c)); }
vec3  AMax3F3(vec3 a, vec3 b, vec3 c)    { return max(a, max(b, c)); }
// ARcpF1 replacement that never divides by exactly zero.
float RcpSafe(float a)      { return 1.0 / (abs(a) < 1.0e-7 ? -1.0e-7 : a); }
)";

const char* const s_szEasu = R"(
layout(binding = 0) uniform sampler2D uInput;
uniform vec4 uCon0;
uniform vec4 uCon1;
uniform vec4 uCon2;
uniform vec4 uCon3;
layout(location = 0) out vec4 oColor;

vec4 FsrEasuRF(vec2 p) { return textureGather(uInput, p, 0); }
vec4 FsrEasuGF(vec2 p) { return textureGather(uInput, p, 1); }
vec4 FsrEasuBF(vec2 p) { return textureGather(uInput, p, 2); }

void FsrEasuTapF(inout vec3 aC, inout float aW, vec2 off, vec2 dir, vec2 len, float lob, float clp, vec3 c)
{
	vec2 v;
	v.x = (off.x * ( dir.x)) + (off.y * dir.y);
	v.y = (off.x * (-dir.y)) + (off.y * dir.x);
	v *= len;
	float d2 = v.x * v.x + v.y * v.y;
	d2 = min(d2, clp);
	float wB = (2.0 / 5.0) * d2 - 1.0;
	float wA = lob * d2 - 1.0;
	wB *= wB;
	wA *= wA;
	wB = (25.0 / 16.0) * wB - (25.0 / 16.0 - 1.0);
	float w = wB * wA;
	aC += c * w;
	aW += w;
}

void FsrEasuSetF(inout vec2 dir, inout float len, vec2 pp, bool biS, bool biT, bool biU, bool biV,
	float lA, float lB, float lC, float lD, float lE)
{
	float w = 0.0;
	if (biS) w = (1.0 - pp.x) * (1.0 - pp.y);
	if (biT) w =        pp.x  * (1.0 - pp.y);
	if (biU) w = (1.0 - pp.x) *        pp.y;
	if (biV) w =        pp.x  *        pp.y;
	float dc = lD - lC;
	float cb = lC - lB;
	float lenX = max(abs(dc), abs(cb));
	lenX = APrxLoRcpF1(lenX);
	float dirX = lD - lB;
	dir.x += dirX * w;
	lenX = ASatF1(abs(dirX) * lenX);
	lenX *= lenX;
	len += lenX * w;
	float ec = lE - lC;
	float ca = lC - lA;
	float lenY = max(abs(ec), abs(ca));
	lenY = APrxLoRcpF1(lenY);
	float dirY = lE - lA;
	dir.y += dirY * w;
	lenY = ASatF1(abs(dirY) * lenY);
	lenY *= lenY;
	len += lenY * w;
}

void main()
{
	uvec2 ip = uvec2(gl_FragCoord.xy);
	vec2 pp = vec2(ip) * uCon0.xy + uCon0.zw;
	vec2 fp = floor(pp);
	pp -= fp;
	vec2 p0 = fp * uCon1.xy + uCon1.zw;
	vec2 p1 = p0 + uCon2.xy;
	vec2 p2 = p0 + uCon2.zw;
	vec2 p3 = p0 + uCon3.xy;
	vec4 bczzR = FsrEasuRF(p0);
	vec4 bczzG = FsrEasuGF(p0);
	vec4 bczzB = FsrEasuBF(p0);
	vec4 ijfeR = FsrEasuRF(p1);
	vec4 ijfeG = FsrEasuGF(p1);
	vec4 ijfeB = FsrEasuBF(p1);
	vec4 klhgR = FsrEasuRF(p2);
	vec4 klhgG = FsrEasuGF(p2);
	vec4 klhgB = FsrEasuBF(p2);
	vec4 zzonR = FsrEasuRF(p3);
	vec4 zzonG = FsrEasuGF(p3);
	vec4 zzonB = FsrEasuBF(p3);
	vec4 bczzL = bczzB * 0.5 + (bczzR * 0.5 + bczzG);
	vec4 ijfeL = ijfeB * 0.5 + (ijfeR * 0.5 + ijfeG);
	vec4 klhgL = klhgB * 0.5 + (klhgR * 0.5 + klhgG);
	vec4 zzonL = zzonB * 0.5 + (zzonR * 0.5 + zzonG);
	float bL = bczzL.x;
	float cL = bczzL.y;
	float iL = ijfeL.x;
	float jL = ijfeL.y;
	float fL = ijfeL.z;
	float eL = ijfeL.w;
	float kL = klhgL.x;
	float lL = klhgL.y;
	float hL = klhgL.z;
	float gL = klhgL.w;
	float oL = zzonL.z;
	float nL = zzonL.w;
	vec2 dir = vec2(0.0);
	float len = 0.0;
	FsrEasuSetF(dir, len, pp, true,  false, false, false, bL, eL, fL, gL, jL);
	FsrEasuSetF(dir, len, pp, false, true,  false, false, cL, fL, gL, hL, kL);
	FsrEasuSetF(dir, len, pp, false, false, true,  false, fL, iL, jL, kL, nL);
	FsrEasuSetF(dir, len, pp, false, false, false, true,  gL, jL, kL, lL, oL);
	vec2 dir2 = dir * dir;
	float dirR = dir2.x + dir2.y;
	bool zro = dirR < (1.0 / 32768.0);
	dirR = APrxLoRsqF1(dirR);
	dirR = zro ? 1.0 : dirR;
	dir.x = zro ? 1.0 : dir.x;
	dir *= vec2(dirR);
	len = len * 0.5;
	len *= len;
	float stretch = (dir.x * dir.x + dir.y * dir.y) * APrxLoRcpF1(max(abs(dir.x), abs(dir.y)));
	vec2 len2 = vec2(1.0 + (stretch - 1.0) * len, 1.0 - 0.5 * len);
	float lob = 0.5 + ((1.0 / 4.0 - 0.04) - 0.5) * len;
	float clp = APrxLoRcpF1(lob);
	vec3 min4 = min(AMin3F3(vec3(ijfeR.z, ijfeG.z, ijfeB.z), vec3(klhgR.w, klhgG.w, klhgB.w), vec3(ijfeR.y, ijfeG.y, ijfeB.y)),
		vec3(klhgR.x, klhgG.x, klhgB.x));
	vec3 max4 = max(AMax3F3(vec3(ijfeR.z, ijfeG.z, ijfeB.z), vec3(klhgR.w, klhgG.w, klhgB.w), vec3(ijfeR.y, ijfeG.y, ijfeB.y)),
		vec3(klhgR.x, klhgG.x, klhgB.x));
	vec3 aC = vec3(0.0);
	float aW = 0.0;
	FsrEasuTapF(aC, aW, vec2( 0.0, -1.0) - pp, dir, len2, lob, clp, vec3(bczzR.x, bczzG.x, bczzB.x)); // b
	FsrEasuTapF(aC, aW, vec2( 1.0, -1.0) - pp, dir, len2, lob, clp, vec3(bczzR.y, bczzG.y, bczzB.y)); // c
	FsrEasuTapF(aC, aW, vec2(-1.0,  1.0) - pp, dir, len2, lob, clp, vec3(ijfeR.x, ijfeG.x, ijfeB.x)); // i
	FsrEasuTapF(aC, aW, vec2( 0.0,  1.0) - pp, dir, len2, lob, clp, vec3(ijfeR.y, ijfeG.y, ijfeB.y)); // j
	FsrEasuTapF(aC, aW, vec2( 0.0,  0.0) - pp, dir, len2, lob, clp, vec3(ijfeR.z, ijfeG.z, ijfeB.z)); // f
	FsrEasuTapF(aC, aW, vec2(-1.0,  0.0) - pp, dir, len2, lob, clp, vec3(ijfeR.w, ijfeG.w, ijfeB.w)); // e
	FsrEasuTapF(aC, aW, vec2( 1.0,  1.0) - pp, dir, len2, lob, clp, vec3(klhgR.x, klhgG.x, klhgB.x)); // k
	FsrEasuTapF(aC, aW, vec2( 2.0,  1.0) - pp, dir, len2, lob, clp, vec3(klhgR.y, klhgG.y, klhgB.y)); // l
	FsrEasuTapF(aC, aW, vec2( 2.0,  0.0) - pp, dir, len2, lob, clp, vec3(klhgR.z, klhgG.z, klhgB.z)); // h
	FsrEasuTapF(aC, aW, vec2( 1.0,  0.0) - pp, dir, len2, lob, clp, vec3(klhgR.w, klhgG.w, klhgB.w)); // g
	FsrEasuTapF(aC, aW, vec2( 1.0,  2.0) - pp, dir, len2, lob, clp, vec3(zzonR.z, zzonG.z, zzonB.z)); // o
	FsrEasuTapF(aC, aW, vec2( 0.0,  2.0) - pp, dir, len2, lob, clp, vec3(zzonR.w, zzonG.w, zzonB.w)); // n
	vec3 pix = min(max4, max(min4, aC * vec3(1.0 / max(aW, 1.0e-6))));
	oColor = vec4(pix, 1.0);
}
)";

const char* const s_szRcas = R"(
#define FSR_RCAS_LIMIT (0.25 - (1.0 / 16.0))
layout(binding = 0) uniform sampler2D uInput;
uniform float uSharpness;   // exp2(-stops)
layout(location = 0) out vec4 oColor;

vec3 FsrRcasLoadF(ivec2 p)
{
	ivec2 size = textureSize(uInput, 0);
	return texelFetch(uInput, clamp(p, ivec2(0), size - 1), 0).rgb;
}

void main()
{
	ivec2 sp = ivec2(gl_FragCoord.xy);
	vec3 b = FsrRcasLoadF(sp + ivec2( 0, -1));
	vec3 d = FsrRcasLoadF(sp + ivec2(-1,  0));
	vec3 e = FsrRcasLoadF(sp);
	vec3 f = FsrRcasLoadF(sp + ivec2( 1,  0));
	vec3 h = FsrRcasLoadF(sp + ivec2( 0,  1));
	float bR = b.r, bG = b.g, bB = b.b;
	float dR = d.r, dG = d.g, dB = d.b;
	float eR = e.r, eG = e.g, eB = e.b;
	float fR = f.r, fG = f.g, fB = f.b;
	float hR = h.r, hG = h.g, hB = h.b;
	float mn4R = min(AMin3F1(bR, dR, fR), hR);
	float mn4G = min(AMin3F1(bG, dG, fG), hG);
	float mn4B = min(AMin3F1(bB, dB, fB), hB);
	float mx4R = max(AMax3F1(bR, dR, fR), hR);
	float mx4G = max(AMax3F1(bG, dG, fG), hG);
	float mx4B = max(AMax3F1(bB, dB, fB), hB);
	vec2 peakC = vec2(1.0, -1.0 * 4.0);
	float hitMinR = min(mn4R, eR) * RcpSafe(4.0 * mx4R);
	float hitMinG = min(mn4G, eG) * RcpSafe(4.0 * mx4G);
	float hitMinB = min(mn4B, eB) * RcpSafe(4.0 * mx4B);
	float hitMaxR = (peakC.x - max(mx4R, eR)) * RcpSafe(4.0 * mn4R + peakC.y);
	float hitMaxG = (peakC.x - max(mx4G, eG)) * RcpSafe(4.0 * mn4G + peakC.y);
	float hitMaxB = (peakC.x - max(mx4B, eB)) * RcpSafe(4.0 * mn4B + peakC.y);
	float lobeR = max(-hitMinR, hitMaxR);
	float lobeG = max(-hitMinG, hitMaxG);
	float lobeB = max(-hitMinB, hitMaxB);
	float lobe = max(-FSR_RCAS_LIMIT, min(AMax3F1(lobeR, lobeG, lobeB), 0.0)) * uSharpness;
	float rcpL = APrxMedRcpF1(4.0 * lobe + 1.0);
	vec3 pix;
	pix.r = (lobe * bR + lobe * dR + lobe * hR + lobe * fR + eR) * rcpL;
	pix.g = (lobe * bG + lobe * dG + lobe * hG + lobe * fG + eG) * rcpL;
	pix.b = (lobe * bB + lobe * dB + lobe * hB + lobe * fB + eB) * rcpL;
	oColor = vec4(clamp(pix, 0.0, 1.0), 1.0);
}
)";


// Depth + camera motion vectors for the SDK backend, written top-down (D3D12
// row order). Same reconstruction as the DLSS path.
const char* const s_szPrepFrag = R"(#version 430 core
layout(binding = 0) uniform sampler2D uSceneDepth;
uniform mat4 uClipToPrevClip;   // unjittered: prevWorldToClip * inverse(worldToClip)
uniform vec2 uJitterNdc;        // jitter added to this frame's projection (GL NDC)
uniform vec2 uRenderSize;

layout(location = 0) out float outDepth;
layout(location = 1) out vec2 outMotion;

void main()
{
	// Output row y holds GL source row (height - 1 - y).
	ivec2 pixel = ivec2(gl_FragCoord.xy);
	pixel.y = int(uRenderSize.y) - 1 - pixel.y;
	float depth = texelFetch(uSceneDepth, pixel, 0).r;
	outDepth = depth;

	// Reproject this pixel's unjittered clip position with last frame's
	// camera. Result in UV units (current -> previous); FSR scales it to
	// pixels with motionVectorScale = render size.
	vec2 uv = (vec2(pixel) + 0.5) / uRenderSize;
	vec2 ndc = uv * 2.0 - 1.0 - uJitterNdc;
	vec4 clip = vec4(ndc, depth * 2.0 - 1.0, 1.0);
	vec4 prevClip = uClipToPrevClip * clip;
	vec2 prevNdc = prevClip.xy / max(abs(prevClip.w), 1e-6) * sign(prevClip.w);
	outMotion = (prevNdc - ndc) * 0.5;
	outMotion.y = -outMotion.y;   // top-down images: +y points down
}
)";

GLuint CompileShader(GLenum type, const char* const* sources, int count, const char* name)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, count, sources, nullptr);
	glCompileShader(shader);
	GLint ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		char log[2048] = {};
		glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
		FsrLog("%s shader failed to compile:\n%s", name, log);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint LinkProgram(const char* const* fragSources, int fragCount, const char* name)
{
	const char* vert[] = { s_szVert };
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vert, 1, name);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragSources, fragCount, name);
	if (vs == 0 || fs == 0)
	{
		if (vs != 0) glDeleteShader(vs);
		if (fs != 0) glDeleteShader(fs);
		return 0;
	}
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glDeleteShader(vs);
	glDeleteShader(fs);
	GLint ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &ok);
	if (!ok)
	{
		char log[2048] = {};
		glGetProgramInfoLog(program, sizeof(log), nullptr, log);
		FsrLog("%s program failed to link:\n%s", name, log);
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

// GL state touched by the FSR passes (the framebuffer is intentionally left
// on the FSR output for the HUD).
struct FSRGLSTATE
{
	GLint program = 0, vao = 0, activeTexture = 0, texture0 = 0, sampler0 = 0;
	GLboolean depthTest = GL_FALSE, blend = GL_FALSE, cull = GL_FALSE, stencil = GL_FALSE, scissor = GL_FALSE;
	GLboolean depthMask = GL_TRUE;
	GLboolean colorMask[4] = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };

	void Save()
	{
		glGetIntegerv(GL_CURRENT_PROGRAM, &program);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
		glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
		glActiveTexture(GL_TEXTURE0);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture0);
		glGetIntegerv(GL_SAMPLER_BINDING, &sampler0);
		depthTest = glIsEnabled(GL_DEPTH_TEST);
		blend = glIsEnabled(GL_BLEND);
		cull = glIsEnabled(GL_CULL_FACE);
		stencil = glIsEnabled(GL_STENCIL_TEST);
		scissor = glIsEnabled(GL_SCISSOR_TEST);
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
		glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_SCISSOR_TEST);
		glDepthMask(GL_FALSE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	static void Set(GLenum cap, GLboolean on)
	{
		if (on) glEnable(cap); else glDisable(cap);
	}

	// Restores state and leaves the final output bound for the HUD with a
	// cleared depth/stencil buffer.
	void RestoreAndBindOutput(int width, int height) const
	{
		glBindSampler(0, static_cast<GLuint>(sampler0));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture0));
		glActiveTexture(static_cast<GLenum>(activeTexture));
		glUseProgram(static_cast<GLuint>(program));
		glBindVertexArray(static_cast<GLuint>(vao));
		Set(GL_DEPTH_TEST, depthTest);
		Set(GL_BLEND, blend);
		Set(GL_CULL_FACE, cull);
		Set(GL_STENCIL_TEST, stencil);
		Set(GL_SCISSOR_TEST, scissor);
		glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);

		glBindFramebuffer(GL_FRAMEBUFFER, s_fsr.outFbo);
		glViewport(0, 0, width, height);
		const GLboolean scissorOn = glIsEnabled(GL_SCISSOR_TEST);
		glDisable(GL_SCISSOR_TEST);
		glClearDepth(1.0);
		glClearStencil(0);
		glStencilMask(0xFF);
		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDepthMask(depthMask);
		Set(GL_SCISSOR_TEST, scissorOn);
	}
};

// =============================================================================
// Final output target (both backends)
// =============================================================================
GLuint CreateColorTarget(int width, int height)
{
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return tex;
}

void DestroyTargets()
{
	if (s_fsr.easuFbo != 0) glDeleteFramebuffers(1, &s_fsr.easuFbo);
	if (s_fsr.outFbo != 0) glDeleteFramebuffers(1, &s_fsr.outFbo);
	if (s_fsr.easuTex != 0) glDeleteTextures(1, &s_fsr.easuTex);
	if (s_fsr.outTex != 0) glDeleteTextures(1, &s_fsr.outTex);
	if (s_fsr.outDepthStencil != 0) glDeleteRenderbuffers(1, &s_fsr.outDepthStencil);
	s_fsr.easuFbo = s_fsr.outFbo = s_fsr.easuTex = s_fsr.outTex = s_fsr.outDepthStencil = 0;
	s_fsr.width = s_fsr.height = 0;
}

bool FEnsureTargets(int width, int height)
{
	if (s_fsr.outFbo != 0 && s_fsr.width == width && s_fsr.height == height)
		return true;
	DestroyTargets();

	GLint prevTex = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex);
	s_fsr.easuTex = CreateColorTarget(width, height);
	s_fsr.outTex = CreateColorTarget(width, height);
	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prevTex));

	glGenRenderbuffers(1, &s_fsr.outDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, s_fsr.outDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	bool ok = true;
	glGenFramebuffers(1, &s_fsr.easuFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, s_fsr.easuFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_fsr.easuTex, 0);
	ok &= glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

	glGenFramebuffers(1, &s_fsr.outFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, s_fsr.outFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_fsr.outTex, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_fsr.outDepthStencil);
	ok &= glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (!ok)
	{
		FsrLog("Could not create %dx%d output targets", width, height);
		DestroyTargets();
		return false;
	}
	s_fsr.width = width;
	s_fsr.height = height;
	return true;
}

// =============================================================================
// FSR 1 (OpenGL fallback)
// =============================================================================
bool FEnsureFsr1()
{
	if (s_fsr.fFsr1Ready)
		return true;
	if (s_fsr.fFsr1Failed)
		return false;

	const char* easu[] = { s_szCommon, s_szEasu };
	const char* rcas[] = { s_szCommon, s_szRcas };
	s_fsr.easuProgram = LinkProgram(easu, 2, "EASU");
	s_fsr.rcasProgram = LinkProgram(rcas, 2, "RCAS");
	if (s_fsr.easuProgram == 0 || s_fsr.rcasProgram == 0)
	{
		s_fsr.fFsr1Failed = true;
		FsrLog("FSR 1 disabled: shaders unavailable on this GPU/driver.");
		return false;
	}
	s_fsr.easuCon[0] = glGetUniformLocation(s_fsr.easuProgram, "uCon0");
	s_fsr.easuCon[1] = glGetUniformLocation(s_fsr.easuProgram, "uCon1");
	s_fsr.easuCon[2] = glGetUniformLocation(s_fsr.easuProgram, "uCon2");
	s_fsr.easuCon[3] = glGetUniformLocation(s_fsr.easuProgram, "uCon3");
	s_fsr.rcasCon = glGetUniformLocation(s_fsr.rcasProgram, "uSharpness");

	glGenVertexArrays(1, &s_fsr.vao);
	glGenSamplers(1, &s_fsr.sampler);
	glSamplerParameteri(s_fsr.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(s_fsr.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(s_fsr.sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(s_fsr.sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	s_fsr.fFsr1Ready = true;
	FsrLog("FSR 1 (OpenGL) ready.");
	return true;
}

void ShutdownFsr1()
{
	if (s_fsr.easuProgram != 0) glDeleteProgram(s_fsr.easuProgram);
	if (s_fsr.rcasProgram != 0) glDeleteProgram(s_fsr.rcasProgram);
	if (s_fsr.vao != 0) glDeleteVertexArrays(1, &s_fsr.vao);
	if (s_fsr.sampler != 0) glDeleteSamplers(1, &s_fsr.sampler);
	s_fsr.easuProgram = s_fsr.rcasProgram = s_fsr.vao = s_fsr.sampler = 0;
	s_fsr.fFsr1Ready = false;
}

bool FEvaluateFsr1(GLuint sceneColorTex, int renderWidth, int renderHeight, int outputWidth, int outputHeight)
{
	if (!FEnsureFsr1() || !FEnsureTargets(outputWidth, outputHeight))
		return false;

	// Size of the texture holding the scene (may exceed the rendered area).
	GLint prevTex = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex);
	GLint inputWidth = renderWidth;
	GLint inputHeight = renderHeight;
	glBindTexture(GL_TEXTURE_2D, sceneColorTex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &inputWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &inputHeight);
	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prevTex));
	if (inputWidth <= 0 || inputHeight <= 0)
		return false;

	FSRGLSTATE state;
	state.Save();
	glBindVertexArray(s_fsr.vao);
	glViewport(0, 0, outputWidth, outputHeight);

	// EASU: scene (render size) -> easuTex (output size)
	const float rw = static_cast<float>(renderWidth);
	const float rh = static_cast<float>(renderHeight);
	const float ow = static_cast<float>(outputWidth);
	const float oh = static_cast<float>(outputHeight);
	const float iw = static_cast<float>(inputWidth);
	const float ih = static_cast<float>(inputHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, s_fsr.easuFbo);
	glUseProgram(s_fsr.easuProgram);
	glUniform4f(s_fsr.easuCon[0], rw / ow, rh / oh, 0.5f * rw / ow - 0.5f, 0.5f * rh / oh - 0.5f);
	glUniform4f(s_fsr.easuCon[1], 1.0f / iw, 1.0f / ih, 1.0f / iw, -1.0f / ih);
	glUniform4f(s_fsr.easuCon[2], -1.0f / iw, 2.0f / ih, 1.0f / iw, 2.0f / ih);
	glUniform4f(s_fsr.easuCon[3], 0.0f, 4.0f / ih, 0.0f, 0.0f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneColorTex);
	glBindSampler(0, s_fsr.sampler);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// RCAS: easuTex -> outTex
	glBindFramebuffer(GL_FRAMEBUFFER, s_fsr.outFbo);
	glUseProgram(s_fsr.rcasProgram);
	glUniform1f(s_fsr.rcasCon, std::exp2(-s_aSharpness[s_fsr.sharpness].stops));
	glBindTexture(GL_TEXTURE_2D, s_fsr.easuTex);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	state.RestoreAndBindOutput(outputWidth, outputHeight);
	return true;
}

// =============================================================================
// AMD FSR SDK backend (D3D12 + OpenGL interop)
// =============================================================================
bool FHasGlExtension(const char* name)
{
	GLint count = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &count);
	for (GLint i = 0; i < count; ++i)
	{
		const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i)));
		if (ext != nullptr && std::strcmp(ext, name) == 0)
			return true;
	}
	return false;
}

template <typename T>
bool LoadGlProc(T& fn, const char* name)
{
	fn = reinterpret_cast<T>(wglGetProcAddress(name));
	return fn != nullptr;
}

bool SdkFail(const char* reason)
{
	s_fsr.sdkStatus = reason;
	FsrLog("FSR 4 / 3.1 unavailable: %s. Using FSR 1.", reason);
	return false;
}

bool LoadGlInterop()
{
	const char* required[] =
	{
		"GL_EXT_memory_object", "GL_EXT_memory_object_win32",
		"GL_EXT_semaphore", "GL_EXT_semaphore_win32"
	};
	for (const char* ext : required)
	{
		if (!FHasGlExtension(ext))
		{
			static std::string reason;
			reason = std::string("OpenGL driver lacks ") + ext;
			return SdkFail(reason.c_str());
		}
	}

	bool ok = true;
	ok &= LoadGlProc(s_glx.GetUnsignedBytev, "glGetUnsignedBytevEXT");
	ok &= LoadGlProc(s_glx.SemaphoreParameterui64v, "glSemaphoreParameterui64vEXT");
	ok &= LoadGlProc(s_glx.CreateMemoryObjects, "glCreateMemoryObjectsEXT");
	ok &= LoadGlProc(s_glx.DeleteMemoryObjects, "glDeleteMemoryObjectsEXT");
	ok &= LoadGlProc(s_glx.MemoryObjectParameteriv, "glMemoryObjectParameterivEXT");
	ok &= LoadGlProc(s_glx.TexStorageMem2D, "glTexStorageMem2DEXT");
	ok &= LoadGlProc(s_glx.ImportMemoryWin32Handle, "glImportMemoryWin32HandleEXT");
	ok &= LoadGlProc(s_glx.GenSemaphores, "glGenSemaphoresEXT");
	ok &= LoadGlProc(s_glx.DeleteSemaphores, "glDeleteSemaphoresEXT");
	ok &= LoadGlProc(s_glx.ImportSemaphoreWin32Handle, "glImportSemaphoreWin32HandleEXT");
	ok &= LoadGlProc(s_glx.SignalSemaphore, "glSignalSemaphoreEXT");
	ok &= LoadGlProc(s_glx.WaitSemaphore, "glWaitSemaphoreEXT");
	if (!ok)
		return SdkFail("OpenGL interop entry points missing");
	return true;
}

uint64_t SignalDx()
{
	const uint64_t value = ++s_fsr.fenceValue;
	s_fsr.queue->Signal(s_fsr.fence, value);
	return value;
}

void WaitDxCpu(uint64_t value)
{
	if (s_fsr.fence == nullptr || s_fsr.fence->GetCompletedValue() >= value)
		return;
	s_fsr.fence->SetEventOnCompletion(value, s_fsr.hFenceEvent);
	if (WaitForSingleObject(s_fsr.hFenceEvent, 2000) == WAIT_TIMEOUT)
		FsrLog("TIMEOUT waiting for D3D12 fence %llu", static_cast<unsigned long long>(value));
}

void WaitDxIdle()
{
	if (s_fsr.queue != nullptr && s_fsr.fence != nullptr)
		WaitDxCpu(SignalDx());
}

// GL -> D3D12: GL writes to these textures are visible to later D3D12 work.
void GlSignalDx(const GLuint* textures, int count)
{
	GLenum layouts[8];
	for (int i = 0; i < count && i < 8; ++i)
		layouts[i] = kGL_LAYOUT_GENERAL_EXT;
	const GLuint64 value = ++s_fsr.fenceValue;
	s_glx.SemaphoreParameterui64v(s_fsr.glFence, kGL_D3D12_FENCE_VALUE_EXT, &value);
	s_glx.SignalSemaphore(s_fsr.glFence, 0, nullptr, static_cast<GLuint>(count), textures, layouts);
	glFlush();
	s_fsr.queue->Wait(s_fsr.fence, value);
}

// D3D12 -> GL: later GL commands wait for the D3D12 work submitted so far.
uint64_t DxSignalGl(const GLuint* textures, int count)
{
	GLenum layouts[8];
	for (int i = 0; i < count && i < 8; ++i)
		layouts[i] = kGL_LAYOUT_GENERAL_EXT;
	const GLuint64 value = SignalDx();
	s_glx.SemaphoreParameterui64v(s_fsr.glFence, kGL_D3D12_FENCE_VALUE_EXT, &value);
	s_glx.WaitSemaphore(s_fsr.glFence, 0, nullptr, static_cast<GLuint>(count), textures, layouts);
	return value;
}

void DestroySharedTex(SHAREDTEX& tex)
{
	if (tex.glTexture != 0)
		glDeleteTextures(1, &tex.glTexture);
	if (tex.glMemory != 0)
		s_glx.DeleteMemoryObjects(1, &tex.glMemory);
	SafeRelease(tex.res);
	if (tex.handle != nullptr)
		CloseHandle(tex.handle);
	tex = SHAREDTEX{};
}

bool CreateSharedTex(SHAREDTEX& tex, int width, int height, DXGI_FORMAT format, GLenum glFormat)
{
	tex.width = width;
	tex.height = height;

	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC rd{};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rd.Width = static_cast<UINT64>(width);
	rd.Height = static_cast<UINT>(height);
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.Format = format;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	if (FAILED(s_fsr.device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_SHARED, &rd,
		D3D12_RESOURCE_STATE_COMMON, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&tex.res))))
		return false;
	if (FAILED(s_fsr.device->CreateSharedHandle(tex.res, nullptr, GENERIC_ALL, nullptr, &tex.handle)))
		return false;

	const D3D12_RESOURCE_ALLOCATION_INFO info = s_fsr.device->GetResourceAllocationInfo(0, 1, &rd);

	while (glGetError() != GL_NO_ERROR) {}
	s_glx.CreateMemoryObjects(1, &tex.glMemory);
	const GLint dedicatedFlag = GL_TRUE;
	s_glx.MemoryObjectParameteriv(tex.glMemory, kGL_DEDICATED_MEMORY_OBJECT_EXT, &dedicatedFlag);
	s_glx.ImportMemoryWin32Handle(tex.glMemory, info.SizeInBytes, kGL_HANDLE_TYPE_D3D12_RESOURCE_EXT, tex.handle);

	GLint prevTex = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex);
	glGenTextures(1, &tex.glTexture);
	glBindTexture(GL_TEXTURE_2D, tex.glTexture);
	s_glx.TexStorageMem2D(GL_TEXTURE_2D, 1, glFormat, width, height, tex.glMemory, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prevTex));

	const GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		FsrLog("OpenGL import of a D3D12 texture failed (0x%x)", err);
		return false;
	}
	return true;
}

// Blit with a vertical flip between two GL textures.
void BlitFlip(GLuint srcTex, int srcW, int srcH, GLuint dstTex, int dstW, int dstH)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, s_fsr.blitReadFbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTex, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_fsr.blitDrawFbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTex, 0);
	glBlitFramebuffer(0, 0, srcW, srcH, 0, dstH, dstW, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void DestroyContext()
{
	if (s_fsr.context != nullptr && s_fsr.DestroyContext != nullptr)
	{
		WaitDxIdle();
		s_fsr.DestroyContext(&s_fsr.context, nullptr);
	}
	s_fsr.context = nullptr;
	s_fsr.contextWidth = s_fsr.contextHeight = 0;
}

void DestroySizedSdkResources()
{
	WaitDxIdle();
	glFinish();
	DestroySharedTex(s_fsr.color);
	DestroySharedTex(s_fsr.depth);
	DestroySharedTex(s_fsr.motion);
	DestroySharedTex(s_fsr.output);
	s_fsr.sizedRenderWidth = s_fsr.sizedRenderHeight = 0;
	s_fsr.sizedOutputWidth = s_fsr.sizedOutputHeight = 0;
}

void ShutdownSdk()
{
	DestroyContext();
	if (s_fsr.device != nullptr)
		DestroySizedSdkResources();

	if (s_fsr.prepProgram != 0) glDeleteProgram(s_fsr.prepProgram);
	if (s_fsr.prepVao != 0) glDeleteVertexArrays(1, &s_fsr.prepVao);
	if (s_fsr.prepFbo != 0) glDeleteFramebuffers(1, &s_fsr.prepFbo);
	if (s_fsr.blitReadFbo != 0) glDeleteFramebuffers(1, &s_fsr.blitReadFbo);
	if (s_fsr.blitDrawFbo != 0) glDeleteFramebuffers(1, &s_fsr.blitDrawFbo);
	s_fsr.prepProgram = s_fsr.prepVao = s_fsr.prepFbo = s_fsr.blitReadFbo = s_fsr.blitDrawFbo = 0;

	if (s_fsr.glFence != 0 && s_glx.DeleteSemaphores != nullptr)
		s_glx.DeleteSemaphores(1, &s_fsr.glFence);
	s_fsr.glFence = 0;

	for (DXSLOT& slot : s_fsr.slots)
	{
		SafeRelease(slot.list);
		SafeRelease(slot.alloc);
		slot.fenceValue = 0;
	}
	SafeRelease(s_fsr.fence);
	SafeRelease(s_fsr.queue);
	SafeRelease(s_fsr.device);
	if (s_fsr.hFence != nullptr) CloseHandle(s_fsr.hFence);
	if (s_fsr.hFenceEvent != nullptr) CloseHandle(s_fsr.hFenceEvent);
	s_fsr.hFence = s_fsr.hFenceEvent = nullptr;
	s_fsr.fenceValue = 0;

	s_fsr.CreateContext = nullptr;
	s_fsr.DestroyContext = nullptr;
	s_fsr.Configure = nullptr;
	s_fsr.Query = nullptr;
	s_fsr.Dispatch = nullptr;
	// The DLLs stay loaded (unloading D3D12 / the loader mid-run is not worth it).
}

void FfxMessageCallback(uint32_t type, const wchar_t* message)
{
	char text[512] = {};
	if (message != nullptr)
		WideCharToMultiByte(CP_UTF8, 0, message, -1, text, sizeof(text), nullptr, nullptr);
	FsrLog("%s: %s", type == FFX_API_MESSAGE_TYPE_ERROR ? "error" : "warning", text);
}

bool InitializeSdk()
{
	if (!LoadGlInterop())
		return false;

	// --- AMD FSR SDK loader -------------------------------------------------------
	const std::filesystem::path loaderPath = ExeDirectory() / L"amd_fidelityfx_loader_dx12.dll";
	if (!std::filesystem::exists(loaderPath))
		return SdkFail("amd_fidelityfx_loader_dx12.dll not found next to the executable");
	if (!std::filesystem::exists(ExeDirectory() / L"amd_fidelityfx_upscaler_dx12.dll"))
		return SdkFail("amd_fidelityfx_upscaler_dx12.dll not found next to the executable");
	if (s_fsr.hmodLoader == nullptr)
		s_fsr.hmodLoader = LoadLibraryW(loaderPath.c_str());
	if (s_fsr.hmodLoader == nullptr)
		return SdkFail("could not load amd_fidelityfx_loader_dx12.dll");
	s_fsr.CreateContext = reinterpret_cast<PfnFfxCreateContext>(GetProcAddress(s_fsr.hmodLoader, "ffxCreateContext"));
	s_fsr.DestroyContext = reinterpret_cast<PfnFfxDestroyContext>(GetProcAddress(s_fsr.hmodLoader, "ffxDestroyContext"));
	s_fsr.Configure = reinterpret_cast<PfnFfxConfigure>(GetProcAddress(s_fsr.hmodLoader, "ffxConfigure"));
	s_fsr.Query = reinterpret_cast<PfnFfxQuery>(GetProcAddress(s_fsr.hmodLoader, "ffxQuery"));
	s_fsr.Dispatch = reinterpret_cast<PfnFfxDispatch>(GetProcAddress(s_fsr.hmodLoader, "ffxDispatch"));
	if (s_fsr.CreateContext == nullptr || s_fsr.DestroyContext == nullptr || s_fsr.Query == nullptr || s_fsr.Dispatch == nullptr)
		return SdkFail("amd_fidelityfx_loader_dx12.dll is missing entry points");

	// --- D3D12 device on the OpenGL GPU -------------------------------------------
	if (s_fsr.hmodD3D12 == nullptr) s_fsr.hmodD3D12 = LoadLibraryW(L"d3d12.dll");
	if (s_fsr.hmodDxgi == nullptr) s_fsr.hmodDxgi = LoadLibraryW(L"dxgi.dll");
	if (s_fsr.hmodD3D12 == nullptr || s_fsr.hmodDxgi == nullptr)
		return SdkFail("D3D12 runtime not available");

	auto pfnCreateDevice = reinterpret_cast<PFN_D3D12_CREATE_DEVICE>(GetProcAddress(s_fsr.hmodD3D12, "D3D12CreateDevice"));
	using PFN_CreateDXGIFactory2 = HRESULT(WINAPI*)(UINT, REFIID, void**);
	auto pfnCreateFactory = reinterpret_cast<PFN_CreateDXGIFactory2>(GetProcAddress(s_fsr.hmodDxgi, "CreateDXGIFactory2"));
	if (pfnCreateDevice == nullptr || pfnCreateFactory == nullptr)
		return SdkFail("D3D12 entry points missing");

	IDXGIFactory4* factory = nullptr;
	if (FAILED(pfnCreateFactory(0, __uuidof(IDXGIFactory4), reinterpret_cast<void**>(&factory))))
		return SdkFail("CreateDXGIFactory2 failed");

	GLubyte luidBytes[8]{};
	s_glx.GetUnsignedBytev(kGL_DEVICE_LUID_EXT, luidBytes);
	LUID luid{};
	std::memcpy(&luid, luidBytes, sizeof(luid));

	IDXGIAdapter1* adapter = nullptr;
	const HRESULT hrAdapter = factory->EnumAdapterByLuid(luid, __uuidof(IDXGIAdapter1), reinterpret_cast<void**>(&adapter));
	factory->Release();
	if (FAILED(hrAdapter) || adapter == nullptr)
		return SdkFail("could not find the OpenGL GPU in DXGI (hybrid graphics?)");

	DXGI_ADAPTER_DESC1 desc{};
	adapter->GetDesc1(&desc);
	const HRESULT hrDevice = pfnCreateDevice(adapter, D3D_FEATURE_LEVEL_12_0,
		__uuidof(ID3D12Device), reinterpret_cast<void**>(&s_fsr.device));
	adapter->Release();
	if (FAILED(hrDevice))
		return SdkFail("D3D12CreateDevice failed");

	char name[128]{};
	WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, name, sizeof(name), nullptr, nullptr);
	FsrLog("D3D12 device: %s", name);

	D3D12_COMMAND_QUEUE_DESC qd{};
	qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (FAILED(s_fsr.device->CreateCommandQueue(&qd, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&s_fsr.queue))))
		return SdkFail("CreateCommandQueue failed");

	for (DXSLOT& slot : s_fsr.slots)
	{
		if (FAILED(s_fsr.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&slot.alloc))) ||
			FAILED(s_fsr.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, slot.alloc, nullptr, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&slot.list))))
			return SdkFail("D3D12 command list creation failed");
		slot.list->Close();
	}

	// One fence shared with OpenGL orders all GL <-> D3D12 work.
	if (FAILED(s_fsr.device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, __uuidof(ID3D12Fence), reinterpret_cast<void**>(&s_fsr.fence))) ||
		FAILED(s_fsr.device->CreateSharedHandle(s_fsr.fence, nullptr, GENERIC_ALL, nullptr, &s_fsr.hFence)))
		return SdkFail("shared D3D12 fence creation failed");
	s_fsr.hFenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

	while (glGetError() != GL_NO_ERROR) {}
	s_glx.GenSemaphores(1, &s_fsr.glFence);
	s_glx.ImportSemaphoreWin32Handle(s_fsr.glFence, kGL_HANDLE_TYPE_D3D12_FENCE_EXT, s_fsr.hFence);
	if (glGetError() != GL_NO_ERROR)
		return SdkFail("OpenGL could not import the D3D12 fence");

	// --- GL objects -------------------------------------------------------------
	const char* prep[] = { s_szPrepFrag };
	s_fsr.prepProgram = LinkProgram(prep, 1, "FSR prep");
	if (s_fsr.prepProgram == 0)
		return SdkFail("motion vector shader failed to compile");
	s_fsr.uClipToPrevClip = glGetUniformLocation(s_fsr.prepProgram, "uClipToPrevClip");
	s_fsr.uJitterNdc = glGetUniformLocation(s_fsr.prepProgram, "uJitterNdc");
	s_fsr.uRenderSize = glGetUniformLocation(s_fsr.prepProgram, "uRenderSize");
	glGenVertexArrays(1, &s_fsr.prepVao);
	glGenFramebuffers(1, &s_fsr.prepFbo);
	glGenFramebuffers(1, &s_fsr.blitReadFbo);
	glGenFramebuffers(1, &s_fsr.blitDrawFbo);

	// Prove the texture import works before committing to this backend.
	SHAREDTEX probe;
	const bool fProbe = CreateSharedTex(probe, 16, 16, DXGI_FORMAT_R8G8B8A8_UNORM, GL_RGBA8);
	DestroySharedTex(probe);
	if (!fProbe)
		return SdkFail("OpenGL could not import D3D12 textures");

	QueryPerformanceCounter(&s_fsr.tPrev);
	FsrLog("AMD FSR SDK loaded (D3D12).");
	return true;
}

bool FEnsureSizedSdkResources(int renderWidth, int renderHeight, int outputWidth, int outputHeight)
{
	if (s_fsr.color.res != nullptr &&
		s_fsr.sizedRenderWidth == renderWidth && s_fsr.sizedRenderHeight == renderHeight &&
		s_fsr.sizedOutputWidth == outputWidth && s_fsr.sizedOutputHeight == outputHeight)
		return true;

	DestroySizedSdkResources();
	bool ok = CreateSharedTex(s_fsr.color, renderWidth, renderHeight, DXGI_FORMAT_R8G8B8A8_UNORM, GL_RGBA8);
	ok = ok && CreateSharedTex(s_fsr.depth, renderWidth, renderHeight, DXGI_FORMAT_R32_FLOAT, GL_R32F);
	ok = ok && CreateSharedTex(s_fsr.motion, renderWidth, renderHeight, DXGI_FORMAT_R16G16_FLOAT, GL_RG16F);
	ok = ok && CreateSharedTex(s_fsr.output, outputWidth, outputHeight, DXGI_FORMAT_R8G8B8A8_UNORM, GL_RGBA8);
	if (ok)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, s_fsr.prepFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_fsr.depth.glTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, s_fsr.motion.glTexture, 0);
		const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, buffers);
		ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	if (!ok)
	{
		FsrLog("Could not create %dx%d -> %dx%d shared textures", renderWidth, renderHeight, outputWidth, outputHeight);
		DestroySizedSdkResources();
		return false;
	}
	s_fsr.sizedRenderWidth = renderWidth;
	s_fsr.sizedRenderHeight = renderHeight;
	s_fsr.sizedOutputWidth = outputWidth;
	s_fsr.sizedOutputHeight = outputHeight;
	s_fsr.fReset = true;
	return true;
}

void UpdateVersionName()
{
	ffxQueryGetProviderVersion query{};
	query.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;
	if (s_fsr.Query(&s_fsr.context, &query.header) != FFX_API_RETURN_OK || query.versionName == nullptr)
	{
		std::snprintf(s_fsr.version, sizeof(s_fsr.version), "FSR");
		return;
	}

	FsrLog("Upscaler provider: %s", query.versionName);
	// Show "FSR <major>.<minor>" (or "FSR 4" for 4.0.x).
	int major = 0;
	int minor = 0;
	const char* p = query.versionName;
	while (*p != '\0' && (*p < '0' || *p > '9'))
		++p;
	char* end = nullptr;
	major = static_cast<int>(std::strtol(p, &end, 10));
	if (end != nullptr && *end == '.')
		minor = static_cast<int>(std::strtol(end + 1, nullptr, 10));
	if (major > 0)
	{
		if (minor > 0)
			std::snprintf(s_fsr.version, sizeof(s_fsr.version), "FSR %d.%d", major, minor);
		else
			std::snprintf(s_fsr.version, sizeof(s_fsr.version), "FSR %d", major);
	}
	else
	{
		std::snprintf(s_fsr.version, sizeof(s_fsr.version), "%s", query.versionName);
	}
}

bool FEnsureContext(int outputWidth, int outputHeight)
{
	if (s_fsr.context != nullptr && s_fsr.contextWidth == outputWidth && s_fsr.contextHeight == outputHeight)
		return true;
	if (s_fsr.fContextFailed)
		return false;
	DestroyContext();

	ffxCreateContextDescUpscaleVersion version{};
	version.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE_VERSION;
	version.header.pNext = nullptr;
	version.version = FFX_UPSCALER_VERSION;

	ffxCreateBackendDX12Desc backend{};
	backend.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
	backend.header.pNext = &version.header;
	backend.device = s_fsr.device;

	ffxCreateContextDescUpscale create{};
	create.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
	create.header.pNext = &backend.header;
	// LDR, gamma-encoded RGBA8 scene, standard (non-inverted, finite) depth.
	create.flags = FFX_UPSCALE_ENABLE_NON_LINEAR_COLORSPACE;
	// Max render size = output size so every quality mode (incl. Native AA)
	// works without recreating the context.
	create.maxRenderSize.width = static_cast<uint32_t>(outputWidth);
	create.maxRenderSize.height = static_cast<uint32_t>(outputHeight);
	create.maxUpscaleSize.width = static_cast<uint32_t>(outputWidth);
	create.maxUpscaleSize.height = static_cast<uint32_t>(outputHeight);
	create.fpMessage = FfxMessageCallback;

	const ffxReturnCode_t rc = s_fsr.CreateContext(&s_fsr.context, &create.header, nullptr);
	if (rc != FFX_API_RETURN_OK || s_fsr.context == nullptr)
	{
		s_fsr.context = nullptr;
		s_fsr.fContextFailed = true;
		FsrLog("ffxCreateContext(upscale) failed (%u); using FSR 1", static_cast<unsigned>(rc));
		return false;
	}
	s_fsr.contextWidth = outputWidth;
	s_fsr.contextHeight = outputHeight;
	s_fsr.fReset = true;
	UpdateVersionName();
	return true;
}

D3D12_RESOURCE_BARRIER Transition(ID3D12Resource* res, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER b{};
	b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	b.Transition.pResource = res;
	b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	b.Transition.StateBefore = before;
	b.Transition.StateAfter = after;
	return b;
}

bool FDetectCameraCut(const DLSSCAMERA& cam)
{
	if (!s_fsr.fHavePrevCamera)
		return true;

	const glm::mat4 view = glm::inverse(cam.matProj) * cam.matWorldToClip;
	const glm::mat4 eye = glm::inverse(view);
	const glm::vec3 forward = -glm::normalize(glm::vec3(eye[2]));
	if (glm::dot(forward, s_fsr.dirPrevForward) < 0.766f)
		return true;

	const glm::mat4 clipToPrev = s_fsr.matPrevWorldToClip * glm::inverse(cam.matWorldToClip);
	const glm::vec4 prev = clipToPrev * glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);
	if (prev.w <= 0.0f)
		return true;
	const glm::vec2 prevNdc = glm::vec2(prev) / prev.w;
	return glm::length(prevNdc) > 1.5f;
}

void RememberCamera(const DLSSCAMERA& cam)
{
	const glm::mat4 view = glm::inverse(cam.matProj) * cam.matWorldToClip;
	const glm::mat4 eye = glm::inverse(view);
	s_fsr.dirPrevForward = -glm::normalize(glm::vec3(eye[2]));
	s_fsr.matPrevWorldToClip = cam.matWorldToClip;
	s_fsr.fHavePrevCamera = true;
}

bool FEvaluateSdk(const DLSSCAMERA& cam, GLuint sceneColorTex, GLuint sceneDepthTex,
	int rw, int rh, int ow, int oh)
{
	if (!FEnsureTargets(ow, oh))
		return false;
	if (!FEnsureContext(ow, oh) || !FEnsureSizedSdkResources(rw, rh, ow, oh))
	{
		s_fsr.fContextFailed = true;   // caller falls back to FSR 1
		return false;
	}

	if (FDetectCameraCut(cam))
		s_fsr.fReset = true;

	FSRGLSTATE state;
	state.Save();

	// --- 1. Scene color -> shared (top-down) -------------------------------------
	BlitFlip(sceneColorTex, rw, rh, s_fsr.color.glTexture, rw, rh);

	// --- 2. Depth + camera motion vectors ----------------------------------------
	const glm::mat4 prevWorldToClip = s_fsr.fHavePrevCamera ? s_fsr.matPrevWorldToClip : cam.matWorldToClip;
	const glm::mat4 clipToPrevClip = prevWorldToClip * glm::inverse(cam.matWorldToClip);

	glBindFramebuffer(GL_FRAMEBUFFER, s_fsr.prepFbo);
	glViewport(0, 0, rw, rh);
	glUseProgram(s_fsr.prepProgram);
	glUniformMatrix4fv(s_fsr.uClipToPrevClip, 1, GL_FALSE, &clipToPrevClip[0][0]);
	glUniform2f(s_fsr.uJitterNdc, s_fsr.jitterNdc.x, s_fsr.jitterNdc.y);
	glUniform2f(s_fsr.uRenderSize, static_cast<float>(rw), static_cast<float>(rh));
	glActiveTexture(GL_TEXTURE0);
	glBindSampler(0, 0);
	glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
	glBindVertexArray(s_fsr.prepVao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	const GLuint inputs[3] = { s_fsr.color.glTexture, s_fsr.depth.glTexture, s_fsr.motion.glTexture };
	GlSignalDx(inputs, 3);

	// --- 3. FSR on the D3D12 queue -------------------------------------------------
	DXSLOT& slot = s_fsr.slots[s_fsr.frameIndex % kDxSlots];
	WaitDxCpu(slot.fenceValue);
	slot.alloc->Reset();
	slot.list->Reset(slot.alloc, nullptr);

	const D3D12_RESOURCE_STATES readState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	D3D12_RESOURCE_BARRIER before[4] =
	{
		Transition(s_fsr.color.res, D3D12_RESOURCE_STATE_COMMON, readState),
		Transition(s_fsr.depth.res, D3D12_RESOURCE_STATE_COMMON, readState),
		Transition(s_fsr.motion.res, D3D12_RESOURCE_STATE_COMMON, readState),
		Transition(s_fsr.output.res, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
	};
	slot.list->ResourceBarrier(4, before);

	LARGE_INTEGER now{};
	LARGE_INTEGER freq{};
	QueryPerformanceCounter(&now);
	QueryPerformanceFrequency(&freq);
	float frameMs = static_cast<float>(static_cast<double>(now.QuadPart - s_fsr.tPrev.QuadPart) * 1000.0 / static_cast<double>(freq.QuadPart));
	s_fsr.tPrev = now;
	frameMs = std::clamp(frameMs, 0.1f, 100.0f);

	ffxDispatchDescUpscale dispatch{};
	dispatch.header.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE;
	dispatch.header.pNext = nullptr;
	dispatch.commandList = slot.list;
	dispatch.color = ffxApiGetResourceDX12(s_fsr.color.res, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	dispatch.depth = ffxApiGetResourceDX12(s_fsr.depth.res, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	dispatch.motionVectors = ffxApiGetResourceDX12(s_fsr.motion.res, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	dispatch.exposure = ffxApiGetResourceDX12(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	dispatch.reactive = ffxApiGetResourceDX12(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	dispatch.transparencyAndComposition = ffxApiGetResourceDX12(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	dispatch.output = ffxApiGetResourceDX12(s_fsr.output.res, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
	dispatch.jitterOffset.x = s_fsr.jitterX;
	dispatch.jitterOffset.y = s_fsr.jitterY;
	dispatch.motionVectorScale.x = static_cast<float>(rw);
	dispatch.motionVectorScale.y = static_cast<float>(rh);
	dispatch.renderSize.width = static_cast<uint32_t>(rw);
	dispatch.renderSize.height = static_cast<uint32_t>(rh);
	dispatch.upscaleSize.width = static_cast<uint32_t>(ow);
	dispatch.upscaleSize.height = static_cast<uint32_t>(oh);
	dispatch.enableSharpening = s_aSharpness[s_fsr.sharpness].sdk > 0.0f;
	dispatch.sharpness = s_aSharpness[s_fsr.sharpness].sdk;
	dispatch.frameTimeDelta = frameMs;
	dispatch.preExposure = 1.0f;
	dispatch.reset = s_fsr.fReset;
	dispatch.cameraNear = cam.sNearClip;
	dispatch.cameraFar = cam.sFarClip;
	dispatch.cameraFovAngleVertical = cam.radFov;
	dispatch.viewSpaceToMetersFactor = 1.0f;
	dispatch.flags = FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_SRGB;

	const ffxReturnCode_t rc = s_fsr.Dispatch(&s_fsr.context, &dispatch.header);

	D3D12_RESOURCE_BARRIER after[4] =
	{
		Transition(s_fsr.color.res, readState, D3D12_RESOURCE_STATE_COMMON),
		Transition(s_fsr.depth.res, readState, D3D12_RESOURCE_STATE_COMMON),
		Transition(s_fsr.motion.res, readState, D3D12_RESOURCE_STATE_COMMON),
		Transition(s_fsr.output.res, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON),
	};
	slot.list->ResourceBarrier(4, after);
	slot.list->Close();
	ID3D12CommandList* lists[] = { slot.list };
	s_fsr.queue->ExecuteCommandLists(1, lists);

	const GLuint outputs[1] = { s_fsr.output.glTexture };
	slot.fenceValue = DxSignalGl(outputs, 1);

	if (rc != FFX_API_RETURN_OK)
	{
		static int s_cErrors = 0;
		if (s_cErrors++ < 5)
			FsrLog("ffxDispatch(upscale) failed (%u)", static_cast<unsigned>(rc));
		state.RestoreAndBindOutput(ow, oh);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return false;
	}

	s_fsr.fReset = false;
	RememberCamera(cam);

	// --- 4. Output (top-down) -> GL output target for the HUD -------------------------
	BlitFlip(s_fsr.output.glTexture, ow, oh, s_fsr.outTex, ow, oh);
	// TV portraits (own camera, screen-fixed): show them as rendered.
	BlitDlssStaticRects(sceneColorTex, rw, rh, s_fsr.outFbo, ow, oh);
	state.RestoreAndBindOutput(ow, oh);
	return true;
}

// Picks the backend the first time FSR is needed.
void EnsureBackend()
{
	if (s_fsr.backend != FSRB_None)
		return;
	if (InitializeSdk())
	{
		s_fsr.backend = FSRB_Sdk;
		std::snprintf(s_fsr.version, sizeof(s_fsr.version), "FSR");
	}
	else
	{
		ShutdownSdk();
		s_fsr.backend = FSRB_Fsr1;
		std::snprintf(s_fsr.version, sizeof(s_fsr.version), "FSR 1");
	}
}

void FallBackToFsr1()
{
	ShutdownSdk();
	s_fsr.backend = FSRB_Fsr1;
	s_fsr.fJitterThisFrame = false;
	std::snprintf(s_fsr.version, sizeof(s_fsr.version), "FSR 1");
	// Same render scale; MSAA comes back next frame (main.cpp asks FFsrTemporal()).
}
} // namespace

// =============================================================================
// Public API
// =============================================================================
void StartupFsr()
{
	LoadFsrSettings();
	if (s_fsr.fsrq != FSRQ_Off)
		EnsureBackend();
}

void ShutdownFsr()
{
	ShutdownSdk();
	DestroyTargets();
	ShutdownFsr1();
	s_fsr.backend = FSRB_None;
}

bool FFsrActive()
{
	if (s_fsr.fsrq == FSRQ_Off || FDlssActive())
		return false;
	if (s_fsr.backend == FSRB_Fsr1 && s_fsr.fFsr1Failed)
		return false;
	return true;
}

bool FFsrTemporal()
{
	return FFsrActive() && s_fsr.backend == FSRB_Sdk;
}

FSRQ FsrQuality()
{
	return s_fsr.fsrq;
}

void SetFsrQuality(FSRQ fsrq)
{
	if (fsrq < FSRQ_Off || fsrq >= FSRQ_Max)
		fsrq = FSRQ_Off;
	s_fsr.fsrq = fsrq;
	s_fsr.fReset = true;
	if (fsrq != FSRQ_Off)
		EnsureBackend();
	SaveFsrSettings();
	ApplyInternalResolutionSettings();
}

void SelectNextFsrQuality()
{
	FSRQ next = static_cast<FSRQ>((s_fsr.fsrq + 1) % FSRQ_Max);
	// FSR 1 cannot do anti-aliasing on its own: skip Native AA there.
	if (next == FSRQ_NativeAA && s_fsr.backend == FSRB_Fsr1)
		next = FSRQ_Quality;
	// FSR and DLSS are alternatives: turning FSR on turns DLSS off.
	if (next != FSRQ_Off && FDlssActive())
		SetDlssQuality(DLSSQ_Off);
	SetFsrQuality(next);
}

const char* PchzFsrQuality()
{
	if (s_fsr.backend == FSRB_Fsr1 && s_fsr.fFsr1Failed)
		return "Unavailable";
	return s_apchzQuality[s_fsr.fsrq];
}

const char* PchzFsrVersion()
{
	return s_fsr.version;
}

void SelectNextFsrSharpness()
{
	s_fsr.sharpness = (s_fsr.sharpness + 1) % kSharpnessCount;
	SaveFsrSettings();
}

const char* PchzFsrSharpness()
{
	return s_aSharpness[s_fsr.sharpness].name;
}

bool FFsrRenderSize(int outputWidth, int outputHeight, int* pRenderWidth, int* pRenderHeight)
{
	if (!FFsrActive())
		return false;
	const float scale = s_aScale[s_fsr.fsrq];
	*pRenderWidth = (std::max)(1, static_cast<int>(std::lround(outputWidth / scale)));
	*pRenderHeight = (std::max)(1, static_cast<int>(std::lround(outputHeight / scale)));
	return true;
}

void BeginFsrFrame(bool fUseFsr, int renderWidth, int renderHeight, int outputWidth, int outputHeight)
{
	++s_fsr.frameIndex;
	s_fsr.fJitterThisFrame = fUseFsr && FFsrTemporal() && renderWidth > 0 && renderHeight > 0;
	if (!s_fsr.fJitterThisFrame)
	{
		s_fsr.jitterX = s_fsr.jitterY = 0.0f;
		s_fsr.jitterNdc = glm::vec2(0.0f);
		return;
	}

	// FSR's sequence: Halton(2,3), 8 * ratio^2 phases.
	const float ratio = static_cast<float>((std::max)(outputWidth, 1)) / static_cast<float>(renderWidth);
	const uint32_t phases = (std::max)(1u, static_cast<uint32_t>(std::ceil(8.0f * ratio * ratio)));
	++s_fsr.jitterIndex;
	const uint32_t index = (s_fsr.jitterIndex % phases) + 1;
	s_fsr.jitterX = Halton(index, 2) - 0.5f;
	s_fsr.jitterY = Halton(index, 3) - 0.5f;

	// FSR jitter is in pixels with +y down; in GL clip space that is
	// (+2x/width, -2y/height).
	s_fsr.jitterNdc = glm::vec2(2.0f * s_fsr.jitterX / static_cast<float>(renderWidth),
		-2.0f * s_fsr.jitterY / static_cast<float>(renderHeight));
	SetDlssExternalJitter(s_fsr.jitterNdc.x, s_fsr.jitterNdc.y);
}

bool FEvaluateFsr(const DLSSCAMERA& cam, GLuint sceneColorTex, GLuint sceneDepthTex,
	int renderWidth, int renderHeight, int outputWidth, int outputHeight)
{
	if (!FFsrActive() || sceneColorTex == 0 || renderWidth <= 0 || renderHeight <= 0 ||
		outputWidth < 64 || outputHeight < 64)
		return false;

	EnsureBackend();
	if (s_fsr.backend == FSRB_Sdk)
	{
		if (s_fsr.fJitterThisFrame && FEvaluateSdk(cam, sceneColorTex, sceneDepthTex,
			renderWidth, renderHeight, outputWidth, outputHeight))
		{
			// The HUD must see the scene's depth/stencil (binocucom TV portraits).
			BlitSceneDepthStencil(sceneDepthTex, renderWidth, renderHeight, s_fsr.outFbo, outputWidth, outputHeight);
			return true;
		}
		if (s_fsr.fContextFailed)
			FallBackToFsr1();
		// This frame was rendered for the temporal path (jittered, no MSAA):
		// show it un-upscaled; FSR 1 takes over next frame after a fallback.
		return false;
	}
	if (!FEvaluateFsr1(sceneColorTex, renderWidth, renderHeight, outputWidth, outputHeight))
		return false;
	BlitSceneDepthStencil(sceneDepthTex, renderWidth, renderHeight, s_fsr.outFbo, outputWidth, outputHeight);
	return true;
}

GLuint FsrOutputTexture()
{
	return s_fsr.outTex;
}

void ResetFsrHistory()
{
	s_fsr.fReset = true;
}

// Options > Reset to Default.
void ResetFsrSettings()
{
	s_fsr.sharpness = 2;   // Medium
	SetFsrQuality(FSRQ_Off);
}

// Settings import: re-read FSR.ini and apply it.
void ReloadFsrSettings()
{
	LoadFsrSettings();
	SetFsrQuality(s_fsr.fsrq);
}

// Changed-setting markers: 0 = FSR mode, 1 = sharpness.
bool FFsrOptionDefault(int which)
{
	return which == 0 ? s_fsr.fsrq == FSRQ_Off : s_fsr.sharpness == 2;
}

void ResetFsrOption(int which)
{
	if (which == 0)
		SetFsrQuality(FSRQ_Off);
	else
	{
		s_fsr.sharpness = 2;
		SaveFsrSettings();
	}
}
