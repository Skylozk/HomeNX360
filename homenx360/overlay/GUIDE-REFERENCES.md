# NXE Guide references and validation

The user requested the original NXE-era Xbox 360 Guide. This update replaces the
previous tall left-hand shortcut panel with the centered five-blade layout.

## Images inspected

The principal reference is the [original NXE Guide screenshot published with
Rodrigo Copetti's Xbox 360 analysis](https://www.copetti.org/writings/consoles/xbox-360/#the-new-xbox-experience-2008):
[direct image](https://www.copetti.org/images/consoles/xbox360/screenshots/nxe/guide.3199fe644a358860acfba56768106022a6312f587ab6f6b45098e97fecb73c26.jpg).
The article attributes its NXE captures to Jacob R TWO and Stubbertville.
The inspected image shows Stubbertville's offline Guide, including the music
player behind it and a Halo 3: ODST play shortcut.

Measured against that 1276×720 image, the native 1280×720 layout uses:

- A 548×316 slate-blue active panel at y=200, profile panel x=362.
- Four narrow silver blades at approximately 38 pixels each, arranged around
  the selected Marketplace, Games, Profile, Media or Settings blade.
- A 472×41 green highlight and 43-pixel row spacing, starting at y=212.
- A vertical profile label centered near y=355; avatar and clock above.
- A/B/X/Y prompts below, with the original colored circular button treatment.

An [Xenia Dashboard screenshot](https://github.com/mfrafael/xenia-dashboard/blob/main/docs/screenshots/nxe-guide.jpg)
was also downloaded and inspected. It showed a different simplified popup and
was rejected as the visual target. The current Metro Guide was distinguished
from the blue NXE design in the article's comparison images/search results.

## Videos researched

- [Xbox 360 NXE dashboard, 2.0.8498.0, by Stubbertville](https://www.youtube.com/watch?v=ucJoSC29UL8): the article's cited capture source.
- [Exploring the Xbox 360 NXE dashboard in 2021, by Jacob R TWO](https://www.youtube.com/watch?v=4Twtx8vC3fY): the article's other cited source.
- [New Xbox Experience Guide, ApFaqTech](https://www.youtube.com/watch?v=NEvJSMt8KsM): period Guide walkthrough located through search.
- [PluginUI NXE Guide demonstration](https://www.reddit.com/r/360hacks/comments/1o7qukm/pluginui_nxe_guide_beta_progress_update/): a modern console restoration project, located through search; its video endpoint returned HTTP 403.

These video streams were not downloaded or viewed here. No animation timings
are claimed to have been measured from them. Panel opening/closing (0.18 s),
blade switching (0.16 s) and row easing are chosen approximations.

## Switch behavior

Your real Switch profile name, image, clock and battery replace the sample
account and hardware values. Marketplace entries route to Nintendo eShop or
installed media apps and say so. Xbox LIVE shows an unavailable message;
no Xbox sign-in, friends, party service or gamerscore is fabricated.

Quick Launch gives access to games, homebrew, pins, search, friends and
notifications. The Games blade opens existing HomeNX library and achievement
features. Media controls operate HomeNX's actual music player. Settings routes
to profile, preferences, system, storage, audio, network and power functions.
These action labels and destinations adapt the Guide to Switch capabilities;
not every original Xbox service or submenu is reproduced.

Y opens the Guide; inside it, Left/Right or L/R changes blades, Up/Down moves,
A selects, B returns/closes, X opens Switch profiles and Y returns to the
Dashboard. Plus also closes the Guide. Touch follows the same animated panel
positions and row/blade rectangles.

The shapes, proportions and sampled panel/highlight colors follow the reference.
Selawik is a font substitute under the SIL Open Font License, not the console's
original font. Physical Switch rendering, frame pacing and an exact pixel
comparison with Xbox hardware have not been performed.
