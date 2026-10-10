ProjectCane Linux Test Package
==============================

This package contains:

- ProjectCane-x86_64.AppImage: portable Sly 1 Linux build
- File Extractor GUI/: self-contained Linux extractor
- Run ProjectCane.sh: game launcher
- Run File Extractor.sh: extractor launcher

The original game ISO and extracted game data are not included.

First-time setup
----------------

1. Extract this entire archive into a normal writable folder. Do not run it
   from inside the archive.

2. Open a terminal in that folder.

3. Launch the extractor:

       bash "Run File Extractor.sh"

4. Use the extractor to prepare the required Worlds and Cutscenes folders in
   this same folder, beside ProjectCane-x86_64.AppImage.

5. Launch the game:

       bash "Run ProjectCane.sh"

The launchers set executable permissions automatically. The ProjectCane
AppImage was built against Ubuntu 22.04 and includes its FFmpeg and SDL2
runtime libraries.

Save games and settings are stored in:

    ~/.local/share/ProjectCane

or under `$XDG_DATA_HOME/ProjectCane` when XDG_DATA_HOME is configured.

If ProjectCane does not start
-----------------------------

Run this and send the resulting projectcane_startup.log file:

    ./ProjectCane-x86_64.AppImage 2>&1 | tee projectcane_startup.log

Steam Deck users should run these commands from Konsole in Desktop Mode.
