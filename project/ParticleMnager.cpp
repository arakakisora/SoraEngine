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
	//ビルボード行列の作成
	backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);
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
	//全パーティクル内の全パーティクルについて二重処理する
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

			//パーティクルのデータを更新
			ParticleForGPU particleForGPU;
			//ビルボード行列を使ってビルボード行列を計算
			particleForGPU.World = MyMath::MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, particleIterator->transform.translate);

			Matrix4x4 worldMatrix = MyMath::MakeScaleMatrix((*particleIterator).transform.scale) * billboardMatrix * MyMath::MakeTranslateMatrix((*particleIterator).transform.translate);
			//waorldViewProjection行列を計算
			Matrix4x4 worldViewProjetionMatrix = worldMatrix * viewMatrix * projectionMatrix;






		}
	}





}

void ParticleMnager::CreateParticleGroup(const std::string name, const std::string textureFilePath)
{
	//登録済みの名前かチェックしてassert
	assert(particleGroups.contains(name));
	//パーティクルグループを作成コンテナに登録
	ParticleGroup particleGroup;
	particleGroups.insert(std::make_pair(name, std::move(particleGroup)));//名前をキーにして登録
	//テクスチャファイルパスを登録
	particleGroups[name].materialdata.textureFilePath = textureFilePath;
	//テクスチャファイルを読み込んでSRVを取得
	TextureManager::GetInstance()->LoadTexture(textureFilePath);//テクスチャファイルの読み込み
	//SRVのインデックスを取得
	particleGroups.at(name).materialdata.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);	//テクスチャ番号の取得
	//インスタンス用のリソースを作成
	particleGroups.at(name).instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * particleGroups.at(name).instanceCount);
	//インスタンス用のリソースをマップ
	particleGroups.at(name).instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).instanceData));
	//インスタンスのデータを初期化
	ParticleForGPU particleForGPU;
	particleForGPU.WVP = transformaitionMatrixData->WVP;
	particleForGPU.World = transformaitionMatrixData->World;
	particleForGPU.color = { 1.0f,1.0f,1.0f,1.0f };
	//インスタンスのデータを登録
	particleGroups.at(name).instanceData->WVP = particleForGPU.WVP;
	particleGroups.at(name).instanceData->World = particleForGPU.World;
	particleGroups.at(name).instanceData->color = particleForGPU.color;
	//insutansing用のsrvインデックス
	particleGroups.at(name).srvIndex = srvManager_->Allocate();
	//srv生成
	srvManager_->CreateSRVforStructuredBuffer(particleGroups.at(name).srvIndex, particleGroups.at(name).instanceResource.Get(), particleGroups.at(name).instanceCount, sizeof(ParticleForGPU));






}

void ParticleMnager::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstans()->FindModel(filepath);
}
