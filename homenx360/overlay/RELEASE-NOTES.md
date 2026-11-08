# HomeNX360 v0.8.2-preview.1 — NXE wallpapers

Adds eleven offline wallpapers with thumbnail previews under **System Settings →
HomeNX Preferences → Dashboard → Change Theme**. Includes NXE Night, four NXE
ring variants, Halo 3 Mythic and Splatter Spree, Gears 2 Destroyed Beauty,
Banjo-Kazooie, Fable II Albion and Winter Holiday. Choose Classic NXE to restore
the default, or Background for your own pictures. Selections save across launches
and work in both preview and HOME.

Fixes the NXE renderer ignoring the saved wallpaper and a pending image decode
restoring an old picture after the default was selected. Wallpapers preserve
their own horizon, with navigation shading for readability.

The preview embeds every wallpaper; the full update installs matching HOME media.
Source URLs and hashes accompany the media. These are static backgrounds; premium
3D scenes and animation are not reproduced. See WALLPAPER-GUIDE.md for controls.

Retains the automatic updater and expanded avatars:

Adds **HomeNX360-Updater.nro**. Launch it to check this repository's releases
and automatically update both the standalone preview and the HomeNX dashboard.
It preserves settings/avatars, verifies downloads, resumes interrupted downloads
and restores replaced files after reported installation errors. Read
UPDATER-GUIDE.md for standard app locations and the HOME restart requirement.

Also includes the original built-in clothing, hair, accessory and facial-feature catalog,
plus 1,108 working Marketplace items from the supplied archive. There are 1,917
assets, including hat-compatible hair meshes, and 166 original animated props.
The creator has 33 fields, scrolling, ten-item jumps, full-body rotation, colour,
skin tone, face-feature colours, height/build, save/cancel and per-profile appearances.

Costumes replace their covered clothing slots; hats use companion hairstyles.
Original clothing-hiding patches and actual nose/chin/ear geometry changes are
retained. Equipped items load in the background from SD; the large wardrobe is
not embedded into the preview NRO. Legacy saved avatars remain unchanged until
they are edited and saved with the expanded creator.

**Existing HomeNX:** extract **HomeNX360-update.zip** to the SD root.
**Standalone app:** install **HomeNX360-preview.nro** and extract
**HomeNX360-avatar-catalog.zip** to the SD root. The NRO alone does not contain
the expanded wardrobe. Back up `config/CardsNX/`; use full-memory homebrew launch
mode. Read AVATAR-GUIDE.md and INSTALL.md for controls and rollback.

This is the supplied Marketplace collection, not the entire historical store.
One female Star-Lord shirt fails source decompression and is excluded. Simplified
lighting, texture resolution and neutral facial-expression frames differ from
the Xbox renderer; face-position sliders and Xbox Live services are not included.
Hardware rendering, memory use and firmware compatibility require Switch testing.

Original artwork belongs to Microsoft and the respective publishers. Source URLs,
package/file hashes, conversion tools, corresponding source and software-rendered
reference images accompany the release. Existing startup, Guide, settings and
Discord Friends functionality remain covered by regression checks.
