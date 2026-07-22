#include "StageData.h"
#include "MapChipDatabase.h"

void StageData::Resize(uint32_t width, uint32_t height)
{
	// ステージデータのリサイズ
	width_ = width;
	height_ = height;
	// 2次元配列のリサイズ
	cells_.clear();
	cells_.resize(height_);

	// 2次元配列の各行のリサイズ
	for (auto& row : cells_) {
		row.resize(width_);
	}

}

StageCell& StageData::At(uint32_t x, uint32_t y)
{
	assert(y < height_);// y座標が範囲内であることを確認
	assert(x < width_);// x座標が範囲内であることを確認
	return cells_[y][x];
}

const StageCell& StageData::At(uint32_t x, uint32_t y) const
{
	assert(y < height_);// y座標が範囲内であることを確認
	assert(x < width_);// x座標が範囲内であることを確認
	return cells_[y][x];
}

MapChipType StageData::GetType(uint32_t x, uint32_t y) const
{
	if (x >= width_ || y >= height_) {
		return MapChipType::Empty; // 範囲外の場合は空のマップチップタイプを返す
	}
	return cells_[y][x].type;
}

void StageData::SetType(uint32_t x, uint32_t y, MapChipType type)
{
	if (x >= width_ || y >= height_) {
		return; // 範囲外の場合は何もしない
	}

	cells_[y][x].type = type;
	// マップチップの耐久値を設定
	const MapChipInfo* chipInfo = MapChipDatabase::GetInstance()->GetById(type);
	if(chipInfo){
		cells_[y][x].hp = chipInfo->hitPoints; // マップチップの耐久値を設定
	}
	else{
		cells_[y][x].hp = 0; // デフォルトの耐久値を設定（壊せない）
	}

	if (type == MapChipType::Empty) {
		cells_[y][x].linkId = -1; // Emptyの場合は壊せない
		cells_[y][x].direction = { 1.0f, 0.0f, 0.0f }; // Emptyの場合は右向き
	}
}

int StageData::GetHP(uint32_t x, uint32_t y) const
{
	if (x >= width_ || y >= height_) {
		return 0; // 範囲外の場合は耐久値0を返す
	}
	return cells_[y][x].hp;
}

void StageData::SetHP(uint32_t x, uint32_t y, int hp)
{
	if (x >= width_ || y >= height_) {
		return; // 範囲外の場合は何もしない
	}
	cells_[y][x].hp = hp;
}

void StageData::DamageHP(uint32_t x, uint32_t y, int damage)
{
	if (x >= width_ || y >= height_) {
		return; // 範囲外の場合は何もしない
	}
	cells_[y][x].hp -= damage;
	
	if (cells_[y][x].hp <= 0) {
		cells_[y][x].hp = 0;
		cells_[y][x].type = MapChipType::Empty;
		cells_[y][x].linkId = -1;
		cells_[y][x].direction = { 1.0f, 0.0f, 0.0f };
	}
}

void StageData::Clear()
{
	for (auto& row : cells_) {
		for (auto& cell : row) {
			cell = StageCell{};
		}
	}
}

