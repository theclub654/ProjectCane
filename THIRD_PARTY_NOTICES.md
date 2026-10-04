# Third-party dependencies

The source/build uses GLFW 3.4 (zlib/libpng), FAudio 25.02 (zlib), SDL2 (zlib), Dear ImGui and ImGuiFileDialog (MIT), GLAD (MIT/public domain components), GLM (MIT), stb (MIT/public domain), nlohmann/json (MIT), PicoSHA2 1.0.1 (MIT), FFmpeg, and Atkinson Hyperlegible (SIL Open Font License).

The vendored headers retain their copyright/license notices. Downloaded GLFW and FAudio releases retain their license files. The font's OFL is in `launcher/assets/OFL.txt`; PicoSHA2's license is in `platform/picosha2.h`. Windows cross builds use Microsoft.XAudio2.Redist 1.2.13 under its included Microsoft license.

Windows FFmpeg ships the LGPL shared build supplied by the existing project. Linux builds use the selected system FFmpeg and libraries; their configuration and corresponding license obligations follow that build. The packaging script collects dependent shared libraries from the build environment. Distributor builds must retain applicable notices and provide corresponding sources where their dependency licenses require them.

AppImages embed the MIT-licensed type2-runtime release 20251108, pinned by SHA-256. Its source is https://github.com/AppImage/type2-runtime/tree/dd6cebedcbddde9c82f89b011e8e1d40b6e43868 and its license is shipped in `licenses/appimage-runtime.txt`.

No original Sly Cooper assets are included in release packages.

Windows offline setup is built with NSIS. Its notices are included in `licenses/nsis/copyright`.
