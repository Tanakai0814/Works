#include "DxLib.h"
#include "../h/maindata.h"
#include "../h/process.h"
#include "../h/keyboard.h"

#define UI_X_ADD 20		//UIのx位置の微調整用


typedef struct {

	int ui_medal[3];	// 0 大 1 中 2 小

}IMAGE;

typedef struct {

	int entry_name;

}SOUND;

typedef struct {

	IMAGE img;
	SOUND snd;

}MATERIAL;

void Result_Phase(MAINDATA *data) {

	RANKING_DATA top_score[TOP_SCORE_NUM];	//ランキング
	RANKING_DATA change;					//記録入れ替え用

	MATERIAL mtl;

	int i, j;
	int update_flg = FALSE;		//更新したかどうか
	int update_no;
	int getname_flg = FALSE;	//更新した場合名前を取得したかどうか

	/*デバッグ用*/
	//int debug = GetRand(3);
	int file_del_flg = FALSE;
	//data->total_score = 150000;

	/*** Image ***/
	LoadDivGraph("Image/ui_medal.png", 3, 3, 1, 13, 15, mtl.img.ui_medal);

	/*** Sound ***/
	mtl.snd.entry_name = LoadSoundMem("Sound/BGM/Result_name_entry.wav");
	PlaySoundMem(mtl.snd.entry_name, DX_PLAYTYPE_LOOP);
	
	/*** ランキングデータの読み込み ***/
	FILE *fp_r = fopen("ranking.dat", "rb");
	
	//ランキングデータがあるかどうか
	if (fp_r == NULL) {
		//初期データを読み込む(初回起動時)
		FILE *fp_ir = fopen("ranking_init.dat", "rb");
		fread(&top_score, sizeof(top_score), 1, fp_ir);
		fclose(fp_ir);
	}
	else {
		//ランキングデータを読み込む
		fread(&top_score, sizeof(top_score), 1, fp_r);
		fclose(fp_r);
	}

	while (Process_Loop() == 0) {

		/***処理***/
		/*更新したかの確認*/
		if (update_flg == FALSE) {

			/*デバッグ用*/
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

			/*スコア更新したかどうか*/
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

			/*順番の変更*/
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

		/***描画***/
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

		/*記録更新した場合*/
		if (update_flg == TRUE) {
			DrawString(40, 210, "PLEASE INPUT YOUR NAME!", YELLOW);

			//名前入力されていない場合8文字まで名前を取得
			if (getname_flg == FALSE) {
				for (i = 0; i < TOP_SCORE_NUM; i++) {
					if (top_score[i].no == update_no) {
						//文字色を黄色
						SetKeyInputStringColor2(DX_KEYINPSTRCOLOR_NORMAL_STR, YELLOW);
						SetKeyInputStringColor2(DX_KEYINPSTRCOLOR_NORMAL_CURSOR, YELLOW);
						
						//入力中Enterで入力終了
						KeyInputSingleCharString(200, 105 + i * 17, 8, top_score[i].name, TRUE);
						getname_flg = TRUE;
						break;
					}
				}
			}
		}

		//デバック
		//ゲーム終了
		if (Keyboard_Get(KEY_INPUT_ESCAPE) == 1) {
			data->end = TRUE;
			break;
		}
		//タイトルに戻る
		if (Keyboard_Get(KEY_INPUT_Z) == 1) {
			break;
		}
		//ランキングデータ削除
		if (Keyboard_Get(KEY_INPUT_F) >= 1 && Keyboard_Get(KEY_INPUT_DELETE) == 1) {
			file_del_flg = TRUE;
		}

	}

	//順位の再振り分け
	for (i = 0; i < TOP_SCORE_NUM; i++) {
		top_score[i].no = i + 1;
	}

	/*** ランキングデータの出力 ***/
	FILE *fp_w = fopen("ranking.dat","wb");
	fwrite(&top_score, sizeof(top_score), 1, fp_w);
	fclose(fp_w);

	//BGMの停止
	StopSoundMem(mtl.snd.entry_name);

	/*** ゲームデータの初期化処理 ***/
	data->mode = BARON;				//BARON or BARON & MAX
	data->dead = FALSE;				//安否確認
	data->life = 1;					//残機
	data->current_stage = 1;		//現在のステージ
	data->score = 0;				//1ステージでのスコア
	data->total_score = 0;			//トータルスコア
	data->hight_score = 0;			//ハイスコア
	data->total_medal = 0;			//トータルメダル
	data->ground_enemy_kill = 0;	//討伐数（地上）
	data->sky_enemy_kill = 0;		//討伐数（上空）
	data->base_enemy_kill = 0;		//討伐数（基地）

	/*デバッグ*/
	//ファイル削除
	if (file_del_flg == TRUE) {
		remove("ranking.dat");
		file_del_flg = FALSE;
	}

	return;
}