# Predictive Input Buffer

A Windows Geode mod project for Geometry Dash **2.2081**.

## Controls

- Default hotkey: **Ctrl+Shift+B**
- Press the hotkey during gameplay to cycle the delay: **1 → 2 → ... → 16 → 1 ticks**.
- The selected tick count is saved and restored after restarting the game.
- You can change the hotkey in Geode's mod settings.

## What it does

This first version delays both button-down and button-up events by a selected number of PlayLayer update ticks. Despite the name, it does **not** predict future physics or collision state; it is a configurable input delay queue. Different gameplay modes may feel different, so test carefully in practice mode first.

## Build

This repository includes a GitHub Actions workflow that builds the Windows package. Open the repository's **Actions** tab, select the latest successful **Build Geode mod** run, and download its artifact.

## Safety/testing note

This source has not been validated in-game. If the build fails or gameplay inputs behave unexpectedly, do not install/use the package until the issue is fixed. The queue is cleared when leaving a level.
