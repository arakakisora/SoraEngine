#pragma once
#include"DirectXCommon.h"
#include"SrvManager.h"
#include "GraphicsPipeline.h"


enum class PostEffectType {
	Fullscreen,
	Grayscale,
	Vignette,
	BoxFilter,
	LuminanceOutline,
	RdialBlur,

};
/// <summary>
/// オフスクリーンレンダーマネージャー
/// </summary>
class OfscreenRenderManager
{
public:
	/// <summary>
	//初期化
	/// </summary>
	/// <param name="dxcommon"></param>
	/// <param name="srvmanager"></param>
	void Initialize(DirectXCommon* dxcommon, SrvManager*srvmanager);
	/// <summary>
	//描画前処理
	/// </summary>
	/// <returns></returns>
	void Begin();
	/// <summary>
	//描画後処理
	/// </summary>
	/// <returns></returns>
	void End();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	/// <summary>
	//RenderTargetTextureの生成
	/// </summary>
	/// <param name="width"></param>
	/// <param name="height"></param>
	/// <param name="format"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTargetTextureResource(uint32_t width, uint32_t height, DXGI_FORMAT format);
	/// <summary>
	/// 現在のポストエフェクトタイプを設定する
	/// </summary>
	/// <param name="type"></param>
	void SetPostEffectType(PostEffectType type) {
		currentEffectType_ = type;
	}
	/// <summary>
	/// ImGui描画
	/// </summary>
	void DrawImGui();
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
	//const Vector4 clearColor = { 0.843f, 0.843f, 0.616f, 1.0f };
	//const Vector4 clearColor = { 0.65f, 0.65f, 0.45f, 1.0f };
	const Vector4 clearColor = { 0.25f,0.25f,0.5f,1.0f };
	uint32_t srvIndex = 0;

	PostEffectType currentEffectType_ = PostEffectType::Fullscreen; 
	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;

	D3D12_RESOURCE_STATES currentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;


};

