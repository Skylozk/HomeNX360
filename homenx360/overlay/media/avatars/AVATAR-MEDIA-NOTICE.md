# Xbox 360 avatar artwork

Original avatar meshes, textures, rigs and animations are Microsoft material.
They are separate from the project's GPL-3.0 source-code license. Nintendo,
Microsoft and 360sona do not endorse this fork.

Source: https://360sona.com/ (public Try the editor demo → starter avatar →
Export a 3D model). Export generator credit: “Made with 360sona,
https://360sona.com”. 360sona is by Ezra Kruger; its viewer code is closed source
and is not copied into HomeNX360. Background: https://360sona.com/about.html.

`xbox-*.glb` are the unchanged six clothed starter exports, with baked lighting,
embedded PNG surfaces, humanoid bone names and animations. `body-*.glb` are
unchanged exports of the same starters with top/trousers/shoes removed using the
editor's ordinary controls, to retain the complete underlying body geometry.

`tools/prepare_avatar360.py` converts the exports for native rendering.
`tools/prepare_avatar_wardrobe.py` combines clothing-fitted body meshes within each compatible rig family, transfers
body UVs to the selected starter's skin surface, and retains complete calf geometry
for shorts. It retains original meshes and skin appearance,
resamples textures to at most 384 pixels, and samples idle/greeting bone motion
at 15 Hz. The six reports in `projects/cards/romfs/avatars/` contain source and
bank SHA-256 hashes. The software renderer and profile UI are original code.
