# macOS tests

The tests cover the macOS port and are only built on Apple platforms
(`if(APPLE)` in `CMakeLists.txt`).

| Test | Needs |
| --- | --- |
| `cane_platform_tests`, `cane_shader_source_tests`, `cane_world_sweep_selftest`, `cane_world_sweep_units` | nothing |
| `cane_shader_tests` | a display and an OpenGL 4.1 context (skips with 77 otherwise) |
| `cane_world_sweep_*` (levels, cutscenes, audio, saves, determinism, display, display-fullscreen, van-door, hub-tour, charm-exit, behaviour, unlock, fuse) | the extracted game data (skips with 77 when no data folder is given; fails when the folder given holds no game data) |

`cane_world_sweep_units` runs game code that needs no data (the sidebag
checks in `world_sweep/sweep_units.cpp`, and the shadow units' placeholder
binds on a hidden OpenGL 4.1 window, skipped with a note when no context can
be made). `cane_world_sweep_van-door` plays A
Stealthy Approach with no input until the van door opens, then on outside the
van (frames, views, every clue bottle and the vault) and loads Prowling the
Grounds from there. `cane_world_sweep_hub-tour` puts Sly on each of Prowling
the Grounds' level doors, volume buttons and tutorial triggers, so Bentley's
TV dialogs play, and fails on driver warnings and empty sampler units.
`cane_world_sweep_charm-exit` enters the Treasure in the Depths door from
inside the object update while an awarded charm still swirls (it used to
crash), plus three controls, each in its own process.

### Behaviour checks

These look for "something should happen and doesn't". The children read the
game's state after every frame (no game code is changed or wrapped for
them; `world_sweep/sweep_behaviour_child.cpp`), and
`world_sweep/sweep_behaviour.cpp` judges what they saw (its rules are covered
by `cane_world_sweep_selftest`). Polling cannot see anything that starts and
ends inside one frame, or changes made while a level loads.

- `cane_world_sweep_behaviour` (about 7 minutes) plays each of the 46 levels
  for 15 s with no input. It fails when an animation segment is applied and
  its channel's curve moves but the object it drives never does ("applied,
  target static"), and when an event listed in
  `world_sweep/must_happen.json` does not happen (A Stealthy Approach's van
  door: its button comes back, an opening animation plays on it, its state
  machine reaches open; the CTest test passes the list with
  `--must-happen`). Each level's structural trace (state machine
  transitions, segments applied and retracted, objects added to and removed
  from the world, custom option writes, cane responses, world and level
  state) goes to `world-sweep/traces/<level>.json`. When
  `world-sweep/baselines/behaviour/` exists (or `--baseline DIR` is given),
  each trace is compared with it as sets of events, never by frame: what is
  new and what is gone is listed in `behaviour.json` but never fails the
  run, because the game is not deterministic (two runs of the same build
  differ by a few events). Segments whose curve channels resolve no object,
  and state machine transitions whose animation stands still, are listed
  too, for a person to look at. Make or refresh the local baseline (it is
  derived from the game data, so it stays out of the repository) with
  `build/macos-release/bin/cane_world_sweep behaviour --data-dir <folder> --out build/macos-release/world-sweep --write-baseline build/macos-release/world-sweep/baselines/behaviour`.
