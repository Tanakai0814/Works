#include "DxLib.h"
#include <math.h>
#include "../h/maindata.h"
#include "../h/process.h"
#include "../h/keyboard.h"

#define PAI 3.1451

//�X�e�[�W�萔
#define STAGE_NUM 21
#define ORDER_NUM 24
#define STAGE_SPEED 1

//�v���C���[�萔
#define PLAYER_SPEED 1
#define LOOP_SPEED 10
#define BARON_CX (baron->x + 17)
#define BARON_CY (baron->y + 17)

//�G�l�~�[�萔
#define ENEMY_CX (enemy[i].x + 17)
#define ENEMY_CY (enemy[i].y + 17)
#define MAX_ENEMY 100
#define ADD_X(n) (Y_SIZE * n)	//n�̓}�b�v����

//�e�萔
#define SHOT_CX (baron_shot[j].x + 9)
#define SHOT_CY (baron_shot[j].y + 9)
#define BOMB_CX (bomb->x + 9)
#define BOMB_CY (bomb->y + 9)

//�N���A�萔
#define CLEAR_ZOON_R (-128 * 12) - 20 + *stage_move_cnt
#define CLEAR_ZOON_L (-128 * 14) + 20 + *stage_move_cnt

//�R���؂�
#define CRUSH_ZOON (-128 * 16) + 20 + *stage_move_cnt

enum enemy_type {
	SKY,
	GROUND,
	BATTLESHIP,
	BASE_RIGHT,
	BASE_LEFT,
};

enum enemy_ai_type {
	TYPE_NORMAL,	//�ʏ�
	TYPE_ATTACK,	//�e������
	TYPE_SPEED,		//�ړ�������
};

enum player_mode {
	MOVE,
	LOOP,
	FALL,
	CRUSH,
	DEAD,
	CLEAR
};

//���萔
#define MAX_SHOT 3
#define SHOT_SPEED 5;

enum player_dire {
	UP,
	NOMAL,
	DOWN
};

typedef struct {

	//OBJ
	int player[8];				// 0-1 �o�������i 2-3 �o�����~�� 4-5 �o�����㏸ 4-6 �o�������Ԃ� 7 �o�����ė�
	int sky_enemy[56];			// �G�i���j
	int ground_enemy[112];		// �G�i�n��j 12���͉��������Ă��Ȃ�
	int battleship_enemy[20];	// �G�i��́j
	int base_enemy[40];			// �G�i��n�j
	int bullet[16];				// 0 �z�u���@���e 1-4 ���@���e 5 �G���e 6 �΋�C 7 �΋�C�U�e 8 �˒����E�l�̎l�U 9 �e�� 10 �e�E 11 �e���� 12 �e�� 13 �e���� 14 �e��A�E�� 15 �e��A�^�� 16 �e��A����
	int other_small[10];		// 0-4 �󒆂̓G���� 5-6 �G�̔��e 7-8 �E���`�����
	int other_middle[8];		// 0-3 �n��̓G���� 4-6 ���@�̔��e����
	int explosion[6];			// ��͔���

	//MAP
	int map[84];				// �}�b�v
	int mapobj[4];				// �}�b�v�I�u�W�F�N�g�i�p�r�͕s���j
	int gimmick[10];			// �M�~�b�N
	int hit_map[84];

	// UI
	int ui_small[2];			// 0 �c�@�A�C�R�� 1 ���n�U��
	int ui_tutorial[5];			// 0 ���o�[ 1-2 �{�^�� 3-4 �U�����
	int ui_bonus[16];			// 0 ���_500 1 ���_1000
	int ui_life[4];				// 0 �c�@
}IMAGE;

typedef struct {

	// BGM
	int march;

	// Effect
	int start;		//�X�^�[�g
	int intro;		//�C���g��
	int baron_shot;	//�o�����V���b�g
	int loop;		//���Ԃ�
	int crush;		//�ė�
	int bomb_catch;	//���e�擾�� or ���A��
	int dead;		//���񂾂Ƃ�
	int gameover;	//�c�@�O�Ŏ��񂾂Ƃ�
	int sky_enemy_hit;	//�󒆂̓G���j��
	int ground_enemy_hit;
	int clear;
	int base_fully_hit;
	int base_half_hit;
	int base_access;

}SOUND;

typedef struct {

	IMAGE img;
	SOUND snd;

}MATERIAL;

typedef struct {
	int x;		// x���W
	int y;		// y���W
	int active;	// �A�N�e�B�u���
	int speed;	// ���x
	int distime;	// ���ł鎞��
	int angle;	// ���ˊp
	int r;		//���a
}BULLET;

typedef struct {
	int x;		// x���W
	int y;		// y���W
	int speed;	// �ړ����x
	int key;	// �L�[����̃I���I�t�iTRUE or FALSE�j
	int mode;	// ��ԁi�ړ�, ���Ԃ�, �{������, ����, ���S�j
	int dire;	// �����i��, ����, ���j
	int angle;	//�p�x
	int keycnt;	//���A�J�E���g
	int r;
	int bomb_catch;	//�{������
	int bomb_catch_ok;
	int invincible;
}PLAYER;

typedef struct {
	int type;		//�G�l�~�[�^�C�v
	int x;
	int y;
	int AI_type;	//AI�̎��(��)
	int active;
	int angle;
	int img_num;	//�G�̎��
	int speed;
	int init_x;		//�ŏ��̈ʒu
	int init_y;		//�ŏ��̈ʒu
	int r;
	int crush_flg;
	int crush_time;
}ENEMY;

typedef struct {
	int x;
	int y;
	int r;
	int active;
	int mode;
	int catch_ok;
}BOMB;



/************************************************************
�֐����F�A�b�v�f�[�g
�����F�Q�[���f�[�^, �C���[�W
�T�v�FFPS�����킹��
************************************************************/
static int mStartTime;      //����J�n����
static int mCount;          //�J�E���^
static float mFps;          //fps
static const int N = 60;	//���ς����T���v����
static const int FPS = 60;	//�ݒ肵��FPS

bool FPS_Update() {
	if (mCount == 0) { //1�t���[���ڂȂ玞�����L��
		mStartTime = GetNowCount();
	}
	if (mCount == N) { //60�t���[���ڂȂ畽�ς��v�Z����
		int t = GetNowCount();
		mFps = 1000.f / ((t - mStartTime) / (float)N);
		mCount = 0;
		mStartTime = t;
	}
	mCount++;
	return true;
}
void Wait() {
	int tookTime = GetNowCount() - mStartTime;		//������������
	int waitTime = mCount * 1000 / FPS - tookTime;	//�҂ׂ�����
	if (waitTime > 0) {
		Sleep(waitTime / 2);						//�ҋ@
	}
}

