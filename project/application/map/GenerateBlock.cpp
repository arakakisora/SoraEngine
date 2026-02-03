#include "GenerateBlock.h"
#include "Object3DCommon.h"

void GenerateBlock::Initialize(MapChipField* map)
{
	mapChipField_ = map;

}

void GenerateBlock::Update()
{

	//3Dオブジェクトの更新
	for (auto& objext3dLine : blockobject3D)
	{
		for (auto& obj : objext3dLine)
		{
			if (!obj) {
				continue;
			}
			obj->Update();
		}
	}

	//壊れないブロックの更新
	for (auto& objext3dLine : unbreakableject3D)
	{
		for (auto& obj : objext3dLine)
		{
			if (!obj) {
				continue;
			}
			obj->Update();
		}
	}
}

void GenerateBlock::Draw()
{

	for (auto& objext3dLine : blockobject3D)
	{

		for (auto& obj : objext3dLine)
		{
			if (!obj) {
				continue;
			}
			obj->Draw();
		}

	}

	for (auto& objext3dLine : unbreakableject3D)
	{
		for (auto& obj : objext3dLine)
		{
			if (!obj) {
				continue;
			}
			obj->Draw();
		}
	}

}

void GenerateBlock::GenerateObject3D()
{
	if (!mapChipField_) return;

	uint32_t numBlokVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlokHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 既存のオブジェクトを全部削除して配列を初期化（unique_ptr が自動で破棄）
	blockobject3D.clear();
	unbreakableject3D.clear();

	blockobject3D.resize(numBlokVirtical);// 配列の縦サイズを確保
	unbreakableject3D.resize(numBlokVirtical);// 配列の縦サイズを確保

	for (uint32_t i = 0; i < numBlokVirtical; ++i) {
		blockobject3D[i].resize(numBlokHorizontal); // デフォルトは空の unique_ptr
		unbreakableject3D[i].resize(numBlokHorizontal); // デフォルトは空の unique_ptr
	}

	// 実際の作成は Sync に任せる（初回はここで生成される）
	SyncBlockObjectsWithMap();
}

void GenerateBlock::SyncBlockObjectsWithMap()
{
	if (!mapChipField_) return;

	uint32_t numBlokVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlokHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 配列サイズが一致している前提
	for (uint32_t y = 0; y < numBlokVirtical; ++y) {
		for (uint32_t x = 0; x < numBlokHorizontal; ++x) {

			int type = mapChipField_->GetMapChipTypeByIndex(x, y);
			Object3D* obj = nullptr;
			Object3D* unbreakableObj = nullptr;

			if (y < blockobject3D.size() && x < blockobject3D[y].size()) {
				obj = blockobject3D[y][x].get();
			}
			if (y < unbreakableject3D.size() && x < unbreakableject3D[y].size()) {
				unbreakableObj = unbreakableject3D[y][x].get();
			}

			// タイプ 1 はブロック（必要に応じ他のIDも対応）
			if (type == 1) {
				// ブロックが存在すべきだがオブジェクトが無ければ作る
				if (!obj) {
					auto newObj = std::make_unique<Object3D>();
					newObj->Initialize(Object3DCommon::GetInstance());
					newObj->SetModel("blokc.obj");
					newObj->SetTranslate(mapChipField_->GetMapChipPostionByIndex(x, y));
					newObj->SetLighting(true);
					newObj->SetDirectionalLightEnable(true);
					newObj->SetDirectionalLightDirection({ 0.88f, -1.90f, 4.0f });

					// 保持配列へ格納
					if (y < blockobject3D.size() && x < blockobject3D[y].size()) {
						blockobject3D[y][x] = std::move(newObj);
					}
				}
			}
			else if (type == 6) {

				if (!unbreakableObj) {
					auto newObj = std::make_unique<Object3D>();
					newObj->Initialize(Object3DCommon::GetInstance());
					newObj->SetModel("cube.obj");
					newObj->SetTranslate(mapChipField_->GetMapChipPostionByIndex(x, y));
					newObj->SetLighting(true);
					newObj->SetDirectionalLightEnable(true);
					newObj->SetDirectionalLightDirection({ 0.88f, -1.90f, 4.0f });
					// 保持配列へ格納
					if (y < unbreakableject3D.size() && x < unbreakableject3D[y].size()) {
						unbreakableject3D[y][x] = std::move(newObj);
					}
				}

			}
			else {
				// ブロックが消えているのにオブジェクトが残っていれば削除（unique_ptr::reset で破棄）
				if (obj) {
					if (y < blockobject3D.size() && x < blockobject3D[y].size()) {
						blockobject3D[y][x].reset();
					}
				}
			}
		}
	}
}