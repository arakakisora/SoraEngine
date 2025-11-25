//#pragma once
//#include <map>
//#include <string>
//#include <vector>
//#include <utility>
//#include "MyMath.h"
//
///// <summary>
///// マップチップタイプ列挙型
///// </summary>
//enum class MapChipType {
//
//	kBlank, // 空白
//	kBlock, // ブロック
//	kEnemy, // 敵
//	kEnemy2, // 2種類目の敵
//	kGoal,  // ゴール
//
//};
//
///// <summary>
///// マップチップ情報構造体
///// </summary>
//struct MapChipInfo {
//	MapChipType type;
//	const char* label;
//	Vector4 color;
//};
//
///// <summary>
///// マップチップ情報リストを取得します
///// </summary>
///// <returns></returns>
//inline const std::vector<MapChipInfo>& GetMapChipInfoList() {
//	static const std::vector<MapChipInfo> kList = {
//		{ MapChipType::kBlank, "Empty", Vector4(0.10f,0.10f,0.10f,1.0f) },
//		{ MapChipType::kBlock, "Block", Vector4(0.20f,0.80f,0.20f,1.0f) },
//		{ MapChipType::kEnemy, "Enemy", Vector4(0.90f,0.20f,0.20f,1.0f) },
//		{ MapChipType::kEnemy2,"Enemy2",Vector4(0.95f,0.85f,0.20f,1.0f) },
//		{ MapChipType::kGoal,  "Goal",  Vector4(0.20f,0.20f,0.90f,1.0f) },
//
//	};
//	return kList;
//}
//
///// <summary>
///// インデックスセット構造体
///// </summary>
//struct IndexSet {
//	uint32_t xIndex;
//	uint32_t yIndex;
//};
//
///// <summary>
///// 矩形構造体
///// </summary>
//struct Rect {
//
//	float left;
//	float right;
//	float bottom;
//	float top;
//};
///// <summary>
///// グリッドセル構造体
///// </summary>
//struct GridCell {
//	int type = 0; // 0=empty, 1=block, 2=enemy, 3=player
//};
///// <summary>
///// ステージ範囲構造体
///// </summary>
//struct StageRange {
//	int minX = 0;
//	int maxX = 40;
//	int minY = 0;
//	int maxY = 25;
//
//	int Width() const { return maxX - minX; }
//	int Height() const { return maxY - minY; }
//};
//
//
///// <summary>
///// MapChipTypeをintに変換します
///// </summary>
//inline int ToInt(MapChipType t) { return static_cast<int>(t); }
//
///// <summary>
///// マップチップテーブル
///// </summary>
//namespace {
//
//	std::map<std::string, MapChipType> mapChipTable = {
//		{"0", MapChipType::kBlank},
//		{"1", MapChipType::kBlock},
//		{"2", MapChipType::kEnemy},
//		{"3",MapChipType::kEnemy2},
//		{"4", MapChipType::kGoal}
//	
//
//	};
//
//}
//