/************************************************************
�֐����F�Q�[���X�^�[�g
�����F�Q�[���f�[�^, �C���[�W
�T�v�F���݂̃X�e�[�W�ɉ������G�Ȃǂ̕\��
************************************************************/
void Game_Start(MAINDATA *data, MATERIAL *mtl, int *start) {

	DrawBox(0, 0, 640, 480, SKYBLUE, TRUE);

	DrawFormatString(110, 90, WHITE, "MISSION");
	DrawFormatString(180, 90, BLUE, "%d", data->current_stage);
	DrawFormatString(80, 110, BLACK, "BOMB THIS TARGET !");

	//�I���W�i���ł͌���A�j���[�V����������
	DrawGraph(110, 140, mtl->img.base_enemy[32], TRUE);
	DrawGraph(142, 140, mtl->img.base_enemy[33], TRUE);

	ScreenFlip();

	PlaySoundMem(mtl->snd.start, DX_PLAYTYPE_NORMAL);
	*start = TRUE;
}

/************************************************************
�֐����F�q�b�g�}�b�v�h���E
�����F�Q�[���f�[�^, �C���[�W
�T�v�F���݂̃X�e�[�W�ɉ������w�i�̕`��
************************************************************/
void Hit_Map_Drow(MAINDATA *data, MATERIAL *mtl, int stage_order[][ORDER_NUM], int *stage_move_cnt, float *time, int *anime_cnt, ENEMY enemy[], int *gameover_flg) {

	int i;
	int order;

	for (i = 0; i < ORDER_NUM; i++) {
		order = stage_order[data->current_stage][i];
		DrawGraph(((X_SIZE - 128) - 128 * i) + *stage_move_cnt, 0, mtl->img.hit_map[order], TRUE);
	}

	if (*time / 60.0f >= 2.8f) {
		if (*gameover_flg == FALSE) {
			*stage_move_cnt += STAGE_SPEED;
		}
	}
}

/************************************************************
�֐����F�}�b�v�q�b�g�W���b�W
�����F���C�����A�o�����̏��
�T�v�F�v���C���[�A�G�̏�Ԃ̕ύX
************************************************************/
void Map_Hit_Judge(MATERIAL *mtl, MAINDATA *data, PLAYER *baron, BULLET baron_shot[], ENEMY enemy[], BULLET enemy_shot[], BOMB *bomb, int *stage_move_cnt, int *return_line, int *crush_time, int *bomb_crush_time, unsigned int *get_color, unsigned int *map_color, int *get_color_flg) {

	int i, j;

	//������F���擾
	if (*get_color_flg == FALSE) {
		*map_color = GetPixel(1, 255);
		*get_color_flg = TRUE;
	}

	// �v���C���[
	if (baron->mode != DEAD && baron->mode != CLEAR) {
		for (i = 0; i < 3; i++) {
			switch (i) {
				case 0:	//�E
					*get_color = GetPixel(BARON_CX + baron->r, BARON_CY);
					break;
				case 1:	//��
					*get_color = GetPixel(BARON_CX, baron->y);
					break;
				case 2:	//��
					*get_color = GetPixel(baron->x, BARON_CY);
					break;
			}

			if (*get_color == *map_color && 30 < BARON_CX && BARON_CX < X_SIZE - 30 && BARON_CY > 30) {
				StopSoundMem(mtl->snd.crush);
				data->dead = TRUE;
				baron->mode = DEAD;
				baron->bomb_catch = FALSE;
			}
		}
	}
	
	//�{��
	if (bomb->mode == DOWN && baron->bomb_catch == FALSE) {

		*get_color = GetPixel(BOMB_CX, BOMB_CY);

		if (*get_color == *map_color) {
			baron->bomb_catch_ok = TRUE;
			bomb->mode = CRUSH;
		}
	}
	
	//�e
	for (i = 0; i < MAX_SHOT; i++) {
		if (baron_shot[i].active == FALSE) continue;

		*get_color = GetPixel(baron_shot[i].x + 8, baron_shot[i].y + 8);

		if (*get_color == *map_color) {
			DrawGraph(baron_shot[i].x, baron_shot[i].y, mtl->img.bullet[8], true);
			baron_shot[i].active = FALSE;
		}
	}

	return;
}

/************************************************************
�֐����F�X�e�[�W�h���E
�����F�Q�[���f�[�^, �C���[�W
�T�v�F���݂̃X�e�[�W�ɉ������w�i�̕`��
************************************************************/
void Stage(MAINDATA *data, MATERIAL *mtl, int stage_order[][ORDER_NUM], int *stage_move_cnt, float *time, int *anime_cnt, ENEMY enemy[], int *gameover_flg) {

	int i;
	int order;

	for (i = 0; i < ORDER_NUM; i++) {
		order = stage_order[data->current_stage][i];
		DrawGraph(((X_SIZE - 128) - 128 * i) + *stage_move_cnt, 0, mtl->img.map[order], TRUE);
	}

	switch (data->current_stage) {

		case 1:
			if (*anime_cnt % 30 <= 15) {
				DrawGraph(X_SIZE / 3 - 30 + *stage_move_cnt, Y_SIZE - 80, mtl->img.ui_tutorial[3], TRUE);
			}
			else {
				DrawGraph(X_SIZE / 3 - 30 + *stage_move_cnt, Y_SIZE - 80, mtl->img.ui_tutorial[4], TRUE);
			}
			break;

		default:
			break;
	}

	if (*time == 1) PlaySoundMem(mtl->snd.intro, DX_PLAYTYPE_BACK);

	if (*time / 60.0f >= 2.8f) {

		if (*time / 60.0f == 2.8f) {
			StopSoundMem(mtl->snd.intro);
			PlaySoundMem(mtl->snd.march, DX_PLAYTYPE_BACK);
		}

		//���ԂŃX�R�A���Z
		if ((int)*time % 60 == 2 && data->dead == FALSE) {
			data->total_score += 10;
			data->score += 10;
		}
	}
}

