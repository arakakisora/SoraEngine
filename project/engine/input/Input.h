#pragma once
#include <Windows.h>

#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>
#include "WinApp.h"
#include <Vector2.h>
#include <array>
template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")
/// <summary>
/// 入力管理クラス
/// </summary>
class Input
{
	/// <summary>
	/// シングルトンインスタンス
	/// </summary>
	static Input* instance;
	Input() = default;
	~Input() = default;
	Input(Input&) = default;
	Input& operator=(Input&) = delete;

public: // インナークラス
	/// <summary>
	/// マウス移動量構造体
	/// </summary>
	struct MouseMove {
		LONG lX;
		LONG lY;
		LONG lZ;
	};
public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	static Input* GetInstance();
	/// <summary>
	//終了
	/// </summary>
	void Finalize();
	/// <summary>
	//初期化
	/// </summary>
	/// <param name="winApp"></param>
	void Initialize(WinApp* winApp);
	/// <summary>
	//更新
	/// </summary>
	void Update();
	/// <summary>
	//キーの状態
	/// </summary>
	/// <param name="keyNumber"></param>
	bool PushKey(BYTE keyNumber);//押してるとき
	/// <summary>
	//キーの状態
	/// </summary>
	/// <param name="keyNumber"></param>
	bool TriggerKey(BYTE keyNumber);//押したとき

	/// <summary>
	//マウスの状態
	/// </summary>
	/// <param name="buttonNumber"></param>
	bool PushMouse(int buttonNumber);
	/// <summary>
	/// マウスの状態
	/// </summary>
	/// <param name="buttonNumber"></param>
	/// <returns></returns>
	bool TriggerMouse(int buttonNumber);
	/// <summary>
	//マウスの座標
	/// </summary>
	/// <returns></returns>
	const Vector2& GetMousePos()const { return mousePos; };
	/// <summary>
	//マウスの移動量
	/// </summary>
	/// <returns></returns>
	MouseMove GetMouseMove()const {
		MouseMove move;
		move.lX = mouse.lX;
		move.lY = mouse.lY;
		move.lZ = mouse.lZ;
		return move;
	};
	/// <summary>
	//ボタンの入力状態
	/// </summary>
	/// <param name="button"></param>
	bool PushGamePadButton(WORD button);
	/// <summary>
	/// ボタンの入力状態
	/// </summary>
	/// <param name="button"></param>
	/// <returns></returns>
	bool TriggerGamePadButton(WORD button);
	
	//スティックとトリガーの値取得
	/// <summary>
	/// スティックのX軸値取得
	/// </summary>
	/// <param name="righ"></param>
	/// <returns></returns>
	float GetGamePadStickX(bool righ = false);
	/// <summary>
	/// スティックのY軸値取得
	/// </summary>
	/// <param name="righ"></param>
	/// <returns></returns>
	float GetGamePadStickY(bool righ = false);
	/// <summary>
	/// トリガーの値取得
	/// </summary>
	/// <param name="righ"></param>
	/// <returns></returns>
	BYTE GetGamePadTrigger(bool righ = false);

	/// <summary>
	// バイブレーション制御
	/// </summary>
	/// <param name="leftMotor">左モーターの強さ（0.0f～1.0f）</param>
	/// <param name="rightMotor">右モーターの強さ（0.0f～1.0f）</param>
	void SetVibration(float leftMotor, float rightMotor);



private:
	ComPtr<IDirectInput8>directInput = nullptr;
	BYTE key[256] = {};
	BYTE preKey[256] = {};
	ComPtr<IDirectInputDevice8>keyboard;
	WinApp* winApp_ = nullptr;

	Microsoft::WRL::ComPtr<IDirectInputDevice8> devMouse_;
	DIMOUSESTATE2 mouse;
	DIMOUSESTATE2 preMouse;
	Vector2 mousePos;

	//ゲームパッド
	XINPUT_STATE state_; // 現在のゲームパッド状態
	XINPUT_STATE prevState_; // 前回のゲームパッド状態
	bool gamepadConnected_ = false;


};

