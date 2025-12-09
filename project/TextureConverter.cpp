#include "TextureConverter.h"
#include <windows.h>

void TextureConverter::ComvertTextureWICToDDS(const std::string& filePath)
{
	LoadWICTextureFromFile(filePath);
	SaveDDSTextureToFile();

}

void TextureConverter::LoadWICTextureFromFile(const std::string& filePath)
{
	std::wstring wfilePath = ConvertMultiByteStringToWideString(filePath);

	// WICテクスチャをファイルから読み込む
	HRESULT hr = DirectX::LoadFromWICFile(
		wfilePath.c_str(),
		DirectX::WIC_FLAGS_NONE,
		&metadata,
		scratchImage
	);
	assert(SUCCEEDED(hr));

	//フォルダパスとファイル名を分解
	SeparateFilePath(wfilePath);

}

std::wstring TextureConverter::ConvertMultiByteStringToWideString(const std::string& input)
{
	// 変換後のワイド文字列の長さを取得
	int wideLength = MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, nullptr, 0);

	// ワイド文字列用のバッファを確保
	std::wstring wideString;
	wideString.resize(wideLength);
	// マルチバイト文字列をワイド文字列に変換
	MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, &wideString[0], wideLength);
	return wideString;
}

void TextureConverter::SeparateFilePath(const std::wstring& filePath)
{
    size_t pos1;
    std::wstring exceptExt;

    //区切り文字'.'が出てくる一番最後の部分を検索
    pos1 = filePath.rfind(L".");
    //拡張子が存在する場合
    if (pos1 != std::wstring::npos) {
        //拡張子を取得
        fileExt_ = filePath.substr(pos1 + 1, filePath.size() - pos1 - 1);
        //拡張子を除いたパスを取得
        exceptExt = filePath.substr(0, pos1);
    }
    else {
        //拡張子が無い場合は空文字を設定
        fileExt_ = L"";
        exceptExt = filePath;
    }
	//区切り文字'\'が出てくる一番最後の部分を検索
	pos1 = exceptExt.rfind(L"\\");
	if (pos1 != std::wstring::npos) {
		//区切り文字の前までをディレクトリパスとして保存
		directoryPath_ = exceptExt.substr(0, pos1 + 1);
		//区切り文字の次から最後までをファイル名として保存
		fileName_ = exceptExt.substr(pos1 + 1, exceptExt.size() - pos1 - 1);
		return;
	}
	//区切り文字’/’が出てくる一番最後の部分を検索
	pos1 = exceptExt.rfind(L"/");
	if (pos1 != std::wstring::npos) {
		//区切り文字の前までをディレクトリパスとして保存
		directoryPath_ = exceptExt.substr(0, pos1 + 1);
		//区切り文字の次から最後までをファイル名として保存
		fileName_ = exceptExt.substr(pos1 + 1, exceptExt.size() - pos1 - 1);
		return;
	}
	//区切り文字が無い場合はディレクトリパスを空文字、ファイル名をパス全体として保存
	directoryPath_ = L"";
	fileName_ = exceptExt;
	
}

void TextureConverter::SaveDDSTextureToFile()
{//絞り込んだテクスチャをSRGBとして扱う
	metadata.format = DirectX::MakeSRGB(metadata.format);
	HRESULT result;
	//出力ファイル名を設定する
	std::wstring filePath = directoryPath_ + fileName_ + L".dds";
	//DDSテクスチャとしてファイル書き出し
	result=SaveToDDSFile(
		scratchImage.GetImages(),
		scratchImage.GetImageCount(),
		metadata,
		DirectX::DDS_FLAGS_NONE,
		filePath.c_str()
	);
	assert(SUCCEEDED(result));
}

