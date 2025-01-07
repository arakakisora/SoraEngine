#pragma once
#include <Windows.h>

#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>
#include "WinApp.h"
template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
class Input
{
	static Input* instance;
	Input() = default;
	~Input() = default;
	Input(Input&) = default;
	Input& operator=(Input&) = delete;
	
public:
	//シングルトンインスタンスの取得
	static Input* GetInstance();
	//終了
	void Finalize();



	//初期化
	void Initialize(WinApp*winApp);
	//更新
	void Update();

	//キーの状態
	bool PushKey(BYTE keyNumber);//押してるとき
	bool TriggerKey(BYTE keyNumber);//押したとき

private:
	ComPtr<IDirectInput8>directInput = nullptr;
	BYTE key[256] = {};
	BYTE preKey[256] = {};
	ComPtr<IDirectInputDevice8>keyboard;
	WinApp* winApp_ = nullptr;

};

