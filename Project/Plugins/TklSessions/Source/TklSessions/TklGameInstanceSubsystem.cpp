#include "TklGameInstanceSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

UTklGameInstanceSubsystem::UTklGameInstanceSubsystem() :
	CreateSessionCompleteDelegate(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	StartSessionCompleteDelegate(
		FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
	DestroySessionCompleteDelegate(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
{
	auto* subsystem = IOnlineSubsystem::Get();

	if (!subsystem)
		return;

	Session = subsystem->GetSessionInterface();
}

void UTklGameInstanceSubsystem::CreatesSession(int32 NumPublicConnections, const FString& MatchType)
{
	const auto* world = GetWorld();

	if (!world)
	{
		OnTklCreateSessionsComplete.Broadcast(false);
		return;
	}

	if (!Session.IsValid())
	{
		OnTklCreateSessionsComplete.Broadcast(false);
		return;
	}

	if (Session->GetNamedSession(NAME_GameSession))
	{
		bRetryCreateSession = true;
		RetryNumPublicConnections = NumPublicConnections;
		RetryMatchType = MatchType;
		
		DestroySession();
		return;
	}

	CreateSessionCompleteHandle = Session->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	auto& settings = SessionSettings;
	settings = MakeShareable(new FOnlineSessionSettings());
	settings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL") ? true : false;
	settings->NumPrivateConnections = 0;
	settings->NumPublicConnections = NumPublicConnections;
	settings->bAllowInvites = true;
	settings->bAllowJoinInProgress = true;
	settings->bAllowJoinViaPresence = true;
	settings->bAllowJoinViaPresenceFriendsOnly = true;
	settings->bIsDedicated = false;
	settings->bShouldAdvertise = true;
	settings->bUsesPresence = true;
	settings->bUseLobbiesIfAvailable = true;
	settings->BuildUniqueId = 1;
	settings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	const auto* localPlayer = world->GetFirstLocalPlayerFromController();
	const auto localUniqueID = localPlayer->GetPreferredUniqueNetId();

	if (!Session->CreateSession(*localUniqueID, NAME_GameSession, *settings))
	{
		Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
		OnTklCreateSessionsComplete.Broadcast(false);
	}
}

void UTklGameInstanceSubsystem::FindSessions(int32 MaxSearchResults)
{
	const auto* world = GetWorld();

	if (!world)
	{
		OnTklFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	if (!Session.IsValid())
	{
		OnTklFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	FindSessionsCompleteHandle = Session->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	auto& settings = SearchSettings;
	settings = MakeShareable(new FOnlineSessionSearch());
	settings->MaxSearchResults = MaxSearchResults;
	settings->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL") ? true : false;
	settings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const auto* localPlayer = world->GetFirstLocalPlayerFromController();
	const auto localUniqueID = localPlayer->GetPreferredUniqueNetId();

	if (!Session->FindSessions(*localUniqueID, settings.ToSharedRef()))
	{
		Session->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
		OnTklFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UTklGameInstanceSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	const auto* world = GetWorld();

	if (!world)
	{
		OnTklJoinSessionsComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	if (!Session.IsValid())
	{
		OnTklJoinSessionsComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteHandle = Session->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const auto* localPlayer = world->GetFirstLocalPlayerFromController();
	const auto localUniqueID = localPlayer->GetPreferredUniqueNetId();

	if (!Session->JoinSession(*localUniqueID, NAME_GameSession, SearchResult))
	{
		Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
		OnTklJoinSessionsComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UTklGameInstanceSubsystem::StartSession()
{
	const auto* world = GetWorld();

	if (!world)
	{
		OnTklStartSessionsComplete.Broadcast(false);
		return;
	}

	if (!Session.IsValid())
	{
		OnTklStartSessionsComplete.Broadcast(false);
		return;
	}

	StartSessionCompleteHandle = Session->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	const auto* localPlayer = world->GetFirstLocalPlayerFromController();
	const auto localUniqueID = localPlayer->GetPreferredUniqueNetId();

	if (!Session->StartSession(NAME_GameSession))
	{
		Session->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteHandle);
		OnTklStartSessionsComplete.Broadcast(false);
	}
}

void UTklGameInstanceSubsystem::DestroySession()
{
	if (!Session.IsValid())
	{
		OnTklDestroySessionsComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if(!Session->DestroySession(NAME_GameSession))
	{
		Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
		OnTklDestroySessionsComplete.Broadcast(false);
	}
}

// Delegate Callbacks

void UTklGameInstanceSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (Session.IsValid())
		Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);

	OnTklCreateSessionsComplete.Broadcast(true);
}

void UTklGameInstanceSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (Session.IsValid())
		Session->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);

	if (SearchSettings->SearchResults.Num() <= 0)
		bWasSuccessful = false;

	OnTklFindSessionsComplete.Broadcast(SearchSettings->SearchResults, bWasSuccessful);
}

void UTklGameInstanceSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Session.IsValid())
		Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);

	OnTklJoinSessionsComplete.Broadcast(Result);
}

void UTklGameInstanceSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (Session.IsValid())
		Session->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteHandle);

	OnTklStartSessionsComplete.Broadcast(bWasSuccessful);
}

void UTklGameInstanceSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (Session.IsValid())
		Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);

	if (bWasSuccessful && bRetryCreateSession)
	{
		bRetryCreateSession = false;
		CreatesSession(RetryNumPublicConnections, RetryMatchType);
	}
	
	OnTklDestroySessionsComplete.Broadcast(bWasSuccessful);
}
