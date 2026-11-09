# HomeNX360 v0.10.0-preview.1 — per-profile themes and colour repair

Fixes the theme colour control staying green. White, Blue, Green, Pink, Orange,
Purple and Gold now change the dashboard, Guide, settings and avatar editor
highlights immediately. The classic NXE background follows the selected colour.
Wallpaper images, avatar clothing and controller button colours stay original.

Colour and wallpaper are saved separately for each real Switch profile and
restored at sign-in, including after restarting. Preview and HOME share these
saved settings. Existing global colour/wallpaper choices are preserved as the
initial default for profiles without a saved theme; index zero remains White.

Open **System Settings → HomeNX Preferences → Dashboard → Theme colour**.
**Theme profile** identifies the selected profile; **Change Theme** chooses an
included wallpaper, and **Background** chooses your own picture. **Reset profile
theme** restores green/classic for the selected profile only. Other preferences
remain shared. See WALLPAPER-GUIDE.md for details.

Retains the v0.9.1 updater staging repair and progress/error screen, category-based
avatar editor with 2,420 thumbnails, full wardrobe, branding, startup and eleven
wallpapers. If your old updater reports **File exists**, manually replace
**sd:/switch/HomeNX360-Updater/HomeNX360-Updater.nro** with the NRO from this
release first. Then launch it to update both preview and HOME. Alternatively,
extract **HomeNX360-update.zip** to the SD root. Restart after updating HOME.

Checks cover legacy migration, two-profile switching, save/restart restoration,
new-profile defaults, shared defaults, malformed entries, account limits and
production settings scene colours. Native app/HOME builds and existing
navigation/avatar/updater checks are required by the release build. Physical
Switch rendering and firmware compatibility remain unverified here.
