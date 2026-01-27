#pragma once
#include <string>
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <unordered_map>



class TextureManager
{
private:

	//テクスチャ1枚分のデータ
	struct TextureData {

		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource>resource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;

	};
public:
	static std::unique_ptr <TextureManager> instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = delete;

	/// <summary>
	//シングルトンインタンス
	/// </summary>
	static TextureManager* GetInstance();
	/// <summary>
	//終了
	/// </summary>
	void Finalize();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvmanager);
	/// <summary>
	//メタデータを取得
	/// </summary>
	/// <param name="filepath"></param>
	///	<returns></returns>
	const DirectX::TexMetadata& GetMetaData(const std::string&filepath);
	/// <summary>
	//テクスチャファイルの読み込み
	/// </summary>
	/// <param name="filePath"></param>
	void LoadTexture(const std::string& filePath);

	/// <summary>
	//SRVインデックスの開始番号
	/// </summary>
	/// <param name="filePath"></param>
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	/// <summary>
	//テクスチャ番号からCPUハンドルを取得
	/// </summary>
	/// <param name="filepath"></param>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filepath);

	/// <summary>
	//Srvの最初
	/// </summary>
	static uint32_t kSRVIndexTop;

private:

	//テクスチャデータ
	DirectXCommon* dxCommon_=nullptr;
	std::unordered_map<std::string, TextureData> textureDatas;
	SrvManager* srvmanager = nullptr;

};

