# NXE settings references and functional scope

## References inspected

The target is the original 2008–2010 blue/slate NXE settings interface, not
the white Kinect or Metro settings screen.

- [Ainex reference page](https://ainex-project.github.io/wiki/faq/quickstart/dumping/)
  and its [1280×720 NXE System Settings capture](https://ainex-project.github.io/wiki/faq/quickstart/images/NXESystemSettingsMenu.png):
  inspected directly. Measured title at approximately x95/y67, main panel
  x194/y110/w890/h484, list x209/y125/w423 with 46-pixel pitch, right-hand
  explanation at x665, and A/B prompts at y650. The seven original categories
  and the silver console, family, memory, network, computer, camera and setup
  icon silhouettes were used as references.
- [Plusnet's older dashboard network guide](https://community.plus.net/t5/Library/Xbox-360-How-to-set-up-a-wireless-connection-on-older-dashboard/ba-p/1322572),
  [System Settings image](https://www.plus.net/images/support/broadband/gaming/nxe-02.jpg),
  and [Configure Network image](https://www.plus.net/images/support/broadband/gaming/nxe-04.jpg):
  inspected directly to cross-check the dark panel, green highlight, right-hand
  explanation and original networking flow.
- [NXE 2.0.8507.0 Console Settings photograph](https://i.ebayimg.com/images/g/chUAAOSwkwhlto23/s-l1200.jpg),
  from [this console listing](https://www.ebay.com/itm/355419477497): inspected
  directly. Confirms Display, Audio, Language and Locale, Clock, Startup and
  Shutdown, Auto-Play, Remote Control and System Info, in that order.
- [August 2009 update notes](https://digiex.net/threads/nxe-update-2-0-8495-0-exclusive-early-download.1968/)
  confirm Console Settings was reorganized during NXE. Different NXE builds
  therefore have different submenu arrangements.

## Video research and access limits

Located [MrMario2011's NXE Dashboard Overview](https://www.youtube.com/watch?v=aUibGJebJv0)
and [Xbox 360 NXE Dashboard walkthrough](https://www.youtube.com/watch?v=SwZ-df9oUwk).
YouTube playback/download access returned 403 in this workspace, and the web
reader could not fetch the latter video's playback page. These videos were
located but their frames were **not watched or analyzed**. No claim is made
to have watched every NXE settings video. The visual measurements above come
from the accessible original-interface screenshots, not video thumbnails.
The 0.22-second page movement, 0.24-second entrance and 0.18-second exit are
chosen approximations rather than measured Xbox animation timings.

## Functional mapping

Every category opens a page. Existing HomeNX setting getters, actions, change
callbacks and persistence remain responsible for the underlying functions.

| NXE category | Switch / HomeNX behavior |
|---|---|
| Console Settings | Eight original category names and order, each opening its submenu |
| Display | TV output, HOME resolution where available, auto-brightness and handheld brightness |
| Audio | Real music and effects volume, shuffle/repeat, Bluetooth headphones and speaker controls |
| Language and Locale | HomeNX language; console/account region is explicitly managed by Nintendo |
| Clock | Dashboard clock format; HOME build also offers automatic/manual console time |
| Startup and Shutdown | Recorded original startup on/off and preview, handheld/TV sleep timers, real power controls |
| Auto-Play | Software launch animation and HOME launch preferences; Xbox disc auto-play explicitly unavailable |
| Remote Control | Bluetooth, pairing, button mapping, vibration, wired controller controls and available HomeNX tests |
| System Info | Actual Switch firmware, Atmosphere, software counts, storage and HomeNX build information |
| Family Settings | Real profile selection and HomeNX lock-screen controls; Nintendo parental restrictions are not simulated |
| Memory | Console/microSD free space and existing installed-software transfer workflow |
| Network Settings | Actual Wi-Fi switch, network/IP information and connection test; HOME can open Nintendo's connection applet |
| Computers | Real USB MTP file transfer, rather than Xbox Windows Media Center pairing |
| Xbox LIVE Vision | Opens an explicit compatibility information page; no fake camera or Xbox LIVE connection |
| Initial Setup | Profile, language, clock, network and startup shortcuts; does not reset the console |
| HomeNX Preferences | Added eighth root entry preserving dashboard, games/apps, notifications, pictures, lock and update tools |

Up/Down and either stick select rows. A selects/changes, Left/Right adjusts
values, B returns to the previous page and row, and Plus closes settings.
Controller repeat, animated focus, scroll and opening/closing are supported.
Touching a category opens it. Touching a different value only focuses it;
touch it again or use the explicit left/right controls to change it. A/B
footer targets work. Read-only information remains read-only.

Only controls that apply to this NXE shell are included; old CardsNX-only bar,
glass/background and tab-order controls are omitted. The upstream HomeNX
updater still installs upstream HomeNX and can replace this customization.

## Fidelity and validation limits

Selawik substitutes the original Xbox font. The circular background, silver
icons and Xbox-style orb are new vector adaptations, not extracted Xbox system
resources. The host preview tool shares native scene primitives, but its SVG
text rasterization is not a Switch framebuffer capture. Nintendo applets and
existing HomeNX tool dialogs retain their own interfaces. The Switch has no
Xbox 360 disc drive, Vision camera service or Xbox LIVE stack.

Host tests compile the **production page builder and input handlers** against
callback fixtures. They cover all 23 pages, both preview/HOME availability,
every category link, saved callback ownership, shared setting values, back
history, actual controller and touch handlers, and long-list scrolling bounds.
These tests do not exercise real console services or establish physical Switch
compatibility. No pixel-perfect hardware comparison has been performed.