/************************************************************
�֐����F�v���C���[
�����F�}�e���A���A�o�������A�e���A�A�j���J�E���g�A����
�T�v�F�v���C���[�̑���A����A�X�V�A�`��
*************************************************************/
void Player(MATERIAL *mtl, PLAYER *baron, BULLET baron_shot[], int *anime_cnt, float *time, int *return_line, int *crush_time, float *dead_cnt, int *clear_time, int *clear_flg, BOMB *bomb, int *bomb_crush_time) {

	int i;

	if (*time / 60.0f == 2.8f) {
		baron->key = TRUE;
	}

	//��ʊO�ɂ����Ă��܂����Ƃ�
	if (baron->mode != DEAD && baron->mode != CLEAR) {
		if (BARON_CX < 30) baron->x += baron->speed * 2;
		if (X_SIZE - 30 < BARON_CX) baron->x -= baron->speed * 2;
		if (Y_SIZE + 30 < BARON_CY) baron->y -= baron->speed * 2;
	}

	//�L�[����
	if (baron->key == TRUE) {

		//�ė���
		if (baron->mode == CRUSH) {
			//���A
			if (Keyboard_Get(KEY_INPUT_UP) >= 1 && Keyboard_Get(KEY_INPUT_Z) == 1) {
				baron->keycnt += 1;
				if (*return_line < baron->keycnt) {
					PlaySoundMem(mtl->snd.bomb_catch, DX_PLAYTYPE_BACK);
					baron->mode = MOVE;
				}
			}
		}

		//���Ԃ蒆
		if (baron->mode == LOOP) {
			//�e�̔���
			if (Keyboard_Get(KEY_INPUT_Z) == 1) {

				for (i = 0; i < MAX_SHOT; i++) {
					if (baron_shot[i].active != TRUE) {
						baron_shot[i].x = baron->x;
						baron_shot[i].y = baron->y + 8;
						baron_shot[i].speed = SHOT_SPEED;
						baron_shot[i].active = TRUE;
						baron_shot[i].distime = 0;
						baron_shot[i].r = 9;

						if (baron->dire == DOWN) baron_shot[i].angle = baron->angle + 180;
						else baron_shot[i].angle = baron->angle;

						break;
					}
				}
				PlaySoundMem(mtl->snd.baron_shot, DX_PLAYTYPE_BACK);
			}
		}

		//�ʏ�
		if (baron->mode == MOVE) {

			baron->dire = NOMAL;
			baron->angle = 0;

			//�ړ�
			if (Keyboard_Get(KEY_INPUT_UP) && 30 < BARON_CY) {
				baron->y -= baron->speed;
				baron->angle = 45;
				baron->dire = UP;
			}
			if (Keyboard_Get(KEY_INPUT_DOWN) && BARON_CY < Y_SIZE - 10) {
				baron->y += baron->speed;
				baron->angle = 315;;
				baron->dire = DOWN;
			}
			if (Keyboard_Get(KEY_INPUT_RIGHT) && BARON_CX < X_SIZE - 30) {
				baron->x += baron->speed;
			}
			if (Keyboard_Get(KEY_INPUT_LEFT) && 30 < BARON_CX) {
				baron->x -= baron->speed;
			}
			//�e����
			if (Keyboard_Get(KEY_INPUT_Z) == 1) {

				for (i = 0; i < MAX_SHOT; i++) {
					if (baron_shot[i].active != TRUE) {
						baron_shot[i].x = baron->x;
						baron_shot[i].y = baron->y + 8;
						baron_shot[i].speed = SHOT_SPEED;
						baron_shot[i].distime = 0;
						baron_shot[i].active = TRUE;
						baron_shot[i].angle = baron->angle;
						baron_shot[i].r = 9;
						break;
					}
				}
				PlaySoundMem(mtl->snd.baron_shot, DX_PLAYTYPE_BACK);
			}
			//���Ԃ�
			if (Keyboard_Get(KEY_INPUT_X) == 1) {
				if (baron->bomb_catch == TRUE) {
					baron->bomb_catch = FALSE;
					bomb->mode = DOWN;
				}
				else {
					baron->mode = LOOP;
					baron->angle = 0;
					PlaySoundMem(mtl->snd.loop, DX_PLAYTYPE_BACK);
				}
			}
		}
	}

	//�`��

	//�ʏ�
	if (baron->mode == MOVE) {

		if (baron->dire == UP) {
			if (*anime_cnt % 10 <= 5) {
				DrawGraph(baron->x, baron->y, mtl->img.player[4], TRUE);
			}
			else {
				DrawGraph(baron->x, baron->y, mtl->img.player[5], TRUE);
			}
		}
		else if (baron->dire == NOMAL) {
			if (*anime_cnt % 10 <= 5) {
				DrawGraph(baron->x, baron->y, mtl->img.player[0], TRUE);
			}
			else {
				DrawGraph(baron->x, baron->y, mtl->img.player[1], TRUE);
			}
		}
		else if (baron->dire == DOWN) {
			if (*anime_cnt % 10 <= 5) {
				DrawGraph(baron->x, baron->y, mtl->img.player[2], TRUE);
			}
			else {
				DrawGraph(baron->x, baron->y, mtl->img.player[3], TRUE);
			}
		}
	}
	else if (baron->mode == LOOP) {
		DrawRotaGraph(baron->x + 17, baron->y + 17, 1.0, (PAI / 180) * baron->angle, mtl->img.player[6], TRUE);
		if (baron->dire == UP) {
			baron->x += baron->speed * ((sin((PAI / 180) * baron->angle) + 1) * 2.5);
			baron->y -= baron->speed * ((cos((PAI / 180) * baron->angle) + 0.4) * 5);
		}
		else if (baron->dire == NOMAL) {
			baron->x += baron->speed * ((sin((PAI / 180) * baron->angle) + 1) * 2.5);
			baron->y -= baron->speed * ((cos((PAI / 180) * baron->angle) + 0.2) * 5);
		}
		else if (baron->dire == DOWN) {
			/*baron->x += baron->speed * ((sin((PAI / 180) * (baron->angle + 30)) + 0.2) * 5);
			baron->y -= baron->speed * ((cos((PAI / 180) * (baron->angle + 30)) + 0.2) * 7);*/
			baron->x += baron->speed * ((cos((PAI / 180) * (baron->angle + 30)) - 0.1) * 5);
			baron->y += baron->speed * ((sin((PAI / 180) * baron->angle) + 0.2) * 4);
		}
		if (baron->angle >= 250) {
			baron->angle = 0;
			baron->mode = MOVE;
		}
		baron->angle += LOOP_SPEED;
	}
	//�ė���
	else if (baron->mode == CRUSH) {

		DrawGraph(baron->x, baron->y, mtl->img.player[7], TRUE);
		baron->y += 1;

		//���x���Đ�����ƈ�a��������̂�
		if (*crush_time == 0) {
			PlaySoundMem(mtl->snd.crush, DX_PLAYTYPE_BACK);
		}
		*crush_time += 1;
	}
	//�N���A��
	else if (baron->mode == CLEAR) {
		if (*clear_time == 0) {
			PlaySoundMem(mtl->snd.clear, DX_PLAYTYPE_BACK);
		}
		else if (*clear_time < 5) {
			baron->y -= 1;
		}
		else if (*clear_time < 10) {
			baron->y += 1;
		}
		if (*clear_time == 80) {
			PlaySoundMem(mtl->snd.dead, DX_PLAYTYPE_BACK);
		}
		if (*clear_time == 300) {
			*clear_flg = TRUE;
		}
		baron->x += STAGE_SPEED;
		DrawGraph(baron->x, baron->y, mtl->img.player[0], TRUE);
		*clear_time += 1;
	}
	//���S��
	else if (baron->mode == DEAD) {
		if (*dead_cnt < 5) {
			DrawGraph(baron->x, baron->y, mtl->img.other_small[(int)(*dead_cnt / 2)], true);
		}
	}


	if (baron->mode != CRUSH) {
		*crush_time = 0;
	}

	//�{��
	if (bomb->active == TRUE) {
		if (baron->bomb_catch == FALSE) {
			if (bomb->mode == NOMAL) {
				DrawGraph(bomb->x, bomb->y, mtl->img.bullet[0], TRUE);
				bomb->x += STAGE_SPEED;
			}
			else if (bomb->mode == DOWN) {
				DrawGraph(bomb->x, bomb->y, mtl->img.bullet[4], TRUE);
				bomb->y += 2;
			}
			else if (bomb->mode == CRUSH) {
				bomb->x += STAGE_SPEED;
				if (*bomb_crush_time < 9) {
					DrawGraph(bomb->x, bomb->y, mtl->img.other_small[(int)(*bomb_crush_time / 2)], TRUE);
				}
				else {
					bomb->active = FALSE;
					*bomb_crush_time = 0;
				}
				*bomb_crush_time += 1;
			}
		}
		else if (baron->bomb_catch == TRUE) {
			if (baron->dire == UP) {
				DrawGraph(bomb->x, bomb->y + 10, mtl->img.bullet[2], TRUE);
				bomb->x = baron->x - 2;
				bomb->y = baron->y + 4;
			}
			else if (baron->dire == NOMAL) {
				DrawGraph(bomb->x, bomb->y + 10, mtl->img.bullet[3], TRUE);
				bomb->x = baron->x + 2;
				bomb->y = baron->y + 8;
			}
			else if (baron->dire == DOWN) {
				DrawGraph(bomb->x, bomb->y + 10, mtl->img.bullet[1], TRUE);
				bomb->x = baron->x + 10;
				bomb->y = baron->y + 8;
			}
		}
	}

	//�e
	for (i = 0; i < MAX_SHOT; i++) {
		if (baron_shot[i].active == FALSE) continue;

		if (baron_shot[i].distime < 20) {
			baron_shot[i].x -= baron_shot[i].speed * (cos((PAI / 180) * baron_shot[i].angle));
			baron_shot[i].y -= baron_shot[i].speed * (sin((PAI / 180) * baron_shot[i].angle));
			DrawGraph(baron_shot[i].x, baron_shot[i].y, mtl->img.bullet[9], true);
			baron_shot[i].distime += 1;
		}
		else {
			DrawGraph(baron_shot[i].x, baron_shot[i].y, mtl->img.bullet[8], true);
			baron_shot[i].active = FALSE;
		}
	}

}

