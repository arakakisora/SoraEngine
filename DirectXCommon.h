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
	void DepthBufferInitialize();
	void DescriptorHeepInitialize();
public:
	//初期化
	void Initialize(WinApp* winApp);
	//<summary>
	//デスクリプタヒープを生成する
	//</summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heaptype,
		UINT numDescriptrs, bool shaderVisible);

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

	//DepthBuffer
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStenciResource;

	//DescriptorHeep
	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;//RTV
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;	//SRV
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;//DVS用のヒープでディスクリプタの数は1．//DSVはShader内で触るものではない



};

