#pragma once
#include <Windows.h>
#include <cstdint>

/// <summary>
/// Windowsアプリケーションクラス
/// </summary>
class WinApp
{

public:

	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public:
	/// <summary>
	/// ウィンドウプロシージャ
	/// </summary>
	/// <param name="hwnd"></param>
	/// <param name="msg"></param>
	/// <param name="wparam"></param>
	/// <param name="lparam"></param>
	/// <returns></returns>
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	//終了処理
	/// </summary>
	void Finalize();

	//Getter
	HWND GetHwnd()const { return hwnd; }
	HINSTANCE GetHInstance()const { return wc.hInstance; }

	bool ProcessMessage();

private:
	//ウィンドウ生成
	HWND hwnd = nullptr;
	WNDCLASS wc{};


};

