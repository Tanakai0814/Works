#include "DxLib.h"
#include <math.h>
#include "../h/maindata.h"
#include "../h/process.h"
#include "../h/keyboard.h"

#define PAI 3.1451

//ステージ定数
#define STAGE_NUM 21
#define ORDER_NUM 24
#define STAGE_SPEED 1

//プレイヤー定数
#define PLAYER_SPEED 1
#define LOOP_SPEED 10
#define BARON_CX (baron->x + 17)
#define BARON_CY (baron->y + 17)

//エネミー定数
#define ENEMY_CX (enemy[i].x + 17)
#define ENEMY_CY (enemy[i].y + 17)
#define MAX_ENEMY 100
#define ADD_X(n) (Y_SIZE * n)	//nはマップ枚数

//弾定数
#define SHOT_CX (baron_shot[j].x + 9)
#define SHOT_CY (baron_shot[j].y + 9)
#define BOMB_CX (bomb->x + 9)
#define BOMB_CY (bomb->y + 9)

//クリア定数
#define CLEAR_ZOON_R (-128 * 12) - 20 + *stage_move_cnt
#define CLEAR_ZOON_L (-128 * 14) + 20 + *stage_move_cnt

//燃料切れ
#define CRUSH_ZOON (-128 * 16) + 20 + *stage_move_cnt

enum enemy_type {
	SKY,
	GROUND,
	BATTLESHIP,
	BASE_RIGHT,
	BASE_LEFT,
};

enum enemy_ai_type {
	TYPE_NORMAL,	//通常
	TYPE_ATTACK,	//弾を撃つ
	TYPE_SPEED,		//移動が速い
};

enum player_mode {
	MOVE,
	LOOP,
	FALL,
	CRUSH,
	DEAD,
	CLEAR
};

//球定数
#define MAX_SHOT 3
#define SHOT_SPEED 5;

enum player_dire {
	UP,
	NOMAL,
	DOWN
};

typedef struct {

	//OBJ
	int player[8];				// 0-1 バロン直進 2-3 バロン降下 4-5 バロン上昇 4-6 バロン宙返り 7 バロン墜落
	int sky_enemy[56];			// 敵（上空）
	int ground_enemy[112];		// 敵（地上） 12枚は何も入っていない
	int battleship_enemy[20];	// 敵（戦艦）
	int base_enemy[40];			// 敵（基地）
	int bullet[16];				// 0 配置自機爆弾 1-4 自機爆弾 5 敵爆弾 6 対空砲 7 対空砲散弾 8 射程限界値の四散 9 弾左 10 弾右 11 弾左下 12 弾上 13 弾左上 14 弾二連右上 15 弾二連真上 16 弾二連左上
	int other_small[10];		// 0-4 空中の敵爆発 5-6 敵の爆弾 7-8 ウンチちゃん
	int other_middle[8];		// 0-3 地上の敵爆発 4-6 自機の爆弾爆発
	int explosion[6];			// 戦艦爆発

	//MAP
	int map[84];				// マップ
	int mapobj[4];				// マップオブジェクト（用途は不明）
	int gimmick[10];			// ギミック
	int hit_map[84];

	// UI
	int ui_small[2];			// 0 残機アイコン 1 着地誘導
	int ui_tutorial[5];			// 0 レバー 1-2 ボタン 3-4 誘導矢印
	int ui_bonus[16];			// 0 得点500 1 得点1000
	int ui_life[4];				// 0 残機
}IMAGE;

typedef struct {

	// BGM
	int march;

	// Effect
	int start;		//スタート
	int intro;		//イントロ
	int baron_shot;	//バロンショット
	int loop;		//宙返り
	int crush;		//墜落
	int bomb_catch;	//爆弾取得時 or 復帰時
	int dead;		//死んだとき
	int gameover;	//残機０で死んだとき
	int sky_enemy_hit;	//空中の敵撃破時
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
	int x;		// x座標
	int y;		// y座標
	int active;	// アクティブ状態
	int speed;	// 速度
	int distime;	// 飛んでる時間
	int angle;	// 発射角
	int r;		//半径
}BULLET;

typedef struct {
	int x;		// x座標
	int y;		// y座標
	int speed;	// 移動速度
	int key;	// キー操作のオンオフ（TRUE or FALSE）
	int mode;	// 状態（移動, 宙返り, ボム所持, 落下, 死亡）
	int dire;	// 向き（上, 普通, 下）
	int angle;	//角度
	int keycnt;	//復帰カウント
	int r;
	int bomb_catch;	//ボム所持
	int bomb_catch_ok;
	int invincible;
}PLAYER;

