#include "ParticleManager.h"
#include "TextureManager.h"
#include <MyMath.h>
#include <numbers>
#include "ModelManager.h"



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
	delete graphicsPipeline;
	delete instance;
	instance = nullptr;

}

void ParticleManager::Initialize(DirectXCommon* dxcommon, SrvManager* srvmnager, Camera* camera)
{
	graphicsPipeline = new GraphicsPipeline();
	graphicsPipeline->Initialize(dxcommon);
	//カメラの設定
	camera_ = camera;
	//初期化
	dxcommon_ = dxcommon;
	//SRVマネージャ
	srvmnager_ = srvmnager;
	//乱数エンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 random(seedGenerator());
	randomEngine = random;
	//グラフィックスパイプラインの作成
	graphicsPipeline->CreateParticle();

	//加速度フィールドの初期化

	accelerationFienld.acceleration = { 0.0f,0.0f,0.0f };
	accelerationFienld.area.min = { 0.0f,0.0f,0.0f };
	accelerationFienld.area.max = { 1.0f,1.0f,1.0f };

	



	//ビルボード行列の作成
	backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);

}

void ParticleManager::Update()
{
	//ビルボード行列の計算
	Matrix4x4 billboardMatrix = backToFrontMatrix * camera_->GetWorldMatrix();
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	//View行列とプロジェクション行列をカメラから取得
	const Matrix4x4& viewMatrix = camera_->GetViewMatrix();
	const Matrix4x4& projectionMatrix = camera_->GetProjextionMatrix();
	//寿命に達していたらグループから外す
	for (auto& [name, particleGroup] : particleGroups) {
		for (std::list<Particle>::iterator particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end();) {
			//寿命に達していたらグループから外す
			if ((*particleIterator).lifetime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup.particles.erase(particleIterator);
				continue;
			}

			if (MyMath::IsCollision(accelerationFienld.area, (*particleIterator).transform.translate)) {

				(*particleIterator).Velocity += accelerationFienld.acceleration * kDeletaTime;
			}
			//位置の更新
			(*particleIterator).transform.translate += (*particleIterator).Velocity * kDeletaTime;
			(*particleIterator).currentTime += kDeletaTime;
			//インスタンシングデータの作成
			//float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifetime);
			//ワールド行列を計算
			Matrix4x4 worldMatrix = MyMath::MakeScaleMatrix((*particleIterator).transform.scale) * billboardMatrix * MyMath::MakeTranslateMatrix((*particleIterator).transform.translate);
			//waorldViewProjection行列を計算
			Matrix4x4 worldViewProjetionMatrix = worldMatrix * viewMatrix * projectionMatrix;




			//instancing用データ1個分の書き込み
			particleGroup.instancingData->WVP = worldViewProjetionMatrix;
			particleGroup.instancingData->World = worldMatrix;




			particleIterator++;

		}

	}






}

void ParticleManager::Draw(const std::string& name, const std::string& modelfilepath)
{


	SetModel(modelfilepath);
	//パーティクルグループが登録されていないならreturn
	if (!particleGroups.contains(name)) { return; }

	//modelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	materialResource = dxcommon_->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色
	materialData->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialData->enableLighting = false;//有効にするか否か
	materialData->uvTransform = MyMath::MekeIdentity4x4();


	//ルートシグネチャを設定
	dxcommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline->GetRootSignatureParticle());
	//psoを設定
	dxcommon_->GetCommandList()->SetPipelineState(graphicsPipeline->GetGraphicsPipelineStateParticle());
	//purimitetopologyを設定
	dxcommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//vertexBufferViewを設定
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = model_->GetVertexBufferView();
	dxcommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
	//テクスチャのSRVのディスクリプタヒープを設定
	//dxcommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	srvmnager_->SetGraficsRootDescriptorTable(1, particleGroups.at(name).instanceingsrvIndex);
	srvmnager_->SetGraficsRootDescriptorTable(2, particleGroups.at(name).materialData.textureIndex);
	dxcommon_->GetCommandList()->DrawInstanced(UINT(model_->GetModelData().vertices.size()), particleGroups.at(name).kNumInstance, 0, 0);


}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath, uint32_t maxInstance)
{


	//すでに登録されているならreturn
	if (particleGroups.contains(name)) { return; }
	//パーティクルグループの作成
	ParticleGroup particleGroup;
	particleGroups.insert(std::make_pair(name, std::move(particleGroup)));//名前をキーにして登録
	TextureManager::GetInstance()->LoadTexture(textureFilePath);//テクスチャファイルの読み込み
	particleGroups.at(name).materialData.textureFilePath = textureFilePath;//テクスチャファイルパス
	particleGroups.at(name).materialData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);	//テクスチャ番号の取得
	particleGroups.at(name).instancingResouce = dxcommon_->CreateBufferResource(sizeof(ParticleForGPU) * maxInstance);//インスタンシング用のリソースを作成
	particleGroups.at(name).instancingResouce->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).instancingData));
	//インスタンシング用にSRVのインデックスを取得
	particleGroups.at(name).instanceingsrvIndex = srvmnager_->Allocate();
	particleGroups.at(name).kNumInstance = maxInstance;
	//SRVの作成
	srvmnager_->CreateSRVforStructuredBuffer(particleGroups.at(name).instanceingsrvIndex, particleGroups.at(name).instancingResouce.Get(), maxInstance, sizeof(ParticleForGPU));

}

void ParticleManager::Emit(const std::string& name, const Vector3& position, uint32_t count)
{
	//登録済みのパーティクルグループ名かチェックしてassert
	assert(particleGroups.contains(name));
	//新たなパーティクルを作成して、指定されたパーティクルに追加
	for (uint32_t i = 0; i < count; ++i) {
		Particle particle;
		particle.transform.translate = position;
		//particle.transform.translate.x +=i*1.0f;
		particle.transform.rotate = { 0.0f,3.0f,0.0f };
		particle.transform.scale = { 1.1f,1.1f,1.1f };
		particle.Velocity = { 0.0f,0.0f,0.0f };
		particle.lifetime = 3.0f;
		particle.currentTime = 0.0f;
		//発生処理

		particleGroups.at(name).particles.push_back(particle);

	}


}



void ParticleManager::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstans()->FindModel(filepath);
}
