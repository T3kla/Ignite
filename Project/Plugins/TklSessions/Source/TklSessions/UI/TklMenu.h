#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "TklMenu.generated.h"

class UButton;
class UTklGameInstanceSubsystem;

UCLASS()
class TKLSESSIONS_API UTklMenu final : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Setup(
		int32 Connections = 4,
		const FString& TypeOfMatch = "FreeForAll",
		const FString& LobbyPath = "/Game/Levels/Lv_Lobby");

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	// Subsystem Callbacks

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	//

private:
	UPROPERTY()
	UTklGameInstanceSubsystem* GameInstanceSubsystem = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* HostButton = nullptr;
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton = nullptr;

	// Button Callbacks

	UFUNCTION()
	void HostButtonClicked();
	UFUNCTION()
	void JoinButtonClicked();

	//

	void Close();

	int32 NumPublicConnections = 2;
	FString MatchType = "FreeForAll";
	FString PathToLobby = "/Game/Levels/Lv_Lobby?listen";
};
