#pragma once

// -----------------------------------------------------------------------------
// Lightweight frame profiler for the "Detailed" FPS counter.
//
// CPU time is split into zones (game update, 3D render submission, upscale +
// HUD, present, frame-cap wait). GPU time comes from GL timestamp queries read
// a few frames later, so measuring never stalls the pipeline. Values are
// averaged over half a second.
// -----------------------------------------------------------------------------

enum PERFZONE
{
	PERFZ_Game = 0,      // input, game logic, physics, animation
	PERFZ_Render,        // 3D scene: building + submitting draw calls
	PERFZ_Post,          // upscaler, HUD/UI, final composite, overlays
	PERFZ_Present,       // swap / DXGI present, window events
	PERFZ_Wait,          // frame limiter sleeping (time to spare)
	PERFZ_Max
};

void PerfFrameStart();               // top of every rendered frame
void PerfZone(PERFZONE zone);        // switch the zone that CPU time is charged to
void PerfGpuSceneEnd();              // after the 3D scene has been submitted
void PerfGpuFrameEnd();              // right before present
void PerfSetSceneStats(int globs, int renderWidth, int renderHeight);

int CPerfLines();
const char* PchzPerfLine(int line);
