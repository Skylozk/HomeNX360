# HomeNX 360 — experimental dashboard fork

Based on `ad3333m/homenx` commit `94f2bd56698fed40f2897131ea09463775840767`.
License: GPL-3.0, with the upstream credits and third-party notices retained.

This changes CardsNX, including CardsNX running as the HomeNX HOME screen, into
an experimental 2008 NXE-inspired dashboard: vertical channels, overlapping
cards, reflections, a Guide, full-screen NXE settings and a skippable recorded Xbox 360 startup. Game artwork and
names come from the Switch's library. Existing HomeNX dialogs remain available.
The startup runs inside HomeNX; it does not replace the console bootloader.
Profiles now show original-style full-body Xbox 360 avatars with a per-profile
creator. See `AVATAR-GUIDE.md` for controls, media credits and fidelity limits.
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
| Y | Open Guide; inside it, return to Dashboard |
| + on a game card | Game options, artwork and collections |
| X | Ask to close running software |
| − | Notifications |
| Touch | Tap a side card to focus, then tap the focused card to activate |

The Guide also opens quick settings, search and power controls. Libraries use
one continuous carousel. During the startup, A/B skips the animation. The
**Startup animation** setting in Look controls the clip. **Animations** controls
channel/card/Guide transitions separately. **Preview startup** replays the clip.
Startup controls also appear in **System Settings → Console Settings → Startup
and Shutdown**.

## Settings controls

System Settings opens the NXE screen. Up/Down and either stick select rows;
A opens or changes a setting, Left/Right adjusts values, B returns to the
previous page and row, and Plus closes settings. Touch a category to open it.
A first touch on a different value only focuses it; touch it again or use
the right-hand minus/plus controls to change it. The A/B footer is touchable.

HomeNX Preferences is the added eighth root entry for dashboard, games/apps,
lock screen, notifications, pictures and update tools. Console Settings uses
the eight NXE category names. Xbox camera, disc auto-play and LIVE features are
explicitly unavailable. Console date/time and Nintendo network configuration
require the HOME frontend. See `SETTINGS-REFERENCES.md` for the complete mapping.

## Avatar profiles

Profiles opens the NXE-style sign-in row. Left/Right selects a Switch profile,
A signs in, X customizes its full-body avatar, Y opens the existing user tools,
and B returns. In the creator use Up/Down for features, Left/Right to change,
L/R to rotate, Y to randomize, X to save and B to cancel. The creator saves only
the appearance for that Nintendo account UID. It never changes Miis or names.

Sign-in follows startup by default. Console Settings → Startup and Shutdown →
Sign-in at startup disables it. Returning HOME from a game keeps your profile.
The NRO embeds avatar assets; the HOME update also needs the complete
`switch/SwitchU/cards/avatars/` folder from the ZIP. Read `AVATAR-GUIDE.md`.

## Undo

Restore the backed-up folders and `frontend.txt` to return to your previous
HomeNX. If HOME stops at a black screen, power off, put the SD card in a computer,
and remove `atmosphere/contents/0100000000001000/` as documented by upstream.
The normal Nintendo HOME menu returns. Keep your backup to reinstall HomeNX.

## Build from this repository

See the root README for the pinned source preparation script and GitHub Actions
workflow. The source overlay is in `homenx360/overlay/`.

## Guide controls

Inside the Guide, Left/Right or L/R changes blades; Up/Down moves between rows.
A selects, B returns/closes, X opens profiles, Y returns to Dashboard, and Plus
also closes it. The Media blade controls the real HomeNX music player.

## Discord Friends

Guide → Friends lists Discord direct and group DMs in the HOME frontend.
A or touch opens a conversation; A/X in the chat opens the reply keyboard.
X in Friends opens HomeNX's existing Discord sign-in and account controls.
The standalone NRO shows the HOME requirement and an external Discord shortcut.
Read `DISCORD-GUIDE.md` for setup, paging, controls and testing limits.

## Expanded avatar catalog

The preview NRO uses `sd:/switch/SwitchU/cards/avatars/catalog/` for its larger
wardrobe. Extract **HomeNX360-avatar-catalog.zip** to the SD root when using the
standalone app. The complete **HomeNX360-update.zip** already includes these
files for HOME. Keep file names and folders intact. Use full-memory homebrew
launch mode; this release has not been verified in applet mode or on hardware.
Back up `config/CardsNX/` before upgrading; older releases ignore new catalog
appearance records. See [the avatar guide](AVATAR-GUIDE.md).
