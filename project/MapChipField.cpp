#include "MapChipField.h"
#include <fstream>
#include <map>
#include <sstream>
#include <algorithm>



void MapChipField::ResetMapChipData() {

	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
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

	// csvからマップチップデータを読み込む
	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {

		std::string line;
		getline(mapChipCsv, line);

		// 1桁分の文字列をストリームに変換して解析しやすくする
		std::istringstream lien_stream(line);

		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {

			std::string word;
			getline(lien_stream, word, ',');

			if (mapChipTable.contains(word)) {
				mapChipData_.data[y][x] = mapChipTable[word];
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {

	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}
	if (yIndex < 0 || kNumBlockVirtical - 1 < yIndex) {
		return MapChipType::kBlank;
	}
	return mapChipData_.data[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPostionByIndex(uint32_t xIndex, uint32_t yIndex) { 

	Vector3 pos{};
	// X はそのまま
	int bx = static_cast<int>(xIndex) + offsetXBlocks_;
	// Y は「上が正」想定で反転（エディタ上の0行=最上段 を ワールドの maxY 側に）
	int by = offsetYBlocks_ + (heightBlocks_ - 1 - static_cast<int>(yIndex));

	// ブロック中心に配置（+0.5）
	pos.x = (bx + 0.5f) * kBlockWidth;
	pos.y = (by + 0.5f) * kBlockHeight;
	pos.z = 0.0f;
	return pos;

}

IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	IndexSet idx{};
	// ブロック中心基準を逆に計算
	float fx = position.x / kBlockWidth - 0.5f;
	float fy = position.y / kBlockHeight - 0.5f;

	int bx = static_cast<int>(std::floor(fx + 0.5f));
	int by = static_cast<int>(std::floor(fy + 0.5f));

	// 反転の逆変換
	int ix = bx - offsetXBlocks_;
	int iy = (heightBlocks_ - 1) - (by - offsetYBlocks_);

	ix = std::clamp(ix, 0, (int)kNumBlockHorizontal - 1);
	iy = std::clamp(iy, 0, (int)kNumBlockVirtical - 1);

	idx.xIndex = (uint32_t)ix;
	idx.yIndex = (uint32_t)iy;
	return idx;

}

Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {

	int bx = static_cast<int>(xIndex) + offsetXBlocks_;
	int by = offsetYBlocks_ + (heightBlocks_ - 1 - static_cast<int>(yIndex));

	Rect r{};
	r.left = bx * kBlockWidth;
	r.right = (bx + 1) * kBlockWidth;
	r.bottom = by * kBlockHeight;
	r.top = (by + 1) * kBlockHeight;
	return r;


}    
  
std::vector<Vector3> MapChipField::GetPositionsByType(MapChipType target) {
	std::vector<Vector3> result;
	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			if (GetMapChipTypeByIndex(x, y) == target) {
				result.push_back(GetMapChipPostionByIndex(x, y));
			}
		}
	}
	return result;
}

const std::vector<std::vector<int>>& MapChipField::GetAllNumbers() const
{
	static std::vector<std::vector<int>> numbers;
	numbers.clear();
	numbers.resize(mapChipData_.data.size());

	for (size_t y = 0; y < mapChipData_.data.size(); ++y) {
		numbers[y].resize(mapChipData_.data[y].size());
		for (size_t x = 0; x < mapChipData_.data[y].size(); ++x) {
			numbers[y][x] = static_cast<int>(mapChipData_.data[y][x]);
		}
	}
	return numbers;
}

const int MapChipField::GetSizeByType(MapChipType target)
{
	int num = 0;
	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			if (GetMapChipTypeByIndex(x, y) == target) {
				++num;
			}
		}
	}
}



//std::vector<EnemySpawn> MapChipField::GetEnemySpawns() 
//{
//	std::vector<EnemySpawn> result;
//	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
//		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
//			MapChipType type = GetMapChipTypeByIndex(x, y);
//			if (type == MapChipType::kEnemy || type == MapChipType::kEnemy2) {
//				EnemySpawn spawn;
//				spawn.pos = GetMapChipPostionByIndex(x, y);
//				spawn.type = type;
//				result.push_back(spawn);
//			}
//		}
//	}
//	return result;
//}




