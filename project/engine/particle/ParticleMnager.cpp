#include "ParticleMnager.h"
#include <ModelManager.h>
#include <TextureManager.h>
#include "CameraManager.h"
#include <MyMath.h>
#include <numbers>
#include <iterator>
#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG

//シングルトンインスタンスの取得

ParticleMnager* ParticleMnager::instance_ = nullptr;
ParticleMnager* ParticleMnager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new ParticleMnager();
	}
	return instance_;


}


void ParticleMnager::Initialize(DirectXCommon* dxcommn, SrvManager* srvmaneger)
{
	//引数で受け取ったポインタをメンバ変数に代入
	dxCommon_ = dxcommn;
	srvManager_ = srvmaneger;
	//乱数エンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 random(seedGenerator());
	randomEngine = random;
	//パイプラインの生成
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateParticle();


	//カメラとモデルのTrandform変数
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	//worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	//ビルボード行列の作成
	backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);

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




void ParticleMnager::Finalize()
{

	delete instance_;
	instance_ = nullptr;


}


void ParticleMnager::Update()
{
	//カメラからビュープロジェクション行列を取得
	//ビルボード行列の計算
	Matrix4x4 billboardMatrix = backToFrontMatrix * CameraManager::GetInstance()->GetActiveCamera()->GetWorldMatrix();
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	//ビルボード行列を使ってビルボード行列を計算
	Matrix4x4 viewMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetViewMatrix();
	Matrix4x4 projectionMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetProjextionMatrix();

	//全パーティクル	グループ内の全パーティクルについて二重処理する
	for (auto& [name, particleGroup] : particleGroups) {
		uint32_t counter = 0;
		for (std::list<Particle>::iterator particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end();) {


			//パーティクルの寿命が尽きたらグループから外す
			//寿命に達していたらグループから外す
			if ((*particleIterator).lifetime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup.particles.erase(particleIterator);
				continue;
			}

			//パーティクルの位置を更新
			(*particleIterator).transform.translate += (*particleIterator).Velocity * 1.0f / 60.0f;
			//パーティクルの寿命を減らす
			(*particleIterator).currentTime += 1.0f / 60.0f;
			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifetime);


			// 毎フレーム回転
			particleIterator->transform.rotate.x += 0.05f;
			particleIterator->transform.rotate.y += 0.1f;
			particleIterator->transform.rotate.z += 0.2f;

			Matrix4x4 rotateX = MyMath::MakeRotateXMatrix(particleIterator->transform.rotate.x);
			Matrix4x4 rotateY = MyMath::MakeRotateYMatrix(particleIterator->transform.rotate.y);
			Matrix4x4 rotateZ = MyMath::MakeRotateZMatrix(particleIterator->transform.rotate.z);

			// 回転行列の合成（順序：Z → Y → X）
			Matrix4x4 rotateMatrix = rotateZ * rotateY * rotateX;
			Matrix4x4 scaleMatrix = MyMath::MakeScaleMatrix(particleIterator->transform.scale);
			Matrix4x4 translateMatrix = MyMath::MakeTranslateMatrix(particleIterator->transform.translate);

			// ビルボードを含めた SRT 合成（順序は演出次第で調整）
			Matrix4x4 worldMatrix = scaleMatrix * rotateMatrix * billboardMatrix * translateMatrix;


			//ワールド行列を計算
			//Matrix4x4 worldMatrix = MyMath::MakeScaleMatrix((*particleIterator).transform.scale) * billboardMatrix * MyMath::MakeTranslateMatrix((*particleIterator).transform.translate);
			//waorldViewProjection行列を計算
			Matrix4x4 worldViewProjetionMatrix = worldMatrix * viewMatrix * projectionMatrix;


			if (counter < particleGroup.instanceCount) {
				particleGroup.instanceData[counter].WVP = worldViewProjetionMatrix;
				particleGroup.instanceData[counter].World = worldMatrix;
				particleGroup.instanceData[counter].color = particleIterator->color;
				particleGroup.instanceData[counter].color.w = alpha;
				++counter;
			}




			//次のパーティクルに進む
			++particleIterator;

		}

		// ここでインスタンス数を更新
		particleGroup.instanceCount = counter;

	}




}

void ParticleMnager::Draw()
{


	// パーティクルグループが設定されていない場合は描画しない
	if (particleGroups.empty()) {
		return;
	}

	//ルートシグネチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureParticle());
	//psoを設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateParticle());
	//purimitetopologyを設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パーティクルグループごとに描画
	for (const auto& [name, particleGroup] : particleGroups) {


		// インスタンス数が 0 の場合は描画しない
		if (particleGroup.instanceCount == 0) {
			continue;
		}

		//VertexBufferViewを設定
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = model_->GetVertexBufferView();
		dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
		//マテリアルのCBufferの場所を設定
		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		// インスタンシングデータの SRV を設定
		srvManager_->SetGraficsRootDescriptorTable(2, particleGroup.materialdata.textureIndex);
		// テクスチャの SRV を設定
		srvManager_->SetGraficsRootDescriptorTable(1, particleGroup.srvIndex);
		//描画！
		dxCommon_->GetCommandList()->DrawInstanced(UINT(model_->GetModelData().vertices.size()), particleGroup.instanceCount, 0, 0);

	}

}

