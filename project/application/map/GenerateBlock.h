#pragma once
#include <vector>
#include <memory>

#include "MyMath.h"
#include "Object3D.h"
#include "MapChipField.h"

struct TileObject {
	MapChipType type = MapChipType::Empty;
	std::unique_ptr<Object3D> object = nullptr;
};

class GenerateBlock
{
public:
	GenerateBlock() = default;
	~GenerateBlock() = default;

	void Initialize(MapChipField* map);
	void Update();
	void Draw();

	void GenerateObject3D();
	void SyncBlockObjectsWithMap();

private:
	std::unique_ptr<Object3D> CreateTileObject(MapChipType type, uint32_t x, uint32_t y);
	const char* GetModelName(MapChipType type) const;
	bool IsDrawableTile(MapChipType type) const;

private:
	std::vector<std::vector<TileObject>> tileObjects_;
	MapChipField* mapChipField_ = nullptr;
};
