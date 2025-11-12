#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include <array>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include<vector>
#include <chrono>
#include <thread>  // std::this_thread
#include <Vector4.h>

/// <summary>
/// DirectX共通処理
/// </summary>
class DirectXCommon
{
	/// <summary>
	/// デバイス初期化
	/// </summary>
	void DeviceInitialize();
	/// <summary>
	/// コマンド初期化
	/// </summary>
	void CommandInitialize();
	/// <summary>
	/// スワップチェーン初期化
	///	</summary>
	void SwapChainInitialize();
	/// <summary>
	/// RTV初期化
	/// </summary>
	void DepthBufferInitialize();
	/// <summary>
	/// デスクリプタヒープ初期化
	/// </summary>
	void DescriptorHeepInitialize();
	/// <summary>
	/// RTV初期化
	/// </summary>
	void RTVInitialize();
	/// <summary>
	/// DSV初期化
	/// </summary>
	void DSVInitialize();
	/// <summary>
	/// フェンス初期化
	/// </summary>
	void FenceInitialize();
	/// <summary>
	/// ビューポート初期化
	/// </summary>
	void ViewportInitialize();
	/// <summary>
	/// シザー矩形初期化
	/// </summary>
	void ScissorInitialize();
	/// <summary>
	/// DxcCompiler初期化
	/// </summary>
	void DxcCompilerInitialize();

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp);
	/// <summary>
	/// 描画前処理
	/// </summary>
	void Begin();
	/// <summary>
	/// 描画後処理
	/// </summary>
	void End();

	//<summary>
	//SRVの指定番号のCPUデスクリプタハンドルを取得
	//</summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriputorHandole(uint32_t index);
	//<summary>
	//SRVの指定番号のGPUデスクリプタハンドルを取得
	//</summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriputorHandole(uint32_t index);

	//<summary>
	//SRVの指定番号のCPUデスクリプタハンドルを取得
	//</summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriputorHandole(uint32_t index);
	//<summary>
	//SRVの指定番号のGPUデスクリプタハンドルを取得
	//</summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriputorHandole(uint32_t index);

	//<summary>
	//SRVの指定番号のCPUデスクリプタハンドルを取得
	//</summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriputorHandole(uint32_t index);
	//<summary>
	//SRVの指定番号のGPUデスクリプタハンドルを取得
	//</summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriputorHandole(uint32_t index);

	/// <summary>
	/// 指定したデスクリプタヒープのCPUデスクリプタハンドルを取得
	/// </summary>
	/// <param name="descriptorHeap"></param>
	/// <param name="descriptorSize"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDesctiptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,
		uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// 指定したデスクリプタヒープのGPUデスクリプタハンドルを取得
	/// </summary>
	/// <param name="descriptorHeap"></param>
	/// <param name="descriptorSize"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDesctiptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,
		uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// デバイスを取得する
	/// </summary>
	/// <returns></returns>
	ID3D12Device* GetDevice() const { return device.Get(); }//デバイスを取得する
	/// <summary>
	/// コマンドリストを取得する
	/// </summary>
	ID3D12GraphicsCommandList* GetCommandList()const { return commandList.Get(); }//コマンドリストを取得する
	/// <summary>
	/// RTVのビュー記述子を取得する
	/// </summary>
	const D3D12_RENDER_TARGET_VIEW_DESC& GetRTVDesc() const { return rtvDesc; }

	/// <summary>
	/// getdsvDescriptorHeap
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDSVDescriptorHeap() { return dsvDescriptorHeap; }
	/// <summary>
	/// getviewport
	/// </summary>
	D3D12_VIEWPORT GetViewport() const { return viewport; }
	/// <summary>
	/// getscissorRect
	/// </summary>
	D3D12_RECT GetScissorRect() const { return scissorRect; }
	/// <summary>
	/// CompileShader関数の作成
	/// </summary>
	/// <param name="filePath">ComilerするSahaderファイルへのパス</param>
	/// <param name="profile">compilerに使用するProfile</param>
	/// <returns></returns>
	IDxcBlob* CompileShader(
		//ComilerするSahaderファイルへのパス
		const std::wstring& filePath,
		//compilerに使用するProfile
		const wchar_t* profile);
	/// <summary>
	//デスクリプタヒープを生成する
	/// </summary>
	///<param name="heaptype">ヒープの種類</param>
	/// <param name="numDescriptrs">デスクリプタの数</param>
	/// <param name="shaderVisible">シェーダーから見えるかどうか</param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heaptype,
		UINT numDescriptrs, bool shaderVisible);
	/// <summary>
	/// バッファーリソースの生成
	/// </summary>
	/// <param name="sizeInBytes">バッファーサイズ</param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	/// <summary>
	//テクスチャリソースの生成
	/// </summary>
	/// <param name="metadata">テクスチャメタデータ</param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	/// <summary>
	/// テクスチャデータの転送
	/// </summary>
	/// <param name="texture">テクスチャリソース</param>
	/// <param name="mipImages">テクスチャイメージ群</param>
	/// <returns></returns>
	[[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource>texture, const DirectX::ScratchImage& mipImages);
	/// <summary>
	/// バックバッファの数を取得
	/// </summary>
	/// <returns></returns>
	size_t GetBackBufferCount()const { return swapChainResources.size(); }
	/// <summary>
	/// コマンドキューを実行しキックする
	/// </summary>
	/// <returns></returns>
	void CommandKick();
	/// <summary>
	/// 最大SRV数(最大テクスチャ枚数)
	/// </summary>
	/// <returns></returns>
	static const uint32_t kMaxSRVCount;
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
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2>swapChainResources;
	//DepthBuffer
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStenciResource;
	//DescriptorHeep
	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;//RTV
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;	//SRV
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;//DVS用のヒープでディスクリプタの数は1．//DSVはShader内で触るものではない


	//RTV
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStarHandle;
	//RVTを2つ作るのでディスクリプタを2つ用意
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> rtvHandles;
	//fence
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	HANDLE fenceEvent;
	uint64_t fenceValue = 0;
	//ビューポート
	D3D12_VIEWPORT viewport{};
	//シザー矩形
	D3D12_RECT scissorRect{};
	//DXC
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;
	//barrier
	D3D12_RESOURCE_BARRIER barrier{};
	//記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;



private:


	//FPS固定初期化
	void InitializeFixFPS();
	//FPS固定更新
	void UodateFixFPS();




};