void ParticleMnager::CreateParticleGroup(const std::string name, const std::string textureFilePath, std::string modelFilePath)
{
	ModelManager::GetInstans()->LoadModel(modelFilePath);
	//モデルのセット
	SetModel(modelFilePath);



	//登録済みなら早期リターン
	if (particleGroups.contains(name)) {
		return;
	}

	//パーティクルグループを作成コンテナに登録
	ParticleGroup particleGroup;
	particleGroups.insert(std::make_pair(name, std::move(particleGroup)));//名前をキーにして登録
	//テクスチャファイルパスを登録
	particleGroups.at(name).materialdata.textureFilePath = textureFilePath;
	//テクスチャファイルを読み込んでSRVを取得
	TextureManager::GetInstance()->LoadTexture(textureFilePath);//テクスチャファイルの読み込み
	//SRVのインデックスを取得
	particleGroups.at(name).materialdata.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);	//テクスチャ番号の取得
	//最大インスタンスカウント
	uint32_t MaxInstanceCount = 1000;
	//インスタンス数を初期化
	particleGroups.at(name).instanceCount = 0;
	//インスタンス用のリソースを作成
	particleGroups.at(name).instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * MaxInstanceCount);
	//インスタンス用のリソースをマップ
	particleGroups.at(name).instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).instanceData));
	//インスタンスのデータを初期化
	ParticleForGPU particleForGPU;
	particleForGPU.WVP = particleForGPU.WVP.MakeIdentity4x4();
	particleForGPU.World = particleForGPU.World.MakeIdentity4x4();
	particleForGPU.color = { 1.0f,1.0f,1.0f,0.0f };
	//インスタンスのデータを登録
	for (uint32_t index = 0; index < MaxInstanceCount; ++index) {
		particleGroups.at(name).instanceData[index] = particleForGPU;
	}

	//insutansing用のsrvインデックス
	particleGroups.at(name).srvIndex = srvManager_->Allocate();
	//srv生成
	srvManager_->CreateSRVforStructuredBuffer(particleGroups.at(name).srvIndex, particleGroups.at(name).instanceResource.Get(), MaxInstanceCount, sizeof(ParticleForGPU));






}

void ParticleMnager::Emit(const std::string& name, const Vector3 position, uint32_t count)
{

	//パーティクルグループが存在するかチェックしてassert
	assert(particleGroups.contains(name));
	//パーティクルグループのパーティクルリストにパーティクルを追加
	for (uint32_t i = 0; i < count; ++i) {


		//パーティクルを追加
		particleGroups.at(name).particles.push_back(MakeNewParticle(randomEngine, position));

	}

	//パーティクルグループのインスタンス数を更新
	particleGroups.at(name).instanceCount = count;
	////インスタンス用のリソースを作成
	//particleGroups.at(name).instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * particleGroups.at(name).instanceCount);
	////インスタンス用のリソースをマップ
	//particleGroups.at(name).instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).instanceData));
}

void ParticleMnager::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstans()->FindModel(filepath);
}

Particle ParticleMnager::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate)
{


	std::uniform_real_distribution<float>distribution(-1.0, 1.0f);
	Vector3 randomTranslate{ distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	std::uniform_real_distribution<float>distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float>distTime(1.0f, 3.0f);

	Particle particle;

	particle.transform.scale = { 1.0f,1.0f,1.0f };
	//particle.transform.rotate = { 0.0f,3.0f,0.0f };
	particle.transform.translate = translate + randomTranslate;
	particle.Velocity = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	particle.lifetime = distTime(randomEngine);
	particle.currentTime = 0;
	return particle;
}

void ParticleMnager::PlayerEmit(const std::string& name, const Vector3 position, uint32_t count, bool isRight)
{
	assert(particleGroups.contains(name));

	auto& group = particleGroups.at(name);

	// 超過分を削除（新しいパーティクルを追加するために古いパーティクルを削除）
	uint32_t currentCount = static_cast<uint32_t>(group.particles.size());
	uint32_t const Maxsize = 15;
	uint32_t toRemove = (currentCount + count > Maxsize) ? (currentCount + count - Maxsize) : 0;
	if (toRemove > 0) {
		auto eraseBegin = group.particles.begin();
		auto eraseEnd = std::next(eraseBegin, toRemove); // リスト用
		group.particles.erase(eraseBegin, eraseEnd);
		group.instanceCount -= toRemove;
	}

	// 実際に追加
	for (uint32_t i = 0; i < count; ++i) {
		group.particles.push_back(PlayerMakeNewParticle(randomEngine, position, isRight));
	}

	group.instanceCount += count;
}


Particle ParticleMnager::PlayerMakeNewParticle(std::mt19937& randomEngine, const Vector3& translate, bool isRight)
{

	//std::uniform_real_distribution<float>distribution(-0.1f, 0.1f);
	std::uniform_real_distribution<float>distrotate(-5.0f, 5.0f);
	std::uniform_real_distribution<float>distColor(0.0f, 1.0f);


	std::uniform_real_distribution<float>distribution(-0.1f, 0.1f);
	Vector3 randomTranslate{ 0.0f,distribution(randomEngine) ,0.0f };

	float distVelocityX = isRight ? 1.5f : -1.5f;
	float distVelocityY = 2.0f;

	Particle particle;
	Vector3 randomRotate{ distrotate(randomEngine),distrotate(randomEngine) ,distrotate(randomEngine) };
	particle.transform.scale = { 0.1f,0.1f,0.1f };
	particle.transform.translate = translate + randomTranslate;
	particle.transform.rotate = randomRotate;
	particle.Velocity = { distVelocityX,distVelocityY ,0.0f };
	particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	particle.lifetime = 0.3f;
	particle.currentTime = 0;
	return particle;
}