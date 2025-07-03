#include "LoadJson.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <json.hpp>


void LoadJson::LoadJsonFile()
{
	//連結してフルパスを得る
	const std::string fullpath = std::string{ "Resources/levels/" } + "noname.json"; // Example path, change as needed
	//ファイルストリーム
	std::ifstream file;
	//ファイルを開く
	file.open(fullpath);
	//ファイルが開けなかったら
	if (file.fail()) {
		assert(0);
	}

	nlohmann::json deserialize;
	//ファイルから読み込む
	file >> deserialize;
	//正しいレベルデータファイルかチェック
	assert(deserialize.is_object());
	assert(deserialize.contains("name"));
	assert(deserialize["name"].is_string());

	//nameを文字列として取得
	std::string name = deserialize["name"].get<std::string>();
	//正しいデータかチェック
	assert(name.compare("scene")==0);

	

}
