#include "Object3DCommon.h"
#include "Logger.h"

Object3DCommon* Object3DCommon::instance_ = nullptr;
Object3DCommon* Object3DCommon::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new Object3DCommon();
	}
	return instance_;

}

void Object3DCommon::Initialize(DirectXCommon* dxCommon)
{

	dxCommon_ = dxCommon;
	graphicsPipelineState_ = new GraphicsPipeline();
	graphicsPipelineState_->Initialize(dxCommon_);
	graphicsPipelineState_->Create();
	

}

void Object3DCommon::Finalize()
{
	delete graphicsPipelineState_;
	delete instance_;
	instance_ = nullptr;
}

void Object3DCommon::CommonDraw()
{

	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipelineState_->GetRootSignature());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState_->GetGraphicsPipelineState());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}


