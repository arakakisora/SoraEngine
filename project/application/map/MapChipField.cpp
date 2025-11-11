#include "MapChipField.h"
#include <fstream>
#include <map>
#include <sstream>

namespace {

	std::map<std::string, MapChipType> mapChipTable = {
		{"0", MapChipType::kBlank},
		{"1", MapChipType::kBlock},
		{"2", MapChipType::kEnemy},
		{"3",MapChipType::kEnemy2},
		{"4", MapChipType::kGoal}

	};

}

void MapChipField::ResetMapChipData() {
	// マップチップデータのリセット
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data) {
		// 横方向のブロック数分リサイズ
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
		// 横方向のブロック数分ループ
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			// カンマ区切りで1単語取得
			std::string word;
			getline(lien_stream, word, ',');
			// マップチップタイプに変換して格納
			if (mapChipTable.contains(word)) {
				mapChipData_.data[y][x] = mapChipTable[word];
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {

	// インデックスが範囲外の場合は空白を返す
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}
	// インデックスが範囲外の場合は空白を返す
	if (yIndex < 0 || kNumBlockVirtical - 1 < yIndex) {
		return MapChipType::kBlank;
	}
	// 指定したインデックスのマップチップタイプを返す
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
	// 敵の座標リストを取得
	std::vector<Vector3> enemyPositions;
	// マップチップデータを走査
	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			// 敵のマップチップの場合、座標リストに追加
			if (GetMapChipTypeByIndex(x, y) == MapChipType::kEnemy) {
				enemyPositions.push_back(GetMapChipPostionByIndex(x, y));
				Enemynumber.push_back(1); //敵の番号を追加
			} // 2種類目の敵の場合
			else if (GetMapChipTypeByIndex(x, y) == MapChipType::kEnemy2) {
				enemyPositions.push_back(GetMapChipPostionByIndex(x, y));
				Enemynumber.push_back(2); //敵の番号を追加
			}

		}
	}
	return enemyPositions;
}

Vector3 MapChipField::GetGoalPosition()
{
	// ゴールの座標を取得
	Vector3 pos;
	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			if (GetMapChipTypeByIndex(x, y) == MapChipType::kGoal) {
				pos = GetMapChipPostionByIndex(x, y);
			}
		}
	}

	return pos;
}
