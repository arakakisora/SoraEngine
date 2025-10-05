#pragma once
#pragma once
#include "Camera.h"	
#include "Model.h"
#include"Sprite.h"
#include "Object3D.h"
#include "Audio.h"
#include "BaseScene.h"
#include "FadeManager.h"
#include "LoadJson.h"
#include <numbers> // 追加


enum class TitleAnimState {
	IntroRun,   // プレイヤーが走り始める（タイトルはまだオフスクリーン）
	HookTitle,  // フックが掛かった瞬間（ワンショットで次へ）
	DragTitle,  // 引っ張り中（スプリングで目標へ近づく）
	Settle,     // 収束（減衰が十分小さくなったら Idle）
	ReturnUnderTitle,
	Idle        // 静止（スペースキー受付など通常待機）
};

class TitleScene :public BaseScene
{

public:

	/// <summary>
	/// シーンの初期化
	/// </summary>
	void Initialize()override;
	/// <summary>
	/// シーンの終了処理
	/// </summary>
	void Finalize()override;
	/// <summary>
	/// シーンの更新
	/// </summary>
	void Update()override;
	/// <summary>
	/// シーンの描画
	/// </summary>
	void Draw()override;

	// 1周分の初期化
	void ResetTitleAnimation();

public:


	//taitorusprite
	Sprite* titleSprite = nullptr;

	FadeManager fadeManager_;

	LoadJson* loadJson_ = nullptr;
	Object3D* object3D_ = nullptr;
	Object3D* titleObj_ = nullptr;
	Camera* camera = nullptr;



	TitleAnimState state_ = TitleAnimState::IntroRun;

	// プレイヤーのX座標（簡単のためフレーム固定Δで進める）
	float playerX_ = -8.0f;
	float playerY_ = -1.0f;    // 見た目調整用
	float playerSpeed_ = 0.12f; // 速さ（好みで調整）

	// フックが掛かるX位置（ワールド座標の目安）
	float hookAtX_ = 1.5f;

	// タイトルスプライトの位置（ピクセル座標）
	Vector3 titlePos3_{ -12.0f, 0.0f, 0.0f };   // 左外から開始（単位はワールド）
	Vector3 titleTarget3_{ 0.0f, 0.0f, 0.0f };  // カメラ中心へ寄せる

	float springX_ = -12.0f;   // ← 初期位置に合わせる
	float springV_ = 0.0f;
	float springK_ = 0.035f;
	float springD_ = 0.78f;
	float titleModelScale_ = 3.0f;
	float titleYOffset_ = 1.0f;
	// 補助
	bool ropeAttached_ = false;


	float playerTargetUnderTitleX_ = 0.5f;   // タイトルの真下(中央)あたり
	float playerTargetUnderTitleY_ = -1.0f;  // 高さは今と同じ
	float playerReturnSpeed_ = 0.08f;        // 戻る速さ


	// レイヤーの向き定数 ---
	float kYawFront = 3.1f; // 正面
	float kYawRight = kYawFront - std::numbers::pi_v<float> *0.5f; // 右
	float kYawLeft = kYawFront + std::numbers::pi_v<float> *0.5f; // 左

	// ループ用
	bool loopFadeOutRequested_ = false;  // 多重呼び出し防止
	float idleTimer_ = 0.0f;             // Idleでの待機時間

	bool requestSceneChange_ = false;

	// ループ制御
	bool loop_ = true;           // 常にループ
	float loopWaitSec_ = 1.5f;   // 静止表示の秒数

	

};

