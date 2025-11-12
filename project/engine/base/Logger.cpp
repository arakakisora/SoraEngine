#include "Logger.h"
#include "Windows.h"

namespace Logger {

	/// <summary>
	/// ログ出力
	/// </summary>
	/// <param name="message"></param>
	void Logger::Log(const std::string& message)
	{
		OutputDebugStringA(message.c_str());

	}

}
