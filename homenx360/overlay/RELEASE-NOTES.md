# HomeNX360 v0.9.1-preview.1 — updater staging repair and NXE updater screen

Fixes the reported **File exists** failure when preparing a staged copy of
HomeNX360-preview.nro. Preview/updater aliases and version markers use a bounded
stdio copy which replaces an existing staging file. Errors clearing a previous
staging folder are now reported directly. Host regression checks cover repeated
replacement and rejection of copying a file onto itself without truncating it.

Adds a green-and-silver updater screen with the approved HomeNX360 logo, version
details, five progress steps, download progress/size/speed, readable errors, and
pause/retry controls. The updater still updates both preview and HOME while
preserving settings and saved avatars. This release still downloads the complete
update package; it does not introduce smaller incremental updates.

**To repair an older updater:** manually download **HomeNX360-Updater.nro** from
this release, replace the old updater NRO on the SD card, then launch it. The old
updater cannot repair its own staging failure before installing this fix.
Alternatively, extract HomeNX360-update.zip to the SD root and restart for HOME.
Read UPDATER-GUIDE.md for paths, controls and the HOME restart requirement.

Retains v0.9.0's category-based avatar editor, 2,420 item thumbnails, live try-on,
full-body/zoom/rotation, profile saves, HomeNX360 launcher names/icons, and all
eleven NXE wallpapers. The avatar/editor and original media are unchanged.

Native builds, staged-file replacement and updater policy/install checks pass.
UI tests render all nine states with real font/logo assets and check framebuffer
bounds and long messages. Reference images are software-rendered. The reported
Switch failure guided this repair; the fix and new UI still require device testing.
