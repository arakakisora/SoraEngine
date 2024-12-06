#include "Object3DCommon.h"
#include "Logger.h"


Object3DCommon::Object3DCommon()
{


}

Object3DCommon::~Object3DCommon()
{
	delete graphicsPipeline_;

}

void Object3DCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	graphicsPipeline_ = new GraphicsPipeline();
	graphicsPipeline_->Create(dxCommon_);

}

void Object3DCommon::CommonDraw()
{

	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignature());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineState());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}


