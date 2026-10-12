AMD FidelityFX SDK v2.3.0 (FSR Upscaling 4.1.1 / 3.1.5), from
https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK (commit 60f4ea8, 2026-06-24).

Only what ProjectCane needs for upscaling:
  api/include, upscalers/include   ffx-api headers (MIT)
  bin/x64                          AMD's signed runtime DLLs (redistributable, see license.md)
The post-build step copies bin/x64/*.dll next to Sly1-Release.exe.
