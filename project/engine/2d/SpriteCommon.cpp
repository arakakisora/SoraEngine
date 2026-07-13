#include "SpriteCommon.h"
#include "Logger.h"
#include <memory> // std::make_unique

std::unique_ptr<SpriteCommon> SpriteCommon::instance_ = nullptr;

SpriteCommon* SpriteCommon::GetInstance()
{
	// 必要に応じて生成（安全に make_unique を使用）
	if (instance_ == nullptr) {
		instance_ = std::make_unique<SpriteCommon>();
	}
	return instance_.get();
}

void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	// パイプラインの生成
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateSprite();
}

void SpriteCommon::Finalize()
{
	// 内部リソースを先に解放する（順序を明示）
	if (graphicsPipeline_) {
		graphicsPipeline_.reset();
	}

	
	instance_.reset();
}

void SpriteCommon::CommonDraw()
{
	// RootSignature を設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureSprite());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateSprite());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}






