#include "DxLib.h"
#include "../h/maindata.h"


/************************************************************
関数名：タイトルフェーズ
引数：ゲームデータ, イメージ
概要：タイトルを描画する。ゲームのプレイ人数選択もここで行う。
************************************************************/
void Title_Phase(MAINDATA *data);


/************************************************************
関数名：ゲームフェーズ
引数：ゲームデータ, イメージ
戻り値：敵の撃破数とスコアと生死状態
概要：以下の処理を行う
ステージで必要な画像の読み込み
キー操作
計算
描画
************************************************************/
void Game_Phase(MAINDATA *data);


/************************************************************
関数名：ボーナスフェーズ
引数：ゲームデータ, イメージ
概要：ステージクリア後の撃破数に応じてボーナス点の加算, 30000点を超えたら残機を１UPする
************************************************************/
void Bonus_Phase(MAINDATA *data);


/************************************************************
関数名：リザルトフェーズ
引数：ゲームデータ, イメージ
概要：ゲーム結果の保存を行う。
************************************************************/
void Result_Phase(MAINDATA *data);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	/*** ウィンドウ制御 ***/
	SetGraphMode(X_SIZE, Y_SIZE, 16);
	SetWindowSizeChangeEnableFlag(TRUE);
	SetWindowSizeExtendRate(2.0);

	SetMainWindowText("SKYKID");	//ウィンドウタイトル
	SetWindowIconID(252);	//252は指定したIDなので特に意味はないです

	/*** Dxlibの初期化 ***/
	if (ChangeWindowMode(TRUE) == -1 || DxLib_Init() == -1 || SetDrawScreen(DX_SCREEN_BACK) == -1) {
		return -1;
	}


	/*** 初期化処理 ***/
	MAINDATA data = {BARON, FALSE, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FALSE, 0, FALSE};


	//ゲーム中のフォント設定
	SetFontSize(12);								//フォントサイズ
	SetFontThickness(5);							//フォントの太さ
	ChangeFont("Pixel Emulator");					//フォント
	ChangeFontType(DX_FONTTYPE_ANTIALIASING);		//アンチエイリアスフォントに変更



	/*** ゲーム本編 ***/

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


	/*** 終了処理 ***/
	DxLib_End();
	return 0;
}
