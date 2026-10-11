// Frame profiler for the detailed FPS counter. See perf.h.

#include "perf.h"
#include "dlss.h"   // FDlssFpsCounterDetailed()

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <cstdio>

extern int g_targetFrameRate;   // clock.cpp

namespace
{
constexpr int kGpuSlots = 4;
constexpr int kMaxLines = 12;

struct GPUSLOT
{
	GLuint queries[3] = {};   // frame start, scene end, frame end
	bool fIssued = false;
	bool fScene = false;
};

struct PERFSTATE
{
	LARGE_INTEGER freq{};
	LARGE_INTEGER tZone{};
	LARGE_INTEGER tWindow{};
	bool fStarted = false;
	PERFZONE zone = PERFZ_Game;

	double zoneMs[PERFZ_Max] = {};      // this window
	int frames = 0;

	double gpuSceneMs = 0.0;
	double gpuFrameMs = 0.0;
	int gpuSamples = 0;
	GPUSLOT gpu[kGpuSlots];
	bool fGpuReady = false;
	unsigned frameIndex = 0;

	int globs = 0;
	int renderWidth = 0;
	int renderHeight = 0;

	char lines[kMaxLines][64] = {};
	int cLines = 0;
};
PERFSTATE s_perf;

double MsBetween(const LARGE_INTEGER& a, const LARGE_INTEGER& b)
{
	return static_cast<double>(b.QuadPart - a.QuadPart) * 1000.0 / static_cast<double>(s_perf.freq.QuadPart);
}

void ChargeZone(const LARGE_INTEGER& now)
{
	s_perf.zoneMs[s_perf.zone] += MsBetween(s_perf.tZone, now);
	s_perf.tZone = now;
}

void BuildLines(double windowMs)
{
	const double frames = static_cast<double>((std::max)(s_perf.frames, 1));
	double avg[PERFZ_Max];
	double busy = 0.0;
	for (int i = 0; i < PERFZ_Max; ++i)
	{
		avg[i] = s_perf.zoneMs[i] / frames;
		if (i != PERFZ_Wait)
			busy += avg[i];
	}
	const double frameMs = windowMs / frames;
	const double gpuScene = s_perf.gpuSamples > 0 ? s_perf.gpuSceneMs / s_perf.gpuSamples : -1.0;
	const double gpuFrame = s_perf.gpuSamples > 0 ? s_perf.gpuFrameMs / s_perf.gpuSamples : -1.0;

	int n = 0;
	auto Line = [&](const char* fmt, auto... args)
	{
		if (n < kMaxLines)
			std::snprintf(s_perf.lines[n++], sizeof(s_perf.lines[0]), fmt, args...);
	};

#ifdef _DEBUG
	Line("DEBUG BUILD - build Release for full speed");
#endif
	Line("Frame      %5.2f ms", frameMs);
	Line(" CPU busy  %5.2f ms", busy);
	Line("  Game     %5.2f ms", avg[PERFZ_Game]);
	Line("  Render   %5.2f ms", avg[PERFZ_Render]);
	Line("  Post/HUD %5.2f ms", avg[PERFZ_Post]);
	Line("  Present  %5.2f ms", avg[PERFZ_Present]);
	Line("  Cap wait %5.2f ms", avg[PERFZ_Wait]);
	if (gpuFrame >= 0.0)
	{
		Line(" GPU scene %5.2f ms", gpuScene);
		Line(" GPU frame %5.2f ms", gpuFrame);
	}
	if (g_targetFrameRate > 0)
		Line("%dx%d  %d globs  cap %d", s_perf.renderWidth, s_perf.renderHeight, s_perf.globs, g_targetFrameRate);
	else
		Line("%dx%d  %d globs  uncapped", s_perf.renderWidth, s_perf.renderHeight, s_perf.globs);

	// Rough bottleneck guess.
	const char* limit = "frame cap";
	if (avg[PERFZ_Wait] < 0.15 * frameMs)
	{
		if (gpuFrame > busy * 0.9)
			limit = "GPU";
		else if (avg[PERFZ_Present] > (std::max)(avg[PERFZ_Game], avg[PERFZ_Render]))
			limit = "present / GPU queue";
		else if (avg[PERFZ_Game] > avg[PERFZ_Render])
			limit = "CPU (game logic)";
		else
			limit = "CPU (draw submission)";
	}
	Line("Limited by: %s", limit);
	s_perf.cLines = n;
}

void ReadGpuSlot(GPUSLOT& slot)
{
	if (!slot.fIssued)
		return;
	GLint available = 0;
	glGetQueryObjectiv(slot.queries[2], GL_QUERY_RESULT_AVAILABLE, &available);
	if (available)
	{
		GLuint64 t[3] = {};
		for (int i = 0; i < 3; ++i)
			glGetQueryObjectui64v(slot.queries[i], GL_QUERY_RESULT, &t[i]);
		if (t[1] >= t[0] && t[2] >= t[0])
		{
			s_perf.gpuSceneMs += static_cast<double>(t[1] - t[0]) / 1.0e6;
			s_perf.gpuFrameMs += static_cast<double>(t[2] - t[0]) / 1.0e6;
			++s_perf.gpuSamples;
		}
	}
	// Never wait: a result that is still pending is simply dropped.
	slot.fIssued = false;
}
} // namespace

