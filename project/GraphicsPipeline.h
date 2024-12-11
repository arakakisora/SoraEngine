#pragma once
#include "DirectXCommon.h"
class GraphicsPipeline
{
public:

	void Create(DirectXCommon* dxCommon);

	//ルートシグネチャの作成
	void RootSignatureCreate();

	void RootSignatureParticleCreate();

	//ゲッター
	ID3D12RootSignature* GetRootSignature()const { return rootSignature.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineState()const { return graphicsPipelineState.Get(); }
	
private:
	DirectXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};

