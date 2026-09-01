#pragma once
#include <vector>
#include <cstdint>
#include <cassert>

#include "MapChipDatabase.h"
#include "Vector3.h"


struct StageCell {
	MapChipType type = MapChipType::Empty;// マップチップタイプ
	int hp = 0; // マップチップの耐久値（0 = 壊せない / 1以上 = 壊せる）
	
	int linkId = -1;// リンクID（-1 = リンクなし / 0以上 = リンクあり）
	Vector3 direction = { 1.0f, 0.0f, 0.0f };//向き（デフォルトは右向き）
	//右 1.0f, 0.0f, 0.0f
	//左 -1.0f, 0.0f, 0.0f
	//上 0.0f, 1.0f, 0.0f
	//下 0.0f, -1.0f, 0.0f

};

class StageData
{
public:
	/// <summary>
	/// ステージデータのリサイズ
	/// </summary>
	/// <param name="width"></param>
	/// <param name="height"></param>
	void Resize(uint32_t width, uint32_t height);
	/// <summary>
	/// 指定した座標のステージセルを取得
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <returns></returns>
	StageCell& At(uint32_t x, uint32_t y);
	const StageCell& At(uint32_t x, uint32_t y) const;

	/// <summary>
	/// 指定した座標のマップチップタイプを取得
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <returns></returns>
	MapChipType GetType(uint32_t x, uint32_t y) const;
	/// <summary>
	/// 指定した座標のマップチップタイプを設定
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="type"></param>
	void SetType(uint32_t x, uint32_t y, MapChipType type);

	/// <summary>
	/// 指定した座標のマップチップの耐久値を取得
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <returns></returns>
	int GetHP(uint32_t x, uint32_t y) const;
	/// <summary>
	/// 指定した座標のマップチップの耐久値を設定
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="hp"></param>
	void SetHP(uint32_t x, uint32_t y, int hp);
	/// <summary>
	/// 指定した座標のマップチップの耐久値をダメージ分減らす
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="damage"></param>
	void DamageHP(uint32_t x, uint32_t y, int damage);

	uint32_t GetWidth() const { return width_; }
	uint32_t GetHeight() const { return height_; }

	void Clear();

private:
	uint32_t width_ = 0;
	uint32_t height_ = 0;
	std::vector<std::vector<StageCell>> cells_;

};

