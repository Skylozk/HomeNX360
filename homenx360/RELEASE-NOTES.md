# HomeNX360 Discord Friends preview

Adds a Friends blade to the NXE Guide with Discord direct and group DMs,
unread markers, and paging through the full inbox. A or touch opens a
conversation using HomeNX's existing chat reader; A/X in chat opens the reply
keyboard. B returns to the DM list and then the Guide. X in Friends opens
Discord sign-in/account controls. Quick Launch Friends and Messages route here.

This view hides server navigation and restricts conversations to DMs. The
ordinary Discord entry elsewhere in HomeNX retains its server navigation.
Replies check the account, connection and destination again when submitted.
Empty/canceled replies enqueue nothing. Existing sending/failed indicators are
retained. The recorded startup and full-screen NXE settings are unchanged.

**Install HomeNX360-update.zip for this feature**, following INSTALL.md and
backing up your existing installation first. Integrated Discord requires the
HOME frontend. The standalone preview NRO shows the HOME requirement and an
external Discord app shortcut; it does not embed the Discord backend.

Host tests cover the production Guide routing and DM helpers in both app/HOME
configurations: paging, unread/group DMs, stable selection, removed conversations,
account changes, server exclusion, and explicit keyboard-submit routing.
All six blades and their touch targets are checked. No live account was accessed
and no test messages were sent. Source, checksums and DISCORD-GUIDE.md are included.

Live login/delivery and physical Switch behavior remain unverified here.
The added sixth Friends blade is a requested extension to the NXE Guide.
Chat/sign-in use HomeNX's existing Discord interface. Fonts and animation
fidelity limits from earlier releases still apply.
