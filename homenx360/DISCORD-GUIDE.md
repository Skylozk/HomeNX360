# Discord DMs in the Guide

## Using Friends

1. Install the **HOME frontend** using `HomeNX360-update.zip` and `INSTALL.md`.
2. Press **Y** to open the Guide, then use **Left/Right or L/R** to select **Friends**.
3. If signed out, choose **Sign in to Discord**. This opens HomeNX's existing
   Discord sign-in/account interface on the console. Existing saved accounts
   are reused. No new login system or credential storage is introduced.
4. Friends shows Discord direct and group DMs. **\*** marks unread conversations.
   **More conversations / Previous conversations** pages through the inbox;
   there is no fixed 30/40-conversation cutoff in this view.
5. Select a DM with **A** or touch. It opens HomeNX's existing chat reader in
   DM-only mode. Server navigation is hidden and server channels cannot be
   opened through this mode.
6. In the chat, **A or X** opens the reply keyboard. Enter text and submit it
   to queue a message. Canceling or submitting blank text sends nothing.
   HomeNX's existing sending/failed indicators show the request's status;
   queued does not mean the recipient has received or read the message.
7. **B** returns from the chat to the DM list, then closes that panel back to
   the Guide. **X in the Friends blade** also opens the DM/account panel.
   The Guide's Quick Launch Friends and Messages shortcuts open Friends too.

The list is the account's existing private conversations, not the complete
Discord friend-relationship list. Starting a brand-new conversation by username,
friend requests, voice calls and media composition are not added here.

## Preview and ordinary Discord

The standalone `.nro` does not embed HomeNX's Discord backend. Its Friends blade
explains that HOME is required and offers the existing external Discord app
shortcut, if SwitchCord is installed. Use the HOME update for integrated DMs.

The normal Discord entry elsewhere in HomeNX keeps its server navigation.
The added Friends mode is restricted to DMs. Nintendo's Friends entry elsewhere
in the dashboard retains its existing behavior.

## Routing and account changes

The Guide caches the rendered rows by channel ID, preserving order across
presence/unread updates so a new message cannot swap the selected person.
Refreshing the Guide clears conversations from a different account. Opening
a conversation rechecks the active account and DM membership. The chat panel
returns to its list if the account changes.

A reply captures its account and channel when the keyboard opens. Submission
rechecks connection state, account identity and DM membership before calling
HomeNX's existing send operation. A change produces an on-screen message to
reopen the conversation. Messages are queued only by explicit keyboard submit.

## Layout and validation

Friends is a requested extension to the original five-blade NXE Guide. The
Guide now has six blades within a centered 738-pixel span; the active panel
remains 548×316, with seven-row maximum and animated blade changes. The extra
blade is not claimed to be an original Xbox feature. Chat and sign-in retain
HomeNX's Discord interface.

Host checks exercise the production DM model, routing and explicit-submit helper,
including empty/signed-out/failed states, 241 conversations, group DMs, unread
updates, duplicate/removed channels, account changes and server exclusion.
The six-blade geometry and every visible row/blade touch target are checked.
These tests use fake identities and an in-memory enqueue callback. No real
account was accessed and no test messages were sent. Live Discord login,
delivery and physical Switch behavior require testing on the user's console.
