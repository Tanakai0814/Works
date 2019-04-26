#include "DxLib.h"
#include "../h/keyboard.h"

int Process_Loop() {

	if (ScreenFlip() != 0) return -1;
	if (ProcessMessage() != 0) return -1;
	if (ClearDrawScreen() != 0) return -1;
	Keyboard_Update();

	return 0;
}