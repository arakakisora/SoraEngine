#include "ParticleMnager.h"
#include <ModelManager.h>
#include <TextureManager.h>
#include "CameraManager.h"
#include <MyMath.h>
#include <numbers>

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
	graphicsPipeline_ = new GraphicsPipeline();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateParticle();
	ModelManager::GetInstans()->LoadModel("plane.obj");
	//モデルのセット
	SetModel("plane.obj");

	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixData));
	//単位行列を書き込む

	transformaitionMatrixData->WVP = transformaitionMatrixData->WVP.MakeIdentity4x4();
	transformaitionMatrixData->World = transformaitionMatrixData->World.MakeIdentity4x4();


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



void ParticleMnager::Update()
{
	//カメラからビュープロジェクション行列を取得
	//ビルボード行列の計算
	Matrix4x4 billboardMatrix = backToFrontMatrix * CameraManager::GetInstans()->GetActiveCamera()->GetWorldMatrix();
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	//ビルボード行列を使ってビルボード行列を計算
	Matrix4x4 viewMatrix = CameraManager::GetInstans()->GetActiveCamera()->GetViewMatrix();
	Matrix4x4 projectionMatrix = CameraManager::GetInstans()->GetActiveCamera()->GetProjextionMatrix();
	uint32_t counter = 0;
	//全パーティクル	グループ内の全パーティクルについて二重処理する
	for (auto& [name, particleGroup] : particleGroups) {
		for (std::list<Particle>::iterator particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end();) {

			//パーティクルの寿命を減らす
			particleIterator->currentTime -= 1.0f / 60.0f;
			//パーティクルの寿命が尽きたらグループから外す
			//寿命に達していたらグループから外す
			if ((*particleIterator).lifetime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup.particles.erase(particleIterator);
				continue;
			}
			//パーティクルの位置を更新
			particleIterator->transform.translate += particleIterator->Velocity;

			Matrix4x4 worldMatrix = MyMath::MakeScaleMatrix((*particleIterator).transform.scale) * billboardMatrix * MyMath::MakeTranslateMatrix((*particleIterator).transform.translate);
			//waorldViewProjection行列を計算
			Matrix4x4 worldViewProjetionMatrix = worldMatrix * viewMatrix * projectionMatrix;

			//パーティクルのデータを更新
			particleGroup.instanceData->WVP = worldViewProjetionMatrix;
			particleGroup.instanceData->World = worldMatrix;

			if (counter < particleGroup.instanceCount) {
				particleGroup.instanceData[counter].WVP = worldViewProjetionMatrix;
				particleGroup.instanceData[counter].World = worldMatrix;
				/*particleGroup.instanceData[counter].color = (*particleIterator).color;
				particleGroup.instanceData[counter].color.w = alpha;*/

				++counter;
			}


			//次のパーティクルに進む
			++particleIterator;

		}
	}





}

void ParticleMnager::Draw()
{
	//ルートシグネチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureParticle());
	//psoを設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateParticle());
	//purimitetopologyを設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//VertexBufferViewを設定
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = model_->GetVertexBufferView();
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	// パーティクルグループごとに描画
	for (const auto& [name, particleGroup] : particleGroups) {
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

void ParticleMnager::CreateParticleGroup(const std::string name, const std::string textureFilePath)
{
	//登録済みの名前かチェックしてassert
	assert(!particleGroups.contains(name));
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
	uint32_t MaxInstanceCount = 100;
	//インスタンス数を初期化
	particleGroups.at(name).instanceCount = 0;
	//インスタンス用のリソースを作成
	particleGroups.at(name).instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * MaxInstanceCount);
	//インスタンス用のリソースをマップ
	particleGroups.at(name).instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).instanceData));
	//インスタンスのデータを初期化
	ParticleForGPU particleForGPU;
	particleForGPU.WVP = transformaitionMatrixData->WVP;
	particleForGPU.World = transformaitionMatrixData->World;
	particleForGPU.color = { 1.0f,1.0f,1.0f,1.0f };
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
	// ランダム分布の定義（位置オフセット用）
	std::uniform_real_distribution<float> randomPositionOffset(-1.0f, 1.0f); // 位置のランダムオフセット


	//パーティクルグループが存在するかチェックしてassert
	assert(particleGroups.contains(name));
	//パーティクルグループのパーティクルリストにパーティクルを追加
	for (uint32_t i = 0; i < count; ++i) {
		Particle particle;
		//パーティクルの初期位置を設定
		particle.transform.translate = position;
		particle.transform.scale = { 1.0f,1.0f,1.0f };
		//パーティクルの初速度を設定
		std::uniform_real_distribution<float> randomVelocity(-0.1f, 0.1f);
		particle.Velocity = { randomVelocity(randomEngine),randomVelocity(randomEngine),randomVelocity(randomEngine) };
		//パーティクルの寿命を設定
		particle.lifetime = 0.5f;
		//パーティクルの現在の寿命を設定
		particle.currentTime = 0.0f;
		//パーティクルを追加
		particleGroups.at(name).particles.push_back(particle);
	}
	//パーティクルグループのインスタンス数を更新
	particleGroups.at(name).instanceCount = count;
	////インスタンス用のリソースを作成
	//particleGroups.at(name).instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * particleGroups.at(name).instanceCount);
	////インスタンス用のリソースをマップ
	//particleGroups.at(name).instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).instanceData));
	//


}

void ParticleMnager::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstans()->FindModel(filepath);
}
