# Main Menu — Editor Setup Guide

The C++ is done and compiles clean. This guide wires it up in the editor and makes the menu
look good. Open the editor, let it hot-reload the new code, then follow along.

## What the C++ now gives Blueprint (on `NG_GameInstance`)

**Callable functions** (drag off a `Cast To NG_GameInstance` node):
- `Start Game (LAN: bool, Server Name: string)` — host. The name is what others see in the list.
- `Find Games (LAN: bool)` — kick off a search.
- `Join Game (Server Index: int)` — join a row by its `SearchIndex`.
- `Destroy Session` — leave / tear down.

**Events to bind** (Assignable delegates — use "Bind Event to ..." or red event nodes):
- `On Sessions Found (Results: Array of NG_SessionInfo)` — fires when a search finishes. Rebuild the list here.
- `On Search Started ()` — fires the instant a search begins. Show a spinner / "Searching…".
- `On Join Failed (Reason: string)` — fires if a join can't complete. Show the message.

**Struct `NG_SessionInfo`** (one per row): `ServerName`, `SearchIndex`, `CurrentPlayers`, `MaxPlayers`, `PingMs`.

> ⚠️ Keep the **LAN checkbox value identical** for both Host (`Start Game`) and Find (`Find Games`).
> A host on LAN won't be found by a non-LAN search and vice-versa.

---

## Part A — Build a server-row widget (`WBP_ServerRow`)

This is one clickable row in the list.

1. **Content/UI → right-click → User Interface → Widget Blueprint**, base `UserWidget`, name `WBP_ServerRow`.
2. Hierarchy: root `Button` named `Btn_Row` → child `Horizontal Box` containing three `TextBlock`s:
   `Txt_Name`, `Txt_Players`, `Txt_Ping`. (Set each Text Block to **Is Variable**.)
3. **Graph → Variables**: add an `Integer` named `SearchIndex` and set it **Instance Editable** + **Expose on Spawn**.
   Optionally add a `String ServerName` etc., or just one variable of type `NG_SessionInfo` (cleaner — expose that on spawn instead).
4. Create a function/event **`Setup`** that takes an `NG_SessionInfo` (or read the exposed var), and sets:
   - `Txt_Name.SetText` ← `ServerName`
   - `Txt_Players.SetText` ← `CurrentPlayers` + "/" + `MaxPlayers` (use `Append`/`Format Text`)
   - `Txt_Ping.SetText` ← `PingMs` + " ms"
   - store `SearchIndex` into the row's variable.
