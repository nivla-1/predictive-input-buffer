# Predictive Input Buffer

A Windows Geode mod prototype for Geometry Dash **2.2081**.

## Controls

- Default hotkey: **Ctrl+Shift+B**
- Press the hotkey during gameplay to cycle the buffer: **1 → 2 → ... → 16 → 1 ticks**.
- The selected tick count is saved and restored after restarting the game.

## Prototype behaviour

This prototype borrows the collision-check approach from an existing public input-buffer project. It buffers button presses, tries to replay a pending press when an activatable effect object is detected, and otherwise replays it when the selected tick window expires. Button releases are passed through immediately.

**Important limitations:** this is not yet a verified predictive/death-prevention buffer. The object activation hook is most relevant to orbs and similar effect objects; it does not reliably predict terrain collisions, and wave-mode early-click recovery is not confirmed. Some input sequences or gameplay modes may behave unexpectedly. Do not treat the feature as working until it builds and is tested in Geometry Dash, preferably in practice mode.

## Build

The GitHub Actions workflow attempts to build a Windows package. Open the repository's **Actions** tab, select the latest successful **Build Geode mod** run, and download its artifact.

## Development status

Prototype code only; not validated in-game. If the build fails, fix the compile errors before installing. The buffer is cleared on level reset, death, or exit.