/************************************************************
�֐����F�G�l�~�[
�����F�Q�[�����A�}�e���A���A�o�������A�X�e�[�W�X�s�[�h�A�A�j���J�E���g�A���ԁA�f�o�b�N�t���O
�T�v�FUI����A�X�V�A�`��
************************************************************/
void Enemy(MATERIAL *mtl, ENEMY enemy[], int *anime_cnt, float *time, BOMB *bomb,BULLET enemy_shot[]) {

	int i = 0;
	static int base_bgm_cnt = 0;
	static int shot_time_cnt = 0;

	/*����*/
	for (i = 0; i < MAX_ENEMY; i++) {
		//�v���C���[�������o���܂Œ�~
		if (*time / 60.0f < 2.8f) {
			break;
		}
		if (enemy[i].active == FALSE) {
			continue;
		}

		enemy[i].x += STAGE_SPEED;

		if (enemy[i].x >= -30) {
			/*�GAI�ɂ�铮��Ǘ�(��)*/
			switch (enemy[i].AI_type) {
			case TYPE_NORMAL:

				break;
			case TYPE_ATTACK:
				switch (enemy[i].type) {
				case SKY:
					if (shot_time_cnt > 50) {
						if (enemy_shot[i].active != TRUE) {
							enemy_shot[i].x = ENEMY_CX + 9;
							enemy_shot[i].y = ENEMY_CY - 8;
							enemy_shot[i].speed = SHOT_SPEED - 3;
							enemy_shot[i].distime = 0;
							enemy_shot[i].active = TRUE;
							enemy_shot[i].angle = 0;
							enemy_shot[i].r = 9;
						}

						if (enemy_shot[i].active == TRUE) {
							if (enemy_shot[i].distime <= 30) {
								enemy_shot[i].x += enemy_shot[i].speed;
								DrawGraph(enemy_shot[i].x, enemy_shot[i].y, mtl->img.bullet[9], true);
								enemy_shot[i].distime++;
							}
							else {
								DrawGraph(enemy_shot[i].x, enemy_shot[i].y, mtl->img.bullet[8], true);
								enemy_shot[i].active = FALSE;
								shot_time_cnt = 0;
							}
						}
					}
					else {
						shot_time_cnt++;
					}
					break;
				case BASE_RIGHT:
					
					break;
				case BASE_LEFT:

					break;
				}
				break;
			case TYPE_SPEED:
				enemy[i].x += 1;
				break;
			}
		}

		//�x�[�X�o�ꉹ�y
		if (enemy[i].type == BASE_RIGHT && enemy[i].x == -50) {
			PlaySoundMem(mtl->snd.base_access, DX_PLAYTYPE_LOOP);
		}
		else if (enemy[i].type == BASE_RIGHT && enemy[i].x == 150) {
			StopSoundMem(mtl->snd.base_access);
		}
	}

	/*�`��*/
	for (i = 0; i < MAX_ENEMY; i++) {
		if (enemy[i].active == TRUE) {
			if (enemy[i].crush_flg == FALSE) {
				switch (enemy[i].type) {
					case SKY:
						if (*anime_cnt % 10 <= 5) {
							DrawTurnGraph(enemy[i].x, enemy[i].y, mtl->img.sky_enemy[enemy[i].img_num], TRUE);
						}
						else {
							DrawTurnGraph(enemy[i].x, enemy[i].y, mtl->img.sky_enemy[enemy[i].img_num + 1], TRUE);
						}
						if (-25 < enemy[i].x && enemy[i].x < X_SIZE + 25 && -25 < enemy[i].y && enemy[i].y < Y_SIZE + 25) {
							enemy[i].x += enemy[i].speed;
						}
						break;
					case GROUND:
						DrawGraph(enemy[i].x, enemy[i].y, mtl->img.ground_enemy[enemy[i].img_num], TRUE);
						break;
					case BATTLESHIP:

						break;
					case BASE_RIGHT:
						DrawGraph(enemy[i].x, enemy[i].y, mtl->img.base_enemy[enemy[i].img_num], TRUE);
						break;
					case BASE_LEFT:
						DrawGraph(enemy[i].x, enemy[i].y, mtl->img.base_enemy[enemy[i].img_num], TRUE);
						break;
				}
			}
			else if (enemy[i].crush_flg == TRUE) {
				if (enemy[i].type == BASE_LEFT || enemy[i].type == BASE_RIGHT) {
					DrawGraph(enemy[i].x, enemy[i].y, mtl->img.base_enemy[enemy[i].img_num], TRUE);
					DrawGraph(enemy[i + 1].x, enemy[i + 1].y, mtl->img.base_enemy[enemy[i + 1].img_num], TRUE);
					DrawGraph(bomb->x, enemy[i].y, mtl->img.other_middle[enemy[i].crush_time / 3], TRUE);
					if (enemy[i].crush_time > 21) {
						bomb->active = FALSE;
						enemy[i].crush_flg = FALSE;
						enemy[i].crush_time = 0;
					}
				}
				else {
					DrawGraph(enemy[i].x, enemy[i].y, mtl->img.other_small[enemy[i].crush_time / 2], TRUE);
					if (enemy[i].crush_time > 8) {
						enemy[i].active = FALSE;
						enemy[i].crush_time = 0;
					}
				}
				enemy[i].crush_time += 1;
			}
		}
	}

	return;
}


