#pragma once
#include "Vector3.h"
#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>

	enum class MapChipType {

		kBlank, // 空白
		kBlock, // ブロック

	};
	struct MapChipData {

		std::vector<std::vector<MapChipType>> data;
	};

	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	struct Rect {

	    float left ;
	    float right ;
	    float bottom ;
	    float top  ;
    };

class MapChipField {


public:

	void ResetMapChipData();
	void LoadMapChipCsv(const std::string& filePath);
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	Vector3 GetMapChipPostionByIndex(uint32_t xIndex, uint32_t yIndex);
	uint32_t GetNumBlockVirtical() { return kNumBlockVirtical; }
	uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; }
	IndexSet GetMapChipIndexSetByPosition(const Vector3& posotopn);
	Rect GetRectByIndex(uint32_t xindex, uint32_t yIndex);

private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// ブロック個数
	static inline const uint32_t kNumBlockVirtical = 25;
	static inline const uint32_t kNumBlockHorizontal = 100;
	MapChipData mapChipData_;
	
};
