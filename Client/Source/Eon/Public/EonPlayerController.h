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

	// Debug Commands
	UFUNCTION(Exec, BlueprintCallable, Category = "Debug")
	void SpawnTestItem(const FString& ItemId = TEXT("health_potion"), int32 Quantity = 1);

	UFUNCTION(Exec, BlueprintCallable, Category = "Debug")
	void GiveItem(const FString& ItemId, int32 Quantity = 1);

	UFUNCTION(Exec, BlueprintCallable, Category = "Debug")
	void ListInventory();

protected:
	virtual void SetupInputComponent() override;

	// SpaceTimeDB settings
	UPROPERTY(EditDefaultsOnly, Category = "SpaceTimeDB")
	bool bEnableSpaceTimeDB = true; // Server deployed to maincloud.spacetimedb.com/eon

	// Position sync settings
	UPROPERTY(EditDefaultsOnly, Category = "Sync")
	float PositionSyncInterval = 0.1f; // 10 Hz update rate

	UPROPERTY(EditDefaultsOnly, Category = "Mobile")
	bool bIsMobileDevice = false;

private:
	void SyncPlayerPosition();
	void DetectPlatform();

	UFUNCTION()
	void OnSpaceTimeDBConnected();

	float LastSyncTime = 0.0f;

	UPROPERTY()
	UEonHUD* EonHUD;
};
