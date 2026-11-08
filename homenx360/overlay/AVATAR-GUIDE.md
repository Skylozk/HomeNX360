# Full-body profiles and expanded avatar creator

Open Profiles from My Xbox or the Guide. A signs into the selected real Switch
profile; X edits its avatar; Y opens Nintendo user management; B returns. The
NXE-style row shows full-body characters and the profiles' actual names. Touch,
swipe, and the existing startup sign-in setting are supported.

## Included catalog

This update contains 809 original built-in assets, including 89 hat-compatible
hair companions, plus 1,108 working Marketplace items from the collection supplied
by the user. There are 1,917 catalog entries / 1,828 directly selectable assets,
before gender filtering and the additional None/Default choices. The 166 props
include three built-in props and 163 Marketplace props.

Independent options include hairstyles, tops, bottoms, shoes, outfits/costumes,
hats, gloves, glasses, wristwear, earrings, rings, eyes, eyebrows, mouths, facial
hair, face features/paint, eye shadow, noses, chins, ears, and props. The creator
also has male/female bodies, 24 skin tones, 32 hair/feature colours, and five
height/build settings. Costume slots hide the clothes or hair they replace;
editing a covered individual slot removes the conflicting costume. Hats select
the original companion hair meshes. Clothes retain their body-hiding patches.

The source archive contains 1,109 Marketplace packages. One female Star-Lord
T-shirt (`00000008-1e75-92c2-cc55-01ed524607d4`) fails LZX decompression and is
excluded from the menus. Its name and failure are retained in `catalog.json`.
This collection is a subset of the historical Marketplace, not its entire
worldwide catalog. No missing item is represented by a nonfunctional menu entry.

## Controls and saved appearances

The NXE-style editor has seven category tabs: **Headwear, Hair, Clothes,
Accessories, Face, Body, and Props**. All 33 customization fields remain available.
Eight item thumbnails surround the live avatar, with pages for the full collection.
The 2,420 thumbnail files use the original meshes, face masks and native lighting;
items supporting both body types have matching male/female previews.

- **L/R:** previous/next category tab.
- **Y / Minus:** next/previous option within a category, such as hairstyle/hair
  colour or shirt/trousers/shoes/costume. On the single-option Headwear and Props
  tabs these buttons switch categories; the footer says Next tab.
- **D-pad / left stick:** browse the two columns of item previews.
- **ZL/ZR:** previous/next page of eight choices.
- **A:** use the item currently being tried on in the draft.
- **X:** use the current preview and save the complete avatar to this profile.
- **B:** cancel the editor and keep the previously saved appearance.
- **Plus:** randomize the draft.
- **Right stick left/right:** rotate. Press the right stick to toggle full-body
  and close-up views. Tall hats adjust the full-body framing automatically.

Browsing only tries an item on the central avatar; A accepts it into the draft.
Changing category cancels an unaccepted try-on. X includes the current try-on when
saving. Tap a tab or field arrows to navigate. Tap an item once to preview and
again to use it. Drag the avatar to rotate; page arrows and footer controls are
also touchable. Colours show swatches. Height/build show shape indicators.

Thumbnail pictures show a neutral sample avatar; the live character shows your
own skin/hair colours and current clothes. The last ready live avatar stays visible
while a new item loads, with a short Updating preview label.

Existing six-starter appearances remain unchanged until edited and saved. Opening
an old appearance in the expanded creator starts a new draft with its body type;
Cancel restores the saved starter. New saves use stable catalog asset IDs, per
full Nintendo profile UID. Editing another user's appearance does not sign into
that user. Nintendo names, pictures, Miis and saved games are unchanged.
Back up `config/CardsNX/` before updating or rolling back.

## Installation

The large wardrobe is stored outside the NRO at:

`sd:/switch/SwitchU/cards/avatars/catalog/`

Extract the complete **HomeNX360-update.zip** to the SD root for an existing
HomeNX installation. To try only the standalone app, install
**HomeNX360.nro** (the older HomeNX360-preview.nro filename is also supplied) and extract **HomeNX360-avatar-catalog.zip** to the SD
root. Both app and HOME use the same catalog. A lone NRO retains the old starters
but needs these SD files for the expanded creator. Use full-memory homebrew
launch mode for this larger creator; applet-mode memory and physical Switch
rendering have not been verified here.

Only equipped catalog meshes/masks are loaded, with one background conversion
job, a bounded appearance cache and one GPU texture upload per frame. A loading
label appears while a new outfit loads; failed files cannot be saved as a valid
preview. The full catalog is never loaded into GPU memory at once. Item pictures use a
separate bounded cache, with only the visible eight requested, and stay on SD
rather than inside the preview NRO.

## Rendering and source

Original artwork comes from the public Xbox system-update avatar packs and the
user-selected archived Marketplace collection. The six legacy starters remain
credited to exports through Ezra Kruger's [360sona](https://360sona.com/).
Original media rights belong to Microsoft and the respective item publishers;
the code license does not grant rights to that artwork.

Native meshes keep their skin weights and original face/clothing textures.
The Switch renderer uses simplified lighting, textures up to 384 pixels,
15 Hz interpolated skeletal clips and compact animation storage. Props play
original skeletal performances and are hidden during the sign-in wave. Facial
feature images use their selected neutral frame; original animated expression
tracks, facial positioning sliders, Marketplace purchasing and Xbox Live are not
implemented. This is not a pixel-identical Xbox renderer.

The source includes the converted files, source package hashes, bounded decoders,
offline conversion scripts and native tests. `AVATAR-CATALOG-SOURCES.json`
records the source URLs and hashes. No bundled Windows utility, Xbox firmware
program or account patcher was executed. Reference PNGs use the native CPU
geometry/lighting with a software depth buffer; they are not Switch captures.

To rebuild the media in a prepared source checkout, install Python with numpy,
Pillow and libarchive-c, Rust, Git and libarchive, then use:

```sh
python3 tools/avatar_catalog/fetch_sources.py AVATAR-CATALOG-SOURCES.json sources
git clone https://github.com/panedivitasei/avatar-aura converter
git -C converter checkout 2d2e4a4cfccd795b8ed3e14a19a942c83d298ad5
python3 tools/avatar_catalog/prepare_converter.py converter
cargo build --manifest-path converter/Cargo.toml --release --bin avatarextract
python3 tools/avatar_catalog/import_marketplace.py sources/marketplace.rar closet --converter converter/target/release/avatarextract
python3 tools/avatar_catalog/build_catalog.py --converter converter/target/release/avatarextract --toc sources/AvatarAssetPack.toc --legacy sources/AvatarAssetPackLegacyV1.toc --closet closet --output rebuilt-catalog
python3 tools/avatar_catalog/finalize_catalog.py rebuilt-catalog
```

Inspect the failure inventory before replacing a shipped catalog. The source
padding and large-mesh fixes are applied to the offline converter by the prepare
script. The upstream decoder carries its own Xenia/libmspack notices; it is an
offline tool, not the renderer shipped on Switch.

To rebuild the item previews after regenerating catalog models, compile
`tools/build_avatar_thumbnails.cpp` with C++20, zlib and the bundled nxui include
directory, then run it with the catalog input and output directories. Merge the
`thumb-*.png` files into the flat catalog folder and regenerate SHA256.json.
