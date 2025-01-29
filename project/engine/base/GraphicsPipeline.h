#pragma once
#include "DirectXCommon.h"
class GraphicsPipeline
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);


	void Create();//3dオブジェクト用
	void CreateParticle();//パーティクル用
	void CreateSprite();//スプライト用


	//ルートシグネチャの作成
	void RootSignatureCreate();//3dオブジェクト用
	void RootSignatureParticleCreate();//パーティクル用
	void RootSignatureSpriteCreate();//スプライト用


	//ゲッター
	ID3D12RootSignature* GetRootSignature()const { return rootSignature.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineState()const { return graphicsPipelineState.Get(); }
	//パーティクル用のPSO
	ID3D12RootSignature* GetRootSignatureParticle()const { return rootSignatureParticle.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateParticle()const { return graphicsPipelineStateParticle.Get(); }

	//スプライト用のPSO
	ID3D12RootSignature* GetRootSignatureSprite()const { return rootSignatureSprite.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateSprite()const { return graphicsPipelineStateSprite.Get(); }


private:
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	//パーティクル用のルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureParticle = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateParticle = nullptr;


	//スプライト用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureSprite = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateSprite = nullptr;


};

