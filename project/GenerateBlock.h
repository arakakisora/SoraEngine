#pragma once
#include <vector>
#include "MyMath.h"
#include "Object3D.h"
#include "MapChipField.h"
#include <memory>

class GenerateBlock
{
public:
	GenerateBlock() = default;
	~GenerateBlock() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="map"></param>
	void Initialize(MapChipField*map);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	/// <summary>
	/// ブロックの生成
	/// </summary>
	void GenerateObject3D();
	/// <summary>
	/// マップとブロックオブジェクトを同期させる
	/// </summary>
	void SyncBlockObjectsWithMap();

	// アクセッサ
	std::vector<std::vector<std::unique_ptr<Object3D>>>& GetBlockObject3D() { return blockobject3D; }

private:
	//ブロック
	std::vector<std::vector<std::unique_ptr<Object3D>>> blockobject3D;
	MapChipField* mapChipField_ = nullptr;


};

