#include "ModelManager.h"
#include <memory> // std::make_unique

// シングルトンの実体を unique_ptr で所有する
std::unique_ptr<ModelManager> ModelManager::instance = nullptr;

// シングルトン取得（必要に応じて生成）
// 返り値は生ポインタ（非所有）を返す。所有は static unique_ptr が保持する。
ModelManager* ModelManager::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_unique<ModelManager>();
	}
	return instance.get();
}

void ModelManager::Finalize()
{
	// 内部リソースを先に解放する
	models.clear();
	if (modelCommon) {
		modelCommon.reset();
	}

	// シングルトンを破棄する（これにより this は破棄される）
	// 注意: reset() の後にメンバにアクセスしてはならない
	instance.reset();
}

void ModelManager::Initialize(DirectXCommon* dxcommon, SrvManager* srvmnager)
{
	// SRVManager のポインタを保存（所有はしない）
	srvmanager_ = srvmnager;

	// ModelCommon を unique_ptr で生成・初期化
	modelCommon = std::make_unique<ModelCommon>();
	modelCommon->Initialize(dxcommon, srvmanager_);
}

void ModelManager::LoadModel(const std::string& modelName)
{

	std::string directoryPath = "Resources/models/" + modelName+"/";
	std::string fileName = modelName + ".obj";
	std::string filePath = directoryPath + fileName;

	// 読み込み済みモデルを検索
	if (models.contains(modelName)) {
		// 読み込み済みなら早期 return
		return;
	}
	// モデルの生成とファイル読み込み、初期化（unique_ptr で所有）
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon.get(), directoryPath, fileName);

	// モデルを map コンテナに格納する
	// modelName をキーにして登録する
	models.insert(std::make_pair(modelName, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath)
{
	// 読み込みモデルを戻り値として返す（生ポインタ、非所属）
	if (models.contains(filePath)) {
		return models.at(filePath).get();
	}

	// ファイル名一致なし
	return nullptr;
}
