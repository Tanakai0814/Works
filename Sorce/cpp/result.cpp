#include "DxLib.h"
#include "../h/maindata.h"
#include "../h/process.h"
#include "../h/keyboard.h"

#define UI_X_ADD 20		//UI��x�ʒu�̔������p


typedef struct {

	int ui_medal[3];	// 0 �� 1 �� 2 ��

}IMAGE;

typedef struct {

	int entry_name;

}SOUND;

typedef struct {

	IMAGE img;
	SOUND snd;

}MATERIAL;

void Result_Phase(MAINDATA *data) {

	RANKING_DATA top_score[TOP_SCORE_NUM];	//�����L���O
	RANKING_DATA change;					//�L�^����ւ��p

	MATERIAL mtl;

	int i, j;
	int update_flg = FALSE;		//�X�V�������ǂ���
	int update_no;
	int getname_flg = FALSE;	//�X�V�����ꍇ���O���擾�������ǂ���

	/*�f�o�b�O�p*/
	//int debug = GetRand(3);
	int file_del_flg = FALSE;
	//data->total_score = 150000;

	/*** Image ***/
	LoadDivGraph("Image/ui_medal.png", 3, 3, 1, 13, 15, mtl.img.ui_medal);

	/*** Sound ***/
	mtl.snd.entry_name = LoadSoundMem("Sound/BGM/Result_name_entry.wav");
	PlaySoundMem(mtl.snd.entry_name, DX_PLAYTYPE_LOOP);
	
	/*** �����L���O�f�[�^�̓ǂݍ��� ***/
	FILE *fp_r = fopen("ranking.dat", "rb");
	
	//�����L���O�f�[�^�����邩�ǂ���
	if (fp_r == NULL) {
		//�����f�[�^��ǂݍ���(����N����)
		FILE *fp_ir = fopen("ranking_init.dat", "rb");
		fread(&top_score, sizeof(top_score), 1, fp_ir);
		fclose(fp_ir);
	}
	else {
		//�����L���O�f�[�^��ǂݍ���
		fread(&top_score, sizeof(top_score), 1, fp_r);
		fclose(fp_r);
	}

	while (Process_Loop() == 0) {

		/***����***/
		/*�X�V�������̊m�F*/
		if (update_flg == FALSE) {

			/*�f�o�b�O�p*/
			/*switch (debug) {
			case 0:
				data->total_score = 14000;
				data->total_medal = 2;
				break;
			case 1:
				data->total_score = 16000;
				data->total_medal = 2;
				break;
			case 2:
				data->total_score = 30000;
				data->total_medal = 5;
				break;
			case 3:
				data->total_score = 24000;
				data->total_medal = 2;
				break;

			}*/

			/*�X�R�A�X�V�������ǂ���*/
			for (i = 0; i < TOP_SCORE_NUM; i++) {
				if (data->total_score >= top_score[i].score) {
					update_flg = TRUE;
					update_no = TOP_SCORE_NUM;
					top_score[TOP_SCORE_NUM - 1].score = data->total_score;
					top_score[TOP_SCORE_NUM - 1].medal = data->total_medal;
					*top_score[TOP_SCORE_NUM - 1].name = NULL;
					break;
				}
			}

			/*���Ԃ̕ύX*/
			if (update_flg == TRUE) {
				for (i = 0; i < TOP_SCORE_NUM; i++) {
					for (j = i + 1; j < TOP_SCORE_NUM; j++) {
						if (top_score[j].score > top_score[i].score || top_score[j].score == top_score[i].score && top_score[j].medal > top_score[i].medal) {
							change = top_score[i];
							top_score[i] = top_score[j];
							top_score[j] = change;
						}
					}
				}
			}
		}

		/***�`��***/
		DrawRotaGraph(50 - UI_X_ADD, 35, 1.0, 0.0, mtl.img.ui_medal[2], TRUE);
		DrawString(70 - UI_X_ADD, 30, "SKYKID  TOP5", RED);

		SetFontSize(13);
		DrawString(60 - UI_X_ADD, 50, "YOU MADE IT!", WHITE);
		DrawString(60 - UI_X_ADD, 62, "WHAT A GREAT FIGHTER!", WHITE);

		DrawString(40 - UI_X_ADD, 85, "NO.",SKYBLUE);
		DrawString(110 - UI_X_ADD, 85, "SCORE", SKYBLUE);
		DrawRotaGraph(190 - UI_X_ADD, 97, 1.0, 0.0, mtl.img.ui_medal[1], TRUE);
		DrawString(230 - UI_X_ADD, 85, "NAME", SKYBLUE);

		for (i = 0; i < TOP_SCORE_NUM; i++) {
			if (top_score[i].no == update_no) {
				DrawFormatString(60 - UI_X_ADD, 105 + i * 17, RED, "%d", i + 1);
				DrawFormatString(100 - UI_X_ADD, 105 + i * 17, RED, "%6d", top_score[i].score);
				DrawFormatString(175 - UI_X_ADD, 105 + i * 17, RED, "%2d", top_score[i].medal);
				DrawFormatString(220 - UI_X_ADD, 105 + i * 17, RED, "%s", top_score[i].name);
			}
			else {
				DrawFormatString(60 - UI_X_ADD, 105 + i * 17, WHITE, "%d", i + 1);
				DrawFormatString(100 - UI_X_ADD, 105 + i * 17, WHITE, "%6d", top_score[i].score);
				DrawFormatString(175 - UI_X_ADD, 105 + i * 17, WHITE, "%2d", top_score[i].medal);
				DrawFormatString(220 - UI_X_ADD, 105 + i * 17, WHITE, "%s", top_score[i].name);
			}
		}

		/*�L�^�X�V�����ꍇ*/
		if (update_flg == TRUE) {
			DrawString(40, 210, "PLEASE INPUT YOUR NAME!", YELLOW);

			//���O���͂���Ă��Ȃ��ꍇ8�����܂Ŗ��O���擾
			if (getname_flg == FALSE) {
				for (i = 0; i < TOP_SCORE_NUM; i++) {
					if (top_score[i].no == update_no) {
						//�����F�����F
						SetKeyInputStringColor2(DX_KEYINPSTRCOLOR_NORMAL_STR, YELLOW);
						SetKeyInputStringColor2(DX_KEYINPSTRCOLOR_NORMAL_CURSOR, YELLOW);
						
						//���͒�Enter�œ��͏I��
						KeyInputSingleCharString(200, 105 + i * 17, 8, top_score[i].name, TRUE);
						getname_flg = TRUE;
						break;
					}
				}
			}
		}

		//�f�o�b�N
		//�Q�[���I��
		if (Keyboard_Get(KEY_INPUT_ESCAPE) == 1) {
			data->end = TRUE;
			break;
		}
		//�^�C�g���ɖ߂�
		if (Keyboard_Get(KEY_INPUT_Z) == 1) {
			break;
		}
		//�����L���O�f�[�^�폜
		if (Keyboard_Get(KEY_INPUT_F) >= 1 && Keyboard_Get(KEY_INPUT_DELETE) == 1) {
			file_del_flg = TRUE;
		}

	}

	//���ʂ̍ĐU�蕪��
	for (i = 0; i < TOP_SCORE_NUM; i++) {
		top_score[i].no = i + 1;
	}

	/*** �����L���O�f�[�^�̏o�� ***/
	FILE *fp_w = fopen("ranking.dat","wb");
	fwrite(&top_score, sizeof(top_score), 1, fp_w);
	fclose(fp_w);

	//BGM�̒�~
	StopSoundMem(mtl.snd.entry_name);

	/*** �Q�[���f�[�^�̏��������� ***/
	data->mode = BARON;				//BARON or BARON & MAX
	data->dead = FALSE;				//���ۊm�F
	data->life = 1;					//�c�@
	data->current_stage = 1;		//���݂̃X�e�[�W
	data->score = 0;				//1�X�e�[�W�ł̃X�R�A
	data->total_score = 0;			//�g�[�^���X�R�A
	data->hight_score = 0;			//�n�C�X�R�A
	data->total_medal = 0;			//�g�[�^�����_��
	data->ground_enemy_kill = 0;	//�������i�n��j
	data->sky_enemy_kill = 0;		//�������i���j
	data->base_enemy_kill = 0;		//�������i��n�j

	/*�f�o�b�O*/
	//�t�@�C���폜
	if (file_del_flg == TRUE) {
		remove("ranking.dat");
		file_del_flg = FALSE;
	}

	return;
}