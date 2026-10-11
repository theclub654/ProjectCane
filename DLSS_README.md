# NVIDIA DLSS in ProjectCane

ProjectCane renders with OpenGL. DLSS only runs on D3D11/D3D12/Vulkan, so
`Sly1-Release/dlss.cpp` creates a small Vulkan device on the same GPU through
NVIDIA Streamline and shares the frame with OpenGL:

```
OpenGL scene (lower render res, sub-pixel jittered)
   -> shared color / depth / motion vectors      (GL_EXT_memory_object_win32)
Vulkan + Streamline DLSS Super Resolution / DLAA
   -> shared output at window resolution          (GL_EXT_semaphore_win32 sync)
OpenGL HUD drawn on top at full resolution -> present
```

If anything is missing (no RTX GPU, old driver, DLLs not next to the exe),
DLSS shows as "Unavailable" and the game runs exactly as before.

## Requirements
- NVIDIA RTX GPU (20 series or newer) and a current Game Ready driver
- Windows 10 20H1 or newer
- Streamline 2.14.1 runtime next to the exe (copied by the post-build step):
  `sl.interposer.dll`, `sl.common.dll`, `sl.dlss.dll`, `nvngx_dlss.dll`

## Using it
- In game: **Options > Video > NVIDIA DLSS** cycles
  Off, DLAA, Quality, Balanced, Performance, Ultra Performance.
- The setting is saved in `DLSS.ini` next to the exe.
- While DLSS is on, MSAA and Internal Resolution are ignored (DLSS does both jobs).
- Developer menu (`Debug Mode.ini` = 1, press F3): **DLSS** menu with mode, model
  preset (J/K/L/M), jitter toggles and status.

## DLSS.ini
```
[DLSS]
Mode=Quality              ; Off, DLAA, Quality, Balanced, Performance, UltraPerformance
Preset=0                  ; 0 = NVIDIA default, 10..13 = J, K, L, M
ShowStreamlineConsole=0   ; 1 opens Streamline's log console
VerboseLog=0
FlipJitterX=0             ; debugging aids if the image looks soft/shimmery
FlipJitterY=0
```
Streamline writes `sl.log` next to the exe.

## Files changed
| File | Change |
|---|---|
| `dlss.cpp/.h` (new) | Streamline loading, Vulkan bridge, GL interop, jitter, motion vectors, DLSS evaluate, menu |
| `gl.h/.cpp` | Scene depth is now a texture (`fbd`); render size comes from DLSS when active |
| `main.cpp` | Jitter per frame, DLSS between the 3D scene and the HUD, present the upscaled image, startup/shutdown |
| `render.cpp`, `tv.cpp` | Camera UBO uploads go through `UploadCmUbo` (adds jitter); TV portraits marked screen-fixed |
| `font.cpp` | Text scissor uses the HUD target size |
| `prompt.cpp`, `resp.h` | "NVIDIA DLSS" entry in the Video menu |
| `debug.cpp` | DLSS menu in the developer menu bar |
| `Sly1-Release.vcxproj(.filters)` | New files, include paths, post-build DLL copy |

Originals are in `_dlss_backup/`.

## Known limitations (v1)
- Motion vectors are camera-only (reconstructed from depth). Moving characters
  can show slight ghosting. Fix: per-object motion vectors from the glob/cel
  shaders using last frame's model and bone matrices.
- Transparent particles and effects have no motion vectors of their own.
- The hidden 16x16 window exists only so Streamline's per-frame present hook
  runs (it is never shown).

## DLSS 5
NVIDIA has said DLSS 5 (neural rendering) will ship as a Streamline feature,
but the public SDK (Streamline 2.14.1) does not include it yet. The integration
point is marked "DLSS 5 (neural rendering) hook point" in `FEvaluateDlss`: the
color, depth and motion-vector inputs it needs are already tagged there.
