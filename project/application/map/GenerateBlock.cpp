#include "GenerateBlock.h"
#include "Object3DCommon.h"

void GenerateBlock::Initialize(MapChipField* map) { mapChipField_ = map; }

void GenerateBlock::Update()
{
	for (auto& line : tileObjects_)
	{
		for (auto& tile : line)
		{
			// 3Dオブジェクトが存在しない場合はスキップ
			if (!tile.object)
			{
				continue;
			}
			// 3Dオブジェクトの更新
			tile.object->Update();
		}
	}
}

void GenerateBlock::Draw()
{
	for (auto& line : tileObjects_)
	{
		for (auto& tile : line)
		{
			// 3Dオブジェクトが存在しない場合はスキップ
			if (!tile.object)
			{
				continue;
			}
			// 3Dオブジェクトの更新
			tile.object->Draw();
		}
	}
}

void GenerateBlock::GenerateObject3D()
{
	// マップチップフィールドが設定されていない場合は処理を中断
	if (!mapChipField_)
	{
		return;
	}

	const uint32_t kHeight = mapChipField_->GetNumBlockVirtical();	// 縦のブロック数を取得
	const uint32_t kWidth = mapChipField_->GetNumBlockHorizontal(); // 横のブロック数を取得

	tileObjects_.clear();		  // 既存のタイルオブジェクトをクリア
	tileObjects_.resize(kHeight); // 縦のブロック数に合わせてタイルオブジェクトの行数をリサイズ

	for (uint32_t y = 0; y < kHeight; ++y)
	{
		tileObjects_[y].resize(kWidth); // 横のブロック数に合わせてタイルオブジェクトの列数をリサイズ
	}
	// 各マップチップに対応する3Dオブジェクトを生成
	SyncBlockObjectsWithMap();
}

void GenerateBlock::SyncBlockObjectsWithMap()
{

	if (!mapChipField_)
	{
		return;
	}

	const uint32_t kHeight = mapChipField_->GetNumBlockVirtical();
	const uint32_t kWidth = mapChipField_->GetNumBlockHorizontal();

	// サイズが合ってなければ作り直す
	if (tileObjects_.size() != kHeight)
	{
		GenerateObject3D();
		return;
	}

	for (uint32_t y = 0; y < kHeight; ++y)
	{
		// サイズが合ってなければ作り直す
		if (tileObjects_[y].size() != kWidth)
		{
			GenerateObject3D();
			return;
		}

		for (uint32_t x = 0; x < kWidth; ++x)
		{
			MapChipType currentType = mapChipField_->GetMapChipTypeByIndex(x, y); // 現在のマップチップタイプを取得
			TileObject& tile = tileObjects_[y][x];								  // 対応するタイルオブジェクトを取得

			// 表示しないチップなら消す
			if (!IsDrawableTile(currentType))
			{
				tile.object.reset();
				tile.type = MapChipType::Empty;
				continue;
			}

			// タイプが変わっていないならそのまま
			if (tile.object && tile.type == currentType)
			{
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
	if (!IsDrawableTile(type))
	{
		return nullptr;
	}

	const char* kModelName = GetModelName(type);
	if (kModelName[0] == '\0')
	{
		return nullptr;
	}
	// 3Dオブジェクトを生成

	auto obj = std::make_unique<Object3D>();
	obj->Initialize(Object3DCommon::GetInstance());
	obj->SetModel(kModelName);
	obj->SetTranslate(mapChipField_->GetMapChipPositionByIndex(x, y));

	obj->SetLighting(true);
	obj->SetDirectionalLightEnable(true);
	obj->SetDirectionalLightDirection({0.88f, -1.90f, 4.0f});

	return obj;
}

const char* GenerateBlock::GetModelName(MapChipType type) const
{
	// マップチップタイプに応じたモデル名を返す
	switch (type)
	{
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
	// 描画可能なマップチップタイプかどうかを判定
	switch (type)
	{
	case MapChipType::Block:
	case MapChipType::UnbreakableBlock:
	case MapChipType::damageBlock:
	case MapChipType::Portal:
		return true;

	default:
		return false;
	}
}
