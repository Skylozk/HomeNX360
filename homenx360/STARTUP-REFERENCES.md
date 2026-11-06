# Original Xbox 360 startup research

The selected target is the original pre-Kinect (2005–2010) animation used with
NXE, as explicitly chosen by the user. This is a bounded reference survey, not
an exhaustive inventory of every video ever uploaded. Search descriptions and
unavailable streams are distinguished from files actually downloaded and inspected.

## Downloaded and inspected

| Reference | Media | Finding |
|---|---|---|
| [DashX360: Blades Startup.mp4](https://github.com/ZivvoZ/dashx360/blob/main/Assets/Boot/Blades%20Startup.mp4) | 1920×1080, H.264/AAC, 288 frames at 30 fps; video 9.6 s, container 9.612993 s | Original close-up metal sphere, glowing X, pullback, wordmark and ripple. Selected playback recording; no added fade. |
| [Xenia Dashboard NXE recording](https://github.com/mfrafael/xenia-dashboard/blob/main/skins/nxe/assets/boot.webm) | 1920×1080, VP9/Opus, 60 fps, 8.85 s | Same original-era composition and sequence, with a different ending/fade and duration. Used to cross-check the variant. |
| [Steam Deck startup collection](https://github.com/kageurufu/steamdeck_startup_animations/blob/main/deck_startup/deck_startup.x360.webm) | 1280×800, VP9/Vorbis, about 7.387 s | Later green ribbon startup with letterboxing. Rejected for the selected original-era target. |
| User-supplied `videoplayback.mp4` | 640×360, 54.404 s | Later ribbon startup followed by a Metro dashboard. Kept as the previous reference; not used for this startup. |

The selected source has SHA-256
`49167ff153513d9e3e9d89b80cf63522d67a1bb28f547bd81ff748c338f920a8`
and Git blob `617b003906e131c87c636696efebd219fb0286ef`.
The independent NXE WebM has SHA-256
`d1de7996c54bbfdc99675a1f0db6b7fc4b167d4f4470892d9358ea07ad0b06ff`.

## Located but not inspected as video

- [Original Xbox 360 Startup Animation, 1440p60](https://www.youtube.com/watch?v=yVaK1oGwDUg): the uploader describes an emulator capture of the original executable. Video access was throttled; it was not downloaded or watched here.
- [All Xbox Startup Screens Evolution](https://www.youtube.com/watch?v=EPClnDFPmpU): search metadata distinguishes the 2005 and 2010 variants; stream not inspected.
- [Ultimate Xbox Startup compilation](https://www.youtube.com/watch?v=yVgamboqH7c): located through search; stream not inspected.
- [Original Startup on Make a GIF](https://makeagif.com/gif/original-xbox-360-startup-1080p-RFReXD): public media download returned HTTP 403.
- [Original Xbox 360 Boot on Steam Deck Repo](https://steamdeckrepo.com/post/ER3bL/xbox_360_boot_original): located; page supplied no inspectable media.
- [TCRF startup comparison](https://tcrf.tiago.zip/wiki/Xbox_360): article located; linked MP4 download returned HTTP 403.
- [Original Startup on Coub](https://coub.com/view/2fbgue): located through search; stream not inspected.

## Playback and fidelity

The app plays the selected recording rather than drawing an approximate logo.
All 288 source frames are converted to logical 1280×720 with Lanczos scaling,
without cropping, recoloring or overlays, then encoded losslessly as WebP.
Every encoded frame is decoded and compared with the resized source RGB pixels.
`projects/cards/romfs/boot/nxe360.json` records the source/bank hashes and each
frame's RGB hash. The source recording and conversion script are included.
The recorded audio is preserved as 44.1 kHz stereo PCM for the existing mixer.

The clip uses a real-time clock, not HomeNX's animation timer. Background music
waits until it ends. A new A/B press skips it and stops its sound. Buttons held while launching
are ignored until released. The dedicated Startup animation and sound settings
are respected; UI transition animations are a separate preference. Returning HOME from software does not replay it.
Missing/corrupt media fails open to the dashboard rather than trapping HOME.

This matches the supplied recording's imagery and timing at 720p. Its capture
chain, the original console's rendering, display calibration, actual Switch
frame pacing, and exact audiovisual latency have not been verified on hardware.
Frames can be skipped to maintain sound synchronization if decoding falls behind.
No claim of pixel-perfect equivalence to every Xbox 360 recording is made.

## Streaming update

Frames are now read individually from the bank. The resident header/audio is
1,697,018 bytes, and the largest encoded frame is 414,166 bytes, instead of
preloading the entire 55,967,192-byte recording bank. A regression check reads
all 288 frames through the same streaming reader and compares their encoded
bytes with the bank. Album/applet mode has limited memory; full-memory homebrew
launch remains recommended. Playback errors are reported rather than silently
skipping. Settings > Look > Preview startup allows an immediate playback check.
