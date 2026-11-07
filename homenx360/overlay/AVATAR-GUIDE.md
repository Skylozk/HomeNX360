# Full-body profiles and avatar creator

Open Profiles from My Xbox or the Guide. Each real Switch profile has a
full-body Xbox 360 avatar and its Nintendo profile name on the gray NXE-style
sign-in floor. A selects the profile used to launch games and shows its greeting.
Left/Right changes selection; X edits that profile's avatar; Y opens the existing
Nintendo user-management tools; B returns. The row scrolls for up to eight users.
Touch a character to select it, then touch again to sign in, or swipe the row.

The profile picker also follows the recorded startup on a fresh app launch or
console boot. Returning HOME from a running game keeps the current account.
Console Settings → Startup and Shutdown → Sign-in at startup controls this.

## Creator controls

Up/Down selects a feature, Left/Right or A changes it, L/R rotates the full-body
preview, Y randomizes, X saves, and B cancels the draft. Touch the row's left or
right side to change it; drag the character to rotate. Footer buttons work by
touch too. Saving an avatar for another profile does not sign into that profile.

Six original-style face/body presets are included. Hair, top, trousers and shoes
can be combined within each of the two body families; accessories can be toggled,
and hair colour and height/build adjusted. Hair colour offers Original, Black,
Brown, Blonde, Ginger and Silver. This is a compact custom creator, not Microsoft's
entire avatar catalog or every facial slider. Preset names identify the exported
starter wardrobes. Actual profile names always come from the Switch.
Hat-cut hairstyles keep their matching cap; the accessories switch controls
the starter's glasses, watch and earrings.

Appearances are stored per full Nintendo account UID in CardsNX's existing
settings under `profileAvatars`. They do not overwrite Nintendo profile pictures,
Miis, account names or saved games. Back up `config/CardsNX/` to preserve them.

## Artwork and rendering

These are Xbox 360 avatar meshes, skin weights, textures and animations exported
through the public demo of [360sona](https://360sona.com/), by Ezra Kruger.
[About 360sona](https://360sona.com/about.html) describes the original artwork and
export capability. No account or online renderer is required inside HomeNX360.
Microsoft owns the original avatar artwork and animations; the GPL code license
does not grant rights to those media. See `media/avatars/AVATAR-MEDIA-NOTICE.md`.

The native renderer retains the original mesh detail and uses baked lighting,
384-pixel maximum textures and animation sampled at 15 Hz with interpolation.
It holds the relaxed standing pose between idle gestures and plays the original
greeting after sign-in. The exact four faces/outfits in the user's screenshot
are not all included. Lighting, font metrics and the Switch controls also differ;
this release does not claim a pixel-identical screenshot recreation.
Original hair colour retains the exported surface; optional dyes use a native
colour shader and are approximations of the console's colour treatment. Some
mixed wardrobe/hairstyle combinations may still intersect.

The converter, unchanged exports, native banks and hashes accompany the source.
Reference previews use the native CPU skinning/projection with a software depth
buffer; they are not photos or captures from Switch hardware. Both native builds
and host checks can be verified here; physical Switch rendering remains untested.
