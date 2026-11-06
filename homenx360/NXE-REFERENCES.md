# NXE reference research

Target: the 2008–2009 New Xbox Experience (NXE). The supplied 54.404-second
`videoplayback.mp4` shows a later Metro dashboard with Home/Social/Settings.
Its startup and menu frames were inspected locally, but it is not evidence for
NXE-specific card movement. External videos below were discovered through web
search. Their video streams were **not downloaded or viewed**: YouTube access
failed, the legacy Xboxdynasty pages returned 404, and the podcast download
redirects were broken or blocked. This list is not exhaustive.

## Video catalog

| Reference | Coverage advertised by its publisher | Access result |
|---|---|---|
| [Xbox 360: The New Xbox Experience – Dashboard](https://www.youtube.com/watch?v=2b0JyOh5m9M) | Dashboard, November 2008 | Search metadata only |
| [New Xbox Experience – Xbox 360 Fall 2008 Dashboard](https://www.youtube.com/watch?v=cELkiUM0q3s) | Fall dashboard | Search metadata only |
| [Xbox 360 NXE Dashboard Tour](https://www.youtube.com/watch?v=0LpAj2kRtgM) | Dashboard tour, October 2008 | Stream unavailable |
| [E3 2008: NXE Dashboard](https://www.youtube.com/watch?v=WbmmnraSy4s) | Early preview | Search metadata only; pre-release layout |
| [New Xbox Experience Guide](https://www.youtube.com/watch?v=NEvJSMt8KsM) | Guide navigation, October 2008 | Video unavailable through tools |
| [AH Guide: NXE walkthrough, part 1](https://www.youtube.com/watch?v=zI8s7UoCw8M) | Feature walkthrough, November 2008 | Search metadata only |
| [Bleeding Edge TV 296](https://www.gearlive.com/video/article/q408-296-new-xbox-experience-definitive-walkthrough-video) | Startup, display options, avatars, Netflix | Publisher article read; legacy M4V/MOV/MP4 downloads unavailable |
| [Xboxdynasty NXE videos](https://www.xboxdynasty.de/news/xbox-360-dashboard/das-neue-xbox-360-dashboard-exklusiv-auf-xboxdynasty/) | My Xbox, Marketplace, avatar creation | Publisher article read; all three linked player pages returned 404 |
| [Official NXE installation video](https://news.xbox.com/en-us/2008/11/14/new-xbox-experience-installing-nxe/) | Installation sequence | Article located; stream not inspected |
| [Joystiq's multi-part video tour](https://www.engadget.com/2008-10-29-video-tour-new-xbox-experience-exposed.html) | Dashboard, Guide, libraries, Marketplace, avatars, parties | Catalog discovered; streams not inspected |

## Primary design references

- [Xbox's launch-day navigation guide](https://news.xbox.com/en-us/2008/11/19/top-10-things-to-do-when-you-get-the-new-xbox-experience/)
  documents the My Xbox channel, a profile in its second slot, themes, Guide,
  library access, and bumper/trigger shortcuts. It supports the channel/carousel
  control model. No timing measurements were possible from this page.
- [NXE UI artist CJ Ramos's portfolio](https://cj-ramos.myportfolio.com/nxe-retail-kiosk-ui)
  supplied indexed design images showing green/gray backgrounds, overlapping
  cards, navigation labels and media libraries. Indexed images were used for
  visual reference; the portfolio page itself returned 403.

## Implemented in this update

- Vertical channels, a receding horizontal card stack, and floor reflections.
- Continuous card focus/scrolling, a 0.30-second vertical channel transition,
  controller repeat and matching trapezoid touch targets. These timings are
  design choices, not measurements of inaccessible videos.
- An original 3.2-second procedural HomeNX startup with green orbiting ribbons;
  A/B skips it, returning HOME does not replay it, and disabling Animations
  removes it. It runs inside HomeNX, after the system boot process.
- A compact Guide with real Switch library, friends, search, settings and power
  actions. Libraries scroll beyond the old ten-item page.

This is an NXE-inspired adaptation, not an exact reproduction of every screen.
Existing HomeNX settings dialogs remain available. Xbox Live, Marketplace,
Netflix, 3D Xbox avatars, achievements and the proprietary Xbox startup audio
are not recreated. No third-party videos, extracted assets or Xbox firmware
are distributed in the update. Additional uploaded NXE clips can supply actual
frame-by-frame motion evidence for later refinement.
