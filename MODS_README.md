# ProjectCane Mods (save states, fast forward, combos)

Pause > Options > **Mods...**

| Entry | What it does |
|---|---|
| Mods: On/Off | Master switch for everything below |
| Load State N | Closes the menu and reloads the level at that state |
| Save State N | Saves the current position/progress to the selected slot |
| State Slot | Cycles slots 1-5 |
| Fast Forward Speed | 2x / 3x / 4x / 8x |
| Six binding rows | Select a row, then hold a modifier button and press a second button. Press and release a single button to turn that combo off. Esc cancels. |

## Defaults
| Action | Controller | Keyboard |
|---|---|---|
| Save state | Select + R1 | F5 |
| Load state | Select + L1 | F8 |
| Previous / next slot | Select + Left / Right | F6 / F7 |
| Fast forward (hold) | Select + R2 | Tab |
| Fast forward (toggle) | Select + L2 | F9 |

Combos and hotkeys only work during gameplay (not in menus). A quick tap of the
modifier on its own still reaches the game when you let go, so Select still
opens the map.

## What a save state contains
Position states, not emulator snapshots: the level, Sly's position and facing,
the level's checkpoint bits (broken/collected/disabled objects), the real last
checkpoint, and game progress (coins, keys, bottles, lives, charms). Loading
reloads the level, so guards and moving objects restart from their spawn
points. Dying after a load returns to the real checkpoint.

States are files in `SaveStates\slot1.state` ... `slot5.state` next to the exe
and survive restarts. Settings and keyboard keys are in `Mods.ini` (keys are
GLFW key codes; F5 = 294, Tab = 258).

## Fast forward
Runs several normal-length game updates per rendered frame, so physics and
collision behave normally. Music stays at normal speed.
