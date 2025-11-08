#include "MenuGameMode.h"
#include "HalcyonMenuController.h"

AMenuGameMode::AMenuGameMode() {
	PlayerControllerClass = AHalcyonMenuController::StaticClass();
}