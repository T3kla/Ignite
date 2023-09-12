#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "TklGameInstanceSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTklOnCreateSessionsComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FTklOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults,  bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FTklOnJoinSessionsComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTklOnStartSessionsComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTklOnDestroySessionsComplete, bool, bWasSuccessful);

UCLASS()
class TKLSESSIONS_API UTklGameInstanceSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UTklGameInstanceSubsystem();
	
	void CreatesSession(int32 NumPublicConnections, const FString &MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SearchResult);
	void StartSession();
	void DestroySession();

	FTklOnCreateSessionsComplete OnTklCreateSessionsComplete;
	FTklOnFindSessionsComplete OnTklFindSessionsComplete;
	FTklOnJoinSessionsComplete OnTklJoinSessionsComplete;
	FTklOnStartSessionsComplete OnTklStartSessionsComplete;
	FTklOnDestroySessionsComplete OnTklDestroySessionsComplete;
	
private:
	IOnlineSessionPtr Session;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SearchSettings;
	
	// Subsystem Delegates

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;

	// Subsystem Delegate handles
	
	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle FindSessionsCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle StartSessionCompleteHandle;
	FDelegateHandle DestroySessionCompleteHandle;

	// Subsystem Delegate callbacks
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	// Retry create session when it fails
	
	bool bRetryCreateSession = false;
	int32 RetryNumPublicConnections = 0;
	FString RetryMatchType = "";
};
