#pragma once
#include <map>
#include <string>
#include <vector>
#include <utility>
#include "MyMath.h"


enum class MapChipType {

	kBlank, // 空白
	kBlock, // ブロック
	kEnemy, // 敵
	kEnemy2, // 2種類目の敵

};

struct MapChipInfo {
	MapChipType type;
	const char* label;
	Vector4 color;
};

inline const std::vector<MapChipInfo>& GetMapChipInfoList() {
	static const std::vector<MapChipInfo> kList = {
		{ MapChipType::kBlank, "Empty", Vector4(0.10f,0.10f,0.10f,1.0f) },
		{ MapChipType::kBlock, "Block", Vector4(0.20f,0.80f,0.20f,1.0f) },
		{ MapChipType::kEnemy, "Enemy", Vector4(0.90f,0.20f,0.20f,1.0f) },
		{ MapChipType::kEnemy2,"Enemy2",Vector4(0.95f,0.85f,0.20f,1.0f) }

	};
	return kList;
}

struct IndexSet {
	uint32_t xIndex;
	uint32_t yIndex;
};

struct Rect {

	float left;
	float right;
	float bottom;
	float top;
};

struct GridCell {
	int type = 0; // 0=empty, 1=block, 2=enemy, 3=player
};

struct StageRange {
	int minX = 0;
	int maxX = 40;
	int minY = 0;
	int maxY = 25;

	int Width() const { return maxX - minX; }
	int Height() const { return maxY - minY; }
};



inline int ToInt(MapChipType t) { return static_cast<int>(t); }

namespace {

	std::map<std::string, MapChipType> mapChipTable = {
		{"0", MapChipType::kBlank},
		{"1", MapChipType::kBlock},
		{"2", MapChipType::kEnemy},
		{"3",MapChipType::kEnemy2}
	

	};

}