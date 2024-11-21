#pragma once
#include <Windows.h>

#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>
class Input
{
public:
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	//初期化
	void Initialize(HINSTANCE hInstance, HWND hwnd);
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

};

