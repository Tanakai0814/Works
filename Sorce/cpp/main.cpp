#include "DxLib.h"
#include "../h/maindata.h"


/************************************************************
�֐����F�^�C�g���t�F�[�Y
�����F�Q�[���f�[�^, �C���[�W
�T�v�F�^�C�g����`�悷��B�Q�[���̃v���C�l���I���������ōs���B
************************************************************/
void Title_Phase(MAINDATA *data);


/************************************************************
�֐����F�Q�[���t�F�[�Y
�����F�Q�[���f�[�^, �C���[�W
�߂�l�F�G�̌��j���ƃX�R�A�Ɛ������
�T�v�F�ȉ��̏������s��
�X�e�[�W�ŕK�v�ȉ摜�̓ǂݍ���
�L�[����
�v�Z
�`��
************************************************************/
void Game_Phase(MAINDATA *data);


/************************************************************
�֐����F�{�[�i�X�t�F�[�Y
�����F�Q�[���f�[�^, �C���[�W
�T�v�F�X�e�[�W�N���A��̌��j���ɉ����ă{�[�i�X�_�̉��Z, 30000�_�𒴂�����c�@���PUP����
************************************************************/
void Bonus_Phase(MAINDATA *data);


/************************************************************
�֐����F���U���g�t�F�[�Y
�����F�Q�[���f�[�^, �C���[�W
�T�v�F�Q�[�����ʂ̕ۑ����s���B
************************************************************/
void Result_Phase(MAINDATA *data);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	/*** �E�B���h�E���� ***/
	SetGraphMode(X_SIZE, Y_SIZE, 16);
	SetWindowSizeChangeEnableFlag(TRUE);
	SetWindowSizeExtendRate(2.0);

	SetMainWindowText("SKYKID");	//�E�B���h�E�^�C�g��
	SetWindowIconID(252);	//252�͎w�肵��ID�Ȃ̂œ��ɈӖ��͂Ȃ��ł�

	/*** Dxlib�̏����� ***/
	if (ChangeWindowMode(TRUE) == -1 || DxLib_Init() == -1 || SetDrawScreen(DX_SCREEN_BACK) == -1) {
		return -1;
	}


	/*** ���������� ***/
	MAINDATA data = {BARON, FALSE, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FALSE, 0, FALSE};


	//�Q�[�����̃t�H���g�ݒ�
	SetFontSize(12);								//�t�H���g�T�C�Y
	SetFontThickness(5);							//�t�H���g�̑���
	ChangeFont("Pixel Emulator");					//�t�H���g
	ChangeFontType(DX_FONTTYPE_ANTIALIASING);		//�A���`�G�C���A�X�t�H���g�ɕύX



	/*** �Q�[���{�� ***/

	do {
		Title_Phase(&data);
		do {
			Game_Phase(&data);
			if (data.dead == FALSE) {
				Bonus_Phase(&data);
			}
		} while (data.dead == FALSE);
		Result_Phase(&data);

	} while (data.end == FALSE);


	/*** �I������ ***/
	DxLib_End();
	return 0;
}
