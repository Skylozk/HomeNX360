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

A retries/checks again after completion; B cancels a check/download or exits.
Downloads resume on a later launch. During file installation, let it finish.
After updating HOME, restart the console to load the new dashboard and finish any
files that were in use. The preview app uses its updated binary on its next launch.

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
preservation, rollback, incomplete packages and cancellation. No live Switch or SD
installation has been performed here; physical-device behavior remains unverified.
