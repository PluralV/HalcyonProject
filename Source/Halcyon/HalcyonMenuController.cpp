// Fill out your copyright notice in the Description page of Project Settings.


#include "HalcyonMenuController.h"
#include "Blueprint/UserWidget.h"

void AHalcyonMenuController::BeginPlay() {
	
	Super::BeginPlay();
    check(GEngine);
	SetShowMouseCursor(true);
	SetInputMode(FInputModeUIOnly());
    /*GEngine->AddOnScreenDebugMessage(-1, 3.0f,FColor::Yellow,TEXT("Showing screen:"));
    if (MainMenuWidgetClass) {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("There is a main menu widget:"));
    }*/
	ShowScreen(0);
}

//Show/hide screen widget:
/*
"which" is an integer determining which menu screen is being accessed (Main Menu, Credits, Help, Level Select)
0: Menu
1: Help
2: Credits

*/
void AHalcyonMenuController::ShowScreen(int32 which) {
    TSubclassOf<UUserWidget> ScreenWidgetClass;
    FName ScreenName = NAME_None;
    switch (which) {
    case 0:
        if (MainMenuWidgetClass) {
            ScreenWidgetClass = MainMenuWidgetClass;
            ScreenName = TEXT("MainMenuScreen");
        }
        break;
    case 1:
        if (HelpScreenWidgetClass) {
            ScreenWidgetClass = HelpScreenWidgetClass;
            ScreenName = TEXT("HelpScreen");
        }
        break;
    case 2:
        if (CreditsScreenWidgetClass) {
            ScreenWidgetClass = CreditsScreenWidgetClass;
            ScreenName = TEXT("CreditsScreen");
        }
        break;
    default:break;
    }
    ScreenWidget = CreateWidget<UUserWidget>(this, ScreenWidgetClass, ScreenName);
    if (ScreenWidget) {
        ScreenWidget->AddToViewport();
    }
}

void AHalcyonMenuController::HideScreen()
{
    if (ScreenWidget)
    {
        ScreenWidget->RemoveFromParent();
        ScreenWidget = nullptr;
    }
}