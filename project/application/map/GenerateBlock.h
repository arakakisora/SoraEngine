#pragma once
#include <memory>
#include <vector>

#include "MapChipField.h"
#include "MyMath.h"
#include "Object3D.h"

struct TileObject
{
	MapChipType type = MapChipType::Empty;
	std::unique_ptr<Object3D> object = nullptr;
};

class GenerateBlock
{
  public:
	GenerateBlock() = default;
	~GenerateBlock() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="map"></param>
	void Initialize(MapChipField* map);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 3Dオブジェクトの生成
	/// </summary>
	void GenerateObject3D();
	/// <summary>
	/// 3Dオブジェクトの同期
	/// </summary>
	void SyncBlockObjectsWithMap();

  private:
	/// <summary>
	/// 指定したマップチップタイプの3Dオブジェクトを生成
	/// </summary>
	/// <param name="type"></param>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <returns></returns>
	std::unique_ptr<Object3D> CreateTileObject(MapChipType type, uint32_t x, uint32_t y);
	/// <summary>
	/// 指定したマップチップタイプのモデル名を取得
	/// 生成時にIDとして使用する
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	const char* GetModelName(MapChipType type) const;
	/// <summary>
	/// 指定したマップチップタイプが描画可能かどうかを判定
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	bool IsDrawableTile(MapChipType type) const;

  private:
	std::vector<std::vector<TileObject>> tileObjects_; // 2D配列でマップチップの3Dオブジェクトを管理
	MapChipField* mapChipField_ = nullptr;			   // マップチップフィールドへのポインタ（非所有）
};
