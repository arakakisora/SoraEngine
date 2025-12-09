#pragma once
#include <string>
#include "externals/DirectXTex/DirectXTex.h"
class TextureConverter
{
public:
	///<summary>
	///テクスチャをWICからDDSに変換する
	///</summary>
	/// <param name="inputFilePath">変換元のテクスチャファイルパス</param>
	void ComvertTextureWICToDDS(const std::string& filePath);

	///<summary>
	///テクスチャファイル読み込み
	/// </summary>
	/// <param name="FilePath">変換元のマルチバイト文字列</param>
	/// <returns>変換後のワイド文字列</returns>
	void LoadWICTextureFromFile(const std::string& filePath);

	///<summary>
	///マルチバイト文字列をワイド文字列に変換する
	/// </summary>
	/// <param name="input">変換元のマルチバイト文字列</param>
	/// <returns>変換後のワイド文字列</returns>
	static std::wstring ConvertMultiByteStringToWideString(const std::string& input);

private:
	/// <summary>
	/// フォルダパスとファイル名を分解する
	/// </summary>
	///<param name="filePath">分解するファイルパス</param>
	void SeparateFilePath(const std::wstring& filePath);
	//ディレクトリパス
	std::wstring directoryPath_;
	//ファイル名
	std::wstring fileName_;
	//拡張子
	std::wstring fileExt_;
private:
/// <summary>
/// DDSテクスチャとしてファイル書き出し
/// </summary>
	void SaveDDSTextureToFile();
private:
	//画像の情報
	DirectX::TexMetadata metadata;
	//画像のイメージコンテナ
	DirectX::ScratchImage scratchImage;



};

