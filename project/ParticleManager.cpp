#include "ParticleManager.h"
#include "TextureManager.h"
#include <MyMath.h>
#include <numbers>



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
	//頂点データの初期化
	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = dxcommon_->CreateBufferResource(sizeof(TransformationMatrix));
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixData));
	//単位行列を書き込む
	transformaitionMatrixData->WVP = transformaitionMatrixData->WVP.MakeIdentity4x4();
	transformaitionMatrixData->World = transformaitionMatrixData->World.MakeIdentity4x4();
	backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);

}

void ParticleManager::Update()
{
	//ビルボード行列の計算
	Matrix4x4 billboardMatrix =backToFrontMatrix* camera_->GetWorldMatrix();
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	//View行列とプロジェクション行列をカメラから取得
	const Matrix4x4& viewMatrix = camera_->GetViewMatrix();
	const Matrix4x4& projectionMatrix = camera_->GetProjextionMatrix();
	//寿命に達していたらグループから外す




}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath, uint32_t maxInstance)
{
	//すでに登録されているならreturn
	if (particleGroups.contains(name)) { return; }
	//パーティクルグループの作成
	ParticleGroup particleGroup;
	particleGroups.insert(std::make_pair(name, std::move(particleGroup)));//名前をキーにして登録
	particleGroups.at(name).kNumMaxInstance = maxInstance;//最大インスタンス数
	particleGroups.at(name).materialData->textureFilePath = textureFilePath;//テクスチャファイルパス
	TextureManager::GetInstance()->LoadTexture(textureFilePath);//テクスチャファイルの読み込み
	particleGroups.at(name).materialData->textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);	//テクスチャ番号の取得
	particleGroups.at(name).instancingResouce = dxcommon_->CreateBufferResource(sizeof(ParticleForGPU) * maxInstance);//インスタンシング用のリソースを作成
	particleGroups.at(name).srvIndex = srvmnager_->Allocate();//SRVのインデックスを取得
	//SRVの作成
	srvmnager_->CreateSRVforStructuredBuffer(particleGroups.at(name).srvIndex, particleGroups.at(name).instancingResouce.Get(), maxInstance, sizeof(ParticleForGPU));



}
