#include "MapChipDatabase.h"
#include "json.hpp"
#include <fstream>

static MapChipDatabase instance_;

// シングルトンインスタンス取得
MapChipDatabase* MapChipDatabase::GetInstance() { return &instance_; }

void MapChipDatabase::LoadJson(const std::string& path)
{
	std::ifstream file(path);							  // ファイルストリームを開く
	assert(file.is_open() && "Failed to open JSON file"); // ファイルが開けなかった場合はアサート

	nlohmann::json jsonData; // JSONデータオブジェクト
	file >> jsonData;		 // JSONデータを読み込む

	chips_.clear();
	chipsById_.clear();

	for (auto& chip : jsonData["chips"])
	{
		MapChipInfo info;
		info.id = chip["id"];		// マップチップIDを取得
		info.label = chip["label"]; // マップチップラベルを取得

		auto color = chip["color"]; // カラー配列を取得
		info.color = Vector4(static_cast<float>(color[0]), static_cast<float>(color[1]), static_cast<float>(color[2]),
							 static_cast<float>(color[3]) // マップチップカラーを設定
		);

		info.collision = chip["collision"]; // 衝突情報を取得
		info.spawn = chip["spawn"];			// スポーン情報を取得

		// 敵番号が存在する場合
		if (chip.contains("enemyNumber"))
		{
			// 敵番号を取得
			info.enemyNumber = chip["enemyNumber"];
		}

		// hitPoints が存在すれば読み込む（なければデフォルト 0）
		if (chip.contains("hitPoints"))
		{
			info.hitPoints = chip["hitPoints"];
		}

		chips_.push_back(info);
		chipsById_[info.id] = info;
	}
}

const MapChipInfo* MapChipDatabase::GetById(MapChipType id) const
{
	//
	auto it = chipsById_.find(id);
	if (it == chipsById_.end())
	{
		return nullptr;
	}
	return &it->second;
}
