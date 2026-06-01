// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GD2P03NetGame.h"
#include "Engine/GameInstance.h"
#include "NG_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class GD2P03NETGAME_API UNG_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	

protected:

	void OnCreateSessionComplete(FName _sessionName, bool _bSuccess);
	void OnFindSessionsComplete(bool _bSuccess);
	void OnJoinSessionComplete(FName _sessionName, EOnJoinSessionCompleteResult::Type _result);
	void OnDestroySessionComplete(FName _sessionName, bool _bSuccess);

	IOnlineSessionPtr SessionInterface;
	virtual void Init() override;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;

	bool HostSession(TSharedPtr<const FUniqueNetId> _userID, FName _sessionName, bool _bIsLAN, bool _bIsPresence, int32 _maxNumPlayers);
};