- `cane_world_sweep_unlock` (about 7 minutes) loads each level, then makes
  the changes it can through the game's own paths: every lock group (LOCKG:
  hub doors, the fuse breaker) gets its keys and Sly walks up to it
  (TriggerLockg, what its volume calls), Sly stands in every volume button
  (switches and mission triggers), every clue is collected by touch, and
  every object whose cane response became breakable is swung at with the
  square button (pressed through the game's own input layer: for the press
  square is bound to a mouse button that the sweep presses, then the
  player's binding comes back). Then the game is saved and loaded
  (`SaveCurrentGameToDisk`, `LoadCurrentSave`), the level is loaded again
  and Sly walks back up to the lock groups that were open. Each lock
  group's outcome is counted in `unlock.json` (`unlock_outcomes`):
  triggered, approach+trigger (Sly put beside it first), self (it opened
  when he came near), forced (the sweep set the unlock's goals itself),
  failed, already open; forced and failed mean the player's path was not
  covered there and are listed in the level's `unlock_notes`.

  A breakable fails when two swings from beside it (Sly standing within
  160 units) do not break it; fewer, or a target out of the world, is
  noted only. How many of the objects that became breakable could be
  judged that way (they broke, or two swings from beside them missed) is
  in the summary line and in `unlock.json` (`swing_coverage`), like the
  fuse check's swing count. "Driven" below means an object whose response the sweep's own
  change set, by a change the game also saved (an unlock that set its
  world flag). Live and reloaded levels are compared, and these fail:
  - an object present live and gone after the reload, if it was driven
    (live play missed what the load path does) or if it is a breakable the
    live response cannot break (the user's fuse);
  - an object gone live and back after the reload (the save lost it),
    unless it is a breakable that was not driven (broken crates are not
    saved);
  - a different response or broken flag on a driven object;
  - a world flag the save lost (one the level's own load sets back is
    noted only);
  - a lock group open live that waits for its keys again when Sly walks up
    to it on the reloaded level.

  Everything else is advisory by design: differences on objects the sweep
  did not drive (they change with level time and with switches the game
  does not save), a driven breakable that is still breakable live (a
  pending break), lock pieces of a group open in both, undriven objects
  that leave the world on reload (an opened group's end state removes
  them), and state machines that wait for Sly to come near. Sly is moved by
  teleporting, so a fall of more than 400 units puts him back where he last
  stood (the checkpoint reload that would revive him is held back).
- `cane_world_sweep_fuse` (under a minute) plays the fuse sequence from real
  play in Tide of Terror: two keys from earlier levels, the third collected
  in A Cunning Disguise, back to Prowling the Grounds through the level's
  door, Sly walks up to the breaker (LOCKG 1843) and unlocks it (segment
  1874 sets the fuse's response from 3, bounce, to 17, hit), swings at the
  fuse (BREAK 1487), and the hub is saved and reloaded. It fails unless the
  unlock runs through the player's path, the fuse becomes breakable and
  breaks, it is still gone after the reload, and the reload agrees.

Failures in the game's shared code that are filed upstream (each entry's
`writeup` names the issue, or says why there is none yet) are listed in `world_sweep/known_findings.json` (the CTest tests pass it with
`--known`): like the sanitizer suppressions, they are reported as known
findings (`known_findings` in the reports) instead of failing the run.
An entry matches one failure exactly (the whole key, field and both values
of a live/reload difference, or the whole reason); entries that matched
nothing on a level that ran are listed as stale. The file is read before
the run, and a malformed one fails it. Mac-only problems are never listed
there. Run by hand (or through
`cane_world_sweep all`), these checks need `--must-happen` and `--known`
given the same way, or the known findings fail them.

`--freeze-root OID` (behaviour), `--inject-stuck OID` and
`--inject-lose-flag LOCKG` (unlock, fuse) are checks of the checks: the
first holds still every object a segment rooted on OID drives (the van
door, `--freeze-root 1862`, is then "applied, target static"); the second
puts the object's response back after its unlock, as if the live change
were missed (`--inject-stuck 1487` fails the fuse check the way the user's
game did); the third clears the group's world flag before the save, as if
the save lost the unlock. `--inject-lose-flag 1843` fails both checks on
three of the rules above: the world flag the save lost, the broken fuse back
in the world after the reload (gone live, back on reload), and the breaker's
state machine back at its fresh state when Sly walks up to it again; the
fuse check also reports "the fuse broke live but is back after the save and
reload". It does not exercise the first rule (present live, gone after the
reload); the selftest covers that one directly.

The must-happen list is checked before a run: no events, an entry without
a level, kind or key, or a level that is not one of the game's fails at
once, and an entry whose level did not run or load fails the run. Blend
curves read the object's original pose, so the motion check does not judge
blend channels (`blend_channels_not_judged` per level).

The sweep links its own build of `basic.cpp` and `sidebag.cpp`
(`world_sweep/sidebag_probe.cpp`) with `EnsureBasicSidebag` and `PsidebagNew`
wrapped, so every check sees when a sidebag the game allocated is replaced by
an empty one (the object's custom options are lost); a level fails on any
(`sidebag_rebuilds` in `levels.json`).

## Running

```sh
cmake --preset macos-release
cmake --build --preset macos-release
ctest --test-dir build/macos-release --output-on-failure
```

The world sweep reads the extracted game (the folder holding `Worlds/` and
`Cutscenes/`) from a path given at run time and never copies it. With no
folder given (`CANE_TEST_DATA_DIR` unset or empty, no `--data-dir`) the
world sweep tests skip; a folder that is given but holds no
`Worlds/Splash/Splash.brx` fails them, so a mistyped path cannot pass as a
skip. Screenshots and logs go to `build/<preset>/world-sweep`, outside the
source tree (also when the sweep is run by hand without `--out`: the default
is `world-sweep` beside the executable's `bin/` folder, not the current
directory):

```sh
CANE_TEST_DATA_DIR=/path/to/extracted-game \
  ctest --test-dir build/macos-release --output-on-failure
```

The tests set `SDL_AUDIO_DRIVER=dummy` for the game processes, and
`SDL_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT=0x0000/0x0000` so they never read a
connected controller (the game listens to controllers in the background, so a
test would otherwise act on the input of someone playing). Test processes are
also background-only (`tests/background_app.cpp`): no Dock icon, and they never
take focus. Child time limits count awake time only, so a system sleep does not
time a child out; a child that is stopped for being stuck is sampled first, and
its stacks go to its log, and it is reported as timed out (not as a crash). A
child that crashes prints its backtrace to its log, also after a stack
overflow (the crash handler runs on its own stack). The checks with a fixed
number of children (audio, saves, determinism, display, van-door, hub-tour,
charm-exit, fuse) stop them in time to end before their 900 s CTest timeout,
sampling included. Run one check
by hand with, for example,
`build/macos-release/bin/cane_world_sweep levels --data-dir <folder> --out <folder> --levels "A Stealthy Approach"`
(see the top of `world_sweep/sweep_main.cpp` for all options; an unknown
option, or one without its value, is an error). `cane_world_sweep all` runs
every check above in turn.

To find the GL call behind a GL error, set `CANE_SWEEP_GL_TRACE=1`: every GL
function the game calls then checks `glGetError` right after itself, and the
child's log gets the function, the error and the call stack (for a draw that
fails with `GL_INVALID_OPERATION`, also the bound program, vertex array and
the driver's `glValidateProgram` message). It is slow; the errors still count.

## Sanitizers (AddressSanitizer + UndefinedBehaviorSanitizer)

The `macos-asan` preset builds everything compiled from source (the game,
the tests, imgui, glad, GLFW and FAudio) with
`-fsanitize=address,undefined -fsanitize-recover=address` as RelWithDebInfo
in `build/macos-asan`. FFmpeg comes from Homebrew (pkg-config) and the
OpenGL driver from the system. SDL2 is the installed one when CMake finds an
SDL2 config package (with Homebrew that is `sdl2-compat`); otherwise CMake
fetches SDL 2.30.11 and builds it with the same flags. Installed libraries are
not instrumented, but ASan's interceptors still check what they do through
`malloc`, `memcpy` and friends.

```sh
cmake --preset macos-asan
cmake --build --preset macos-asan
CANE_TEST_DATA_DIR=/path/to/extracted-game \
  ctest --test-dir build/macos-asan --output-on-failure
```

`CANE_SANITIZE=ON` (set by the preset) gives every test these runtime options
and a `FAIL_REGULAR_EXPRESSION` matching a report (`runtime error: ` or
`==ERROR: AddressSanitizer: ` / `UndefinedBehaviorSanitizer: `), because with
`halt_on_error=0` a test that hits a report keeps running and can still exit
0. Run the tests through `ctest` (as above) to get that check; running a test
binary by hand only prints the reports:

- `ASAN_OPTIONS`: `detect_leaks=0` (LeakSanitizer is not supported on arm64
  macOS), `halt_on_error=0` (with `-fsanitize-recover=address`, so one level
  reports all its errors), `abort_on_error=1`, `allow_user_segv_handler=0`
  (ASan reports a SEGV itself instead of the game's handler),
  `check_initialization_order=1:strict_init_order=1`,
  `detect_stack_use_after_return=1`, and `suppressions=tests/sanitizers/asan.supp`.
- `UBSAN_OPTIONS`: `print_stacktrace=1:halt_on_error=0:report_error_type=1`
  and `suppressions=tests/sanitizers/ubsan.supp`.

Reports go to stderr. The world sweep runs the game in child processes whose
stdout and stderr go to `world-sweep/logs/*.log`, and every check counts the
reports in its children's logs and fails on any. The levels check attributes
each report to the level that was running (`levels.json`: `sanitizer_reports`
and `sanitizer_report` per level, `startup_sanitizer_reports` before the first
level). UBSan prints each source location once per process, so a problem shared
by many levels shows up under the first level that hit it.

To run the sanitizer build by hand, export the same options first (copy them
from `build/macos-asan/CTestTestfile.cmake`) and look for those lines in the
output yourself.

`tests/sanitizers/*.supp` lists the known reports that are not fixed here:
findings in the game's shared code (filed upstream, not fixed in the
macOS port) and in third-party or system code. Each entry says why it is
there. Mac-only code (`CANE_GL41` blocks, `Sly1-Release/gl41`, `platform/`
`__APPLE__` paths, `tests/`) must stay clean and is never suppressed.
