# HomeNX 360 — NXE preview

An experimental native Nintendo Switch dashboard for an existing HomeNX install,
inspired by the Xbox 360's **2008 New Xbox Experience (NXE)**.

- Vertical channels and overlapping cards with a reflective floor.
- Animated card focus and vertical channel switching; respects Animations settings.
- The recorded original NXE-era Xbox 360 startup.
- A centered NXE Guide with animated blades, profile/clock, green selection and an added Discord Friends blade.
- Full-screen NXE settings with the original Console Settings categories, animated submenus and real HomeNX controls.
- Full-body Xbox 360 avatars, an NXE-style sign-in row, and an expanded creator with built-in/Marketplace clothing, facial options and animated props.
- Real Switch games, homebrew, friends, media and settings actions.

This is an NXE-inspired adaptation, not an exact reproduction of every Xbox
screen. Existing HomeNX dialogs and Nintendo applets retain their interfaces.
The startup uses recorded Xbox media, credited separately from the GPL code.
Avatar meshes and animations are media from the original packs, the supplied
Marketplace archive and legacy 360sona exports;
see [the avatar guide](homenx360/AVATAR-GUIDE.md) for controls and fidelity limits.
Xbox Live services and Xbox firmware are not included.
[Startup recording, analysis and fidelity limits](homenx360/STARTUP-REFERENCES.md).
[Dashboard reference research](homenx360/NXE-REFERENCES.md).
[Guide references and action mapping](homenx360/GUIDE-REFERENCES.md).
[Settings references, controls and functional mapping](homenx360/SETTINGS-REFERENCES.md).
[Discord Friends setup and controls](homenx360/DISCORD-GUIDE.md).
[Startup media notice](homenx360/overlay/media/STARTUP-MEDIA-NOTICE.md).

## Download

Open [Releases](https://github.com/Skylozk/HomeNX360/releases) and choose the
NXE preview. For the standalone app, download **HomeNX360-preview.nro** and
extract **HomeNX360-avatar-catalog.zip** to the SD root.
**HomeNX360-update.zip** contains the SD card update for an existing HomeNX
installation. Do not install the GitHub-generated source ZIP as an update.

Read [installation and rollback instructions](homenx360/INSTALL.md).
Atmosphere 1.12.1 / system 22.5.0 was reported by the user; this combination
has **not been tested on hardware here**. Builds are experimental prereleases.

This is the dedicated HomeNX360 repository. The initial NXE preview preserves
the verified build from the original development repository, including its
checksums, source revision and complete corresponding source archive.

## Controls

Up/Down or ZL/ZR changes channel. Left/Right or L/R changes card. A activates,
B returns to My Xbox, Y opens the Guide, + opens game options, and − shows
notifications. Tap a side card to focus it, then tap again to activate it.
A fresh A/B press skips startup; inherited launch-button presses are ignored.
Returning HOME from software does not replay it. Look settings include
**Startup animation** and **Preview startup**, separate from UI animations.

System Settings opens the full-screen NXE menu. Up/Down selects, A opens or
changes a setting, Left/Right adjusts values, B returns to the previous page,
and Plus closes settings. HomeNX Preferences retains the additional tools.
Startup controls also appear under Console Settings → Startup and Shutdown.

In the HOME frontend, Guide → Friends lists Discord DMs. A opens a conversation;
A/X in the chat opens the reply keyboard. X on Friends opens Discord sign-in and
accounts. This view hides server channels; the normal Discord entry retains them.
The standalone NRO shows the HOME requirement and an external Discord shortcut.

## Builds and release publishing

The **Build HomeNX 360** workflow compiles the standalone app, HOME frontend and
matching daemon, checks controller navigation, and uploads an artifact. Run it
from **Actions → Build HomeNX 360 → Run workflow**, with an optional tag such as
`v0.2.1-preview.1` to also publish a prerelease after a successful build. The release includes executable manifests,
checksums and the complete prepared source tree with populated submodules.

The readable modified files are in [the overlay](homenx360/overlay/), with a
[code patch](homenx360/HomeNX360.patch). Startup media is supplied separately
in the overlay. Upstream HomeNX is pinned to
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
Selawik Light, Regular and Semibold are Copyright 2015 Microsoft Corporation, distributed unmodified
under its bundled SIL Open Font License. Not affiliated with Nintendo or Microsoft.
