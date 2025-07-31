#include "LoadJson.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <string>
#include <vector>
#include "ModelManager.h"
#include "engine/3d/Object3DCommon.h"




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
	assert(name.compare("SCENE") == 0);

	levelData = new LevelData();
	
	for (const auto& object : deserialize["objects"]) {
		ParseObjectRecursive(object);
	}
}

void LoadJson::Initialize()
{

	// レベルデータからオブジェクトを生成、配置
	for (auto& objectData : levelData->objects) {
		// ファイル名から登録済みモデルを検索

		ModelManager::GetInstans()->LoadModel(objectData.fileName);

		Object3D* newObject = new Object3D();// 新規オブジェクト生成
		newObject->Initialize(Object3DCommon::GetInstance());
		newObject->SetModel(objectData.fileName);// モデル名をセット
		// トランスフォームをセット
		newObject->SetTransform(objectData.transform);
		// 配列に登録
		objects.push_back(newObject);
	}

}

void LoadJson::Finalize()
{
	// オブジェクトの解放
	for (auto& object : objects) {
		delete object;
	}
	objects.clear();
	// レベルデータの解放
	delete levelData;
	levelData = nullptr;
}

void LoadJson::Update()
{
	// オブジェクトの更新処理
	for (auto& object : objects) {
		object->Update();
	}


}

void LoadJson::Draw()
{
	// オブジェクトの描画処理
	for (auto& object : objects) {
		object->Draw();
	}

}

void LoadJson::ParseObjectRecursive(const nlohmann::json& object)
{
	

	assert(object.contains("type"));
	std::string type = object["type"].get<std::string>();

	if (type == "MESH") {
		levelData->objects.emplace_back(LevelData::ObjectData{});
		LevelData::ObjectData& objectData = levelData->objects.back();

		if (object.contains("file_name") && object["file_name"].is_string()) {
			objectData.fileName = object["file_name"];
		}

		const auto& transform = object["transform"];

		objectData.transform.translate.x = (float)transform["translation"][0];
		objectData.transform.translate.y = (float)transform["translation"][2];
		objectData.transform.translate.z = (float)transform["translation"][1];

		objectData.transform.rotate.x = -(float)transform["rotation"][0];
		objectData.transform.rotate.y = -(float)transform["rotation"][2];
		objectData.transform.rotate.z = -(float)transform["rotation"][1];

		objectData.transform.scale.x = (float)transform["scale"][0];
		objectData.transform.scale.y = (float)transform["scale"][2];
		objectData.transform.scale.z = (float)transform["scale"][1];
	} else if (type == "PlayerSpawn") {
		// 配列に要素を追加
		levelData->players.emplace_back(LevelData::PlayerSpawnData{});
		LevelData::PlayerSpawnData& spawnData = levelData->players.back();

		// トランスフォームデータのパース（MESHと同様）
		const auto& transform = object["transform"];
		spawnData.transform.translate.x = (float)transform["translation"][0];
		spawnData.transform.translate.y = (float)transform["translation"][2];
		spawnData.transform.translate.z = (float)transform["translation"][1];

		spawnData.transform.rotate.x = -(float)transform["rotation"][0];
		spawnData.transform.rotate.y = -(float)transform["rotation"][2];
		spawnData.transform.rotate.z = -(float)transform["rotation"][1];

		spawnData.transform.scale.x = (float)transform["scale"][0];
		spawnData.transform.scale.y = (float)transform["scale"][2];
		spawnData.transform.scale.z = (float)transform["scale"][1];
	}

	// 子オブジェクトがあれば再帰呼び出し
	if (object.contains("children") && object["children"].is_array()) {
		for (const auto& child : object["children"]) {
			ParseObjectRecursive(child);
		}
	}
}
