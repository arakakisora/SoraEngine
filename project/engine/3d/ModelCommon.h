#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
/// <summary>
/// モデル共通クラス
/// </summary>
class ModelCommon
{
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvMnager);

	//アクセッサ
	/// <summary>
	/// DXCommon
	/// </summary>
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	/// <summary>
	/// SRVManager
	/// </summary>
	SrvManager* GetSRVManager() { return srvMnager_; }

private:
	DirectXCommon* dxCommon_;
	SrvManager* srvMnager_ = nullptr;


};

