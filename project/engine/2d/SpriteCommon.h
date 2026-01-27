#pragma once
#include <memory> // std::unique_ptr
#include "DirectXCommon.h"
#include "GraphicsPipeline.h"
/// <summary>
/// スプライト共通クラス
/// </summary>
class SpriteCommon
{
public:

	static SpriteCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	void Finalize();

	//共通描画設定
	void CommonDraw();
	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }


private:

	
	static std::unique_ptr<SpriteCommon> instance_;

	DirectXCommon* dxCommon_;//dxcommnをポインタ参照

	//グラフィックスパイプライン（unique_ptr）
	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;


};

