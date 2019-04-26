#pragma once

#define TRUE 1
#define FALSE 0

#define BARON 1
#define BARON_MAX 2

#define X_SIZE 300
#define Y_SIZE 240

#define WHITE GetColor(255, 255, 255)
#define RED GetColor(255, 0, 0)
#define BLUE GetColor(0, 0, 255)
#define BLACK GetColor(0, 0, 0)
#define SKYBLUE GetColor(0, 215, 210)
#define YELLOW GetColor(255, 255, 0)
#define PURPLE GetColor(255,0,200)

#define TOP_SCORE_NUM 5		//�����L���O�̐l��

typedef struct {
	int mode;				//BARON or BARON & MAX
	int dead;				//���ۊm�F
	int life;				//�c�@
	int current_stage;		//���݂̃X�e�[�W
	int score;				//�P�X�e�[�W�ł̃X�R�A
	int total_score;		//�g�[�^���X�R�A
	int hight_score;		//�n�C�X�R�A
	int total_medal;		//�g�[�^�����_��
	int ground_enemy_kill;	//�������i�n��j
	int sky_enemy_kill;		//�������i���j
	int base_enemy_kill;	//�������i��n�j
	int ground_medal;       //���_����(�n��)
	int sky_medal;          //���_����(���)
	int base_medal;         //���_����(��n)
	int end;				//�I���m�F
	int credit;				// ���ꂽ����
	int debug_flg;
} MAINDATA;

typedef struct ranking_date {
	int no;
	int score;
	int medal;
	char name[9];
}RANKING_DATA;