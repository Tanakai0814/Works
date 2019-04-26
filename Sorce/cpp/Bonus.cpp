#include "DxLib.h"
#include "../h/maindata.h"
#include "../h/process.h"
#include "../h/keyboard.h"

//�f�o�b�N
#include <time.h>

#define draw_time_cnt 40	//�\���܂ł̃J�E���g

/*** �\���� ***/

typedef struct {

	int ui_life[4];		// 0 �c�@
	int ui_medal[3];	// 0 �� 1 �� 2 ��
	int ui_enemy[3];	// 0 �� 1 �n�� 2 ��n
	int ui_result[2];   // 0�@�o�����@1 �~

}IMAGE;

typedef struct {

	int ok;				//�{�[�i�X�擾
	int no;				//�{�[�i�X���l��
	int excellent;		//�S�{�[�i�X�擾

}SOUND;

typedef struct {

	IMAGE img;
	SOUND snd;

}MATERIAL;

/************************************************************
�֐����F���j���[
�����F�Q�[���f�[�^, �C���[�W
�T�v�F���j���ɉ����ă{�[�i�XUI�̕\���A�X�R�A�̉��Z�A�c�@�A�b�v������
************************************************************/
void Bonus_Phase(MAINDATA *data) {

	MATERIAL mtl;


	/*** Image ***/
	LoadDivGraph("Image/ui_life.png", 4, 4, 1, 8, 8, mtl.img.ui_life);
	LoadDivGraph("Image/ui_enemy.png", 3, 3, 1, 30, 15, mtl.img.ui_enemy);
	LoadDivGraph("Image/ui_medal.png", 3, 3, 1, 13, 15, mtl.img.ui_medal);
	LoadDivGraph("Image/ui_life.png", 4, 4, 1, 8, 12, mtl.img.ui_result);


	/*** Sound ***/
	//���ʉ�
	mtl.snd.ok = LoadSoundMem("Sound/Effect/Bonus.wav");
	mtl.snd.no = LoadSoundMem("Sound/Effect/Game_baron_shot.wav");
	mtl.snd.excellent = LoadSoundMem("Sound/Effect/Bonus_high_score.wav");

	/*�ϐ�*/
	int sky_bonus = 0, ground_bonus = 0, base_bonus = 0;
	int ANIME_CNT = 0;
	int i;
	int sky_bonus_flg = FALSE;
	int ground_bonus_flg = FALSE;
	int baron_UI_flg = FALSE;
	int baron_lifeup_flg = FALSE;
	/***�f�o�b�N***/
	/*data->sky_enemy_kill = 17;
	data->ground_enemy_kill = 37;
	data->base_enemy_kill = 1;*/


	while (Process_Loop() == 0) {

		/*+�@�^�X�N�@++++++++++
		1.���ʉ��̒ǉ��ƁA�X�R�A��30000��������PUP������B
		�@������悭���āB
		++++++++++++++++++++*/

		// �Œ�UI
		DrawString(30, 0, "1UP", RED);
		DrawString(130, 0, "TOP", YELLOW);
		DrawString(40, 35, "MISSION  1  BATTLE REPORT", YELLOW);
		DrawString(32, 60, "PLAYER   1", RED);
		DrawString(200, 60, "BONUS", WHITE);

		/*** �󒆓G�̃{�[�i�X ***/
		DrawRotaGraph(60, 95, 1.0, 0.0, mtl.img.ui_enemy[0], TRUE);
		DrawRotaGraph(90, 100, 1.0, 0.0, mtl.img.ui_result[1], TRUE);

		DrawFormatString(100, 92, WHITE, "%02d", data->sky_enemy_kill);		//������

		/*���_���̕\��*/
		if (data->sky_enemy_kill >= 10) {
			for (i = 0; i < data->sky_enemy_kill / 10; i++) {
				DrawRotaGraph(140 + 10 * i, 100, 1.0, 0.0, mtl.img.ui_medal[0], TRUE);
				data->sky_medal = 1 + i;
			}
			
		}

		/*�X�R�A���Z*/
		if (sky_bonus_flg == FALSE) {
			if (data->sky_enemy_kill >= 10) {
				for (i = 0; i < data->sky_enemy_kill / 10; i++) {
					sky_bonus += 1000;
				}
			}
			else {
				sky_bonus = 0;
			}
			sky_bonus_flg = TRUE;
		}

		if (ANIME_CNT >= draw_time_cnt * 3) {
			DrawFormatString(200, 92, WHITE, "%5d", sky_bonus);
			if (ANIME_CNT == draw_time_cnt * 3) {
				data->total_score += sky_bonus;
				PlaySoundMem(mtl.snd.ok, DX_PLAYTYPE_NORMAL);
			}
		}

		/*** �n��G�̃{�[�i�X ***/
		DrawRotaGraph(60, 115, 1.0, 0.0, mtl.img.ui_enemy[1], TRUE);
		DrawRotaGraph(90, 120, 1.0, 0.0, mtl.img.ui_result[1], TRUE);

		DrawFormatString(100, 112, WHITE, "%02d", data->ground_enemy_kill);

		/*���_���̕\��*/
		if (data->ground_enemy_kill >= 10) {
			for (i = 0; i < data->ground_enemy_kill / 10; i++) {
				DrawRotaGraph(140 + 10 * i, 120, 1.0, 0.0, mtl.img.ui_medal[0], TRUE);
				data-> ground_medal = 1 + i;
				
			}
		}

		/*�X�R�A���Z*/
		if (ground_bonus_flg == FALSE) {
			if (data->ground_enemy_kill >= 10) {
				for (i = 0; i < data->ground_enemy_kill / 10; i++) {
					DrawRotaGraph(150 + 10 * i, 120, 1.0, 0.0, mtl.img.ui_medal[0], TRUE);
					ground_bonus += 1000;
				}
			}
			else {
				ground_bonus = 0;
			}
			ground_bonus_flg = TRUE;
		}

		if (ANIME_CNT >= draw_time_cnt * 3.5) {
			DrawFormatString(200, 112, WHITE, "%5d", ground_bonus);
			if (ANIME_CNT == draw_time_cnt * 3.5) {
				data->total_score += ground_bonus;
				PlaySoundMem(mtl.snd.ok, DX_PLAYTYPE_NORMAL);
			}
		}

		/*** ��n�̃{�[�i�X ***/
		DrawRotaGraph(60, 135, 1.0, 0.0, mtl.img.ui_enemy[2], TRUE);
		DrawRotaGraph(90, 140, 1.0, 0.0, mtl.img.ui_result[1], TRUE);

		DrawFormatString(100, 132, WHITE, "%2d", data->base_enemy_kill);

		/*�X�R�A���Z*/
		if (data->base_enemy_kill == TRUE) {
			/*���_���̕\��*/
			DrawRotaGraph(140, 137, 1.0, 0.0, mtl.img.ui_medal[2], TRUE);
			/*data->base_medal += 1;*/
			base_bonus = 10000;

			DrawString(60, 148, "MISSION  COMPLETED", SKYBLUE);
			DrawString(55, 170, "GOOD! BUT YOU CAN'T REST .", SKYBLUE);
			DrawString(55, 185, "THE BATTLE IS TO CONTINUE.", SKYBLUE);
			DrawString(55, 200, "FIGHT ON!", SKYBLUE);
		}

		else {
			base_bonus = 0;
		}

		if (ANIME_CNT >= draw_time_cnt * 4) {
			DrawFormatString(200, 132, WHITE, "%5d", base_bonus);
			if (ANIME_CNT == draw_time_cnt * 4 && data->base_enemy_kill == TRUE) {
				data->total_score += base_bonus;
				PlaySoundMem(mtl.snd.no, DX_PLAYTYPE_NORMAL);
			}
			else if (ANIME_CNT == draw_time_cnt * 4) {
				PlaySoundMem(mtl.snd.ok, DX_PLAYTYPE_NORMAL);
			}
		}
		ANIME_CNT++;

		//���v���X�R�A�ɉ��Z
		if (baron_lifeup_flg == FALSE) {

			for (i = 0; i < ((data->score + sky_bonus + ground_bonus + base_bonus) / 30000); i++) {
				data->life += 1;
			}

			baron_lifeup_flg = TRUE;
		}


		for (i = 0; i < data->life; i++) {
			DrawRotaGraph(93 + 10 * i, 7, 1.0, 0.0, mtl.img.ui_result[0], TRUE);//�o�����̉摜�\��
		}
		if (data->total_score < data->hight_score) {
			DrawFormatString(75, 15, WHITE, "%d", data->total_score);
			DrawFormatString(175, 15, WHITE, "%d", data->hight_score);
		}
		else {
			DrawFormatString(75, 15, WHITE, "%d", data->total_score);
			DrawFormatString(175, 15, WHITE, "%d", data->total_score);
		}
		data->total_medal = data->sky_medal + data->ground_medal + data->base_medal;
		/*** �f�o�b�O ***/
		if (Keyboard_Get(KEY_INPUT_SPACE) == 1) {
			srand((unsigned)time(NULL));
			data->sky_enemy_kill = rand() % 29 + 1;

			srand((unsigned)time(NULL));
			data->ground_enemy_kill = rand() % 25 + 1;

			srand((unsigned)time(NULL));
			data->base_enemy_kill = rand() % 2;
		}
		if (Keyboard_Get(KEY_INPUT_Z) == 1) {
			break;
		}
		if (Keyboard_Get(KEY_INPUT_ESCAPE) == 1) {
			data->dead = TRUE;
			data->end = TRUE;
			break;
		}

	}
	//���j���̏�����
	data->sky_enemy_kill = 0;
	data->ground_enemy_kill = 0;
	data->base_enemy_kill = 0;
	

	return;
}