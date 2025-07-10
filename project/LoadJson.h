#pragma once
#include "Model.h"
#include "MyMath.h"
#include "Object3D.h"
#include <json.hpp>


struct LevelData {
	struct ObjectData {
		std::string fileName; // ファイル名
		EulerTransform transform; // 位置、回転、スケール
	};
	std::vector<ObjectData> objects;

};

class LoadJson
{

public:
	LoadJson() = default;
	//読み込み
	void LoadJsonFile();
	//初期化
	void Initialize();
	void Finalize();
	//更新　　
	void Update();
	//描画
	void Draw(); // シーンに配置したオブジェクトを描画


	void ParseObjectRecursive(const nlohmann::json& object);
   
private:
	
	LevelData* levelData = nullptr;
	std::vector<Object3D*> objects; // シーンに配置するオブジェクトのリスト



};

