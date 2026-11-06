# HomeNX 360 — NXE preview

An experimental native Nintendo Switch dashboard for an existing HomeNX install,
inspired by the Xbox 360's **2008 New Xbox Experience (NXE)**.

- Vertical channels and overlapping cards with a reflective floor.
- Animated card focus and vertical channel switching; respects Animations settings.
- A skippable, original HomeNX startup and a compact Guide.
- Real Switch games, homebrew, friends, media and settings actions.

This is an NXE-inspired adaptation, not an exact reproduction of every Xbox
screen. Existing HomeNX dialogs and Nintendo applets retain their interfaces.
No Xbox Live services, Xbox firmware, proprietary startup video or audio are bundled.
[Research sources and access limitations](homenx360/NXE-REFERENCES.md).

## Download

Open [Releases](https://github.com/Skylozk/randomgit/releases) and choose the
NXE preview. Download **HomeNX360-preview.nro** to try the standalone app first.
**HomeNX360-update.zip** contains the SD card update for an existing HomeNX
installation. Do not install the GitHub-generated source ZIP as an update.

Read [installation and rollback instructions](homenx360/INSTALL.md).
Atmosphere 1.12.1 / system 22.5.0 was reported by the user; this combination
has **not been tested on hardware here**. Builds are experimental prereleases.

A dedicated HomeNX360 repository is prepared locally, but the connected GitHub
integration cannot create repositories. Release downloads can live in this
repository until the account owner creates a separate empty repository.

## Controls

Up/Down or ZL/ZR changes channel. Left/Right or L/R changes card. A activates,
B returns to My Xbox, Y opens the Guide, + opens game options, and − shows
notifications. Tap a side card to focus it, then tap again to activate it.
A/B skips the startup. Returning HOME from software does not replay startup.

## Builds and release publishing

The **Build HomeNX 360** workflow compiles the standalone app, HOME frontend and
matching daemon, checks controller navigation, and uploads an artifact. Run it
manually, after merging the workflow into the default branch, with an optional tag such as `v0.2.1-preview.1` to also publish a
prerelease after a successful build. The release includes executable manifests,
checksums and the complete prepared source tree with populated submodules.

The readable modified files are in [the overlay](homenx360/overlay/), with a
[patch](homenx360/HomeNX360.patch). Upstream HomeNX is pinned to
`94f2bd56698fed40f2897131ea09463775840767`.

With devkitPro devkitA64, its Switch packages, xmake 3.1.2, CMake and Python 3:

```sh
python3 tools/prepare_homenx360.py
cd build/homenx360-source
g++ -std=c++20 tests/dashboard360_test.cpp -o /tmp/dashboard360-test
/tmp/dashboard360-test
xmake f --yes -p cross -m release -a aarch64 --toolchain=devkita64 --homebrew=n
xmake build HomeNX-Loader
mkdir -p projects/menu/src/generated
python3 tools/embed_loader.py build/cross/aarch64/release/HomeNX-Loader.nro projects/menu/src/generated/loader_nro.inc
xmake build -j2 CardsNX
xmake build -j2 SwitchU
python3 tools/package_dashboard360.py --output ../artifacts
```

## Credits

Based on [HomeNX by ad3333m](https://github.com/ad3333m/homenx), under
[GPL-3.0](LICENSE), with [upstream notices](THIRD_PARTY_NOTICES.md) retained.
Selawik Light is Copyright 2015 Microsoft Corporation, distributed unmodified
under its bundled SIL Open Font License. Not affiliated with Nintendo or Microsoft.
