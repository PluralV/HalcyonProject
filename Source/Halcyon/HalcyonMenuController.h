#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HalcyonMenuController.generated.h"

class UUserWidget;

USTRUCT(BlueprintType)
struct FMissionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MissionNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MissionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MissionDesc;
};
/**
 * 
 */
UCLASS()
class HALCYON_API AHalcyonMenuController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HelpScreenWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> CreditsScreenWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> LevelSelectScreenWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> MissionDescWidgetClass;

	UPROPERTY()
	UUserWidget* ScreenWidget;

	FMissionInfo CurrentActiveMissionMode;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowScreen(int32 which);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideScreen();

	//Used to determine what appears on the mission description screen
	UFUNCTION(BlueprintCallable, Category = "Mission Gamemode")
	void SetActiveMissionGameMode(FMissionInfo ActiveMissionMode);


	UFUNCTION(BlueprintCallable, Category = "Mission Gamemode")
	FMissionInfo GetActiveMissionGameMode();

};
