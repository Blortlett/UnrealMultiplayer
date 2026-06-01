// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_GameInstance.h"
#include <Online/OnlineSessionNames.h>

void UNG_GameInstance::Init()
{
	Super::Init();

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

bool UNG_GameInstance::HostSession(TSharedPtr<const FUniqueNetID> _userID, FName _sessionName, bool _bIsLAN, bool _bIsPresence, int32 _maxNumPlayers)
{
	if (!SessionInterface.IsValid() || !_userID.IsValid()) return false;

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

	return SessionSettings->CreateSession(*_userId, _sessionName, *SessionSettings);
}

void UNG_GameInstance::OnCreateSessionComplete(FName _sessionName, bool _bSuccess)
{

}

void UNG_GameInstance::OnFindSessionsComplete(bool _bSuccess)
{

}

void UNG_GameInstance::OnJoinSessionComplete(FName _sessionName, EOnJoinSessionCompleteResult::Type _result)
{

}

void UNG_GameInstance::OnDestroySessionComplete(FName _sessionName, bool _bSuccess)
{

}
