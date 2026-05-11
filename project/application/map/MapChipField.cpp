#include "MapChipField.h"
#include <fstream>
#include <map>
#include <sstream>
#include "MapChipDatabase.h"



void MapChipField::ResetMapChipData() {
	// マップチップデータのリセット
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (auto& line : mapChipData_.data) {
		line.clear();
		line.resize(kNumBlockHorizontal, MapChipType::Empty);
	}

	// hpData_ のリセット
	hpData_.clear();
	hpData_.resize(kNumBlockVirtical);
	for (auto& line : hpData_) {
		line.clear();
		line.resize(kNumBlockHorizontal, 0);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	// マップチップデータをリセット
	ResetMapChipData();

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	// マップチップCSV
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();
	// ファイルを閉じる
	file.close();

	std::string line;
	uint32_t y = 0;

	while (std::getline(mapChipCsv, line)) {
		std::stringstream lineStream(line);
		std::string cell;
		uint32_t x = 0;

		while (std::getline(lineStream, cell, ',')) {
			if (y < kNumBlockVirtical && x < kNumBlockHorizontal) {
				// CSV の文字列をそのまま int に変換して保存
				MapChipType id = static_cast<MapChipType>(std::stoi(cell));
				mapChipData_.data[y][x] = id;

				
				const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(id);
				if (info) {
					hpData_[y][x] = info->hitPoints;
				}
				else {
					hpData_[y][x] = 0;
				}
			}
			++x;
		}
		++y;
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {

	// 範囲チェック（unsigned なので 0 未満チェックは不要）
	if (xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVirtical) {
		return MapChipType::Empty; // 範囲外は Empty 扱い（id 0 を想定）
	}
	return mapChipData_.data[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPostionByIndex(uint32_t xIndex, uint32_t yIndex) {
	// 指定したインデックスのマップチップのワールド座標を返す
	return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0);
}

IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	// 指定したワールド座標からマップチップのインデックスを返す
	IndexSet indexSet = {};
	// インデックスを計算
	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2) / kBlockWidth);
	indexSet.yIndex = kNumBlockVirtical - 1 - static_cast<uint32_t>((position.y + kBlockHeight / 2) / kBlockHeight);
	return indexSet;

}

Rect MapChipField::GetRectByIndex(uint32_t xindex, uint32_t yIndex) {
	// 指定したインデックスからマップチップの矩形情報を取得
	Vector3 center = GetMapChipPostionByIndex(xindex, yIndex);
	// 矩形情報を計算して返す
	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;
	return rect;


}

std::vector<Vector3> MapChipField::GetEnemyPositions()
{
	std::vector<Vector3> enemyPositions;
	Enemynumber.clear();

	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {

			MapChipType typeId = GetMapChipTypeByIndex(x, y);
			const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(typeId);
			if (!info) {
				continue;
			}

			// JSON で spawn == "enemy" のタイルを敵として扱う
			if (info->spawn == "enemy") {
				enemyPositions.push_back(GetMapChipPostionByIndex(x, y));
				Enemynumber.push_back(info->enemyNumber); // enemyNumber をそのまま保存
			}
		}
	}
	return enemyPositions;
}

Vector3 MapChipField::GetGoalPosition() {
	Vector3 pos{ 0.0f, 0.0f, 0.0f };

	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {

			MapChipType typeId = GetMapChipTypeByIndex(x, y);
			const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(typeId);
			if (!info) {
				continue;
			}

			// JSON で spawn == "goal" のタイルをゴールとして扱う
			if (info->spawn == "goal") {
				pos = GetMapChipPostionByIndex(x, y);
				// 複数ある場合は最後のものが採用される
			}
		}
	}
	return pos;
}

bool MapChipField::IsSolid(uint32_t xIndex, uint32_t yIndex) 
{
	MapChipType typeId = GetMapChipTypeByIndex(xIndex, yIndex);
	const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(typeId);
	if (!info) {
		return false;
	}
	// JSON の collision == "solid" を「通れないブロック」として扱う
	return info->collision == "solid";
}

std::vector<Vector3> MapChipField::GetPositionBySpwan(const std::string& spawnTag)
{
	std::vector<Vector3> result;

	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {

			MapChipType typeId = GetMapChipTypeByIndex(x, y);
			const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(typeId);

			if (info->spawn == spawnTag) {
				result.push_back(GetMapChipPostionByIndex(x, y));
			}
		}
	}

	return result;
}

//  指定インデックスのHPを取得
int MapChipField::GetMapChipHPByIndex(uint32_t xIndex, uint32_t yIndex) const {
	if (xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVirtical) return 0;
	return hpData_[yIndex][xIndex];
}

//  指定インデックスにダメージを与える（HPが0以下になったらタイルを 0 にする）
void MapChipField::DamageMapChipByIndex(uint32_t xIndex, uint32_t yIndex, int damage) {
	if (xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVirtical) return;

	int& hp = hpData_[yIndex][xIndex];
	if (hp <= 0) return; // 既に壊れない/空
	hp -= damage;
	if (hp <= 0) {
		// 壊れた -> 空にする
		mapChipData_.data[yIndex][xIndex] = MapChipType::Empty;
		hp = 0;

	}
}

//  ワールド座標からダメージを与えるユーティリティ
void MapChipField::DamageMapChipByPosition(const Vector3& position, int damage) {
	IndexSet idx = GetMapChipIndexSetByPosition(position);
	DamageMapChipByIndex(idx.xIndex, idx.yIndex, damage);
}
