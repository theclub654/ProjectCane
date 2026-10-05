#!/usr/bin/env python3
"""Create an offline release bundle from a completed native or cross build."""
import argparse, hashlib, json, os, pathlib, shutil, subprocess, tarfile, urllib.request, zipfile
p=argparse.ArgumentParser()
p.add_argument('--build',required=True,type=pathlib.Path)
p.add_argument('--output',default='dist',type=pathlib.Path)
p.add_argument('--windows',action='store_true')
p.add_argument('--windows-package',choices=['both','portable','setup'],default='both')
p.add_argument('--makensis',type=pathlib.Path)
p.add_argument('--appimagetool',type=pathlib.Path)
p.add_argument('--appimage-runtime',type=pathlib.Path)
a=p.parse_args()
source=pathlib.Path(__file__).resolve().parent.parent
output=a.output.resolve();output.mkdir(parents=True,exist_ok=True)
name='ProjectCane-Windows-x86_64' if a.windows else 'ProjectCane-Linux-x86_64'
stage=output/name
if stage.exists():shutil.rmtree(stage)
subprocess.run(['cmake','--install',str(a.build.resolve()),'--config','Release','--prefix',str(stage)],check=True)
if a.windows:
 # LLVM/MinGW may use shared C++ support libraries. Bundle imports found in
 # the toolchain; native MSVC builds use their statically linked CRT instead.
 toolchain=os.environ.get('CANE_MINGW_ROOT')
 if toolchain:
  for folder in [pathlib.Path(toolchain)/'bin',pathlib.Path(toolchain)/'x86_64-w64-mingw32/bin']:
   for dllname in ['libc++.dll','libunwind.dll','libwinpthread-1.dll']:
    dll=folder/dllname
    if dll.exists():shutil.copy2(dll,stage/'bin'/dll.name)
 (stage/'Launch ProjectCane.cmd').write_text('@echo off\r\nstart "" "%~dp0bin\\cane-launcher.exe"\r\n')
 if a.windows_package in ['both','portable']:
  portable_name=name+'-Portable'
  temporary=output/(portable_name+'.zip.building')
  with zipfile.ZipFile(temporary,'w',zipfile.ZIP_DEFLATED) as archive:
   for file in stage.rglob('*'):
    if file.is_file():archive.write(file,pathlib.Path(portable_name)/file.relative_to(stage))
   archive.writestr(portable_name+'/portable.txt','ProjectCane portable mode. Keep this file beside the bin folder.\nUser files are stored under data/.\n')
  temporary.replace(output/(portable_name+'.zip'))
 if a.windows_package in ['both','setup']:
  from windows.package_setup import build_setup
  print(build_setup(source,stage,a.build.resolve(),output,a.makensis))
else:
 # Debian/Ubuntu copyright files cover the shared libraries collected by
 # CMake, including transitive codec dependencies. Keep exact package/source
 # versions with those notices so distributors can reproduce the bundle.
 if shutil.which('dpkg-query') and shutil.which('ldconfig'):
  available={}
  for line in subprocess.check_output(['ldconfig','-p'],text=True).splitlines():
   if ' => ' in line:
    left,right=line.split(' => ',1);available[left.strip().split()[0]]=right
  packages={}
  for library in (stage/'lib').glob('*.so*'):
   original=available.get(library.name)
   if not original:continue
   query=subprocess.run(['dpkg-query','-S',original],capture_output=True,text=True)
   if query.returncode:query=subprocess.run(['dpkg-query','-S',str(pathlib.Path(original).resolve())],capture_output=True,text=True)
   if query.returncode:continue
   package=query.stdout.split(': ')[0].strip()
   if package in packages:continue
   metadata=subprocess.check_output(['dpkg-query','-W','-f','${binary:Package}\t${Version}\t${source:Package}\t${source:Version}',package],text=True).split('\t')
   packages[package]={'binary_package':metadata[0],'version':metadata[1],'source_package':metadata[2],'source_version':metadata[3]}
   notice=pathlib.Path('/usr/share/doc')/package.split(':')[0]/'copyright'
   if notice.exists():
    destination=stage/'licenses/system'/package.replace(':','_');destination.mkdir(parents=True,exist_ok=True);shutil.copy2(notice,destination/'copyright')
  (stage/'licenses/system').mkdir(parents=True,exist_ok=True)
  (stage/'licenses/system/packages.json').write_text(json.dumps(list(packages.values()),indent=2)+'\n')
  (stage/'licenses/system/SOURCES.txt').write_text('These libraries are from the build distribution. Obtain matching source packages with apt-get source SOURCE_PACKAGE=SOURCE_VERSION, using packages.json and the distribution source repositories. Dependency build configuration is supplied by those source packages.\n')
 # The release contains executable artifacts and notices rather than SDKs.
 for folder in ['include','lib/cmake','lib/pkgconfig','share/aclocal']:
  shutil.rmtree(stage/folder,ignore_errors=True)
 for archive in (stage/'lib').glob('*.a'):archive.unlink()
 for tool in ['sdl2-config']:
  (stage/'bin'/tool).unlink(missing_ok=True)
 launcher=stage/'Launch ProjectCane.sh'
 launcher.write_text('#!/bin/sh\nROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)\nexec "$ROOT/bin/cane-launcher" "$@"\n')
 launcher.chmod(0o755)
 with tarfile.open(output/(name+'.tar.gz'),'w:gz') as archive:archive.add(stage,arcname=name)
 if a.appimagetool:
  runtime=(a.appimage_runtime or (a.build.resolve()/'packaging-runtime-x86_64')).resolve()
  runtime_hash='2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d'
  if not runtime.exists() or hashlib.sha256(runtime.read_bytes()).hexdigest()!=runtime_hash:
   if a.appimage_runtime:raise RuntimeError('The supplied AppImage runtime does not match pinned release 20251108')
   with urllib.request.urlopen('https://github.com/AppImage/type2-runtime/releases/download/20251108/runtime-x86_64') as response:payload=response.read()
   if hashlib.sha256(payload).hexdigest()!=runtime_hash:raise RuntimeError('AppImage runtime checksum mismatch')
   runtime.write_bytes(payload)
  appdir=output/'ProjectCane.AppDir'
  if appdir.exists():shutil.rmtree(appdir)
  (appdir/'usr').mkdir(parents=True)
  for item in stage.iterdir():
   if item.is_dir():shutil.copytree(item,appdir/'usr'/item.name,symlinks=True)
   else:shutil.copy2(item,appdir/'usr'/item.name)
  (appdir/'AppRun').write_text('#!/bin/sh\nROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)\nexec "$ROOT/usr/Launch ProjectCane.sh" "$@"\n')
  (appdir/'AppRun').chmod(0o755)
  shutil.copy2(stage/'share/applications/projectcane.desktop', appdir/'projectcane.desktop')
  shutil.copy2(stage/'share/icons/hicolor/256x256/apps/projectcane.png', appdir/'projectcane.png')
  temporary=output/'ProjectCane-x86_64.AppImage.building'
  temporary.unlink(missing_ok=True)
  subprocess.run([str(a.appimagetool.resolve()),'--runtime-file',str(runtime),str(appdir),str(temporary)],check=True,env={**os.environ,'ARCH':'x86_64','APPIMAGE_EXTRACT_AND_RUN':'1'})
  temporary.replace(output/'ProjectCane-x86_64.AppImage')
print(stage)
