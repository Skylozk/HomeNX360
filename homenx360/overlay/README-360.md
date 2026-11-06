# HomeNX 360 — experimental dashboard fork

Based on `ad3333m/homenx` commit `94f2bd56698fed40f2897131ea09463775840767`.
License: GPL-3.0, with the upstream credits and third-party notices retained.

This changes CardsNX, including CardsNX running as the HomeNX HOME screen. It
recreates the gray background, green tile groups, large featured panel, navigation
tabs and selection outline in the supplied Xbox 360 dashboard reference. Game
artwork and names come from the Switch's library. It does not install Xbox
firmware, provide Xbox services, or change Nintendo's separate system applets.
HomeNX's existing dialogs remain available with gray panels and green accents.
Switching tabs adds a subtle 180 ms slide and fade. The existing **Animations**
setting turns this motion off, and navigation remains active during transitions.

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

The supplied package uses the unmodified official daemon from upstream workflow
198, for the same source revision as this fork. `BUILD-MANIFEST.json` records its
origin and hashes alongside the custom frontend and assets. Building the commands
below locally rebuilds the daemon from source instead.

## Controls

| Control | Action |
|---|---|
| D-pad / either stick | Move between tiles; Up from the top row enters the tab bar |
| L / R | Previous / next hub |
| A | Select or launch |
| B | Return to Home |
| Y | Guide: Search, pages/pins, quick settings, settings, power |
| ZR | Quick settings |
| + on a game tile | Game options, artwork, collections and achievements |
| X | Ask to close the running software |
| − | Notifications |
| Touch | Select a tab or activate a tile; Prev/Next below the library changes pages |

The library shows ten items at a time. Continue right past the last tile to
reach the next page. The **bing** tab searches installed games and homebrew;
its browser tile uses HomeNX's existing BrowseNX integration. **My Pins** opens
HomeNX's page/collection picker. **tv & movies** shows the existing media apps;
**music** opens HomeNX's player and sound settings. The preview image uses Zelda
art only as a sample and is not a screenshot from Switch hardware.

## Undo

Restore the backed-up folders and `frontend.txt` to return to your previous
HomeNX. If HOME stops at a black screen, power off, put the SD card in a computer,
and remove `atmosphere/contents/0100000000001000/` as documented by upstream.
The normal Nintendo HOME menu returns. Keep your backup to reinstall HomeNX.

## Build from this source

Use the upstream devkitPro devkitA64 environment, xmake and Python 3. Initialize
the upstream submodules when working from Git. The supplied source archive
includes their source trees.

Alternatively, run the manual **Build HomeNX 360** GitHub Actions workflow in
your own copy of the repository. It builds and uploads the package as an artifact
without publishing a release. That workflow has not been run in this session.

```sh
g++ -std=c++20 tests/dashboard360_test.cpp -o /tmp/dashboard360-test
/tmp/dashboard360-test
xmake f --yes -p cross -m release -a aarch64 --toolchain=devkita64 --homebrew=n
xmake build HomeNX-Loader
mkdir -p projects/menu/src/generated
python3 tools/embed_loader.py build/cross/aarch64/release/HomeNX-Loader.nro projects/menu/src/generated/loader_nro.inc
xmake build CardsNX
xmake build SwitchU
python3 tools/package_dashboard360.py --output build/dashboard360
```

The package script fails if required binaries or assets are missing. Native
hardware checks still needed: cold boot, sleep/wake, launching/resuming/closing
games, HOME from a game, multiple controllers, docked mode, and rollback.
