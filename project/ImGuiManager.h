#pragma once
#include "DirectXCommon.h"
#include "WinApp.h"
class ImGuiManager
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon,WinApp*winapp);
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>srvHeap_;

	//DirectXCommon* dxCommon = nullptr;
};