5. **`Btn_Row → On Clicked`**: Get Game Instance → Cast To `NG_GameInstance` → `Join Game (SearchIndex)`.
   (You can do this in the row directly — that's the simplest. Alternatively dispatch an event up to the menu.)

**Flash:** give `Btn_Row` a transparent normal style and a colored hover style; add a subtle scale-up
on hover via an animation, or just rely on the button hover tint.

---

## Part B — Wire the main menu (`WBP_MainMenu`)

Layout (suggested): a left panel with **Host** / **Find Servers** / **Quit** buttons + a
`CheckBox Chk_LAN` + an `EditableTextBox Txt_ServerName`; a right panel with a `ScrollBox SB_ServerList`
and a `Throbber`/`CircularThrobber Spinner_Searching` (hidden by default) and a `TextBlock Txt_Status`.

1. **Event Construct**
   - Get Game Instance → Cast To `NG_GameInstance` → **promote the result to a variable** `GI` (saves re-casting).
   - From `GI`: **Bind Event to On Sessions Found** → custom event `HandleSessionsFound`.
   - **Bind Event to On Search Started** → custom event `HandleSearchStarted`.
   - **Bind Event to On Join Failed** → custom event `HandleJoinFailed`.

2. **Host button → On Clicked**
   - `GI → Start Game`, `LAN` = `Chk_LAN.IsChecked`, `Server Name` = `Txt_ServerName.GetText` → ToString.
   - (The host then ServerTravels automatically via C++.)

3. **Find Servers button → On Clicked**
   - `GI → Find Games`, `LAN` = `Chk_LAN.IsChecked`.

4. **`HandleSearchStarted`**
   - `Spinner_Searching.SetVisibility(Visible)`, `Txt_Status.SetText("Searching…")`,
     `SB_ServerList.ClearChildren`.

5. **`HandleSessionsFound (Results)`**
   - `Spinner_Searching.SetVisibility(Collapsed)`.
   - `SB_ServerList.ClearChildren`.
   - `Branch` on `Results` length == 0 → set `Txt_Status` to "No servers found." and return.
   - else set `Txt_Status` to "" (or "X servers"), then **For Each** `Results`:
     - `Create Widget` class `WBP_ServerRow` (Owning Player = self).
       - If you exposed an `NG_SessionInfo` on spawn, plug the array element straight in.
       - Otherwise call `Setup(element)` after creating.
     - `SB_ServerList → Add Child` (the new row).

6. **`HandleJoinFailed (Reason)`**
   - `Txt_Status.SetText(Reason)` (or pop a small toast). Optionally re-enable buttons.

7. **Quit button** → `Quit Game`.

**Flash ideas (optional, all UMG):**
- A looping background (panning material or a `Background Blur` over a level-render).
- Fade the menu in on Construct with a widget **Animation** (opacity 0→1, ~0.4s).
- Buttons: hover sound + a small translate/scale animation.
- The `Txt_Status` color: white normally, red on join-fail (set color in `HandleJoinFailed`).

---

## Part C — "Waiting for players" overlay (lobby)

The host now travels into `Lvl_ThirdPerson` but the match **waits** until `MinPlayersToStart` (default 2)
players are connected. Show that state. Easiest: add it to `WBP_HUD`.

1. In `WBP_HUD`, add an `Overlay`/`Border` `Panel_Waiting` (full-screen, dim background) with a big
   `TextBlock Txt_Waiting`. Mark `Panel_Waiting` **Is Variable**.
2. Drive visibility with a binding or on Tick:
   - Get Game State → Cast To `NG_GameState` → `Is Waiting For Players`.
   - If true → `Panel_Waiting` Visible; else Collapsed.
3. Set `Txt_Waiting` text via `Format Text`:
   `Waiting for players ({cur}/{min})` where
   - `{cur}` = `NG_GameState → Get Num Connected Players`
   - `{min}` = `NG_GameState → Min Players To Start`.
   (Both are replicated, so each client shows the right numbers.)

When the count reaches the minimum, `ReadyToStartMatch` flips true on the server, `StartMatch()` fires
automatically, `Is Waiting For Players` becomes false, and the overlay collapses on every client.

> To test solo, set `MinPlayersToStart = 1` on the match GameMode (see Part D), then put it back to 2.

---

## Part D — Editor settings to verify

1. **Lvl_ThirdPerson must use the match GameMode.** The global default in `DefaultGame.ini` is
   `BP_ThirdPersonGameMode`, so the level needs an override or the wait/score logic never runs.
   - Open `Lvl_ThirdPerson` → **Window → World Settings → Game Mode → GameMode Override** =
     your match GameMode BP (a Blueprint based on `ANG_GameMode_Match`).
   - If you don't have a BP for it yet: create one (right-click → Blueprint Class → search
     `NG_GameMode_Match`), name it e.g. `BP_GameMode_Match`, and set it as the override.
2. On that GameMode BP (or its Class Defaults), confirm **Min Players To Start** and **Score To Win**.
3. `GameInstanceClass` is already `NG_GameInstance` in `DefaultEngine.ini` — good, the menu can reach it.

---

## Part E — Testing over LAN

Two PIE windows under one editor share a Steam login and usually fail to discover/join. Use **two
standalone instances** instead:

- **Editor:** dropdown next to Play → **Number of Players = 2**, **Net Mode = Play Standalone**,
  and run **New Editor Window** twice — OR just package and run the `.exe` twice (most reliable; it's
  the build you submit anyway).

Flow: Instance 1 → type a server name → check LAN → **Host** → it travels to `Lvl_ThirdPerson` and shows
"Waiting for players (1/2)". Instance 2 → check LAN → **Find Servers** → the row appears → click it →
both land in the level and the match starts at 2 players.

**If the list is empty** (discovery problem): for dev reliability, temporarily set
`DefaultEngine.ini → [OnlineSubsystem] DefaultPlatformService=NULL` (pure LAN/IP, zero Steam
dependency), test, then switch back to `Steam` for the Steam demo. Confirm the LAN flag matches on both
sides first.

**If the list populates but Join does nothing:** that's now surfaced — `On Join Failed` will tell you why,
and the C++ logs the resolved `TravelURL`/result. Check the on-screen message.
