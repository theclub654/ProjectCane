#!/usr/bin/env python3
"""Check profile locations and migration through the actual launcher entry point."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import shutil

launcher = str(Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory(prefix="cane-profile-") as temporary:
    root = Path(temporary) / "spaced path caf\u00e9"
    env = dict(os.environ)
    env.update(HOME=str(root / "home"),
               XDG_CONFIG_HOME=str(root / "config"),
               XDG_DATA_HOME=str(root / "data"),
               XDG_STATE_HOME=str(root / "state"),
               LOCALAPPDATA=str(root / "Local"),
               APPDATA=str(root / "Roaming"))
    if os.name == "nt":
        base = root / "Local" / "ProjectCane"
        saves, logs, config = base / "user/Saves", base / "logs", base / "config"
        bulk = base
    else:
        bulk = root / "data/projectcane"
        config = root / "config/projectcane"
        saves, logs = config / "Saves", config / "logs"
        old_saves, old_logs = bulk / "user/Saves", root / "state/projectcane/logs"
        old_saves.mkdir(parents=True)
        old_logs.mkdir(parents=True)
        (old_saves / "system_settings.dat").write_bytes(b"previous settings\x00\xff")
        (old_logs / "launcher.log").write_text("previous log\n")
    assets = bulk / "installations/sly1"
    assets.mkdir(parents=True)
    (assets / "keep-assets").write_bytes(b"assets")
    runtime = bulk / "user/runtime/bin"
    runtime.mkdir(parents=True)
    config.mkdir(parents=True, exist_ok=True)
    selection = {"iso_path": str(root / "Sly USA.iso"), "runtime_dir": str(runtime)}
    (config / "launcher.json").write_text(json.dumps(selection))

    def launch():
        subprocess.run([launcher, "--help"], env=env, check=True,
                       capture_output=True, timeout=30)
        assert saves.is_dir() and logs.is_dir(), "Incorrect save/log locations"
        assert json.loads((config / "launcher.json").read_text()) == selection
        assert (assets / "keep-assets").read_bytes() == b"assets"
        assert runtime.is_dir(), "Runtime storage moved with saves"

    launch()
    if os.name != "nt":
        assert (saves / "system_settings.dat").read_bytes() == (old_saves / "system_settings.dat").read_bytes()
        assert (logs / "launcher.log").read_bytes() == (old_logs / "launcher.log").read_bytes()
        (saves / "system_settings.dat").write_bytes(b"new settings")
        (logs / "launcher.log").write_text("new log\n")
        launch()
        assert (saves / "system_settings.dat").read_bytes() == b"new settings"
        assert (logs / "launcher.log").read_text() == "new log\n"
        assert (old_saves / "system_settings.dat").read_bytes() == b"previous settings\x00\xff"
        assert (old_logs / "launcher.log").read_text() == "previous log\n"
    # A portable copy uses only its own folder, including after relocation.
    portable = root / "portable copy"
    portable_bin = portable / "bin"
    portable_bin.mkdir(parents=True)
    copied = portable_bin / Path(launcher).name
    shutil.copy2(launcher, copied)
    for name in ('libc++.dll', 'libunwind.dll', 'libwinpthread-1.dll'):
        dll = Path(launcher).parent / name
        if dll.is_file():
            shutil.copy2(dll, portable_bin / name)
    (portable / 'portable.txt').write_text('ProjectCane portable mode\n')
    subprocess.run([str(copied), '--help'], env=env, check=True,
                   capture_output=True, timeout=30)
    portable_data = portable / 'data'
    assert (portable_data / 'user/Saves').is_dir()
    assert (portable_data / 'logs').is_dir()
    assert not (portable_data / 'logs/launcher.log').exists(), 'Imported external logs'
    (portable_data / 'user/Saves/slot.dat').write_bytes(b'portable save')
    portable_config = {'iso_path': str(root / 'Sly USA.iso'),
                       'runtime_dir': str(portable_data / 'user/runtime/bin')}
    (portable_data / 'config/launcher.json').write_text(json.dumps(portable_config))
    if os.name != 'nt':
        custom = portable_data / 'user/runtime/bin'
        custom.mkdir(parents=True)
        game = custom / 'Sly1-Release'
        game.write_text('#!/bin/sh\n[ "$1" = --data-dir ] && [ "$3" = --user-dir ] || exit 1\n'
                        'printf "%s\\n%s\\n" "$2" "$4" > "$4/runtime-paths.txt"\n')
        game.chmod(0o755)
        installed = portable_data / 'installations/sly1'
        installed.mkdir(parents=True, exist_ok=True)
        (installed / 'installation.json').write_text(json.dumps({
            'schema': 1, 'asset_version': 1,
            'disc_sha256': 'd4ba3ce30047ffb2a825ef13d37a48c4df63aa57977157a194f66bd0b859796c'}))
    relocated = root / 'relocated caf\u00e9 portable'
    portable.rename(relocated)
    moved_launcher = relocated / 'bin' / copied.name
    subprocess.run([str(moved_launcher), '--help'], env=env, check=True,
                   capture_output=True, timeout=30)
    assert (relocated / 'data/user/Saves/slot.dat').read_bytes() == b'portable save'
    assert not portable.exists(), 'Launcher recreated the old portable location'
    if os.name != 'nt':
        subprocess.run([str(moved_launcher), '--play'], env=env, check=True,
                       capture_output=True, timeout=30)
        paths = (relocated / 'data/user/runtime-paths.txt').read_text().splitlines()
        assert paths == [str(relocated / 'data/installations/sly1'), str(relocated / 'data/user')]
    assert json.loads((config / 'launcher.json').read_text()) == selection
print("Launcher profile paths and migration passed")
