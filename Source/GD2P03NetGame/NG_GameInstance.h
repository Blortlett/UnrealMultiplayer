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

	UFUNCTION()
	void NetworkFailureHappened(UWorld* _world, UNetDriver* _netDriver, ENetworkFailure::Type _failureType, const FString& _errorString);

	UFUNCTION(BlueprintCallable)
	void StartGame(bool _bLAN);

	UFUNCTION(BlueprintCallable)
	void FindGames(bool _bLAN);

	UFUNCTION(BlueprintCallable)
	void JoinGame(int _iServerIndex);

	UFUNCTION(BlueprintCallable)
	void DestroySession();

	bool HostSession(TSharedPtr<const FUniqueNetId> _userId, FName _sessionName, bool _bIsLAN, bool _bIsPresence, int32 _maxNumPlayers);
	void FindSessions(TSharedPtr <const FUniqueNetId> _userId, bool _bIsLan, bool _bIsPresence);
	bool JoinSession(TSharedPtr<const FUniqueNetId> _userId, FName _sessionName, const FOnlineSessionSearchResult& _searchResult);

public:

	UPROPERTY(BlueprintReadWrite)
	FString FindSessionResultText = "";

	UPROPERTY(BlueprintReadWrite)
	FString NumSearchResultText = "";

	UPROPERTY(BlueprintReadWrite)
	FString JoinSessionResultText = "";
};
