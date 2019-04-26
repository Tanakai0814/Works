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

#define TOP_SCORE_NUM 5		//ランキングの人数

typedef struct {
	int mode;				//BARON or BARON & MAX
	int dead;				//安否確認
	int life;				//残機
	int current_stage;		//現在のステージ
	int score;				//１ステージでのスコア
	int total_score;		//トータルスコア
	int hight_score;		//ハイスコア
	int total_medal;		//トータルメダル
	int ground_enemy_kill;	//討伐数（地上）
	int sky_enemy_kill;		//討伐数（上空）
	int base_enemy_kill;	//討伐数（基地）
	int ground_medal;       //メダル数(地上)
	int sky_medal;          //メダル数(上空)
	int base_medal;         //メダル数(基地)
	int end;				//終了確認
	int credit;				// 入れたお金
	int debug_flg;
} MAINDATA;

typedef struct ranking_date {
	int no;
	int score;
	int medal;
	char name[9];
}RANKING_DATA;