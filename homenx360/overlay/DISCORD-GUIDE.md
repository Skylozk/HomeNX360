# Discord chats inside the NXE Guide

## Open your conversations

1. Install the **HOME frontend** from **HomeNX360-update.zip**. The standalone
   preview NRO does not include HomeNX's Discord service.
2. Press **Y** for the Guide and use **Left/Right or L/R** to select **Friends**.
3. Friends now opens the centered **Chat** screen: silver conversation rows,
   a highlight in your profile's theme colour, and a gray detail pane.
4. Sign in through HomeNX's existing Discord interface if asked. Existing saved
   accounts are reused; this update adds no credential store or login mechanism.
5. Your existing **direct messages and group DMs** appear in the list. Unread
   conversations show an indicator and an Unread label. The right pane describes
   the selected conversation. **More conversations / Previous conversations**
   page through the inbox without a fixed total-conversation cutoff.
6. Press **A** or touch a conversation to open its messages **inside the Guide**.
   B returns to this same conversation list. B again closes the Guide.

## Read and reply

- **A or X:** open the reply keyboard. Explicitly submit text to queue it.
  Canceling or submitting blank text sends nothing.
- **Up / Down:** scroll message history. **L / R:** move a page at a time.
- **ZL**, or the history heading: load an older page. A failed initial history
  request can be retried through the same control. Up to the latest **200 cached
  messages** are displayed; this is not an unlimited full-history browser.
- **ZR:** jump to the newest messages. New arrivals follow automatically when
  already at the bottom. Reading older messages keeps your visible position
  stable; a notice appears when newer messages arrive below.
- **Touch:** drag the message area to scroll, use its arrow controls, or tap
  the Write a message bar to reply.
- **B:** return to chats. **Y:** close the Guide and return to the dashboard.
- **X in the conversation list:** open HomeNX's existing Discord/account panel.

Message rows show author, timestamp, body text and Sending / Failed status.
Queued or Sending does not claim that the recipient received or read a message.
Text preserves line breaks; long words/URLs wrap at Unicode character boundaries.
The Guide reader supports text replies. Image, GIF, sticker and attachment
messages show a placeholder; open the ordinary Discord panel to view those media.
Voice/video calls, Xbox LIVE parties, starting new group chats, friend requests,
message editing and attachment composition are not implemented by this screen.

## Accounts and ordinary Discord

The inbox contains existing Discord private conversations, rather than the
complete friend-relationship list. The normal Discord entry elsewhere in HomeNX
retains its server navigation and original reader. This new Guide screen uses
only direct/group DMs; server channels are excluded by the service and Guide.
Sign-in and account management retain HomeNX's existing interface.

The standalone preview shows a HOME-required explanation and can open the
existing external Discord app shortcut if installed. It does not show fake DMs
or send messages without the HOME service.

## Routing and validation

Visible rows retain channel IDs across presence/unread updates, so changes in
Discord's recent-chat order cannot redirect a selection. Opening a conversation
rechecks the active account and DM membership. Signing out, disconnecting,
changing accounts or removing the conversation returns the reader to the inbox.

The reply keyboard captures the account and channel. Submission rechecks those
values, the currently open conversation, connection state and DM membership.
Messages are queued only on explicit submission. Async history/send completions
are guarded by account, store and service lifetime before updating cached state.
Chat rendering, list refresh, navigation and preview generation do not send chat
messages. Visible newest messages use the existing Discord read acknowledgement.

Host checks cover production open/reply actions with fake identities and a
memory-only send callback, account changes while typing, removed conversations,
server exclusion, groups, empty/cancelled drafts, inbox paging, Unicode and long
messages, layout/touch targets, stable history positions and new-message notices.
The preview pictures contain fictional conversations and are software renders,
not Switch screenshots. No live account was accessed or test message sent.
Live login, delivery and physical Switch behavior still require device testing.
