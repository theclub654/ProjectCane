"""Compile the offline Windows setup from an already installed CMake payload."""
from pathlib import Path
import os
import re
import shutil
import subprocess


def nsis_string(value):
    value = str(value)
    if '\n' in value or '\r' in value:
        raise ValueError('Newline in installer path')
    return value.replace('$', '$$').replace('"', '$\\"')


def build_setup(source, payload, build, output, compiler=None):
    compiler = str(compiler) if compiler else shutil.which('makensis')
    if not compiler and os.name == 'nt':
        for env in ('ProgramFiles(x86)', 'ProgramFiles'):
            candidate = Path(os.environ.get(env, '')) / 'NSIS/makensis.exe'
            if candidate.is_file():
                compiler = str(candidate)
                break
    if not compiler:
        raise RuntimeError('NSIS is required to create Windows setup. Install NSIS 3 '
                           '(Windows: choco install nsis; Ubuntu: apt install nsis), '
                           'or pass --makensis /path/to/makensis. '
                           'Use --windows-package portable to create only the ZIP.')
    cache = (build / 'CMakeCache.txt').read_text(encoding='utf-8')
    version = re.search(r'^CMAKE_PROJECT_VERSION:STATIC=(\d+\.\d+\.\d+)$', cache, re.M)
    if not version:
        raise RuntimeError('Missing three-part ProjectCane version in CMake cache')
    generated = build / 'windows-installer'
    generated.mkdir(parents=True, exist_ok=True)
    files = sorted(p for p in payload.rglob('*') if p.is_file())
    install, uninstall, checks = ['ClearErrors'], [], []
    directories = set()
    for n, file in enumerate(files):
        rel = file.relative_to(payload)
        relative = nsis_string(str(rel).replace('/', '\\'))
        parent = nsis_string(str(rel.parent).replace('/', '\\'))
        install += [f'SetOutPath "$INSTDIR\\{parent}"',
                    f'File "{nsis_string(file)}"', 'IfErrors install_failed']
        uninstall.append(f'Delete "$INSTDIR\\{relative}"')
        directories.update(p for p in rel.parents if str(p) != '.')
        if file.suffix.lower() in ('.exe', '.dll'):
            checks += [f'IfFileExists "$INSTDIR\\{relative}" 0 unlocked_{n}',
                       'ClearErrors', f'FileOpen $0 "$INSTDIR\\{relative}" a',
                       'IfErrors files_locked', 'FileClose $0', f'unlocked_{n}:']
    checks += ['Goto files_unlocked', 'files_locked:',
               'MessageBox MB_OK|MB_ICONSTOP "Close ProjectCane and the game before updating or uninstalling the launcher." /SD IDOK',
               'SetErrorLevel 2', 'Quit', 'files_unlocked:']
    uninstall.extend(f'RMDir "$INSTDIR\\{nsis_string(str(p).replace(chr(47), chr(92)))}"'
                     for p in sorted(directories, key=lambda p: (-len(p.parts), str(p))))
    defines = {}
    for key, lines in [('PAYLOAD_FILES', install), ('UNINSTALL_FILES', uninstall),
                       ('CHECK_LOCKS', checks)]:
        path = generated / (key.lower() + '.nsh')
        path.write_text('\n'.join(lines) + '\n', encoding='utf-8')
        defines[key] = nsis_string(path)
    target = output / 'ProjectCane-Windows-x86_64-Setup.exe'
    temporary = target.with_suffix('.exe.building')
    temporary.unlink(missing_ok=True)
    defines.update(VERSION=version[1], OUTPUT_EXE=nsis_string(temporary),
                   APP_ICON=nsis_string(source / 'launcher/assets/projectcane.ico'))
    prefix = '/' if os.name == 'nt' else '-'
    subprocess.run([compiler, prefix + 'V2', prefix + 'INPUTCHARSET', 'UTF8',
                    *[prefix + 'D' + key + '=' + value for key, value in defines.items()],
                    str(source / 'scripts/windows/setup.nsi')], check=True)
    temporary.replace(target)
    return target
