#pragma once
#include "Vector3.h"
#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "MapType.h"


struct MapChipData {

	std::vector<std::vector<MapChipType>> data;
};



struct EnemySpawn {
	Vector3 pos;
	MapChipType type;
};

class MapChipField {


public:


	void SetBlockRange(int minX, int minY, int maxX, int maxY) {
		offsetXBlocks_ = minX;
		offsetYBlocks_ = minY;
		widthBlocks_ = maxX - minX;
		heightBlocks_ = maxY - minY;
	}


	void ResetMapChipData();
	void LoadMapChipCsv(const std::string& filePath);
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	Vector3 GetMapChipPostionByIndex(uint32_t xIndex, uint32_t yIndex);
	uint32_t GetNumBlockVirtical() { return kNumBlockVirtical; }
	uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; }
	IndexSet GetMapChipIndexSetByPosition(const Vector3& posotopn);
	Rect GetRectByIndex(uint32_t xindex, uint32_t yIndex);


	std::vector<Vector3> GetEnemyPositions();
	std::vector<MapChipType> GetEnemyNumbers() { return Enemynumber; } //敵の番号を取得
	std::vector<EnemySpawn> GetEnemySpawns();
	//ゴール

	const std::vector<std::vector<int>>& GetAllNumbers() const;
	const int GetSizeByType(MapChipType target);
	std::vector<Vector3> GetPositionsByType(MapChipType target);

private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	int offsetXBlocks_ = 0;   // エディタの minX
	int offsetYBlocks_ = 0;   // エディタの minY
	int widthBlocks_ = 0;   // エディタの幅（ブロック数）
	int heightBlocks_ = 0;   // エディタの高さ（ブロック数）

	// ブロック個数
	static inline const uint32_t kNumBlockVirtical = 25;
	static inline const uint32_t kNumBlockHorizontal = 100;
	MapChipData mapChipData_;

	std::vector<MapChipType> Enemynumber;

};
