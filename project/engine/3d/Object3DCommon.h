#pragma once
#include "DirectXCommon.h"
#include "Camera.h"

class Object3DCommon
{
public:

	static Object3DCommon* GetInstance();


	
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	//終了
	void Finalize();

	//共通描画設定
	void CommonDraw();
	
	//ルートシグネチャの作成
	void RootSignatureInitialize();
	//グラフィックスパイプライン
	void GraphicsPipelineInitialize();

	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

	//void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	//Camera* GetDefaultCamera()const { return defaultCamera; }

private:

	Object3DCommon() = default;
	~Object3DCommon() = default;
	Object3DCommon(const Object3DCommon&) = delete;
	Object3DCommon& operator=(const Object3DCommon&) = delete;

private:

	//インスタンス
	static Object3DCommon* instance_;

	DirectXCommon* dxCommon_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	Camera* defaultCamera = nullptr;
};

