# Portable Linux release

The normal `linux-release` preset builds against the libraries installed on
the current Linux machine. That is useful for development, but the resulting
binary is not suitable for distribution: its glibc and FFmpeg requirements
may be newer than those available on SteamOS or another tester's system.

The portable build uses Ubuntu 22.04 (glibc 2.35) as its compatibility
baseline and packages the game plus its non-system shared libraries into an
AppImage.

## Requirements

- Docker Desktop with Linux containers, or Docker Engine on Linux
- Internet access while building dependencies and downloading `linuxdeploy`

## Build

From the repository root:

```bash
docker build -t projectcane-linux-builder packaging/linux
docker run --rm \
  -v "$PWD:/src:ro" \
  -v "$PWD/dist:/out" \
  projectcane-linux-builder
```

PowerShell:

```powershell
docker build -t projectcane-linux-builder packaging/linux
New-Item -ItemType Directory -Force dist | Out-Null
docker run --rm `
  --mount "type=bind,source=$((Get-Location).Path),target=/src,readonly" `
  --mount "type=bind,source=$((Get-Location).Path)\dist,target=/out" `
  projectcane-linux-builder
```

The result is `dist/ProjectCane-x86_64.AppImage`.

## Running

Keep the AppImage in the folder where the extracted `Worlds` and `Cutscenes`
directories are stored, then run:

```bash
chmod +x ProjectCane-x86_64.AppImage
./ProjectCane-x86_64.AppImage
```

Do not bundle glibc, the Linux loader, OpenGL/Vulkan drivers, or display-server
libraries manually. Those components must come from the target system.
