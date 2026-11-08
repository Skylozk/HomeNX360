# NXE wallpapers

Open **System Settings → HomeNX Preferences → Dashboard → Change Theme**.
Move with the D-pad or left stick, press A to apply, B to cancel, or X to pick a
random wallpaper. The first tile, **Classic NXE**, restores the original green
background. Tapping a thumbnail applies it. The choice is saved automatically.

Eleven wallpapers are included: NXE Night; NXE Rings in green, orange, blue and
purple; Halo 3 Mythic; Halo 3 Splatter Spree; Gears 2 Destroyed Beauty;
Banjo-Kazooie; Fable II Albion; and Winter Holiday.

The game wallpapers are original image bytes from the community-preserved Xbox
360 Themes Archive. Night is a community-preserved NXE background. The four ring
variants come from the Pegasus NPE community theme; they are not presented as
unmodified Microsoft default themes. See the included **SOURCES.json** and
**WALLPAPER-MEDIA-NOTICE.md** for source links and SHA-256 hashes.

These are static wallpapers. Premium theme props, animated backgrounds and the
Xbox renderer are not included. The dashboard preserves each image's horizon and
adds light shading behind navigation/footer text for readability. The existing
classic background remains the default until you choose another one.

For your own pictures, use **Dashboard → Background**. Put JPG/PNG pictures in
`sd:/config/CardsNX/backgrounds/` or choose a recent screenshot there. This picker
is separate from the included collection. The selected background is shared by
the preview and HOME through CardsNX settings. Built-in choices use stable IDs,
so the selection works in both launch modes.

The preview NRO embeds all eleven wallpapers and works offline. The full update
also installs them under `sd:/switch/SwitchU/cards/wallpapers/` for HOME. Launch
**HomeNX360-Updater.nro** to update both installations, or extract the complete
**HomeNX360-update.zip** to the SD root. Restart after updating HOME. Your saved
avatars and settings are preserved. The separate avatar catalog is still needed
for the expanded wardrobe when manually installing only the preview NRO.

Native builds, image decoding, saved-setting round trips, settings navigation,
asset hashes and package placement are checked. Physical Switch rendering and
firmware compatibility remain unverified here.
