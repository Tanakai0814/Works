#include "DxLib.h"

int Key[256];

void Keyboard_Update() {

	char tmpKey[256];

	GetHitKeyStateAll(tmpKey);

	for (int i = 0; i < 256; i++) {

		if (tmpKey[i] != 0) {

			Key[i]++;
		}
		else {

			Key[i] = 0;
		}
	}
	return;
}

int Keyboard_Get(int KeyCode) {

	return	Key[KeyCode];
}