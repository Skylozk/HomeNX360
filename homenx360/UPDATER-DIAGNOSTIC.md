# HomeNX360 Updater diagnostic build

This is a standalone updater NRO with clearer errors for the reported
"Unable to stage ... Unable to open staged update file: I/O error" failure.
It identifies the actual source/destination file and operation, preserves the
first error during cleanup, and records the last native Switch FS result for
I/O failures. File handles close during exception cleanup. Regression checks
cover missing extracted updater files, late read failures, preservation of
installed files and removal of partially prepared files.

This build helps identify the underlying failure; it does not establish that
the SD card is faulty or guarantee that an existing I/O error is repaired.
It retains the updater's checksums, staging, rollback, GUI and dual-install flow.

## Install

Close the updater, fully shut down the Switch, and replace:

`sd:/switch/HomeNX360-Updater/HomeNX360-Updater.nro`

with **HomeNX360-Updater.nro** from the diagnostic prerelease. If you normally
launch the legacy `sd:/switch/HomeNX360-Updater.nro`, replace that copy too.
Safely eject the SD card and launch the updater with Wi-Fi connected. Keep the
existing settings, saved avatars and cached ZIP. If the failure returns, send
the complete error including any **Switch FS 0x...** code, or the updater log at
`sd:/config/SwitchU/homenx360-updater.log`.

The diagnostic release contains only the updater and its source/build records.
It checks for the latest complete dashboard release, currently
**v0.10.0-preview.1**. It is deliberately not offered as a complete dashboard
update, and a successful complete update may replace this diagnostic NRO with
that release's normal updater. Restart after updating HOME.

## Source and rebuild

The binary uses build identifier **v0.10.0-diag.1**. Its GitHub prerelease tag is
**updater-v0.10.0-diag.1**. The release manifest identifies the exact source
revision and SHA-256 of every supplied asset.

Complete corresponding source is the prepared **HomeNX360-source.zip** from
[v0.10.0-preview.1](https://github.com/Skylozk/HomeNX360/releases/tag/v0.10.0-preview.1)
plus the supplied **HomeNX360-Updater-diagnostics.patch**. That patch is relative
to the extracted `HomeNX360-source/` directory and includes the production change
and regression tests. The original source ZIP includes populated upstream
submodules, media, build scripts and license notices. The manifest records its
immutable GitHub asset ID, size and SHA-256. The repository's normal preparation
script also reproduces the same sources from the pinned upstream commit plus
this release's overlay revision.

With the devkitPro Switch SDK/portlibs and xmake 3.1.2 installed, run from the
extracted complete source directory:

```sh
git apply --check ../HomeNX360-Updater-diagnostics.patch
git apply ../HomeNX360-Updater-diagnostics.patch
export HOMENX360_RELEASE_TAG=v0.10.0-diag.1
xmake f --yes -p cross -m release -a aarch64 --toolchain=devkita64 --homebrew=n
xmake build -j2 HomeNX360-Updater
```

The result is `build/cross/aarch64/release/HomeNX360-Updater.nro`.
When running xmake as root, add its `--root` option to each command.

This download is built locally with the same native SDK used for earlier local
release validation. Native compilation, production updater regression checks,
NRO name/icon/version inspection, embedded GUI asset hashes, and the published
download hash are checked. Physical Switch behavior remains unverified.
