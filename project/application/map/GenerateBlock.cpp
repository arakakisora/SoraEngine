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

	//damageブロックの更新
	for (auto& objext3dLine : damageBlockobject3D)
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

	for (auto& objext3dLine : damageBlockobject3D)
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
	damageBlockobject3D.clear();

	blockobject3D.resize(numBlokVirtical);// 配列の縦サイズを確保
	unbreakableject3D.resize(numBlokVirtical);// 配列の縦サイズを確保
	damageBlockobject3D.resize(numBlokVirtical);// 配列の縦サイズを確保

	for (uint32_t i = 0; i < numBlokVirtical; ++i) {
		blockobject3D[i].resize(numBlokHorizontal); // デフォルトは空の unique_ptr
		unbreakableject3D[i].resize(numBlokHorizontal); // デフォルトは空の unique_ptr
		damageBlockobject3D[i].resize(numBlokHorizontal); // デフォルトは空の unique_ptr
	}

	// 実際の作成は Sync に任せる（初回はここで生成される）
	SyncBlockObjectsWithMap();
}

void GenerateBlock::SyncBlockObjectsWithMap()
{
	if (!mapChipField_) return;

	uint32_t numBlokVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlokHorizontal = mapChipField_->GetNumBlockHorizontal();

	for (uint32_t y = 0; y < numBlokVirtical; ++y) {
		for (uint32_t x = 0; x < numBlokHorizontal; ++x) {

			MapChipType type = mapChipField_->GetMapChipTypeByIndex(x, y);

			Object3D* obj = nullptr;
			Object3D* unbreakableObj = nullptr;
			Object3D* damageBlockObj = nullptr;

			if (y < blockobject3D.size() && x < blockobject3D[y].size()) {
				obj = blockobject3D[y][x].get();
			}
			if (y < unbreakableject3D.size() && x < unbreakableject3D[y].size()) {
				unbreakableObj = unbreakableject3D[y][x].get();
			}
			if (y < damageBlockobject3D.size() && x < damageBlockobject3D[y].size()) {
				damageBlockObj = damageBlockobject3D[y][x].get();
			}

			if (type == MapChipType::Block) {
				if (!obj) {
					auto newObj = std::make_unique<Object3D>();
					newObj->Initialize(Object3DCommon::GetInstance());
					newObj->SetModel("blokc.obj");
					newObj->SetTranslate(mapChipField_->GetMapChipPostionByIndex(x, y));
					newObj->SetLighting(true);
					newObj->SetDirectionalLightEnable(true);
					newObj->SetDirectionalLightDirection({ 0.88f, -1.90f, 4.0f });

					blockobject3D[y][x] = std::move(newObj);
				}

				// 他タイプが残っていたら消す
				if (unbreakableObj) {
					unbreakableject3D[y][x].reset();
				}
				if (damageBlockObj) {
					damageBlockobject3D[y][x].reset();
				}
			} else if (type == MapChipType::UnbreakableBlock) {
				if (!unbreakableObj) {
					auto newObj = std::make_unique<Object3D>();
					newObj->Initialize(Object3DCommon::GetInstance());
					newObj->SetModel("unbreakableBlokc.obj");
					newObj->SetTranslate(mapChipField_->GetMapChipPostionByIndex(x, y));
					newObj->SetLighting(true);
					newObj->SetDirectionalLightEnable(true);
					newObj->SetDirectionalLightDirection({ 0.88f, -1.90f, 4.0f });

					unbreakableject3D[y][x] = std::move(newObj);
				}

				if (obj) {
					blockobject3D[y][x].reset();
				}
				if (damageBlockObj) {
					damageBlockobject3D[y][x].reset();
				}
			} else if (type == MapChipType::damageBlock) {
				if (!damageBlockObj) {
					auto newObj = std::make_unique<Object3D>();
					newObj->Initialize(Object3DCommon::GetInstance());
					newObj->SetModel("damageblock.obj");
					newObj->SetTranslate(mapChipField_->GetMapChipPostionByIndex(x, y));
					newObj->SetLighting(true);
					newObj->SetDirectionalLightEnable(true);
					newObj->SetDirectionalLightDirection({ 0.88f, -1.90f, 4.0f });

					damageBlockobject3D[y][x] = std::move(newObj);
				}

				if (obj) {
					blockobject3D[y][x].reset();
				}
				if (unbreakableObj) {
					unbreakableject3D[y][x].reset();
				}
			} else {
				if (obj) {
					blockobject3D[y][x].reset();
				}
				if (unbreakableObj) {
					unbreakableject3D[y][x].reset();
				}
				if (damageBlockObj) {
					damageBlockobject3D[y][x].reset();
				}
			}
		}
	}
}