void PerfFrameStart()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	if (!s_perf.fStarted)
	{
		QueryPerformanceFrequency(&s_perf.freq);
		s_perf.tZone = now;
		s_perf.tWindow = now;
		s_perf.fStarted = true;
	}
	else
	{
		ChargeZone(now);
		++s_perf.frames;
		const double windowMs = MsBetween(s_perf.tWindow, now);
		if (windowMs >= 500.0)
		{
			BuildLines(windowMs);
			for (double& ms : s_perf.zoneMs)
				ms = 0.0;
			s_perf.frames = 0;
			s_perf.gpuSceneMs = s_perf.gpuFrameMs = 0.0;
			s_perf.gpuSamples = 0;
			s_perf.tWindow = now;
		}
	}
	s_perf.zone = PERFZ_Game;
	++s_perf.frameIndex;

	if (!FDlssFpsCounterDetailed())
		return;

	if (!s_perf.fGpuReady)
	{
		for (GPUSLOT& slot : s_perf.gpu)
			glGenQueries(3, slot.queries);
		s_perf.fGpuReady = true;
	}
	GPUSLOT& slot = s_perf.gpu[s_perf.frameIndex % kGpuSlots];
	ReadGpuSlot(slot);
	glQueryCounter(slot.queries[0], GL_TIMESTAMP);
	slot.fScene = false;
}

void PerfZone(PERFZONE zone)
{
	if (!s_perf.fStarted || zone == s_perf.zone)
		return;
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	ChargeZone(now);
	s_perf.zone = zone;
}

void PerfGpuSceneEnd()
{
	if (!s_perf.fGpuReady || !FDlssFpsCounterDetailed())
		return;
	GPUSLOT& slot = s_perf.gpu[s_perf.frameIndex % kGpuSlots];
	glQueryCounter(slot.queries[1], GL_TIMESTAMP);
	slot.fScene = true;
}

void PerfGpuFrameEnd()
{
	if (!s_perf.fGpuReady || !FDlssFpsCounterDetailed())
		return;
	GPUSLOT& slot = s_perf.gpu[s_perf.frameIndex % kGpuSlots];
	if (!slot.fScene)
		glQueryCounter(slot.queries[1], GL_TIMESTAMP);
	glQueryCounter(slot.queries[2], GL_TIMESTAMP);
	slot.fIssued = true;
}

void PerfSetSceneStats(int globs, int renderWidth, int renderHeight)
{
	s_perf.globs = globs;
	s_perf.renderWidth = renderWidth;
	s_perf.renderHeight = renderHeight;
}

int CPerfLines()
{
	return s_perf.cLines;
}

const char* PchzPerfLine(int line)
{
	return (line >= 0 && line < s_perf.cLines) ? s_perf.lines[line] : "";
}
