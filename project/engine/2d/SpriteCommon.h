#pragma once
#include "DirectXCommon.h"
#include "GraphicsPipeline.h"
class SpriteCommon
{
public:
	SpriteCommon();
	~SpriteCommon();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);
	//共通描画設定
	void CommonDraw();
	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
private:

	

private:
	DirectXCommon* dxCommon_;//dxcommnをポインタ参照
	GraphicsPipeline* graphicsPipeline_ = nullptr;
	

	


};

