#pragma once
#include"DirectXCommon.h"
#include"SrvManager.h"
#include "GraphicsPipeline.h"
#include <Vector2.h>

// OfscreenRenderManager.h の上部 or 外部ファイルで定義
//enum class PostEffectType {
//
//	Fullscreen,
//	Grayscale,
//	Vignette,
//	BoxFilter,
//	LuminanceOutline
//};

enum class PostEffectType {
	Fullscreen,
	Grayscale,
	Vignette,
	BoxFilter,
	LuminanceOutline,
	RdialBlur,
	LightTrail,

};

struct TrailParamsCPU {
	float t;          // 0→1
	float direction;  // +1 L→R, -1 R→L
	float width;
	float softness;
	float headRound;
	float glow;
	float trailLen;
	Vector2 vignette;
	float _pad[2];
};

class OfscreenRenderManager
{
public:
	//初期化
	void Initialize(DirectXCommon* dxcommon, SrvManager*srvmanager);
	//描画前処理
	void Begin();
	//描画後処理
	void End();
	
	void Draw();
	
	//RenderTargetTextureの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTargetTextureResource(uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& ClearColor);

	void SetPostEffectType(PostEffectType type) {
		currentEffectType_ = type;
	}

	void DrawImGui();

	// ---- LightTrail用関数 ----
	void BeginLightTrail(float durationSec = 0.55f, int dir = +1);
	void UpdateLightTrail(float dt);
	void DrawLightTrail();     // backbufferへ出す直前に実行

private:

	//DirectXCommonのポインタ
	DirectXCommon* dxCommon_ = nullptr;
	//SRVManagerのポインタ
	SrvManager* srvManager_ = nullptr;
	//レンダーテクスチャ
	DirectX::TexMetadata renderTargetMetadata_;
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTextureResource;//レンダーテクスチャ
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetTextureHandle;//レンダーテクスチャのハンドル
	//const Vector4 clearColor = { 0.1f,0.25f,0.5f,1.0f };//とりあえず赤
	const Vector4 clearColor = { 0.843f, 0.843f, 0.616f, 1.0f };
	uint32_t srvIndex = 0;

	PostEffectType currentEffectType_ = PostEffectType::Fullscreen; // ←追加
	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;

	D3D12_RESOURCE_STATES currentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;


	//ライトトレイル用CPUパラメータ

	 // ---- LightTrail用 ----
	Microsoft::WRL::ComPtr<ID3D12Resource> prevTex_;   // 前シーン
	D3D12_CPU_DESCRIPTOR_HANDLE prevRTV_{};            // 必要なら
	uint32_t prevSrvIndex_ = 0;                        // t0にバインドするSRV
	D3D12_RESOURCE_STATES prevState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	bool trailActive_ = false;
	float trailTime_ = 0.f, trailDuration_ = 0.55f;
	TrailParamsCPU trailParams_{ // 推奨初期値
		0.f, +1.f, 0.22f, 0.03f, 0.8f, 0.7f, 0.45f, {0.2f, 0.6f}, {0,0}
	};

	// 小さなCBV（256Bアライン）
	Microsoft::WRL::ComPtr<ID3D12Resource> trailCB_;
};

