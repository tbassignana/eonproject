// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EonPlayerController.generated.h"

class USpaceTimeDBManager;
class UEonHUD;

UCLASS()
class EON_API AEonPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AEonPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB")
	USpaceTimeDBManager* GetSpaceTimeDBManager() const;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInventoryUI(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInstanceBrowser(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowCreateInstanceDialog();

protected:
	virtual void SetupInputComponent() override;

	// Position sync settings
	UPROPERTY(EditDefaultsOnly, Category = "Sync")
	float PositionSyncInterval = 0.1f; // 10 Hz update rate

	UPROPERTY(EditDefaultsOnly, Category = "Mobile")
	bool bIsMobileDevice = false;

private:
	void SyncPlayerPosition();
	void DetectPlatform();

	float LastSyncTime = 0.0f;

	UPROPERTY()
	UEonHUD* EonHUD;
};
