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

void UNG_GameInstance::StartGame(bool _bLAN)
{
	ULocalPlayer* const Player = GetFirstGamePlayer();

	HostSession(Player->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, _bLAN, true, 4);
}

void UNG_GameInstance::FindGames(bool _bLAN)
{
	ULocalPlayer* const Player = GetFirstGamePlayer();

	FindSessions(Player->GetPreferredUniqueNetId().GetUniqueNetId(), _bLAN, true);
}

void UNG_GameInstance::JoinGame(int _iServerIndex)
{
	ULocalPlayer* const Player = GetFirstGamePlayer();

	FOnlineSessionSearchResult SearchResult;

	if (SessionSearch->SearchResults.Num() > _iServerIndex)
	{
		SearchResult = SessionSearch->SearchResults[_iServerIndex];

		JoinSession(Player->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, SearchResult);
	}
}

bool UNG_GameInstance::HostSession(TSharedPtr<const FUniqueNetId> _userId, FName _sessionName, bool _bIsLAN, bool _bIsPresence, int32 _maxNumPlayers)
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
	if (!SessionInterface.IsValid()) OnFindSessionsComplete(false);
	if (!_userId.IsValid()) OnFindSessionsComplete(false);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	SessionSearch->bIsLanQuery = _bIsLan;
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->PingBucketSize = 50;

	SessionInterface->FindSessions(*_userId, SessionSearch.ToSharedRef());
}

bool UNG_GameInstance::JoinSession(TSharedPtr<const FUniqueNetId> _userId, FName _sessionName, const FOnlineSessionSearchResult& _searchResult)
{
	if (!SessionInterface.IsValid()) OnFindSessionsComplete(false);

	return SessionInterface->JoinSession(*_userId, _sessionName, _searchResult);
}

void UNG_GameInstance::OnFindSessionsComplete(bool _bSuccess)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
		FString::Printf(TEXT("FindSessionsComplete bSuccess: %d"), _bSuccess));
	if (!SessionInterface.IsValid()) return;

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
		FString::Printf(TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num()));

	for (size_t i = 0; i < SessionSearch->SearchResults.Num(); i++)
	{
		FString::Printf(TEXT("Session Number: %d | Session Name: %s"), i+1, *SessionSearch->SearchResults[i].Session.OwningUserName);
	}
}

void UNG_GameInstance::OnJoinSessionComplete(FName _sessionName, EOnJoinSessionCompleteResult::Type _result)
{
	if (!SessionInterface.IsValid()) return;

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
		FString::Printf(TEXT("OnJoinSessionComplete $s, %d"), *_sessionName.ToString(), static_cast<int32>(_result)));
	if (!SessionInterface.IsValid()) return;

	APlayerController* const PlayerController = GetFirstLocalPlayerController();

	FString TravelURL;

	if(PlayerController && SessionInterface->GetResolvedConnectString(_sessionName, TravelURL))
	{
		PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
	}
}

void DestroySession()
{
	if (!SessionInterface.IsValid()) return;

	SessionInterface->DestroySession(NAME_GameSession);
}

void UNG_GameInstance::OnDestroySessionComplete(FName _sessionName, bool _bSuccess)
{
	if (!SessionInterface.IsValid()) return;

	UGameplayStatics::OpenLevel(GetWorld(), FName("L_MainMenu", true));
}
