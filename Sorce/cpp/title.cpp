#include "DxLib.h"
#include "../h/maindata.h"
#include "../h/process.h"
#include "../h/keyboard.h"


typedef struct {

	int title;		// タイトル画像

}IMAGE;

typedef struct {

	int start;		// スタート音

}SOUND;

typedef struct {

	IMAGE img;
	SOUND snd;

}MATERIAL;


void Title_Phase(MAINDATA *data) {

	/*** 初期化処理 ***/

	MATERIAL mtl;

	/*** Image ***/
	mtl.img.title = LoadGraph("Image/title.png");

	/*** Sound ***/
	mtl.snd.start = LoadSoundMem("Sound/Effect/Title_select.wav");


	/*** ハイスコアの読み込み ***/
	//ランキング初期データ
	RANKING_DATA init_top[TOP_SCORE_NUM] {
		{1, 10000, 5, "SILYUGAR"},
		{2, 1000, 4, "HELL25MK"},
		{3, 100, 3, "DAZIRIN"},
		{4, 10, 2, "TANAKAI"},
		{5, 0, 1, "NONAME"},
	};

	RANKING_DATA top_score[TOP_SCORE_NUM];

	//ランキング初期データを書き込む
	FILE *fp_iw = fopen("ranking_init.dat", "wb");
	fwrite(&init_top, sizeof(init_top), 1, fp_iw);
	fclose(fp_iw);

	//ランキングデータの読み込
	FILE *fp_r = fopen("ranking.dat", "rb");

	//ランキングデータがあるかどうか
	if (fp_r == NULL) {
		//初期データを読み込む(初回起動時等で使用)
		FILE *fp_ir = fopen("ranking_init.dat", "rb");
		fread(&top_score, sizeof(top_score), 1, fp_ir);
		fclose(fp_ir);
	}
	else {
		//ランキングデータを読み込む
		fread(&top_score, sizeof(top_score), 1, fp_r);
		fclose(fp_r);
	}

	data->hight_score = top_score[0].score;		//ランキングのトップスコアをハイスコアに入れる

	/*** １フレーム処理 ***/
	while (Process_Loop() == 0) {

		// 画像の描画
		DrawRotaGraph(150, 80, 1, 0.0, mtl.img.title, TRUE);

		// 文字の描画（左上から）
		DrawString(30, 0, "1UP", RED);								// 1UP
		DrawString(130, 0, "TOP", YELLOW);							// TOP
		DrawFormatString(75, 15, WHITE, "%d", data->total_score);	// 現在スコア
		DrawFormatString(175, 15, WHITE, "%d", data->hight_score);	// ハイスコア
		DrawString(90, 170, "c 1985 NAMCO", WHITE);					// コピーライト（Ⓒだと文字化けするからcで代用中）
		DrawString(70, 185, "ALL RIGHTS RESERVED", WHITE);			// コピーライト

		ChangeFont("namco regular");								// namcoロゴ
		SetFontSize(8);												// フォントの都合で一瞬だけサイズ変更してる
		DrawString(120, 205, "namco", RED);
		ChangeFont("Pixel Emulator");
		SetFontSize(12);


		// 右下のクレジット
		DrawString(220, 225, "CREDIT", WHITE);
		DrawFormatString(280, 225, WHITE, "%d", data->credit);


		// クレジット数に応じて出てくる中央の文字
		if (data->credit == 1) {
			DrawString(90, 140, "TO START PUSH", SKYBLUE);			// 1クレ
			DrawString(55, 155, "ONLY 1 PLAYER’S BUTTON", SKYBLUE);
		}
		if (data->credit >= 2) {
			DrawString(90, 140, "TO START PUSH", SKYBLUE);			// 2クレ以上
			DrawString(55, 155, "1 OR 2 PLAYER’S BUTTON", SKYBLUE);	// なお表示のみ
		}

		if (data->debug_flg == TRUE) {
			DrawString(10, Y_SIZE - 20, "Debug ON", RED);
		}

		// xキーで100円入れる
		if (Keyboard_Get(KEY_INPUT_C) == 1) {
			data->credit++;
			PlaySoundMem(mtl.snd.start, DX_PLAYTYPE_BACK);
		}

		// Zキーで次へ進む
		if (data->credit > 0 && Keyboard_Get(KEY_INPUT_Z) == 1) {
			data->dead = FALSE;
			data->credit--;
			break;
		}

		// Dキーでデバックを可能にする切り替え
		if (data->debug_flg == FALSE && data->credit >= 3 && Keyboard_Get(KEY_INPUT_D) == 1) {
			PlaySoundMem(mtl.snd.start, DX_PLAYTYPE_BACK);
			data->debug_flg = TRUE;
			data->credit -= 3;
		}
		else if (data->debug_flg == TRUE &&  Keyboard_Get(KEY_INPUT_D) == 1) {
			data->debug_flg = FALSE;
		}

		// ESCキーで終了
		if (Keyboard_Get(KEY_INPUT_ESCAPE) == 1) {
			DxLib_End;
			exit(1);
		}

	}

	return;
}