// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GD2P03NetGame.h"
#include "Engine/GameInstance.h"
#include "NG_GameInstance.generated.h"

/**
 * One row of the server browser. Built from an FOnlineSessionSearchResult and
 * handed to Blueprint so the menu can show a list and join by SearchIndex.
 */
USTRUCT(BlueprintType)
struct FNG_SessionInfo
{
	GENERATED_BODY()

	// Display name set by the host (the "SERVER_NAME" session setting).
	UPROPERTY(BlueprintReadOnly, Category = "Sessions")
	FString ServerName;

	// Index into SessionSearch->SearchResults. Pass this straight to JoinGame().
	UPROPERTY(BlueprintReadOnly, Category = "Sessions")
	int32 SearchIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Sessions")
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Sessions")
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Sessions")
	int32 PingMs = 0;
};

// Fired when a search finishes. Bind on the menu to (re)build the server list.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNG_OnSessionsFound, const TArray<FNG_SessionInfo>&, Results);

// Fired the moment a search begins, so the menu can show a "searching..." spinner.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNG_OnSearchStarted);

// Fired when a join attempt fails, so the menu can show why instead of silently doing nothing.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNG_OnJoinFailed, const FString&, Reason);

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

	// Host a new session. _serverName is what shows up in other players' server lists.
	UFUNCTION(BlueprintCallable, Category = "Sessions")
	void StartGame(bool _bLAN, const FString& _serverName);

	UFUNCTION(BlueprintCallable, Category = "Sessions")
	void FindGames(bool _bLAN);

	UFUNCTION(BlueprintCallable, Category = "Sessions")
	void JoinGame(int _iServerIndex);

	UFUNCTION(BlueprintCallable, Category = "Sessions")
	void DestroySession();

	bool HostSession(TSharedPtr<const FUniqueNetId> _userId, FName _sessionName, bool _bIsLAN, bool _bIsPresence, int32 _maxNumPlayers, const FString& _serverName);
	void FindSessions(TSharedPtr <const FUniqueNetId> _userId, bool _bIsLan, bool _bIsPresence);
	bool JoinFoundSession(TSharedPtr<const FUniqueNetId> _userId, FName _sessionName, const FOnlineSessionSearchResult& _searchResult);

public:

	// Bind these on WBP_MainMenu to drive the server browser UI.
	UPROPERTY(BlueprintAssignable, Category = "Sessions")
	FNG_OnSessionsFound OnSessionsFound;

	UPROPERTY(BlueprintAssignable, Category = "Sessions")
	FNG_OnSearchStarted OnSearchStarted;

	UPROPERTY(BlueprintAssignable, Category = "Sessions")
	FNG_OnJoinFailed OnJoinFailed;

	UPROPERTY(BlueprintReadWrite)
	FString FindSessionResultText = "";

	UPROPERTY(BlueprintReadWrite)
	FString NumSearchResultText = "";

	UPROPERTY(BlueprintReadWrite)
	FString JoinSessionResultText = "";
};
