#pragma once
#pragma once
#include "Camera.h"	
#include "Model.h"
#include"Sprite.h"
#include "Object3D.h"
#include "Audio.h"
#include "BaseScene.h"
#include "FadeManager.h"
#include <numbers> 
#include <memory>
#include "GateInOut.h"   



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
	
	void PlayerParticle();

	void ImguiDraw();
private:


private:


	enum class LRDirecion {
		kright,
		kLeft,
	};

	
	//std::unique_ptr<Sprite> titleSprite_;
	float exhaustTimer_ = 0.0f;
	FadeManager fadeManager_;

	std::unique_ptr<GateInOut> gate_; 
	bool gateOutRequested_ = false;
	bool fadeOutRequested_ = false;


	std::unique_ptr<Object3D> object3D_;
	std::unique_ptr<Object3D> titleObj_;
	std::unique_ptr<Camera> camera;
	// ディレクションライトの向き（右 or 左）
	LRDirecion lrDirection_;
	TitleAnimState state_ = TitleAnimState::IntroRun;

	// プレイヤーのX座標（簡単のためフレーム固定Δで進める）
	float playerX_ = -8.0f;
	float playerY_ = -1.0f;    // 見た目調整用
	float playerSpeed_ = 0.12f; // 速さ（好みで調整）

	// フックが掛かるX位置（ワールド座標の目安）
	float hookAtX_ = 1.5f;

	// タイトルスプライトの位置（ワールド座標）
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


	// レイヤーの向き定数 
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

	// マジックナンバーの定数化（しきい値・UIサイズなど）
	static inline constexpr float kPlayerMaxX = 12.0f;
	static inline constexpr float kTitleStartX = -12.0f;
	static inline constexpr float kTitleSpritePosX = 490.0f;
	static inline constexpr float kTitleSpritePosY = 600.0f;
	static inline constexpr float kTitleSpriteW = 300.0f;
	static inline constexpr float kTitleSpriteH = 100.0f;

	static inline constexpr float kSpringCloseDist1 = 0.5f;
	static inline constexpr float kSpringVelThresh1 = 0.05f;
	static inline constexpr float kSpringCloseDist2 = 0.1f;
	static inline constexpr float kSpringVelThresh2 = 0.02f;
	static inline constexpr float kPlayerReturnEpsilon = 0.001f;

	static inline constexpr float kExhaustInterval = 1.0f / 15.0f; // 1/15秒ごとに出す

	bool loopFadePlaying_ = false;
	bool loopDoReset_ = false;
};

