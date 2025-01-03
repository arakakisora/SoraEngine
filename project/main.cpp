








#include "Game.h"
#include "Framework.h"



// windowアプリでのエントリ―ポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;

	CoInitializeEx(0, COINIT_MULTITHREADED);
	//出力ウィンドウへの文字出力
	OutputDebugStringA("HEllo,DIrectX!\n");


#pragma region 基盤システム初期化

	Framework* game = new Game();
	//ゲーム初期化
	game->Run();
	
	delete game;

	return 0;

}