typedef struct {
	int type;		//エネミータイプ
	int x;
	int y;
	int AI_type;	//AIの種類(仮)
	int active;
	int angle;
	int img_num;	//敵の種類
	int speed;
	int init_x;		//最初の位置
	int init_y;		//最初の位置
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
関数名：アップデート
引数：ゲームデータ, イメージ
概要：FPSを合わせる
************************************************************/
static int mStartTime;      //測定開始時刻
static int mCount;          //カウンタ
static float mFps;          //fps
static const int N = 60;	//平均を取るサンプル数
static const int FPS = 60;	//設定したFPS

bool FPS_Update() {
	if (mCount == 0) { //1フレーム目なら時刻を記憶
		mStartTime = GetNowCount();
	}
	if (mCount == N) { //60フレーム目なら平均を計算する
		int t = GetNowCount();
		mFps = 1000.f / ((t - mStartTime) / (float)N);
		mCount = 0;
		mStartTime = t;
	}
	mCount++;
	return true;
}
void Wait() {
	int tookTime = GetNowCount() - mStartTime;		//かかった時間
	int waitTime = mCount * 1000 / FPS - tookTime;	//待つべき時間
	if (waitTime > 0) {
		Sleep(waitTime / 2);						//待機
	}
}

/************************************************************
関数名：ゲームスタート
引数：ゲームデータ, イメージ
概要：現在のステージに応じた敵などの表示
************************************************************/
void Game_Start(MAINDATA *data, MATERIAL *mtl, int *start) {

	DrawBox(0, 0, 640, 480, SKYBLUE, TRUE);

	DrawFormatString(110, 90, WHITE, "MISSION");
	DrawFormatString(180, 90, BLUE, "%d", data->current_stage);
	DrawFormatString(80, 110, BLACK, "BOMB THIS TARGET !");

	//オリジナルでは光るアニメーションがある
	DrawGraph(110, 140, mtl->img.base_enemy[32], TRUE);
	DrawGraph(142, 140, mtl->img.base_enemy[33], TRUE);

	ScreenFlip();

	PlaySoundMem(mtl->snd.start, DX_PLAYTYPE_NORMAL);
	*start = TRUE;
}

/************************************************************
関数名：ヒットマップドロウ
引数：ゲームデータ, イメージ
概要：現在のステージに応じた背景の描画
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
関数名：マップヒットジャッジ
引数：メイン情報、バロンの情報
概要：プレイヤー、敵の状態の変更
************************************************************/
void Map_Hit_Judge(MATERIAL *mtl, MAINDATA *data, PLAYER *baron, BULLET baron_shot[], ENEMY enemy[], BULLET enemy_shot[], BOMB *bomb, int *stage_move_cnt, int *return_line, int *crush_time, int *bomb_crush_time, unsigned int *get_color, unsigned int *map_color, int *get_color_flg) {

	int i, j;

	//当たる色を取得
	if (*get_color_flg == FALSE) {
		*map_color = GetPixel(1, 255);
		*get_color_flg = TRUE;
	}

	// プレイヤー
	if (baron->mode != DEAD && baron->mode != CLEAR) {
		for (i = 0; i < 3; i++) {
			switch (i) {
				case 0:	//右
					*get_color = GetPixel(BARON_CX + baron->r, BARON_CY);
					break;
				case 1:	//下
					*get_color = GetPixel(BARON_CX, baron->y);
					break;
				case 2:	//左
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
	
	//ボム
	if (bomb->mode == DOWN && baron->bomb_catch == FALSE) {

		*get_color = GetPixel(BOMB_CX, BOMB_CY);

		if (*get_color == *map_color) {
			baron->bomb_catch_ok = TRUE;
			bomb->mode = CRUSH;
		}
	}
	
	//弾
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
関数名：ステージドロウ
引数：ゲームデータ, イメージ
概要：現在のステージに応じた背景の描画
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

		//時間でスコア加算
		if ((int)*time % 60 == 2 && data->dead == FALSE) {
			data->total_score += 10;
			data->score += 10;
		}
	}
}

/************************************************************
関数名：プレイヤー
引数：マテリアル、バロン情報、弾情報、アニメカウント、時間
概要：プレイヤーの操作、制御、更新、描画
*************************************************************/
void Player(MATERIAL *mtl, PLAYER *baron, BULLET baron_shot[], int *anime_cnt, float *time, int *return_line, int *crush_time, float *dead_cnt, int *clear_time, int *clear_flg, BOMB *bomb, int *bomb_crush_time) {

	int i;

	if (*time / 60.0f == 2.8f) {
		baron->key = TRUE;
	}

	//画面外にいってしまったとき
	if (baron->mode != DEAD && baron->mode != CLEAR) {
		if (BARON_CX < 30) baron->x += baron->speed * 2;
		if (X_SIZE - 30 < BARON_CX) baron->x -= baron->speed * 2;
		if (Y_SIZE + 30 < BARON_CY) baron->y -= baron->speed * 2;
	}

	//キー操作
	if (baron->key == TRUE) {

		//墜落時
		if (baron->mode == CRUSH) {
			//復帰
			if (Keyboard_Get(KEY_INPUT_UP) >= 1 && Keyboard_Get(KEY_INPUT_Z) == 1) {
				baron->keycnt += 1;
				if (*return_line < baron->keycnt) {
					PlaySoundMem(mtl->snd.bomb_catch, DX_PLAYTYPE_BACK);
					baron->mode = MOVE;
				}
			}
		}

		//宙返り中
		if (baron->mode == LOOP) {
			//弾の発射
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

		//通常
		if (baron->mode == MOVE) {

			baron->dire = NOMAL;
			baron->angle = 0;

			//移動
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
			//弾発射
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
			//宙返り
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

	//描画

	//通常
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
	//墜落時
	else if (baron->mode == CRUSH) {

		DrawGraph(baron->x, baron->y, mtl->img.player[7], TRUE);
		baron->y += 1;

		//何度も再生すると違和感があるので
		if (*crush_time == 0) {
			PlaySoundMem(mtl->snd.crush, DX_PLAYTYPE_BACK);
		}
		*crush_time += 1;
	}
	//クリア時
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
	//死亡時
	else if (baron->mode == DEAD) {
		if (*dead_cnt < 5) {
			DrawGraph(baron->x, baron->y, mtl->img.other_small[(int)(*dead_cnt / 2)], true);
		}
	}


	if (baron->mode != CRUSH) {
		*crush_time = 0;
	}

	//ボム
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

	//弾
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
関数名：エネミー
引数：ゲーム情報、マテリアル、バロン情報、ステージスピード、アニメカウント、時間、デバックフラグ
概要：UI制御、更新、描画
************************************************************/
void Enemy(MATERIAL *mtl, ENEMY enemy[], int *anime_cnt, float *time, BOMB *bomb,BULLET enemy_shot[]) {

	int i = 0;
	static int base_bgm_cnt = 0;
	static int shot_time_cnt = 0;

	/*処理*/
	for (i = 0; i < MAX_ENEMY; i++) {
		//プレイヤーが動き出すまで停止
		if (*time / 60.0f < 2.8f) {
			break;
		}
		if (enemy[i].active == FALSE) {
			continue;
		}

		enemy[i].x += STAGE_SPEED;

		if (enemy[i].x >= -30) {
			/*敵AIによる動作管理(仮)*/
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

		//ベース登場音楽
		if (enemy[i].type == BASE_RIGHT && enemy[i].x == -50) {
			PlaySoundMem(mtl->snd.base_access, DX_PLAYTYPE_LOOP);
		}
		else if (enemy[i].type == BASE_RIGHT && enemy[i].x == 150) {
			StopSoundMem(mtl->snd.base_access);
		}
	}

	/*描画*/
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
関数名：UI
引数：ゲーム情報、マテリアル、バロン情報、ステージスピード、アニメカウント、時間、デバックフラグ
概要：UI制御、更新、描画
**************************************************************************************************/
void UI(MAINDATA *data, MATERIAL *mtl, PLAYER *baron, BOMB *bomb, int *stage_move_cnt, int *anime_cnt, float *time, int *debug_flg) {

	int i;

	//常時
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


	//期間

	/***スタート***/
	if (*time / 60.0f > 2.8f && *time / 60.0f <= 5.0f) {
		DrawString(110, 80, "TAKE OFF!", PURPLE);
	}
	else if (*time / 60.0f <= 2.8f) {
		DrawString(125, 80, "READY", YELLOW);
	}

	/*** ボム ***/
	if (bomb->x > -100 && bomb->x <= 0) {
		DrawString(100, 80, "GET THE BOMB!", BLACK);
	}

	/***ゴールの合図***/
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

	//デバック表示
	if (*debug_flg == TRUE) {
		DrawFormatString(10, Y_SIZE - 30, WHITE, "X:%d  Y:%d", baron->x, baron->y);
		DrawFormatString(10, Y_SIZE - 15, WHITE, "FPS %.1f", mFps);
		DrawFormatString(80, Y_SIZE - 15, WHITE, "inv %d", baron->invincible);
	}
}

/************************************************************
関数名：ジャッジ
引数：メイン情報、バロンの情報
概要：プレイヤー、敵の状態の変更
************************************************************/
void Judge(MATERIAL *mtl, MAINDATA *data, PLAYER *baron, BULLET baron_shot[], ENEMY enemy[], BULLET enemy_shot[], BOMB *bomb, int *stage_move_cnt, int *return_line, int *crush_time, int *bomb_crush_time, unsigned int *get_color,unsigned int *map_color,int *get_color_flg) {

	int i, j;

	/*----------
	hypotではなく、自分で距離計算関数を作ったほうが早い（杉先輩アドバイス）
	----------*/

	/*** プレイヤーの弾と敵 ***/
	for (i = 0; i < MAX_ENEMY; i++) {
		for (j = 0; j < MAX_SHOT; j++) {
			//shot active false なら 見ない 
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

	/*プレイヤーと敵の弾*/
	for (i = 0; i < MAX_ENEMY; i++) {
			if (enemy_shot[i].active == FALSE) continue;

			if (hypot(BARON_CX - (enemy_shot[i].x + 9), BARON_CY - (enemy_shot[i].y + 9)) <= (baron->r + enemy_shot[i].r)) {
				enemy_shot[i].active = FALSE;
				baron->mode = CRUSH;
			}
	}


	/***ボムのキャッチ***/
	if (bomb->catch_ok == TRUE && bomb->active == TRUE && baron->mode == MOVE && hypot(BOMB_CX - BARON_CX, BOMB_CY - BARON_CY) <= baron->r + bomb->r && baron->bomb_catch == FALSE && baron->bomb_catch_ok == TRUE && baron->bomb_catch == FALSE && bomb->mode != DOWN) {
		PlaySoundMem(mtl->snd.bomb_catch, DX_PLAYTYPE_BACK);
		baron->bomb_catch = TRUE;
		baron->bomb_catch_ok = FALSE;
		bomb->catch_ok = FALSE;
	}

	/*** ボムと基地との当たり判定 ***/
	if (bomb->active == TRUE) {
		for (i = 0; i < MAX_ENEMY; i++) {
			if (enemy[i].type != BASE_RIGHT) continue;
			//同時破壊
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
				//右翼破壊
				else if (enemy[i + 1].x + 34 / 3 * 4 < bomb->x && bomb->x < enemy[i].x + 34 && enemy[i].y - 10 < bomb->y && bomb->y < enemy[i].y + 10) {
					PlaySoundMem(mtl->snd.base_half_hit, DX_PLAYTYPE_BACK);
					enemy[i].img_num += 2;
					enemy[i].crush_flg = TRUE;
					baron->bomb_catch_ok = TRUE;
					bomb->mode = CRUSH;
					data->score += 500;
					data->total_score += 500;
				}
				//左翼破壊
				else if (enemy[i + 1].x < bomb->x && bomb->x < enemy[i].x - 34 / 3 * 1 && enemy[i].y - 10 < bomb->y && bomb->y < enemy[i].y + 10) {
					PlaySoundMem(mtl->snd.base_half_hit, DX_PLAYTYPE_BACK);
					enemy[i + 1].img_num = 34;		//+2ではバグが発生
					enemy[i + 1].crush_flg = TRUE;
					baron->bomb_catch_ok = TRUE;
					bomb->mode = CRUSH;
					data->score += 500;
					data->total_score += 500;
				}
			}
		}
	}


	/*** プレイヤーと敵 ***/
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

	/*** クリア ***/
	if (baron->mode != DEAD && baron->y > Y_SIZE - 50 && (CLEAR_ZOON_L < baron->x && baron->x < CLEAR_ZOON_R)) {
		baron->mode = CLEAR;
		//クリア時に音楽止める。いらないかも？
		StopSoundMem(mtl->snd.start);
	}

	//燃料切れ
	if (baron->x < CRUSH_ZOON && baron->mode != CRUSH) {
		baron->mode = CRUSH;
		baron->bomb_catch = FALSE;
		bomb->mode = DOWN;
		*return_line += 100;
	}

	//水面下でのあたり判定
	if (baron->y > 200) {
		StopSoundMem(mtl->snd.crush);
		data->dead = TRUE;
		baron->mode = DEAD;
		baron->bomb_catch = FALSE;
	}
	return;
}

/************************************************************
関数名：ゲームフェーズ
引数：メイン情報
概要：ゲーム
************************************************************/
void Game_Phase(MAINDATA *data) {

	/*** 初期化 ***/
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
	mtl.snd.start = LoadSoundMem("Sound/Effect/Game_start_music.wav");			//スタート
	mtl.snd.intro = LoadSoundMem("Sound/Effect/Game_skykidmarch_intro.wav");	//イントロ
	mtl.snd.baron_shot = LoadSoundMem("Sound/Effect/Game_baron_shot.wav");		//バロンショット
	mtl.snd.loop = LoadSoundMem("Sound/Effect/Game_loop.wav");					//宙返り
	mtl.snd.crush = LoadSoundMem("Sound/Effect/Game_crush.wav");				//墜落
	mtl.snd.bomb_catch = LoadSoundMem("Sound/Effect/Game_bomb_catch.wav");		//ボム取得時と復帰時
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

	int get_color_flg = FALSE;			//色を取得したかどうか
	unsigned int get_color = 0;				//プレイヤー周囲の色取得
	unsigned int map_color = 0;				//地形データの色取得

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

	/*** 処理 ***/

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

		//デバック

		//スコア加算
		if (Keyboard_Get(KEY_INPUT_A) == 1) {
			data->score += 30000 + data->hight_score;
			data->total_score += 30000 + data->hight_score;
		}
		//無敵モード切り替え
		if (Keyboard_Get(KEY_INPUT_S) == 1) {
			if (baron.invincible == FALSE) {
				baron.invincible = TRUE;
			}
			else {
				baron.invincible = FALSE;
			}
		}
		//デバック表示切替
		if (Keyboard_Get(KEY_INPUT_D) == 1) {
			if (debug_flg == FALSE) {
				debug_flg = TRUE;
			}
			else {
				debug_flg = FALSE;
			}
		}
		//リザルトへ
		if (Keyboard_Get(KEY_INPUT_ESCAPE) == 1) {
			data->dead = TRUE;
			baron.mode = DEAD;
			break;
		}
		//ボーナスへ
		if (Keyboard_Get(KEY_INPUT_Q) == 1) {
			break;
		}
		//クラッシュ
		if (Keyboard_Get(KEY_INPUT_W) == 1) {
			StopSoundMem(mtl.snd.intro);
			StopSoundMem(mtl.snd.start);
			baron.mode = CRUSH;
			baron.keycnt = 0;
			crush_time = 0;
			return_line += 2;
		}
		//即死
		if (Keyboard_Get(KEY_INPUT_E) == 1) {
			StopSoundMem(mtl.snd.march);
			StopSoundMem(mtl.snd.intro);
			StopSoundMem(mtl.snd.start);
			data->dead = TRUE;
			baron.mode = DEAD;
			baron.bomb_catch = FALSE;
			bomb.mode = DOWN;
		}

		FPS_Update();	//更新
		Wait();			//待機
		time++;

		anime_cnt++;

		if (clear_flg == TRUE) {
			//バロンの初期化
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

			//敵の初期化
			for (i = 0; i < MAX_ENEMY; i++) {
				enemy[i].active = TRUE;
				enemy[i].x = enemy[i].init_x;
				enemy[i].y = enemy[i].init_y;
			}

			// ボムの初期化
			bomb.active = TRUE;
			bomb.x = -1000;
			bomb.y = 200;

			//その他の初期化
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

		//死亡処理
		if (data->dead == TRUE) {

			StopSoundMem(mtl.snd.march);
			if (dead_time == 2.0f) PlaySoundMem(mtl.snd.dead, DX_PLAYTYPE_BACK);

			if (dead_time / 60.0f >= 4.0f) {
				//残機あり
				if (data->life > 1) {
					//バロンの初期化
					baron.x = X_SIZE / 2 + 20;
					baron.y = Y_SIZE - 50;
					baron.speed = PLAYER_SPEED;
					baron.key = FALSE;
					baron.mode = MOVE;
					baron.dire = NOMAL;
					baron.angle = 0;
					baron.keycnt = 0;

					//敵の初期化
					for (i = 0; i < MAX_ENEMY; i++) {
						enemy[i].x = enemy[i].init_x;
						enemy[i].y = enemy[i].init_y;
					}

					//その他の初期化
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
				//残機無し
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