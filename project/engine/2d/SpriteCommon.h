#pragma once
#include "DirectXCommon.h"
class SpriteCommon
{
public:
	


	/// <summary>
		/// 初期化
		/// </summary>
	void Initialize(DirectXCommon* dxCommon);
	//共通描画設定
	void CommonDraw();
	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
private:

	//ルートシグネチャの作成
	void RootSignatureInitialize();
	//グラフィックスパイプライン
	void GraphicsPipelineInitialize();

private:
	DirectXCommon* dxCommon_;//dxcommnをポインタ参照
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;//パイプラインステイトオブジェクト

	


};

