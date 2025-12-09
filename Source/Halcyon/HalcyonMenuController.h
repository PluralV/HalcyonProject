#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HalcyonMenuController.generated.h"

class UUserWidget;

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

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowScreen(int32 which);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideScreen();

};
