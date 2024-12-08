#include "ParticleManager.h"


ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstans()
{
	if (instance == nullptr) {

		instance = new ParticleManager;
	}
	return instance;
}

void ParticleManager::Finalize()
{
	delete instance;
	instance = nullptr;

}

void ParticleManager::Initialize(DirectXCommon* dxcommon, SrvManager* srvmnager)
{

	//初期化
	dxcommon_ = dxcommon;
	srvmnager_ = srvmnager;
	//乱数エンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 random(seedGenerator());
	randomEngine = random;
	//グラフィックスパイプラインの作成
	graphicsPipeline->Create(dxcommon_);
	//頂点データの初期化
	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = dxcommon_->CreateBufferResource(sizeof(TransformationMatrix));
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixData));
	//単位行列を書き込む
	transformaitionMatrixData->WVP = transformaitionMatrixData->WVP.MakeIdentity4x4();
	transformaitionMatrixData->World = transformaitionMatrixData->World.MakeIdentity4x4();

}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath, uint32_t maxInstance)
{
	if (particleGroups.contains(name)) { return; }
	ParticleGroup particleGroup;
	particleGroups.insert(std::make_pair(name, std::move(particleGroup)));
	particleGroups[name].kNumMaxInstance = maxInstance;
	//マテリアルデータのテクスチャファイルパスを設定
	particleGroups[name].materialData->textureFilePath = textureFilePath;
	
}
