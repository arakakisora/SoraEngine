#pragma once
#include "Vector3.h"
#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "MapChipDatabase.h"

/// <summary>
/// マップチップのインデックス（配列上の位置）
/// </summary>
struct MapChipData {

	std::vector<std::vector<int>> data;
};
/// <summary>
/// マップチップのインデックス（配列上の位置）
/// </summary>
struct IndexSet {
	uint32_t xIndex;
	uint32_t yIndex;
};
/// <summary>
/// マップチップ1マス分のワールド上の矩形
/// </summary>
struct Rect {

	float left;
	float right;
	float bottom;
	float top;
};
/// <summary>
/// マップチップフィールドクラス
/// </summary>
class MapChipField {


public:
	/// <summary>
	/// マップチップデータのリセット
	/// </summary>
	void ResetMapChipData();
	/// <summary>
	/// マップチップCSVの読み込み
	/// </summary>
	void LoadMapChipCsv(const std::string& filePath);
	/// <summary>
	/// 指定したインデックスのマップチップID（= JSON の id）を取得
	/// </summary>
	int  GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	/// <summary>
	/// 指定したインデックスのマップチップのワールド座標を取得
	/// </summary>
	Vector3 GetMapChipPostionByIndex(uint32_t xIndex, uint32_t yIndex);
	/// <summary>
	/// 縦のブロック数を取得
	/// </summary>
	uint32_t GetNumBlockVirtical() { return kNumBlockVirtical; }
	/// <summary>
	/// 横のブロック数を取得
	/// </summary>
	uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; }
	/// <summary>
	/// 指定したワールド座標からマップチップのインデックスを取得
	/// </summary>
	IndexSet GetMapChipIndexSetByPosition(const Vector3& posotopn);
	/// <summary>
	/// 指定したインデックスからマップチップの矩形情報を取得
	/// </summary>
	Rect GetRectByIndex(uint32_t xindex, uint32_t yIndex);
	/// <summary>
	/// 敵の座標リストを取得
	/// </summary>
	/// <returns></returns>
	std::vector<Vector3> GetEnemyPositions();
	/// <summary>
	/// 敵の番号リストを取得します
	/// </summary>
	std::vector<int> GetEnemyNumbers() { return Enemynumber; } //敵の番号を取得
	/// <summary>
	//ゴール
	/// </summary>
	/// <returns>ゴールの座標を返す</returns>
	Vector3 GetGoalPosition();

	bool IsSolid(uint32_t xIndex, uint32_t yIndex);

	std::vector<Vector3>GetPositionBySpwan(const std::string& spawnTag);


	int GetMapChipHPByIndex(uint32_t xIndex, uint32_t yIndex) const;
	void DamageMapChipByIndex(uint32_t xIndex, uint32_t yIndex, int damage);
	void DamageMapChipByPosition(const Vector3& position, int damage);

	// 1ブロックの幅と高さを取得
	float GetBlockWidth() const { return kBlockWidth; }
	// 1ブロックの高さを取得
	float GetBlockHeight() const { return kBlockHeight; }

private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// ブロック個数
	static inline const uint32_t kNumBlockVirtical = 25;
	static inline const uint32_t kNumBlockHorizontal = 100;
	MapChipData mapChipData_;

	std::vector<int> Enemynumber;


	std::vector<std::vector<int>> hpData_;

};
