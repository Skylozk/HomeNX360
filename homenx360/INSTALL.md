# HomeNX 360 — experimental dashboard fork

Based on `ad3333m/homenx` commit `94f2bd56698fed40f2897131ea09463775840767`.
License: GPL-3.0, with the upstream credits and third-party notices retained.

This changes CardsNX, including CardsNX running as the HomeNX HOME screen, into
an experimental 2008 NXE-inspired dashboard: vertical channels, overlapping
cards, reflections, a Guide and an original skippable startup. Game artwork and
names come from the Switch's library. Existing HomeNX dialogs remain available.
The startup runs inside HomeNX; it does not replace the console bootloader.
See [the reference log](NXE-REFERENCES.md) for the evidence and remaining gaps.

## Your console

Reported configuration: Atmosphere 1.12.1, system 22.5.0, HomeNX already installed.
These versions have not been tested on hardware here. Compilation and the host
navigation checks do not establish console compatibility. This is an experimental
build for testing, not a verified firmware-specific release.

## Try the standalone app

Copy `HomeNX360-preview.nro` into `sdmc:/switch/HomeNX360/` and run it through
your existing homebrew launcher with full application memory. This app does not
replace HOME. It shares CardsNX's existing configuration and artwork cache;
back up `config/CardsNX/` first. HOME-only functions such as HomeNX News are
available in the menu replacement build. Press HOME to leave the standalone app.

## Update your installed HomeNX

1. Power off and back up the SD card's `atmosphere/contents/0100000000001000/`,
   `switch/SwitchU/`, `config/SwitchU/frontend.txt`, and `config/CardsNX/`.
2. Extract `HomeNX360-update.zip` to the SD card root, merging folders. The
   archive contains the rebuilt HOME frontend, its matching daemon, its assets,
   and a `frontend.txt` containing `cards` to select this dashboard.
3. Reinsert the SD card and restart. In NXToggle, the look is still named
   **CardsNX**. In this fork that choice opens the 360 dashboard.
4. HomeNX/NXToggle's normal online updater downloads upstream HomeNX and will
   replace this customization. Reapply this custom build after an upstream update.

The update archive is for an existing, functioning HomeNX installation; it does
not bundle a fresh Atmosphere installation or change your firmware.

`BUILD-MANIFEST.json` records the exact executable hashes and daemon origin.
CI builds the daemon from the pinned source. Local preview packages can reuse
an unmodified official daemon from the same upstream revision; this is recorded
explicitly in their manifest. Native compilation is not a console test.

## Controls

| Control | Action |
|---|---|
| D-pad Up/Down or either stick | Previous/next vertical channel |
| D-pad Left/Right or L/R | Previous/next card |
| ZL/ZR | Previous/next channel |
| A | Activate the selected card |
| B | Return to My Xbox; close the Guide |
| Y | Open/close the Guide |
| + on a game card | Game options, artwork and collections |
| X | Ask to close running software |
| − | Notifications |
| Touch | Tap a side card to focus, then tap the focused card to activate |

The Guide also opens quick settings, search and power controls. Libraries use
one continuous carousel. During the startup, A/B skips the animation. The
existing **Animations** setting disables startup and channel/card animations.

## Undo

Restore the backed-up folders and `frontend.txt` to return to your previous
HomeNX. If HOME stops at a black screen, power off, put the SD card in a computer,
and remove `atmosphere/contents/0100000000001000/` as documented by upstream.
The normal Nintendo HOME menu returns. Keep your backup to reinstall HomeNX.

## Build from this repository

See the root README for the pinned source preparation script and GitHub Actions
workflow. The source overlay is in `homenx360/overlay/`.
