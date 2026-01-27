#include "SkyBoxCommon.h"


std::unique_ptr <SkyBoxCommon> SkyBoxCommon::instance_ = nullptr;
SkyBoxCommon* SkyBoxCommon::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = std::make_unique <SkyBoxCommon>();
	}
	return instance_.get();
}

void SkyBoxCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvmanager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvmanager;

	graphicsPipeline_ = std::make_unique <GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateSkybox();

}

void SkyBoxCommon::Finalize()
{

	graphicsPipeline_.reset();
	instance_.reset();

}

void SkyBoxCommon::commonDraw()
{

	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureSkybox());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateSkybox());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}
