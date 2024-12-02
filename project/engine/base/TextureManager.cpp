#include "TextureManager.h"
#include "StringUtility.h"



using namespace StringUtility;

TextureManager* TextureManager::instance = nullptr;

TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr) {

		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Finalize()
{

	delete instance;
	instance = nullptr;

}

void TextureManager::Initialize(DirectXCommon* dxCommon,SrvManager*srvmanager)
{
	dxCommon_ = dxCommon;
	this->srvmanager = srvmanager;
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);

}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filepath)
{
	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);
	TexturData& textureData = textureDatas[filepath];
	return textureData.metadata;
}

//Imgui で０番を使用するため１番から使用
uint32_t TextureManager::kSRVIndexTop = 1;
void TextureManager::LoadTexture(const std::string& filePath)
{

	////読み込み済みテクスチャを検索
	//auto it = std::find_if(
	//	textureDatas.begin(),
	//	textureDatas.end(),
	//	[&](TexturData& textureData) {return textureData.filePath == filePath; }
	//);
	//if (it != textureDatas.end()) {
	//	return;//酔いこみ済みなら早期return

	//}

	if (textureDatas.contains(filePath)) {

		return;//酔いこみ済みなら早期return

	}

	assert(srvmanager->CheckTexturesNumber());


	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミニマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	////テクスチャデータを追加
	//textureDatas.resize(textureDatas.size() + 1);
	//追加したデータの参照を取得する
	TexturData& textureData = textureDatas[filePath];

	//textureData.filePath= ConvertString(filePathW);
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);

	Microsoft::WRL::ComPtr<ID3D12Resource>  intermediateResource = dxCommon_->UploadTextureData(textureData.resource, mipImages);
	dxCommon_->CommandKick();

	////テクスチャデータの要素番号をSRVのインデックスとする
	//uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size()-1)+kSRVIndexTop;

	textureData.srvIndex = srvmanager->Allocate();
	textureData.srvHandleCPU = srvmanager->GetCPUDrscriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvmanager->GetGPUDrscriptorHandle(textureData.srvIndex);

	//meraDaraを気にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{  };
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);


}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filepath)
{
	
	if (textureDatas.contains(filepath)) {

		return textureDatas.at(filepath).srvIndex;

		
		
	}

	////読み込み済みテクスチャを検索
	

	//if (it != textureDatas.end()) {
	//	//読み込み済みなら要素番号を返す
	//	uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
	//	return textureIndex;
	//}

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filepath)
{
	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

	TexturData& textureData = textureDatas.at(filepath);

	return textureData.srvHandleGPU;
}



