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
	
	graphicsPipeline = new GraphicsPipeline();
	graphicsPipeline->Initialize(dxCommon_);
	graphicsPipeline->CreateParticle();

	//Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * model_->GetModelData().vertices.size());
	//vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&model_->GetVertexData()));
	//std::memcpy(&model_->GetVertexData(), model_->GetModelData().vertices.data(), model_->GetModelData().vertices.size());

	//マテリアル
	//modelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	materialResource = dxCommon_->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色
	materialData->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };

	materialData->enableLighting = false;//有効にするか否か
	materialData->uvTransform = materialData->uvTransform.MakeIdentity4x4();

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
	particleGroups[groupName].instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance);
	//インスタンスデータのリソースをマップ
	particleGroups[groupName].instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups[groupName].instanceData));

	//単位行列を書き込んでおく
	for (uint32_t i = 0; i < kNumMaxInstance; i++) {
		particleGroups[groupName].instanceData[i].WVP = MyMath::MekeIdentity4x4();
		particleGroups[groupName].instanceData[i].World = MyMath::MekeIdentity4x4();

	}

	//インスタンシング用のSRVインデックスを取得してSRVインデックスを記録
	particleGroups[groupName].instanceIndex = srvManager_->Allocate();
	//インスタンシング用のSRVを作成
	srvManager_->CreateSRVforStructuredBuffer(particleGroups[groupName].instanceIndex, particleGroups[groupName].instanceResource.Get(), kNumMaxInstance, sizeof(ParticleForGPU));



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
			if (particleGroup.instanceCount < kNumMaxInstance) {


				particleGroup.instanceData[particleGroup.instanceCount].World = worldMatrix;
				particleGroup.instanceData[particleGroup.instanceCount].World = worldMatrix;
				particleGroup.instanceData[particleGroup.instanceCount].color = (*particleIterator).color;
				++particleGroup.instanceCount;
			}


			++particleIterator;
		}
	}
}


void ParticleManager::Draw()
{
	


	//ルートシグネチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline->GetRootSignatureParticle());
	//psoを設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline->GetGraphicsPipelineStateParticle());
	//purimitetopologyを設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//頂点バッファビューを設定
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	//すべてのパーティクルグループに対して描画処理を行う
	for (auto& [groupName, particleGroup] : particleGroups) {
		// インスタンス数がゼロの場合はスキップ
		if (particleGroup.instanceCount == 0) {
			continue;
		}

		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		//テクスチャを設定
		//dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDrscriptorHandle(particleGroup.material.textureIndex));
		////インスタンシング用のSRVを設定
		//dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDrscriptorHandle(particleGroup.instanceIndex));

		srvManager_->SetGraficsRootDescriptorTable(1, particleGroup.instanceIndex);
		srvManager_->SetGraficsRootDescriptorTable(2, particleGroup.material.textureIndex);

		//インスタンシング描画
		dxCommon_->GetCommandList()->DrawInstanced(UINT(model_->GetModelData().vertices.size()), particleGroup.instanceCount, 0, 0);
	}


}

void ParticleManager::Emit(const std::string& groupName, const Transform& transform, uint32_t count)
{
	//パーティクルグループが存在しない場合はアサート
	if (!particleGroups.contains(groupName)) {
		assert(false);
	}

	ParticleGroup& group = particleGroups[groupName];

	//// 既存のインスタンス数が上限を超える場合、生成数を調整
	//if (group.particles.size() + count > kNumMaxInstance) {
	//	count = kNumMaxInstance - group.particles.size();
	//	OutputDebugStringA("Warning: Particle count adjusted to avoid exceeding maximum instances.\n");
	//}

	//新たなパーティクルを作成し、指定されたパーティクルグループに登録
	for (uint32_t i = 0; i < count; ++i) {
		Particle particle;
		particle.transform = transform;
		particle.Velocity = { 0.0f,0.0f,0.0f };
		particle.color = { Vector4(1.0f,1.0f,1.0f,1.0f) };
		particle.lifetime = 1.0f;
		particle.currentTime = 0.0f;

		particleGroups[groupName].particles.push_back(particle);
	};
	

}

void ParticleManager::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstans()->FindModel(filepath);
}