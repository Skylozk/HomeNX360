# HomeNX360 v0.11.0-preview.1 — Discord chats inside the NXE Guide

Friends now opens a centered Chat screen based on the supplied NXE reference:
silver conversation rows, your profile's theme-colour selection, a gray detail
pane, unread indicators and private/group DM icons. A opens a conversation's
messages inside the Guide, instead of handing it to the separate Discord reader.

Message history shows authors, timestamps, text and sending/failed states.
A or X opens the reply keyboard; only explicit nonblank submission queues a
reply. B returns to the chat list. Up/Down scroll, L/R page, ZL loads older
history and ZR jumps to the newest messages. Touch scrolling and a reply bar
are included. The reader displays up to the latest 200 cached messages; long
text and URLs wrap without clipping whole words, and new arrivals preserve
your position while reading older messages.

Uses HomeNX's existing Discord sign-in and service. Account/conversation changes
are rechecked on open and reply submission, including changes while the keyboard
is open. Server channels are excluded. Async completions check account identity,
store identity and service lifetime. Attachment/media messages show a placeholder
in this text reader; use the ordinary Discord panel for media and account tools.
Voice calls and Xbox LIVE party services are not added.

**Integrated Discord still requires the HOME frontend.** The standalone preview
shows that requirement and can open an installed external Discord app. See
DISCORD-GUIDE.md for controls and limits. The two Guide preview pictures are
software renders with fictional conversations, not device captures.

Retains the working per-profile colours/wallpapers, avatar editor/catalog,
branding and startup. Includes the tested diagnostic updater changes: I/O errors
identify the actual source/destination operation and last native Switch FS code
when available. These diagnostics do not establish or repair an SD-card fault.

Install **HomeNX360-update.zip** at the SD root and restart for HOME. Manual
installation is available while the user's updater issue remains unverified.
Settings and saved avatar appearances are preserved. The complete update includes
HomeNX360.nro and HomeNX360-Updater.nro; source ZIPs are not install packages.

Native app/HOME builds and host checks cover chat routing, groups, explicit
submission, account/connection changes, Unicode/long messages, paging, anchored
history, unread arrivals and touch geometry. Existing settings, avatar, package
and updater checks remain in CI. No real Discord test messages were sent; live
login/delivery and physical Switch behavior remain unverified here.
