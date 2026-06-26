# Main Menu / Sessions — Handoff Plan

Goal: make Host / Find / Join fully work over LAN and satisfy the **Main Menu (10%)** +
**Connection (15%)** rubric. The brief requires: host a server, **search and show found
sessions in a server list**, join a selected session through the menu, and (host) open the
level and **wait for players before starting the match**.

All session code is in `Source/GD2P03NetGame/NG_GameInstance.{h,cpp}`.
Menu widget: `Content/UI/WBP_MainMenu`. HUD: `Content/UI/WBP_HUD`. Menu level: `L_MainMenu`.

---

## Root causes (diagnose in this order)

1. **Found sessions are never surfaced to the UI.** `OnFindSessionsComplete`
   (`NG_GameInstance.cpp:117`) only calls `AddOnScreenDebugMessage`. There is no
   Blueprint-readable result list or event, so the menu cannot build a server list or
   know which index to join. The Join button is likely hardcoded to `JoinGame(0)`.
   → This is BOTH the rubric gap and the diagnostic. Fix it first.

2. **Steam subsystem + test method.** `DefaultEngine.ini:97` uses `DefaultPlatformService=Steam`
   (appid 480). Two PIE instances under one editor share one Steam login and often fail to
   find/join. The LAN flag must also match on host and client.

3. **`JoinSession` hides a base virtual** (build warnings C4263/C4264 in `NG_GameInstance.h:48`).
   Harmless but rename the custom one (e.g. `JoinFoundSession`) to avoid confusion.

---

## Task 1 — Expose the session list to the menu (DO THIS FIRST)

In `NG_GameInstance.h`, add a Blueprint struct + an assignable delegate:

```cpp
USTRUCT(BlueprintType)
struct FNG_SessionInfo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString ServerName;
    UPROPERTY(BlueprintReadOnly) int32   SearchIndex = 0;   // pass this to JoinGame()
    UPROPERTY(BlueprintReadOnly) int32   CurrentPlayers = 0;
    UPROPERTY(BlueprintReadOnly) int32   MaxPlayers = 0;
    UPROPERTY(BlueprintReadOnly) int32   PingMs = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNG_OnSessionsFound, const TArray<FNG_SessionInfo>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNG_OnJoinFailed, FString, Reason);
```

Add `public` members:

```cpp
UPROPERTY(BlueprintAssignable, Category = "Sessions")
FNG_OnSessionsFound OnSessionsFound;

UPROPERTY(BlueprintAssignable, Category = "Sessions")
FNG_OnJoinFailed OnJoinFailed;
```

In `OnFindSessionsComplete` (`NG_GameInstance.cpp:117`), replace the debug-only body with a
real result build + broadcast:

```cpp
TArray<FNG_SessionInfo> Results;
if (SessionSearch.IsValid())
{
    for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
    {
        const FOnlineSessionSearchResult& R = SessionSearch->SearchResults[i];

        FNG_SessionInfo Info;
        Info.SearchIndex = i;
        R.Session.SessionSettings.Get(FName("SERVER_NAME"), Info.ServerName);
        if (Info.ServerName.IsEmpty()) Info.ServerName = FString::Printf(TEXT("Server %d"), i + 1);
        Info.MaxPlayers     = R.Session.SessionSettings.NumPublicConnections;
        Info.CurrentPlayers = Info.MaxPlayers - R.Session.NumOpenPublicConnections;
        Info.PingMs         = R.PingInMs;
        Results.Add(Info);
    }
}
OnSessionsFound.Broadcast(Results);   // menu binds to this to build the list
```

Also broadcast a failure from `OnJoinSessionComplete` when `_result != Success` (so the UI can
show "join failed" instead of silently doing nothing).

### Editor wiring (`WBP_MainMenu`)
- On menu construct: get GameInstance, cast to `NG_GameInstance`, **Bind Event to `OnSessionsFound`**
  (and `OnJoinFailed`).
- "Find Servers" button → `FindGames(bLAN)`.
- In the `OnSessionsFound` handler: clear a ScrollBox, then for each `FNG_SessionInfo` create a
  row widget showing ServerName / players / ping and storing its `SearchIndex`.
- Row "Join" button → `JoinGame(SearchIndex)`.
- Make sure the **LAN checkbox value is the same** passed to both `StartGame` and `FindGames`.

---

## Task 2 — Host waits for players before match start

Brief: "When hosting, the gameplay level should open but wait until other players join before
starting the match." Right now the match starts immediately.

In `ANG_GameMode_Match` (extends `AGameMode`, which has match states):
- Add `UPROPERTY(EditDefaultsOnly) int32 MinPlayersToStart = 2;`
- Override `ReadyToStartMatch_Implementation()` to return true only once
  `NumPlayers >= MinPlayersToStart`. `AGameMode` polls this while in `WaitingToStart` and calls
  `StartMatch()` automatically.
- VERIFY against engine source (`GameMode.cpp`) that `ReadyToStartMatch` is the live hook in
  5.7 before relying on it; the alternative is a host-only "Start Match" button calling a
  server function that calls `StartMatch()`.
- Confirm which GameMode the gameplay level actually uses: `DefaultGame.ini` global default is
  `BP_ThirdPersonGameMode`, so `Lvl_ThirdPerson` must have a **World Settings → GameMode Override**
  to the match mode (a BP based on `ANG_GameMode_Match`). If it doesn't, the match logic never runs.
- Optional polish: show a "Waiting for players (1/2)" HUD/widget while in `WaitingToStart`
  (bind to GameState match state).

---

## Task 3 — Cleanups
- Rename custom `JoinSession` (`NG_GameInstance.h:48`) to `JoinFoundSession` to clear the
  C4263/C4264 warnings (it currently hides `UGameInstance::JoinSession`).
- `OnFindSessionsComplete` guards `if (!SessionInterface.IsValid()) OnFindSessionsComplete(false)`
  call themselves recursively without `return` — tidy those early-out branches.

---

## Testing (LAN)
1. **Use two standalone instances**, not two PIE windows under one editor (Steam/login sharing
   breaks discovery). Either: New Editor Window (Standalone) x2, OR package and run the .exe twice
   (most reliable, and it's the build you submit anyway).
2. Host with LAN checked → confirm it travels to `Lvl_ThirdPerson?listen`.
3. On the 2nd instance, Find with LAN checked → **the server list should now populate** (Task 1).
   - List empty → discovery problem: confirm both use the same LAN flag; for dev reliability
     consider temporarily setting `DefaultPlatformService=NULL` in `DefaultEngine.ini` (LAN/IP
     works with zero Steam dependency), then switch back to Steam for the Steam demo.
   - List populated but Join does nothing → travel/connect-string problem in
     `OnJoinSessionComplete`; log `TravelURL` and the `_result` enum.
4. Join from the list → both players in the same level, match waits then starts at 2 players.

## Submission reminders (separate from menu)
- Remove template **Variants** (`Variant_Combat`/`SideScrolling`/`Platforming`) — still in Content.
- `DefaultGame.ini` build config is **Development**; package **Shipping** for the release zip.
- If submitting the Steam build, add `steam_appid.txt` containing `480` to `Binaries/Win64`.
- Rename `ProjectName` (currently "Third Person Game Template") in `DefaultGame.ini`.

## Already done in the prior session (don't redo)
- HUD local score: `AGD2P03NetGamePlayerController::GetLocalCaptureScore()` (BlueprintPure).
- Control point: 5s-hold scoring gated to one point per force-field down-cycle
  (`GatingForceField` must be wired on the BP_ControlPoint level instance).
