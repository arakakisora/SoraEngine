#pragma once
#include <string>

/// <summary>
/// 文字列ユーティリティ
/// </summary>
namespace StringUtility {

	/// <summary>
	//DirectX12デバイス
	/// </summary>
	/// <param name="str"></param>
	/// <returns></returns>
	std::wstring ConvertString(const std::string& str);
	/// <summary>
	//DXGIファクトリ
	/// </summary>
	/// <param name="str"></param>
	std::string ConvertString(const std::wstring& str);
}
