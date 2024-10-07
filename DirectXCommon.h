#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"

class DirectXCommon
{
	void DeviceInitialize();
	void CommandInitialize();
	void SwapChainInitialize();
public:
	//初期化
	void Initialize(WinApp* winApp);

private:

	//WindowsAPI
	WinApp* winApp_ = nullptr;
	HRESULT hr;
	//device
	Microsoft::WRL::ComPtr< IDXGIFactory7> dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	//Command
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	//SwapChain
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
};

