#pragma once
#include "MapChipDatabase.h"
#include "Vector3.h"
#include <StageData.h>
#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>

/// <summary>
/// マップチップのインデックス（配列上の位置）
/// </summary>
struct MapChipData
{

	std::vector<std::vector<MapChipType>> data;
};
/// <summary>
/// マップチップのインデックス（配列上の位置）
/// </summary>
struct IndexSet
{
	uint32_t xIndex;
	uint32_t yIndex;
};
/// <summary>
/// マップチップ1マス分のワールド上の矩形
/// </summary>
struct Rect
{

	float left;
	float right;
	float bottom;
	float top;
};

struct PortalInfo
{
	int pairId = -1;
	uint32_t x = 0;
	uint32_t y = 0;
	Vector3 dir = {1.0f, 0.0f, 0.0f};
};

/// <summary>
/// マップチップフィールドクラス
/// </summary>
class MapChipField
{

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
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	/// <summary>
	/// 指定したインデックスのマップチップのワールド座標を取得
	/// </summary>
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);
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
	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);
	/// <summary>
	/// 指定したインデックスからマップチップの矩形情報を取得
	/// </summary>
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);
	/// <summary>
	/// 敵の座標リストを取得
	/// </summary>
	/// <returns></returns>
	std::vector<Vector3> GetEnemyPositions();
	/// <summary>
	/// 敵の番号リストを取得します
	/// </summary>
	std::vector<int> GetEnemyNumbers() { return enemyNumber_; } // 敵の番号を取得
	/// <summary>
	// ゴール
	/// </summary>
	/// <returns>ゴールの座標を返す</returns>
	Vector3 GetGoalPosition();
	/// <summary>
	/// 指定したインデックスのマップチップが通行可能かどうかを判定
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	bool IsSolid(uint32_t xIndex, uint32_t yIndex);
	/// <summary>
	/// 指定した spawn タグに対応するマップチップの座標リストを取得
	/// </summary>
	/// <param name="spawnTag"></param>
	/// <returns></returns>
	std::vector<Vector3> GetPositionBySpawn(const std::string& spawnTag);

	/// <summary>
	/// 指定したインデックスのマップチップの耐久値を取得
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	int GetMapChipHPByIndex(uint32_t xIndex, uint32_t yIndex) const;
	/// <summary>
	/// 指定したインデックスのマップチップにダメージを与える（HPが0以下になったらタイルを 0 にする）
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <param name="damage"></param>
	void DamageMapChipByIndex(uint32_t xIndex, uint32_t yIndex, int damage);
	/// <summary>
	/// 指定したワールド座標のマップチップにダメージを与える（HPが0以下になったらタイルを 0 にする）
	/// </summary>
	/// <param name="position"></param>
	/// <param name="damage"></param>
	void DamageMapChipByPosition(const Vector3& position, int damage);

	// 1ブロックの幅と高さを取得
	float GetBlockWidth() const { return kBlockWidth; }
	// 1ブロックの高さを取得
	float GetBlockHeight() const { return kBlockHeight; }
	/// <summary>
	/// ポータル情報のリストを取得
	/// </summary>
	/// <returns></returns>
	const std::vector<PortalInfo>& GetPortals() const { return portals_; }
	/// <summary>
	/// 指定した座標にポータルが存在するかどうかを判定し、存在する場合はその情報を out に格納
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="out"></param>
	/// <returns></returns>
	bool TryGetPortal(uint32_t x, uint32_t y, PortalInfo& out) const;
	/// <summary>
	/// 指定したポータルのペアとなるポータル情報を取得し、存在する場合はその情報を out に格納
	/// </summary>
	/// <param name="in"></param>
	/// <param name="out"></param>
	/// <returns></returns>
	bool TryGetPairPortal(const PortalInfo& in, PortalInfo& out) const;
	/// <summary>
	/// 指定した文字列から方向ベクトルを取得
	/// </summary>
	/// <param name="dir"></param>
	/// <returns></returns>
	Vector3 DirFromString(const std::string& dir);

	/// <summary>
	/// ステージデータを取得
	/// </summary>
	/// <returns></returns>
	StageData& GetStageData() { return stageData_; }
	/// <summary>
	/// ステージデータを取得（const版）
	/// </summary>
	/// <returns></returns>
	const StageData& GetStageData() const { return stageData_; }
	/// <summary>
	/// ステージデータを適用
	/// </summary>
	/// <param name="stageData"></param>
	void ApplyStageData(const StageData& stageData);

  private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// ブロック個数
	static inline const uint32_t kNumBlockVirtical = 25;
	static inline const uint32_t kNumBlockHorizontal = 100;
	MapChipData mapChipData_;

	// 敵の番号リスト
	std::vector<int> enemyNumber_;
	// ポータルのペアIDと位置、向きを管理する構造体のリスト
	std::vector<PortalInfo> portals_;

	std::vector<std::vector<int>> hpData_;

	// ステージデータ
	StageData stageData_;
};