/**************************************************************************************************
�֐����FUI
�����F�Q�[�����A�}�e���A���A�o�������A�X�e�[�W�X�s�[�h�A�A�j���J�E���g�A���ԁA�f�o�b�N�t���O
�T�v�FUI����A�X�V�A�`��
**************************************************************************************************/
void UI(MAINDATA *data, MATERIAL *mtl, PLAYER *baron, BOMB *bomb, int *stage_move_cnt, int *anime_cnt, float *time, int *debug_flg) {

	int i;

	//�펞
	DrawFormatString(30, 0, GetColor(255, 0, 0), "1UP");
	DrawFormatString(75, 15, WHITE, "%d", data->total_score);

	for (i = 0; i < data->life - 1; i++) {
		DrawGraph(65 + 10 * i, 3, mtl->img.ui_life[0], TRUE);
	}

	DrawFormatString(130, 0, YELLOW, "TOP");

	if (data->total_score < data->hight_score) {
		DrawFormatString(175, 15, WHITE, "%d", data->hight_score);
	}
	else {
		DrawFormatString(175, 15, WHITE, "%d", data->total_score);
	}

	DrawFormatString(280, 225, WHITE, "%d", data->current_stage);
	DrawGraph(250, 225, mtl->img.ui_small[0], TRUE);


	//����

	/***�X�^�[�g***/
	if (*time / 60.0f > 2.8f && *time / 60.0f <= 5.0f) {
		DrawString(110, 80, "TAKE OFF!", PURPLE);
	}
	else if (*time / 60.0f <= 2.8f) {
		DrawString(125, 80, "READY", YELLOW);
	}

	/*** �{�� ***/
	if (bomb->x > -100 && bomb->x <= 0) {
		DrawString(100, 80, "GET THE BOMB!", BLACK);
	}

	/***�S�[���̍��}***/
	if (baron->mode != CLEAR) {
		if (CLEAR_ZOON_R >= -100 && CLEAR_ZOON_R <= 80) {
			DrawGraph(90, 80, mtl->img.ui_small[1], TRUE);
			DrawString(110, 80, "LAND HERE!", PURPLE);
		}
	}

	if (baron->mode == CLEAR) {
		DrawString(80, 110, "1UP", RED);
		DrawString(120, 110, "NICE LANDING!", YELLOW);
	}

	//�f�o�b�N�\��
	if (*debug_flg == TRUE) {
		DrawFormatString(10, Y_SIZE - 30, WHITE, "X:%d  Y:%d", baron->x, baron->y);
		DrawFormatString(10, Y_SIZE - 15, WHITE, "FPS %.1f", mFps);
		DrawFormatString(80, Y_SIZE - 15, WHITE, "inv %d", baron->invincible);
	}
}

