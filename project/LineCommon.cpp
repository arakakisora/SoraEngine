#include "LineCommon.h"

LineCommon* LineCommon::instance_ = nullptr;
LineCommon* LineCommon::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new LineCommon();
	}
	return instance_;
}

void LineCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	//パイプラインの生成
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateLine();
}
void LineCommon::Finalize()
{
	delete instance_;
	instance_ = nullptr;
}
void LineCommon::CommonDraw()
{
	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureLine());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateLine());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
}
