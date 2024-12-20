#include "SpriteCommon.h"
#include "Logger.h"

SpriteCommon::SpriteCommon()
{
}

SpriteCommon::~SpriteCommon()
{
	delete graphicsPipeline_;

}

void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	
	graphicsPipeline_ = new GraphicsPipeline();
	graphicsPipeline_->Initialize(dxCommon);
	graphicsPipeline_->Create();
}

void SpriteCommon::CommonDraw()
{
	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignature());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineState());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}






