#include "Object3DCommon.h"
#include "Logger.h"
#include "SrvManager.h"

Object3DCommon* Object3DCommon::instance_ = nullptr;
Object3DCommon* Object3DCommon::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new Object3DCommon();
	}
	return instance_;

}

void Object3DCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager_)
{

	dxCommon_ = dxCommon;
	srvManager_ = srvManager_;
	//パイプラインの生成
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->Create();
	//コンピュートパイプラインの生成
	skinningComputeGraphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	skinningComputeGraphicsPipeline_->Initialize(dxCommon_);
	skinningComputeGraphicsPipeline_->CreateCompute();

}

void Object3DCommon::Finalize()
{
	
	delete instance_;
	instance_ = nullptr;
}

void Object3DCommon::CommonDraw()
{

	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignature());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineState());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//コンピュートシェーダーを設定
	dxCommon_->GetCommandList()->SetComputeRootSignature(skinningComputeGraphicsPipeline_->GetRootSignatureCompute());
	dxCommon_->GetCommandList()->SetPipelineState(skinningComputeGraphicsPipeline_->GetGraphicsPipelineStateCompute());

}


