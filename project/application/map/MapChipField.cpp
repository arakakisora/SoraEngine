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

	// 新しいStageDataも同じサイズで初期化
	stageData_.Resize(kNumBlockHorizontal, kNumBlockVirtical);
	stageData_.Clear();
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	// マップチップデータをリセット
	ResetMapChipData();
	portals_.clear();

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

				std::string cellText = cell;

				// 例: "3:0:right"
				std::stringstream cellParser(cellText);
				std::string typeText;
				std::getline(cellParser, typeText, ':');

				// CSV の文字列をそのまま int に変換して保存
				MapChipType id = static_cast<MapChipType>(std::stoi(typeText));
				mapChipData_.data[y][x] = id;

				
				stageData_.SetType(x, y, id);

				if (id == MapChipType::Portal) {
					std::string pairText;
					std::string dirText;

					if (std::getline(cellParser, pairText, ':') &&
						std::getline(cellParser, dirText, ':')) {

						PortalInfo portal;
						portal.pairId = std::stoi(pairText);
						portal.x = x;
						portal.y = y;
						portal.dir = DirFromString(dirText);

						portals_.push_back(portal);

						// StageData側にもギミック情報として保存
						StageCell& cellData = stageData_.At(x, y);
						cellData.linkId = portal.pairId;
						cellData.direction = portal.dir;
					}
				}
				
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
	if (xIndex >= stageData_.GetWidth() || yIndex >= stageData_.GetHeight()) {
		return MapChipType::Empty;
	}

	return stageData_.GetType(xIndex, yIndex);
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
	MapChipType type = GetMapChipTypeByIndex(xIndex, yIndex);

	const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(type);
	if (!info) {
		return false;
	}
	//collision == "solid" を「通れないブロック」として扱う
	return info->collision == "solid";
}

std::vector<Vector3> MapChipField::GetPositionBySpawn(const std::string& spawnTag)
{
	std::vector<Vector3> result;

	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {

			MapChipType typeId = GetMapChipTypeByIndex(x, y);
			const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(typeId);
			if (!info) {
				continue;
			}
			if (info->spawn == spawnTag) {
				result.push_back(GetMapChipPostionByIndex(x, y));
			}
		}
	}

	return result;
}

//  指定インデックスのHPを取得
int MapChipField::GetMapChipHPByIndex(uint32_t xIndex, uint32_t yIndex) const {
	if (xIndex >= stageData_.GetWidth() || yIndex >= stageData_.GetHeight()) {
		return 0;
	}

	return stageData_.GetHP(xIndex, yIndex);
}

//  指定インデックスにダメージを与える（HPが0以下になったらタイルを 0 にする）
void MapChipField::DamageMapChipByIndex(uint32_t xIndex, uint32_t yIndex, int damage) {
	if (xIndex >= stageData_.GetWidth() || yIndex >= stageData_.GetHeight()) {
		return;
	}

	stageData_.DamageHP(xIndex, yIndex, damage);

	// 並走期間中だけ、旧データにも反映しておく
	if (yIndex < mapChipData_.data.size() && xIndex < mapChipData_.data[yIndex].size()) {
		mapChipData_.data[yIndex][xIndex] = stageData_.GetType(xIndex, yIndex);
	}

	if (yIndex < hpData_.size() && xIndex < hpData_[yIndex].size()) {
		hpData_[yIndex][xIndex] = stageData_.GetHP(xIndex, yIndex);
	}
}

//  ワールド座標からダメージを与えるユーティリティ
void MapChipField::DamageMapChipByPosition(const Vector3& position, int damage) {
	IndexSet indexSet = GetMapChipIndexSetByPosition(position);
	DamageMapChipByIndex(indexSet.xIndex, indexSet.yIndex, damage);
}

bool MapChipField::TryGetPortal(uint32_t x, uint32_t y, PortalInfo& out) const
{
	for (const PortalInfo& portal : portals_) {
		if (portal.x == x && portal.y == y) {
			out = portal;
			return true;
		}
	}
	return false;
}

bool MapChipField::TryGetPairPortal(const PortalInfo& in, PortalInfo& out) const
{
	for (const PortalInfo& portal : portals_) {
		if (portal.pairId == in.pairId &&
			!(portal.x == in.x && portal.y == in.y)) {
			out = portal;
			return true;
		}
	}
	return false;
}

Vector3 MapChipField::DirFromString(const std::string& dir)
{
	{
		if (dir == "right") return { 1.0f, 0.0f, 0.0f };
		if (dir == "left")  return { -1.0f, 0.0f, 0.0f };
		if (dir == "up")    return { 0.0f, 1.0f, 0.0f };
		if (dir == "down")  return { 0.0f, -1.0f, 0.0f };

		return { 1.0f, 0.0f, 0.0f };
	}
}

void MapChipField::ApplyStageData(const StageData& stageData)
{
	stageData_ = stageData;

	// 旧データにも同期しておく
	mapChipData_.data.clear();
	mapChipData_.data.resize(stageData_.GetHeight());

	hpData_.clear();
	hpData_.resize(stageData_.GetHeight());

	for (uint32_t y = 0; y < stageData_.GetHeight(); ++y) {
		mapChipData_.data[y].resize(stageData_.GetWidth(), MapChipType::Empty);
		hpData_[y].resize(stageData_.GetWidth(), 0);

		for (uint32_t x = 0; x < stageData_.GetWidth(); ++x) {
			mapChipData_.data[y][x] = stageData_.GetType(x, y);
			hpData_[y][x] = stageData_.GetHP(x, y);
		}
	}

	// ポータル情報も再構築
	portals_.clear();

	for (uint32_t y = 0; y < stageData_.GetHeight(); ++y) {
		for (uint32_t x = 0; x < stageData_.GetWidth(); ++x) {
			const StageCell& cell = stageData_.At(x, y);

			if (cell.type == MapChipType::Portal) {
				PortalInfo portal;
				portal.pairId = cell.linkId;
				portal.x = x;
				portal.y = y;
				portal.dir = cell.direction;

				portals_.push_back(portal);
			}
		}
	}


}
