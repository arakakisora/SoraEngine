#include "ParticleManager.h"
#include"TextureManager.h"
#include <MyMath.h>
#include <numbers>




ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance()
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



void ParticleManager::Initialize(DirectXCommon* dxcmmon, SrvManager* srvmnager, const std::string& Modelfilepath)
{
	dxCommon_ = dxcmmon;
	srvManager_ = srvmnager;
	//乱数エンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 random(seedGenerator());
	randomEngine = random;
	SetModel(Modelfilepath);
	//頂点バッファビューの初期化		
	vertexBufferView = model_->GetVertexBufferView();
	//graphicsPipelineの初期化
	//グラフィックスパイプライン
	GraphicsPipeline* graphicsPipeline = nullptr;
	graphicsPipeline->Initialize(dxCommon_);

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * model_->GetModelData().vertices.size());
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&model_->GetVertexData()));
	std::memcpy(&model_->GetVertexData(), model_->GetModelData().vertices.data(), model_->GetModelData().vertices.size());

}

void ParticleManager::CreateParticleGroup(const std::string& groupName, const std::string& textureFilePath)
{
	//登録済みならアサート
	if (particleGroups.contains(groupName)) {

		assert(false);

	}
	//新たな空のパーティクルグループを作成しコンテナに登録
	ParticleGroup particleGroup;
	particleGroups.insert(std::make_pair(groupName, std::move(particleGroup)));//名前をキーにして登録
	//テクスチャファイルパスを登録
	//テクスチャファイルを読み込む
	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	//テクスチャファイルパスを登録
	particleGroups[groupName].material.textureFilePath = textureFilePath;
	//テクスチャファイルパスからテクスチャ番号を取得
	particleGroups[groupName].material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
	//インスタンスデータのリソースを作成
	particleGroups[groupName].instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * particleGroups[groupName].instanceCount);
	//インスタンスデータのリソースをマップ
	particleGroups[groupName].instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups[groupName].instanceData));
	//インスタンスデータのリソースにデータを書き込む
	std::memset(particleGroups[groupName].instanceData, 0, sizeof(ParticleForGPU) * particleGroups[groupName].instanceCount);
	//インスタンシング用のSRVインデックスを取得してSRVインデックスを記録
	particleGroups[groupName].instanceIndex = srvManager_->Allocate();
	//インスタンシング用のSRVを作成
	srvManager_->CreateSRVforStructuredBuffer(particleGroups[groupName].instanceIndex, particleGroups[groupName].instanceResource.Get(), particleGroups[groupName].instanceCount, sizeof(ParticleForGPU));



	//ビルボード行列の作成
	backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);

}

void ParticleManager::Update(Camera* camera)
{
	//ビルボード行列の計算
	Matrix4x4 billboardMatrix = backToFrontMatrix * camera->GetWorldMatrix();
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	//View行列とプロジェクション行列をカメラから取得
	const Matrix4x4& viewMatrix = camera->GetViewMatrix();
	const Matrix4x4& projectionMatrix = camera->GetProjextionMatrix();

	//すべてのパーティクルグループに対して更新処理を行う
	for (auto& [groupName, particleGroup] : particleGroups) {
		//パーティクルの更新
		for (std::list<Particle>::iterator particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end();) {

			//寿命に対していたらグループから削除
			if ((*particleIterator).lifetime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup.particles.erase(particleIterator);
				continue;
			}
			//加速度フィールドの適用
			if (MyMath::IsCollision(accelerationFienld.area, (*particleIterator).transform.translate)) {

				(*particleIterator).Velocity += accelerationFienld.acceleration * kDeletaTime;
			}
			//速度を加算
			(*particleIterator).transform.translate += (*particleIterator).Velocity * kDeletaTime;
			//時間を加算
			(*particleIterator).currentTime += kDeletaTime;
			//ワールド行列を計算
			Matrix4x4 worldMatrix = MyMath::MakeScaleMatrix((*particleIterator).transform.scale) * billboardMatrix * MyMath::MakeTranslateMatrix((*particleIterator).transform.translate);
			//ワールドビュープロジェクション行列を計算
			Matrix4x4 wvpMatrix = worldMatrix * viewMatrix * projectionMatrix;
			//インスタンシング用のデータ１個分の書き込み
			particleGroup.instanceData->WVP = wvpMatrix;
			particleGroup.instanceData->World = worldMatrix;


			++particleIterator;
		}
	}
}


void ParticleManager::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstans()->FindModel(filepath);
}