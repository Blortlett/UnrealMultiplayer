// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_GameInstance.h"
#include <Online/OnlineSessionNames.h>

void UNG_GameInstance::Init()
{
	Super::Init();

	GEngine->OnNetworkFailure().AddUObject(this, &UNG_GameInstance::NetworkFailureHappened);

	if (IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get())
	{
		SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNG_GameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNG_GameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UNG_GameInstance::OnJoinSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UNG_GameInstance::OnDestroySessionComplete);
		}
	}
}

void UNG_GameInstance::NetworkFailureHappened(UWorld* _world, UNetDriver* _netDriver, ENetworkFailure::Type _failureType, const FString& _errorString)
{
	DestroySession();
}

void UNG_GameInstance::StartGame(bool _bLAN, const FString& _serverName)
{
	ULocalPlayer* const Player = GetFirstGamePlayer();

	const FString ServerName = _serverName.IsEmpty() ? FString(TEXT("John's Server")) : _serverName;

	HostSession(Player->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, _bLAN, true, 4, ServerName);
}

void UNG_GameInstance::FindGames(bool _bLAN)
{
	ULocalPlayer* const Player = GetFirstGamePlayer();

	FindSessions(Player->GetPreferredUniqueNetId().GetUniqueNetId(), _bLAN, true);
}

void UNG_GameInstance::JoinGame(int _iServerIndex)
{
	ULocalPlayer* const Player = GetFirstGamePlayer();

	if (SessionSearch.IsValid() && SessionSearch->SearchResults.IsValidIndex(_iServerIndex))
	{
		const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[_iServerIndex];

		JoinFoundSession(Player->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, SearchResult);
	}
	else
	{
		OnJoinFailed.Broadcast(TEXT("That server is no longer available."));
	}
}

bool UNG_GameInstance::HostSession(TSharedPtr<const FUniqueNetId> _userId, FName _sessionName, bool _bIsLAN, bool _bIsPresence, int32 _maxNumPlayers, const FString& _serverName)
{
	if (!SessionInterface.IsValid() || !_userId.IsValid()) return false;

	SessionSettings = MakeShareable(new FOnlineSessionSettings());

	SessionSettings->bIsLANMatch = _bIsLAN;
	SessionSettings->bUsesPresence = _bIsPresence;
	SessionSettings->NumPublicConnections = _maxNumPlayers;
	SessionSettings->NumPrivateConnections = 0;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->Set(SEARCH_KEYWORDS, FString("JohnsGame"), EOnlineDataAdvertisementType::ViaOnlineService);

	SessionSettings->Set(FName("SERVER_NAME"), _serverName, EOnlineDataAdvertisementType::ViaOnlineService);

	SessionSettings->Set(SETTING_MAPNAME, FString("Lvl_ThirdPerson"), EOnlineDataAdvertisementType::ViaOnlineService);

	return SessionInterface->CreateSession(*_userId, _sessionName, *SessionSettings);
}

void UNG_GameInstance::OnCreateSessionComplete(FName _sessionName, bool _bSuccess)
{
	if (_bSuccess)
	{
		FString LevelOptions = "Lvl_ThirdPerson?listen";
		if (SessionSettings->bIsLANMatch) LevelOptions.Append("?bIsLanMatch=1");
		GetWorld()->ServerTravel(LevelOptions);
	}
}

void UNG_GameInstance::FindSessions(TSharedPtr<const FUniqueNetId> _userId, bool _bIsLan, bool _bIsPresence)
{
	if (!SessionInterface.IsValid() || !_userId.IsValid())
	{
		OnFindSessionsComplete(false);
		return;
	}

	// Tell the UI a search is underway (drive a spinner / "Searching..." label).
	OnSearchStarted.Broadcast();

	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	SessionSearch->bIsLanQuery = _bIsLan;
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->PingBucketSize = 50;
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, FString("JohnsGame"), EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(*_userId, SessionSearch.ToSharedRef());
}

bool UNG_GameInstance::JoinFoundSession(TSharedPtr<const FUniqueNetId> _userId, FName _sessionName, const FOnlineSessionSearchResult& _searchResult)
{
	if (!SessionInterface.IsValid() || !_userId.IsValid())
	{
		OnJoinFailed.Broadcast(TEXT("Online subsystem unavailable."));
		return false;
	}

	return SessionInterface->JoinSession(*_userId, _sessionName, _searchResult);
}

void UNG_GameInstance::OnFindSessionsComplete(bool _bSuccess)
{
	// Build a Blueprint-readable result list and hand it to the menu so it can
	// render a server browser and know which index to join.
	TArray<FNG_SessionInfo> Results;

	if (_bSuccess && SessionSearch.IsValid())
	{
		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
		{
			const FOnlineSessionSearchResult& R = SessionSearch->SearchResults[i];

			FNG_SessionInfo Info;
			Info.SearchIndex = i;

			R.Session.SessionSettings.Get(FName("SERVER_NAME"), Info.ServerName);
			if (Info.ServerName.IsEmpty())
			{
				Info.ServerName = FString::Printf(TEXT("Server %d"), i + 1);
			}

			Info.MaxPlayers     = R.Session.SessionSettings.NumPublicConnections;
			Info.CurrentPlayers = Info.MaxPlayers - R.Session.NumOpenPublicConnections;
			Info.PingMs         = R.PingInMs;

			Results.Add(Info);
		}
	}

	NumSearchResultText = FString::Printf(TEXT("Num Search Results: %d"), Results.Num());

	OnSessionsFound.Broadcast(Results);
}

void UNG_GameInstance::OnJoinSessionComplete(FName _sessionName, EOnJoinSessionCompleteResult::Type _result)
{
	JoinSessionResultText = FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *_sessionName.ToString(), static_cast<int32>(_result));

	if (!SessionInterface.IsValid())
	{
		OnJoinFailed.Broadcast(TEXT("Online subsystem unavailable."));
		return;
	}

	if (_result != EOnJoinSessionCompleteResult::Success)
	{
		FString Reason;
		switch (_result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:        Reason = TEXT("That server is full."); break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:  Reason = TEXT("That server no longer exists."); break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:     Reason = TEXT("Already in a session."); break;
		default:                                                 Reason = TEXT("Could not join the session."); break;
		}
		OnJoinFailed.Broadcast(Reason);
		return;
	}

	APlayerController* const PlayerController = GetFirstLocalPlayerController();

	FString TravelURL;

	if (PlayerController && SessionInterface->GetResolvedConnectString(_sessionName, TravelURL))
	{
		PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		OnJoinFailed.Broadcast(TEXT("Could not resolve the server address."));
	}
}

void UNG_GameInstance::DestroySession()
{
	if (!SessionInterface.IsValid()) return;

	SessionInterface->DestroySession(NAME_GameSession);
}

void UNG_GameInstance::OnDestroySessionComplete(FName _sessionName, bool _bSuccess)
{
	if (!SessionInterface.IsValid()) return;

	UGameplayStatics::OpenLevel(GetWorld(), FName("L_MainMenu", true));
}
