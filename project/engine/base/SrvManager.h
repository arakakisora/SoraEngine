#pragma once
//#include <d3d12.h>
//#include <dxgi1_6.h>
//#include <wrl.h>
#include "DirectXCommon.h"


class SrvManager
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxcommon);

	/// <summary>
	//アロケータ（ヒープのアドレスを指定するやつ）
	/// </summary>
	uint32_t Allocate();

	/// <summary>
	//cpu、gpuの計算用関数
	/// </summary>
	/// <param name="index"></param>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	/// <summary>
	//SRV生成（テクスチャ用）
	/// </summary>
	/// <param name="srvIndex"></param>
	/// <param name="pResource"></param>
	/// <param name="foemat"></param>
	/// <param name="MipLevels"></param>
	/// <param name="metadata"></param>
	/// </returns></returns>
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT foemat, UINT MipLevels, DirectX::TexMetadata metadata);
	/// <summary>
	//SRV生成(structured Buffer用)
	/// </summary>
	/// <param name="srvIndex"></param>
	/// <param name="pResourece"></param>
	/// <param name="numElements"></param>
	/// <param name="structureByteStride"></param>
	/// </returns></returns>
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResourece, UINT numElements,
		UINT structureByteStride);
	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();
	/// <summary>
	/// グラフィックスパイプラインにSRVをセット
	/// </summary>
	/// <param name="RootprameterIndex"></param>
	/// <param name="srvIndex"></param>
	void SetGraficsRootDescriptorTable(UINT RootprameterIndex, uint32_t srvIndex);
	/// <summary>
	/// コンピュートパイプラインにSRVをセット
	/// </summary>
	/// <returns></returns>
	bool CheckTexturesNumber();
private:
	DirectXCommon* directXCommon = nullptr;
	//最大SRV数（最大テクスチャ枚数）
	static const uint32_t kMaxSRVCount;
	//SRV用のデスクリプタサイズ
	uint32_t descriptorSize;
	//SRV用のデスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	//次に使用するSRVインデックス
	uint32_t useIndex = 0;


};

