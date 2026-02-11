#include "TextureManager.h"
#include "StringUtility.h"
#include <cassert>
#include <filesystem>


using namespace StringUtility;

std::unique_ptr <TextureManager> TextureManager::instance = nullptr;

TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr) {

		instance = std::make_unique <TextureManager>();
	}
	return instance.get();
}

void TextureManager::Finalize()
{

	instance.reset();

}

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* provisionalsrvmanager)
{
	dxCommon_ = dxCommon;
	this->srvmanager = provisionalsrvmanager;
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);

}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filepath)
{
	
	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);
	//テクスチャデータを取得
	TextureData& textureData = textureDatas[filepath];
	return textureData.metadata;
}

//Imgui で０番を使用するため１番から使用
uint32_t TextureManager::kSRVIndexTop = 1;
void TextureManager::LoadTexture(const std::string& filePath)
{



	if (textureDatas.contains(filePath)) {

		return;//酔いこみ済みなら早期return

	}

	assert(srvmanager->CheckTexturesNumber());


	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr;
	if (filePathW.ends_with(L".dds")) {
		//DDSファイルの場合はLoadFromDDSFileを使用
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		//WIC対応ファイルの場合はLoadFromWICFileを使用
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(hr));


	//ミニマップの作成
	DirectX::ScratchImage mipImages{};
	if(DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	} else {
		//非圧縮テクスチャの場合はGenerateMipMapsを使用
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImages);
	}
	
	assert(SUCCEEDED(hr));

	
	//追加したデータの参照を取得する
	TextureData& textureData = textureDatas[filePath];

	//textureData.filePath= ConvertString(filePathW);
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);

	Microsoft::WRL::ComPtr<ID3D12Resource>  intermediateResource = dxCommon_->UploadTextureData(textureData.resource, mipImages);
	dxCommon_->CommandKick();

	//SRVの作成
	textureData.srvIndex = srvmanager->Allocate();
	textureData.srvHandleCPU = srvmanager->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvmanager->GetGPUDescriptorHandle(textureData.srvIndex);

	srvmanager->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metadata.format, (UINT)textureData.metadata.mipLevels, textureDatas[filePath].metadata);

}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filepath)
{

	if (textureDatas.contains(filepath)) {

		return textureDatas[filepath].srvIndex;

	}

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filepath)
{
	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

	return textureDatas.at(filepath).srvHandleGPU;
}

std::string TextureManager::PreferDDSPath(const std::string& originalPath)
{
	namespace fs = std::filesystem;

	fs::path p(originalPath);

	// すでに .dds ならそのまま
	if (p.has_extension() && p.extension() == ".dds") {
		return originalPath;
	}

	// 拡張子だけ .dds に差し替え
	fs::path ddsPath = p;
	ddsPath.replace_extension(".dds");

	// 同名ddsが存在するならdds優先
	if (fs::exists(ddsPath)) {
		return ddsPath.string();
	}

	// 無ければ元のまま
	return originalPath;
}



