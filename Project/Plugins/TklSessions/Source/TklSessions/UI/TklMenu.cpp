#include "TklMenu.h"

#include "Components/Button.h"
#include "Logging/StructuredLog.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

#include "TklSessions/TklGameInstanceSubsystem.h"
#include "TklSessions/TklSessions.h"
#include "TklSessions/Utils.h"

void UTklMenu::Setup(int32 Connections, const FString &TypeOfMatch,
                     const FString &LobbyPath) {
  const auto *world = GetWorld();

  if (!world)
    return;

  AddToViewport();
  SetVisibility(ESlateVisibility::Visible);
  SetIsFocusable(true);

  NumPublicConnections = Connections;
  MatchType = TypeOfMatch;
  PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);

  if (auto *controller = world->GetFirstPlayerController()) {
    auto mode = FInputModeUIOnly();
    mode.SetWidgetToFocus(TakeWidget());
    mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    controller->SetInputMode(mode);
    controller->SetShowMouseCursor(true);
  }

  if (const auto *gameInstance = world->GetGameInstance())
    GameInstanceSubsystem =
        gameInstance->GetSubsystem<UTklGameInstanceSubsystem>();

  if (!GameInstanceSubsystem)
    return;

  GameInstanceSubsystem->OnTklCreateSessionsComplete.AddDynamic(
      this, &UTklMenu::OnCreateSession);
  GameInstanceSubsystem->OnTklFindSessionsComplete.AddUObject(
      this, &UTklMenu::OnFindSessions);
  GameInstanceSubsystem->OnTklJoinSessionsComplete.AddUObject(
      this, &UTklMenu::OnJoinSession);
  GameInstanceSubsystem->OnTklDestroySessionsComplete.AddDynamic(
      this, &UTklMenu::OnDestroySession);
  GameInstanceSubsystem->OnTklStartSessionsComplete.AddDynamic(
      this, &UTklMenu::OnStartSession);
}

bool UTklMenu::Initialize() {
  if (!Super::Initialize())
    return false;

  if (HostButton && JoinButton) {
    HostButton->OnClicked.AddDynamic(this, &UTklMenu::HostButtonClicked);
    JoinButton->OnClicked.AddDynamic(this, &UTklMenu::JoinButtonClicked);
  } else {
    UE_LOGFMT(LogTklSessions, Error, "HostButton or JoinButton is nullptr");
  }

  return true;
}

void UTklMenu::NativeDestruct() {
  Close();
  Super::NativeDestruct();
}

// Subsystem Callbacks

void UTklMenu::OnCreateSession(bool bWasSuccessful) {
  auto *world = GetWorld();

  if (!world)
    return;

  if (bWasSuccessful) {
    LogScreen(Green, 3, -1, "Session created successfully");
    world->ServerTravel(PathToLobby);
  } else {
    LogScreen(Red, 3, -1, "Failed to create session");
    HostButton->SetIsEnabled(true);
  }
}

void UTklMenu::OnFindSessions(
    const TArray<FOnlineSessionSearchResult> &SessionResults,
    bool bWasSuccessful) {
  if (!GameInstanceSubsystem)
    return;

  auto FilterMatchType = [](const FOnlineSessionSearchResult &result,
                            const FString &MatchType) {
    auto matchType = FString();
    result.Session.SessionSettings.Get({"MatchType"}, matchType);
    return matchType == MatchType;
  };

  for (const auto &result : SessionResults) {
    // Filters

    if (!FilterMatchType(result, MatchType))
      continue;

    // Join

    const auto id = result.GetSessionIdStr();
    const auto user = result.Session.OwningUserName;

    LogScreen(Green, 3, -1, "Id: '%s', User: '%s'", *id, *user);
    GameInstanceSubsystem->JoinSession(result);
    return;
  }

  if (!bWasSuccessful || SessionResults.Num() == 0)
    JoinButton->SetIsEnabled(true);

  LogScreen(Red, 3, -1, "Failed to find suitable session");
}

void UTklMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result) {
  auto *system = IOnlineSubsystem::Get();

  if (!system)
    return;

  auto interface = system->GetSessionInterface();

  if (!interface.IsValid())
    return;

  auto address = FString();
  interface->GetResolvedConnectString(NAME_GameSession, address);

  if (auto *controller = GetGameInstance()->GetFirstLocalPlayerController())
    controller->ClientTravel(address, ETravelType::TRAVEL_Absolute);

  if (Result != EOnJoinSessionCompleteResult::Success) {
    LogScreen(Red, 3, -1, "Failed to join session");
    JoinButton->SetIsEnabled(true);
  }
}

void UTklMenu::OnStartSession(bool bWasSuccessful) {}

void UTklMenu::OnDestroySession(bool bWasSuccessful) {}

// Button Callbacks

void UTklMenu::HostButtonClicked() {
  HostButton->SetIsEnabled(false);

  if (!GameInstanceSubsystem)
    return;

  GameInstanceSubsystem->CreatesSession(NumPublicConnections, MatchType);
}

void UTklMenu::JoinButtonClicked() {
  JoinButton->SetIsEnabled(false);

  if (!GameInstanceSubsystem)
    return;

  GameInstanceSubsystem->FindSessions(10000);
}

//

void UTklMenu::Close() {
  const auto *world = GetWorld();

  if (!world)
    return;

  RemoveFromParent();

  if (auto *controller = world->GetFirstPlayerController()) {
    controller->SetInputMode(FInputModeGameOnly());
    controller->SetShowMouseCursor(false);
  }
}
