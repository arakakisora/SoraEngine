#include "GenerateBlock.h"
#include "Object3DCommon.h"

void GenerateBlock::Initialize(MapChipField* map)
{
	mapChipField_ = map;

}

void GenerateBlock::Update()
{
	for (auto& line : tileObjects_) {
		for (auto& tile : line) {
			if (!tile.object) {
				continue;
			}

			tile.object->Update();
		}
	}
}

void GenerateBlock::Draw()
{
	for (auto& line : tileObjects_) {
		for (auto& tile : line) {
			if (!tile.object) {
				continue;
			}

			tile.object->Draw();
		}
	}
}
void GenerateBlock::GenerateObject3D()
{
	if (!mapChipField_) {
		return;
	}

	const uint32_t height = mapChipField_->GetNumBlockVirtical();
	const uint32_t width = mapChipField_->GetNumBlockHorizontal();

	tileObjects_.clear();
	tileObjects_.resize(height);

	for (uint32_t y = 0; y < height; ++y) {
		tileObjects_[y].resize(width);
	}

	SyncBlockObjectsWithMap();
}

void GenerateBlock::SyncBlockObjectsWithMap()
{
	if (!mapChipField_) {
		return;
	}

	const uint32_t height = mapChipField_->GetNumBlockVirtical();
	const uint32_t width = mapChipField_->GetNumBlockHorizontal();

	// サイズが合ってなければ作り直す
	if (tileObjects_.size() != height) {
		GenerateObject3D();
		return;
	}

	for (uint32_t y = 0; y < height; ++y) {
		if (tileObjects_[y].size() != width) {
			GenerateObject3D();
			return;
		}

		for (uint32_t x = 0; x < width; ++x) {
			MapChipType currentType = mapChipField_->GetMapChipTypeByIndex(x, y);
			TileObject& tile = tileObjects_[y][x];

			// 表示しないチップなら消す
			if (!IsDrawableTile(currentType)) {
				tile.object.reset();
				tile.type = MapChipType::Empty;
				continue;
			}

			// タイプが変わっていないならそのまま
			if (tile.object && tile.type == currentType) {
				continue;
			}

			// タイプが変わった、または未生成なら作り直す
			tile.object = CreateTileObject(currentType, x, y);
			tile.type = currentType;
		}
	}
}

std::unique_ptr<Object3D> GenerateBlock::CreateTileObject(MapChipType type, uint32_t x, uint32_t y)
{
	if (!IsDrawableTile(type)) {
		return nullptr;
	}

	const char* modelName = GetModelName(type);
	if (modelName[0] == '\0') {
		return nullptr;
	}

	auto obj = std::make_unique<Object3D>();
	obj->Initialize(Object3DCommon::GetInstance());
	obj->SetModel(modelName);
	obj->SetTranslate(mapChipField_->GetMapChipPostionByIndex(x, y));

	obj->SetLighting(true);
	obj->SetDirectionalLightEnable(true);
	obj->SetDirectionalLightDirection({ 0.88f, -1.90f, 4.0f });

	return obj;
}

const char* GenerateBlock::GetModelName(MapChipType type) const
{
	switch (type) {
	case MapChipType::Block:
		return "block";

	case MapChipType::UnbreakableBlock:
		return "unbreakableBlock";

	case MapChipType::damageBlock:
		return "damageblock";

	case MapChipType::Portal:
		return "damageblock";

	default:
		return "";
	}
}

bool GenerateBlock::IsDrawableTile(MapChipType type) const
{
	switch (type) {
	case MapChipType::Block:
	case MapChipType::UnbreakableBlock:
	case MapChipType::damageBlock:
	case MapChipType::Portal:
		return true;

	default:
		return false;
	}
}
