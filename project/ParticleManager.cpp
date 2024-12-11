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
	delete instance;
	instance = nullptr;

}

void ParticleManager::Initialize(DirectXCommon* dxcommon, SrvManager* srvmnager, Camera* camera)
{
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
	graphicsPipeline->Create(dxcommon_);

	//加速度フィールドの初期化

	accelerationFienld.acceleration = { 15.0f,0.0f,0.0f };
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
		for (std::list<Particle>::iterator particleIterator =particleGroup.particles.begin(); particleIterator !=particleGroup.particles.end();) {
			//寿命に達していたらグループから外す
			if ((*particleIterator).lifetime <= (*particleIterator).currentTime) {
				particleIterator =particleGroup.particles.erase(particleIterator);
				continue;
			}
			
			if (MyMath::IsCollision(accelerationFienld.area, (*particleIterator).transform.translate)) {

				(*particleIterator).Velocity += accelerationFienld.acceleration * kDeletaTime;
			}
			//位置の更新
			(*particleIterator).transform.translate += (*particleIterator).Velocity * kDeletaTime;
			(*particleIterator).currentTime += kDeletaTime;
			//インスタンシングデータの作成
			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifetime);
			//ワールド行列を計算
			Matrix4x4 worldMatrix = MyMath::MakeScaleMatrix((*particleIterator).transform.scale) * billboardMatrix * MyMath::MakeTranslateMatrix((*particleIterator).transform.translate);
			//waorldViewProjection行列を計算
			Matrix4x4 worldViewProjetionMatrix = worldMatrix * viewMatrix * projectionMatrix;
		
			//instancing用データ1個分の書き込み
			particleGroup.instancingData->WVP = worldViewProjetionMatrix;
			particleGroup.instancingData->World = worldMatrix;
			
			if (particleGroup.kNumMaxInstance < particleGroup.particles.size()) {
				
			}
			particleGroup.kNumMaxInstance++;


		}
	}
	
	




}

void ParticleManager::Draw(const std::string& name)
{
	//ルートシグネチャを設定
	dxcommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline->GetRootSignature());
	//psoを設定
	dxcommon_->GetCommandList()->SetPipelineState(graphicsPipeline->GetGraphicsPipelineState());
	//purimitetopologyを設定
	dxcommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//vertexBufferViewを設定
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = model_->GetVertexBufferView();
	dxcommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
	//テクスチャのSRVのディスクリプタヒープを設定
	srvmnager_->SetGraficsRootDescriptorTable(2, particleGroups.at(name).srvIndex);
	srvmnager_->SetGraficsRootDescriptorTable(3, particleGroups.at(name).materialData->textureIndex);
	dxcommon_->GetCommandList()->DrawInstanced(UINT(model_->GetModelData().vertices.size()), particleGroups.at(name).kNumMaxInstance, 0, 0);


}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath, uint32_t maxInstance, const std::string& modelfilepath)
{
	SetModel(modelfilepath);

	//すでに登録されているならreturn
	if (particleGroups.contains(name)) { return; }
	//パーティクルグループの作成
	ParticleGroup particleGroup;
	particleGroups.insert(std::make_pair(name, std::move(particleGroup)));//名前をキーにして登録
	particleGroups.at(name).materialData->textureFilePath = textureFilePath;//テクスチャファイルパス
	TextureManager::GetInstance()->LoadTexture(textureFilePath);//テクスチャファイルの読み込み
	particleGroups.at(name).materialData->textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);	//テクスチャ番号の取得
	particleGroups.at(name).instancingResouce = dxcommon_->CreateBufferResource(sizeof(ParticleForGPU) * maxInstance);//インスタンシング用のリソースを作成
	particleGroups.at(name).srvIndex = srvmnager_->Allocate();//SRVのインデックスを取得
	//SRVの作成
	srvmnager_->CreateSRVforStructuredBuffer(particleGroups.at(name).srvIndex, particleGroups.at(name).instancingResouce.Get(), maxInstance, sizeof(ParticleForGPU));

}

void ParticleManager::Emit(const std::string& name, const Vector3& position, uint32_t count)
{
	//登録済みのパーティクルグループ名かチェックしてassert
	assert(particleGroups.contains(name));
	//新たなパーティクルを作成して、指定されたパーティクルに追加
	for (uint32_t i = 0; i < count; i++) {
		Particle particle;
		particle.transform.translate = position;
		particle.transform.scale = { 0.1f,0.1f,0.1f };
		particle.Velocity = { 0.0f,0.0f,0.0f };
		particle.lifetime = 1.0f;
		particle.currentTime = 0.0f;
		particleGroups.at(name).particles.push_back(particle);
	}
	

}



void ParticleManager::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstans()->FindModel(filepath);
}
