#pragma once
#include "DirectXCommon.h"
#include "GraphicsPipeline.h"
#include "SrvManager.h"

class LineCommon
{
public:
	static LineCommon* GetInstance();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon,SrvManager *srvManager);
	//終了
	void Finalize();
	//共通描画設定
	void CommonDraw();
	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	//SrvMnager
	SrvManager* GetSrvmanager()const { return srvManager_; }

private:
	LineCommon() = default;
	~LineCommon() = default;
	LineCommon(const LineCommon&) = delete;
	LineCommon& operator=(const LineCommon&) = delete;
private:
	//インスタンス
	static LineCommon* instance_;
	DirectXCommon* dxCommon_;
	SrvManager* srvManager_;
	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;


};

