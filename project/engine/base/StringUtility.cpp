#include "StringUtility.h"
#include "Windows.h"

namespace StringUtility {

	std::wstring ConvertString(const std::string& str)
	{
		// 空文字チェック
		if (str.empty()) {
			return std::wstring();
		}
		// 必要なサイズを取得
		auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
		if (sizeNeeded == 0) {
			return std::wstring();
		}
		// 変換後の文字列を格納するwstringを生成
		std::wstring result(sizeNeeded, 0);
		// 文字列を変換
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
		return result;
	}

	std::string ConvertString(const std::wstring& str)
	{
		/// 空文字チェック
		if (str.empty()) {
			return std::string();
		}
		// 必要なサイズを取得
		auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
		if (sizeNeeded == 0) {
			return std::string();
		}
		// 変換後の文字列を格納するstringを生成
		std::string result(sizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
		return result;
	}

}