/************************************************************
�֐����F�W���b�W
�����F���C�����A�o�����̏��
�T�v�F�v���C���[�A�G�̏�Ԃ̕ύX
************************************************************/
void Judge(MATERIAL *mtl, MAINDATA *data, PLAYER *baron, BULLET baron_shot[], ENEMY enemy[], BULLET enemy_shot[], BOMB *bomb, int *stage_move_cnt, int *return_line, int *crush_time, int *bomb_crush_time, unsigned int *get_color,unsigned int *map_color,int *get_color_flg) {

	int i, j;

	/*----------
	hypot�ł͂Ȃ��A�����ŋ����v�Z�֐���������ق��������i����y�A�h�o�C�X�j
	----------*/

	/*** �v���C���[�̒e�ƓG ***/
	for (i = 0; i < MAX_ENEMY; i++) {
		for (j = 0; j < MAX_SHOT; j++) {
			//shot active false �Ȃ� ���Ȃ� 
			if (baron_shot[j].active == FALSE) continue;

			if (enemy[i].crush_flg == FALSE && hypot(ENEMY_CX - SHOT_CX, ENEMY_CY - SHOT_CY) <= (baron_shot[j].r + enemy[i].r)) {
				if (enemy[i].type == SKY) {
					data->sky_enemy_kill += 1;
				}
				else if (enemy[i].type == GROUND) {
					data->ground_enemy_kill += 1;
				}
				data->total_score += 100;
				data->score += 100;
				PlaySoundMem(mtl->snd.sky_enemy_hit, DX_PLAYTYPE_BACK);
				enemy[i].crush_flg = TRUE;
				baron_shot[j].active = FALSE;
			}
		}// for shot end
	}// for enemy end

	/*�v���C���[�ƓG�̒e*/
	for (i = 0; i < MAX_ENEMY; i++) {
			if (enemy_shot[i].active == FALSE) continue;

			if (hypot(BARON_CX - (enemy_shot[i].x + 9), BARON_CY - (enemy_shot[i].y + 9)) <= (baron->r + enemy_shot[i].r)) {
				enemy_shot[i].active = FALSE;
				baron->mode = CRUSH;
			}
	}


	/***�{���̃L���b�`***/
	if (bomb->catch_ok == TRUE && bomb->active == TRUE && baron->mode == MOVE && hypot(BOMB_CX - BARON_CX, BOMB_CY - BARON_CY) <= baron->r + bomb->r && baron->bomb_catch == FALSE && baron->bomb_catch_ok == TRUE && baron->bomb_catch == FALSE && bomb->mode != DOWN) {
		PlaySoundMem(mtl->snd.bomb_catch, DX_PLAYTYPE_BACK);
		baron->bomb_catch = TRUE;
		baron->bomb_catch_ok = FALSE;
		bomb->catch_ok = FALSE;
	}

	/*** �{���Ɗ�n�Ƃ̓����蔻�� ***/
	if (bomb->active == TRUE) {
		for (i = 0; i < MAX_ENEMY; i++) {
			if (enemy[i].type != BASE_RIGHT) continue;
			//�����j��
			if (enemy[i].crush_flg == FALSE) {
				if (enemy[i + 1].x + 34 /3 * 2 < bomb->x && bomb->x < enemy[i].x + 34 / 3 * 1 && enemy[i].y - 10 < bomb->y && bomb->y < enemy[i].y + 10) {
					PlaySoundMem(mtl->snd.base_fully_hit, DX_PLAYTYPE_BACK);
					enemy[i].img_num += 2;
					enemy[i + 1].img_num += 2;
					enemy[i].crush_flg = TRUE;
					enemy[i + 1].crush_flg = TRUE;
					baron->bomb_catch_ok = TRUE;
					bomb->mode = CRUSH;
					data->base_enemy_kill += 1;
					data->score += 1000;
					data->total_score += 1000;
				}
				//�E���j��
				else if (enemy[i + 1].x + 34 / 3 * 4 < bomb->x && bomb->x < enemy[i].x + 34 && enemy[i].y - 10 < bomb->y && bomb->y < enemy[i].y + 10) {
					PlaySoundMem(mtl->snd.base_half_hit, DX_PLAYTYPE_BACK);
					enemy[i].img_num += 2;
					enemy[i].crush_flg = TRUE;
					baron->bomb_catch_ok = TRUE;
					bomb->mode = CRUSH;
					data->score += 500;
					data->total_score += 500;
				}
				//�����j��
				else if (enemy[i + 1].x < bomb->x && bomb->x < enemy[i].x - 34 / 3 * 1 && enemy[i].y - 10 < bomb->y && bomb->y < enemy[i].y + 10) {
					PlaySoundMem(mtl->snd.base_half_hit, DX_PLAYTYPE_BACK);
					enemy[i + 1].img_num = 34;		//+2�ł̓o�O������
					enemy[i + 1].crush_flg = TRUE;
					baron->bomb_catch_ok = TRUE;
					bomb->mode = CRUSH;
					data->score += 500;
					data->total_score += 500;
				}
			}
		}
	}


	/*** �v���C���[�ƓG ***/
	if (baron->invincible == FALSE) {
		if (baron->mode != LOOP) {
			for (i = 0; i < MAX_ENEMY; i++) {
				if (enemy[i].crush_flg == FALSE && baron->mode != DEAD) {
					if (hypot(ENEMY_CX - BARON_CX, ENEMY_CY - BARON_CY) <= baron->r + enemy[i].r) {
						data->total_score += 100;
						data->score += 100;
						baron->mode = CRUSH;
						baron->keycnt = 0;
						crush_time = 0;
						*return_line += 2;
						enemy[i].crush_flg = TRUE;
						if (enemy[i].type == SKY) {
							data->sky_enemy_kill += 1;
						}
						if (enemy[i].type == GROUND) {
							data->ground_enemy_kill += 1;
						}
						if (baron->bomb_catch == TRUE) {
							baron->bomb_catch = FALSE;
							bomb->mode = DOWN;
						}
					}
				}
			}
		}
	}

	/*** �N���A ***/
	if (baron->mode != DEAD && baron->y > Y_SIZE - 50 && (CLEAR_ZOON_L < baron->x && baron->x < CLEAR_ZOON_R)) {
		baron->mode = CLEAR;
		//�N���A���ɉ��y�~�߂�B����Ȃ������H
		StopSoundMem(mtl->snd.start);
	}

	//�R���؂�
	if (baron->x < CRUSH_ZOON && baron->mode != CRUSH) {
		baron->mode = CRUSH;
		baron->bomb_catch = FALSE;
		bomb->mode = DOWN;
		*return_line += 100;
	}

	//���ʉ��ł̂����蔻��
	if (baron->y > 200) {
		StopSoundMem(mtl->snd.crush);
		data->dead = TRUE;
		baron->mode = DEAD;
		baron->bomb_catch = FALSE;
	}
	return;
}

