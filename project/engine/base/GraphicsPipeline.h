#pragma once
#include "DirectXCommon.h"
#include <map>




enum class PostEffectType;
/// <summary>
/// グラフィックスパイプライン
/// </summary>
class GraphicsPipeline
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 作成
	/// </summary>
	void Create();//3dオブジェクト用
	/// <summary>
	/// ルートシグネチャ作成
	/// </summary>
	void RootSignatureCreate();//3dオブジェクト用

	/// <summary>
	/// パーティクル作成
	/// </summary>
	void CreateParticle();//パーティクル用
	/// <summary>
	/// パーティクル用ルートシグネチャ作成
	/// </summary>
	void RootSignatureParticleCreate();//パーティクル用
	/// <summary>
	/// スプライト作成
	/// </summary>
	void CreateSprite();//スプライト用
	/// <summary>
	/// スプライト用ルートシグネチャ作成
	/// </summary>
	void RootSignatureSpriteCreate();//スプライト用
	/// <summary>
	/// コピーイメージ作成
	/// </summary>
	/// <param name="type"></param>
	/// <param name="psFilename"></param>
	void CreateCopyImage(PostEffectType type, const std::wstring& psFilename);
	/// <summary>
	/// コピーイメージ用ルートシグネチャ作成
	/// </summary>
	void CreateAllPostEffects();
	/// <summary>
	/// コピーイメージ用ルートシグネチャ作成
	/// </summary>
	void RootSignatureCopyImageCreate();
	/// <summary>
	/// ライン作成
	/// </summary>
	void CreateLine();//ライン用
	/// <summary>
	/// ライン用ルートシグネチャ作成
	/// </summary>
	void RootSignatureLineCreate();//ライン用
	/// <summary>
	/// スキニング作成
	/// </summary>
	void CreateSkinning();//スキニング用
	/// <summary>
	/// スキニング用ルートシグネチャ作成
	/// </summary>
	void RootSignatureSkinningCreate();//スキニング用
	/// <summary>
	/// Skybox作成
	/// </summary>
	void CreateSkybox();//Skybox用
	/// <summary>
	/// Skybox用ルートシグネチャ作成
	/// </summary>
	void RootSignatureSkyboxCreate();//Skybox用

	//アクセッサ

	//ゲッター
	ID3D12RootSignature* GetRootSignature()const { return rootSignature.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineState()const { return graphicsPipelineState.Get(); }
	//パーティクル用のPSO
	ID3D12RootSignature* GetRootSignatureParticle()const { return rootSignatureParticle.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateParticle()const { return graphicsPipelineStateParticle.Get(); }

	//スプライト用のPSO
	ID3D12RootSignature* GetRootSignatureSprite()const { return rootSignatureSprite.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateSprite()const { return graphicsPipelineStateSprite.Get(); }

	//コピーイメージ用のPSO
	ID3D12RootSignature* GetRootSignatureCopyImage()const { return rootSignatureCopyImage.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateCopyImage(PostEffectType type);

	//ライン用のPSO
	ID3D12RootSignature* GetRootSignatureLine()const { return rootSignatureLine.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateLine()const { return graphicsPipelineStateLine.Get(); }

	//スキニング用のPSO
	ID3D12RootSignature* GetRootSignatureSkinning()const { return rootSignatureSkinning.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateSkinning()const { return graphicsPipelineStateSkinning.Get(); }

	//Skybox用のPSO
	ID3D12RootSignature* GetRootSignatureSkybox()const { return rootSignatureSkybox.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateSkybox()const { return graphicsPipelineStateSkybox.Get(); }

private:
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	//スキニング用のルートシグネチャとパイプラインステートオブジェクト
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureSkinning = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateSkinning = nullptr;

	//パーティクル用のルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureParticle = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateParticle = nullptr;


	//スプライト用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureSprite = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateSprite = nullptr;


	//コピーイメージ用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureCopyImage = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateCopyImage = nullptr;
	std::map<PostEffectType, Microsoft::WRL::ComPtr<ID3D12PipelineState>> copyImagePipelines_; 


	//ライン用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureLine = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateLine = nullptr;

	//Skybox用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureSkybox = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateSkybox = nullptr;



};

