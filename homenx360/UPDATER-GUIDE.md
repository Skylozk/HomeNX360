# HomeNX360 Updater NRO

Put **HomeNX360-Updater.nro** in `sd:/switch/HomeNX360-Updater/` and launch it
from the homebrew menu with Wi-Fi connected. It checks this repository's public
releases automatically, including preview releases, and installs a newer version
of **both the preview app and the HomeNX dashboard**. No GitHub login is needed.

The canonical preview is installed at:

`sd:/switch/HomeNX360/HomeNX360.nro`

Existing standard copies named `switch/HomeNX360-preview.nro`,
`switch/HomeNX360.nro`, `switch/HomeNX360-preview/HomeNX360-preview.nro`, and
`switch/HomeNX360/HomeNX360-preview.nro` are refreshed too. For a differently
named copy, use the canonical HomeNX360 app after the first update. The updater
also refreshes its standard copies when the release includes a newer updater.

The updater has an NXE-style green-and-silver screen with the shared logo,
installed/available versions, update stages, progress, download size and speed,
and a readable error panel. It keeps the adaptive Switch heap and streams the
archive to SD. If graphics cannot initialize, it falls back to the text console.

A checks again or retries after completion. B pauses checking/downloading before
installation; A resumes/retries from the paused screen, and B exits. Pressing B
again while a pause is pending exits after the worker stops safely. During file
installation, let it finish. Downloads resume when the same release is retried.
After updating HOME, restart the console to load the new dashboard and finish any
files that were in use. The preview app uses its updated binary on its next launch.

## Fix for the reported File exists error

Version v0.9.1-preview.1 replaces the std::filesystem copy operation used to stage
older preview filenames with a bounded stdio copy which truncates an existing
staging file. It also checks errors while clearing the previous staging folder.
The same routine is used for preview/updater aliases and version markers.

If an older updater stops with **File exists** while copying cardsnx.nro to a
staged HomeNX360-preview.nro, manually download the new **HomeNX360-Updater.nro**
and replace the old updater NRO on the SD card first. Then launch the new updater.
You do not need to remove CardsNX settings or saved avatars. The error occurs in
update staging; replacing the updater is the intended repair. The complete update
ZIP is also available for manual SD installation.

CardsNX settings and per-profile avatar appearances are preserved. The selected
HomeNX frontend becomes CardsNX; an existing disabled HOME override stays disabled.
The updater does not change Nintendo profiles, Miis or game saves.

The full wardrobe makes releases large. The updater downloads to SD, stages the
files and checks free space before installation. Keep several GiB free for the
archive, staging and replacements. SHA-256, HTTPS, ZIP CRCs, restricted destination
paths and complete-package checks run before replacing files. Reported install
errors trigger rollback; pending files finish at startup. This is not a guarantee
against power loss or a failing SD card. Keep the console powered during installation.

Logs are written under `config/SwitchU/homenx360-updater.log`; download/staging data
is under `config/HomeNX360/update/`. Partial downloads can be deleted there if you
want to restart a download. Existing settings under `config/CardsNX/` are untouched.

The updater compiles natively, and host tests cover release/version selection,
trusted URLs and hashes, path restrictions, both installation locations, settings
preservation, rollback, incomplete packages, cancellation and overwriting staged
legacy preview copies. The software-rendered UI tests cover every screen state,
long errors and framebuffer bounds. No live Switch or SD
installation has been performed here; physical-device behavior remains unverified.