/************************************************************
�֐����F�Q�[���t�F�[�Y
�����F���C�����
�T�v�F�Q�[��
************************************************************/
void Game_Phase(MAINDATA *data) {

	/*** ������ ***/
	MATERIAL mtl;

	/*** Image ***/
	//OBJ
	LoadDivGraph("Image/player.png", 8, 4, 2, 34, 34, mtl.img.player);
	LoadDivGraph("Image/sky_enemy.png", 56, 7, 8, 34, 34, mtl.img.sky_enemy);
	LoadDivGraph("Image/ground_enemy.png", 112, 14, 8, 18, 18, mtl.img.ground_enemy);
	LoadDivGraph("Image/battleship_enemy.png", 20, 4, 5, 66, 34, mtl.img.battleship_enemy);
	LoadDivGraph("Image/base_enemy.png", 40, 8, 5, 34, 34, mtl.img.base_enemy);
	LoadDivGraph("Image/bullet.png", 18, 9, 2, 18, 18, mtl.img.bullet);
	LoadDivGraph("Image/other_small.png", 10, 5, 2, 18, 18, mtl.img.other_small);
	LoadDivGraph("Image/other_middle.png", 8, 4, 2, 34, 34, mtl.img.other_middle);
	LoadDivGraph("Image/explosion.png", 6, 3, 2, 66, 66, mtl.img.explosion);

	//MAP
	LoadDivGraph("Image/map.png", 84, 4, 21, 128, 256, mtl.img.map);
	LoadDivGraph("Image/mapobj.png", 4, 4, 1, 32, 32, mtl.img.mapobj);
	LoadDivGraph("Image/gimmick.png", 12, 6, 2, 34, 66, mtl.img.gimmick);
	LoadDivGraph("Image/hitmap.png", 84, 4, 21, 128, 256, mtl.img.hit_map);

	//UI
	LoadDivGraph("Image/ui_small.png", 2, 2, 1, 16, 14, mtl.img.ui_small);
	LoadDivGraph("Image/ui_tutorial.png", 5, 5, 1, 32, 32, mtl.img.ui_tutorial);
	LoadDivGraph("Image/ui_bonus.png", 16, 8, 2, 18, 18, mtl.img.ui_bonus);
	LoadDivGraph("Image/ui_life.png", 4, 4, 1, 8, 8, mtl.img.ui_life);

	/*** Sound ***/

	// BGM
	mtl.snd.march = LoadSoundMem("Sound/BGM/Game_skykidmarch.wav");

	// Effect
	mtl.snd.start = LoadSoundMem("Sound/Effect/Game_start_music.wav");			//�X�^�[�g
	mtl.snd.intro = LoadSoundMem("Sound/Effect/Game_skykidmarch_intro.wav");	//�C���g��
	mtl.snd.baron_shot = LoadSoundMem("Sound/Effect/Game_baron_shot.wav");		//�o�����V���b�g
	mtl.snd.loop = LoadSoundMem("Sound/Effect/Game_loop.wav");					//���Ԃ�
	mtl.snd.crush = LoadSoundMem("Sound/Effect/Game_crush.wav");				//�ė�
	mtl.snd.bomb_catch = LoadSoundMem("Sound/Effect/Game_bomb_catch.wav");		//�{���擾���ƕ��A��
	mtl.snd.dead = LoadSoundMem("Sound/Effect/Game_dead.wav");
	mtl.snd.gameover = LoadSoundMem("Sound/Effect/Game_gameover.wav");
	mtl.snd.sky_enemy_hit = LoadSoundMem("Sound/Effect/Game_aireborne_enemy_hit.wav");
	mtl.snd.ground_enemy_hit = LoadSoundMem("Sound/Effect/Game_ground_enemy_hit.wav");
	mtl.snd.clear = LoadSoundMem("Sound/Effect/Game_clear.wav");
	mtl.snd.base_fully_hit = LoadSoundMem("Sound/Effect/Game_target_fully_destroyed.wav");
	mtl.snd.base_half_hit = LoadSoundMem("Sound/Effect/Game_target_half_destroyed.wav");
	mtl.snd.base_access = LoadSoundMem("Sound/Effect/Game_target_access.wav");

	int start = FALSE;
	int stage_move_cnt = 0;
	int anime_cnt = 0;
	float time = 0.0f;
	float dead_time = 0.0f;
	int debug_flg = FALSE;
	int start_flg = FALSE;
	int return_line = 1;
	int crush_time = 0;
	int clear_time = 0;
	int bomb_crush_time = 0;
	int clear_flg = FALSE;
	int i;
	int gameover_flg = FALSE;

	int get_color_flg = FALSE;			//�F���擾�������ǂ���
	unsigned int get_color = 0;				//�v���C���[���͂̐F�擾
	unsigned int map_color = 0;				//�n�`�f�[�^�̐F�擾

	int stage_order[STAGE_NUM][ORDER_NUM] = {
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 01
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 02
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 03
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 04
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 05
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 06
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 07
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 08
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 09
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 10
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 11
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 12
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 13
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 14
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 15
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 16
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 17
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 18
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 19
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8},	// 20
		{2, 1, 0, 6, 61, 62, 4, 7, 61, 11, 26, 50, 27, 54, 2, 1, 25, 10, 9, 8, 8, 8, 8, 8}	// 21
	};

	PLAYER baron = {X_SIZE / 2 + 20, Y_SIZE - 50, PLAYER_SPEED, FALSE, MOVE, NOMAL, 0, 0, 16, FALSE, TRUE, FALSE};
	BULLET baron_shot[5] = { NULL };
	ENEMY enemy[MAX_ENEMY] = {
		{GROUND, -100, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},
		{GROUND, -125, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},
		{GROUND, -150, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},
		{GROUND, -250, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},
		{GROUND, -275, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},
		{GROUND, -300, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},

		{SKY, -500, 20, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},
		{SKY, -540, 45, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},
		{SKY, -510, 70, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},

		{GROUND, -600, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},
		{GROUND, -625, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},
		{GROUND, -650, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},
		{GROUND, -675, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},
		{GROUND, -700, 200, TYPE_NORMAL, TRUE, 0, 21, 1, -100, 200, 9, FALSE, 0},
		{GROUND, -725, 200, TYPE_NORMAL, TRUE, 0, 104, 1, -100, 200, 9, FALSE, 0},

		{SKY, -670, 45, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},
		{SKY, -650, 70, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},
		{SKY, -675, 95, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},
		{SKY, -700, 120, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},

		{SKY, -900, 0, TYPE_SPEED, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},
		{SKY, -930, 70, TYPE_SPEED, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},

		{SKY, -970, 10, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},
		{SKY, -1010, 20, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},
		{SKY, -980, 55, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},

		{BASE_RIGHT, -1150, 162, TYPE_NORMAL, TRUE, 0, 33, 1, -100, 200, 9, FALSE, 0},
		{BASE_LEFT, -1180, 162, TYPE_NORMAL, TRUE, 0, 32, 1, -100, 200, 9, FALSE, 0},

		{SKY, -1350, 10, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},
		{SKY, -1425, 20, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0},
		{SKY, -1375, 55, TYPE_NORMAL, TRUE, 0, 28, 1, -100, 200, 9, FALSE, 0}
	};
	BULLET enemy_shot[MAX_ENEMY] = { NULL };
	BOMB bomb = {-1000, 200, 5, TRUE, NOMAL, TRUE};

	for (i = 0; i < MAX_ENEMY; i++) {
		enemy[i].init_x = enemy[i].x;
		enemy[i].init_y = enemy[i].y;
	}

	/*** ���� ***/

	while (Process_Loop() == 0) {

		if (start == FALSE) Game_Start(data, &mtl, &start);

		Hit_Map_Drow(data, &mtl, stage_order, &stage_move_cnt, &time, &anime_cnt, enemy, &gameover_flg);

		Map_Hit_Judge(&mtl, data, &baron, baron_shot, enemy, enemy_shot, &bomb, &stage_move_cnt, &return_line, &crush_time, &bomb_crush_time, &get_color, &map_color, &get_color_flg);

		Stage(data, &mtl, stage_order, &stage_move_cnt, &time, &anime_cnt, enemy, &gameover_flg);

		if (gameover_flg == FALSE) {
			Player(&mtl, &baron, baron_shot, &anime_cnt, &time, &return_line, &crush_time, &dead_time, &clear_time, &clear_flg, &bomb, &bomb_crush_time);

			Enemy(&mtl, enemy, &anime_cnt, &time, &bomb, enemy_shot);

			UI(data, &mtl, &baron, &bomb, &stage_move_cnt, &anime_cnt, &time, &debug_flg);

			Judge(&mtl, data, &baron, baron_shot, enemy, enemy_shot, &bomb, &stage_move_cnt, &return_line, &crush_time, &bomb_crush_time, &get_color, &map_color,&get_color_flg);
		}

		//�f�o�b�N

		//�X�R�A���Z
		if (Keyboard_Get(KEY_INPUT_A) == 1) {
			data->score += 30000 + data->hight_score;
			data->total_score += 30000 + data->hight_score;
		}
		//���G���[�h�؂�ւ�
		if (Keyboard_Get(KEY_INPUT_S) == 1) {
			if (baron.invincible == FALSE) {
				baron.invincible = TRUE;
			}
			else {
				baron.invincible = FALSE;
			}
		}
		//�f�o�b�N�\���ؑ�
		if (Keyboard_Get(KEY_INPUT_D) == 1) {
			if (debug_flg == FALSE) {
				debug_flg = TRUE;
			}
			else {
				debug_flg = FALSE;
			}
		}
		//���U���g��
		if (Keyboard_Get(KEY_INPUT_ESCAPE) == 1) {
			data->dead = TRUE;
			baron.mode = DEAD;
			break;
		}
		//�{�[�i�X��
		if (Keyboard_Get(KEY_INPUT_Q) == 1) {
			break;
		}
		//�N���b�V��
		if (Keyboard_Get(KEY_INPUT_W) == 1) {
			StopSoundMem(mtl.snd.intro);
			StopSoundMem(mtl.snd.start);
			baron.mode = CRUSH;
			baron.keycnt = 0;
			crush_time = 0;
			return_line += 2;
		}
		//����
		if (Keyboard_Get(KEY_INPUT_E) == 1) {
			StopSoundMem(mtl.snd.march);
			StopSoundMem(mtl.snd.intro);
			StopSoundMem(mtl.snd.start);
			data->dead = TRUE;
			baron.mode = DEAD;
			baron.bomb_catch = FALSE;
			bomb.mode = DOWN;
		}

		FPS_Update();	//�X�V
		Wait();			//�ҋ@
		time++;

		anime_cnt++;

		if (clear_flg == TRUE) {
			//�o�����̏�����
			baron.x = X_SIZE / 2 + 20;
			baron.y = Y_SIZE - 50;
			baron.speed = PLAYER_SPEED;
			baron.key = FALSE;
			baron.mode = MOVE;
			baron.dire = NOMAL;
			baron.angle = 0;
			baron.keycnt = 0;
			baron.bomb_catch = FALSE;
			baron.bomb_catch_ok = TRUE;
			baron.invincible = FALSE;

			//�G�̏�����
			for (i = 0; i < MAX_ENEMY; i++) {
				enemy[i].active = TRUE;
				enemy[i].x = enemy[i].init_x;
				enemy[i].y = enemy[i].init_y;
			}

			// �{���̏�����
			bomb.active = TRUE;
			bomb.x = -1000;
			bomb.y = 200;

			//���̑��̏�����
			start = FALSE;
			stage_move_cnt = 0;
			anime_cnt = 0;
			time = 0.0f;
			dead_time = 0.0f;
			debug_flg = FALSE;
			start_flg = FALSE;
			return_line = 1;
			crush_time = 0;
			get_color_flg = FALSE;

			break;
		}

		//���S����
		if (data->dead == TRUE) {

			StopSoundMem(mtl.snd.march);
			if (dead_time == 2.0f) PlaySoundMem(mtl.snd.dead, DX_PLAYTYPE_BACK);

			if (dead_time / 60.0f >= 4.0f) {
				//�c�@����
				if (data->life > 1) {
					//�o�����̏�����
					baron.x = X_SIZE / 2 + 20;
					baron.y = Y_SIZE - 50;
					baron.speed = PLAYER_SPEED;
					baron.key = FALSE;
					baron.mode = MOVE;
					baron.dire = NOMAL;
					baron.angle = 0;
					baron.keycnt = 0;

					//�G�̏�����
					for (i = 0; i < MAX_ENEMY; i++) {
						enemy[i].x = enemy[i].init_x;
						enemy[i].y = enemy[i].init_y;
					}

					//���̑��̏�����
					start = FALSE;
					stage_move_cnt = 0;
					anime_cnt = 0;
					time = 0.0f;
					dead_time = 0.0f;
					debug_flg = FALSE;
					start_flg = FALSE;
					return_line = 1;
					crush_time = 0;
					get_color_flg = FALSE;

					data->life -= 1;
					data->dead = FALSE;
				}
				//�c�@����
				else {
					gameover_flg = TRUE;
					DrawFormatString(110, 80, RED, "GAME OVER");
					if (dead_time / 60.0f == 4.0f) PlaySoundMem(mtl.snd.gameover, DX_PLAYTYPE_BACK);
					if (dead_time / 60.0f == 8.0f) {
						break;
					}
				}
			}
			dead_time++;
		}
	}

	StopSoundMem(mtl.snd.march);
	StopSoundMem(mtl.snd.intro);
	StopSoundMem(mtl.snd.start);

	return;
}