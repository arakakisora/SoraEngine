#include "ParticleMnager.h"
#include <ModelManager.h>
#include <TextureManager.h>
#include "CameraManager.h"
#include <MyMath.h>
#include <numbers>

#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI

#include <json.hpp>
#include <fstream>
#include <ChargeBehabiaor.h>

using json = nlohmann::json;

namespace
{
	// 文字列 → 頂点形状 enum 変換
	VerticesType ToVerticesType(const std::string& meshStr)
	{
		if (meshStr == "Ring")     return VerticesType::Ring;
		if (meshStr == "Cylinder") return VerticesType::Cylinder;
		if (meshStr == "Triangle") return VerticesType::Triangle;
		// デフォルトは Quad
		return VerticesType::Quad;
	}

	// 文字列 → IParticleBehavior 派生クラス 生成
	std::unique_ptr<IParticleBehavior> CreateBehaviorByName(const std::string& name)
	{
		if (name == "Explosion") {
			return std::make_unique<ExplosionBehavior>();
		}
		if (name == "Charge") {
			return std::make_unique<ChargeBehabiaor>();
		}
		if (name == "ExhaustGas") {
			return std::make_unique<ExhaustGasBehavior>();
		}

		// ここに新しいビヘイビアを追加していく
		// if (name == "Falling") { return std::make_unique<FallingBehavior>(); }

		assert(false && "Unknown behaviorType in CreateBehaviorByName()");
		return nullptr;
	}
}

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

	
}


void ParticleMnager::LoadFromJson(const std::string& filepath)
{
	// ファイルを開く
	std::ifstream ifs(filepath);
	if (!ifs) {
		// 見つからない場合はとりあえず assert しておく（後でログに変えても良い）
		assert(false && "Failed to open particle json file");
		return;
	}

	json root;
	ifs >> root;

	// "effects" 配列がない or 配列じゃない場合は何もしない
	if (!root.contains("effects") || !root["effects"].is_array()) {
		return;
	}

	for (auto& e : root["effects"]) {

		// id（パーティクルの名前）
		std::string id = e.value("id", "");
		if (id.empty()) {
			continue; // id が無いのはスキップ
		}

		// behaviorType（どのビヘイビアクラスを使うか）
		std::string behaviorType = e.value("behaviorType", "");
		if (behaviorType.empty()) {
			continue;
		}

		// mesh（形状名 → enum に変換）
		std::string meshStr = e.value("mesh", "Quad");
		VerticesType verticesType = ToVerticesType(meshStr);

		// texture（テクスチャのファイルパス）
		std::string texture = e.value("texture", "");
		if (texture.empty()) {
			continue;
		}

		// ビヘイビアインスタンス生成
		auto behavior = CreateBehaviorByName(behaviorType);
		if (!behavior) {
			continue;
		}

		// TODO: params 読み込みは後でやる
		// if (e.contains("params")) {
		//     behavior->LoadParams(e["params"]);
		// }

		// いつもの CreateParticleGroup を呼んで登録
		CreateParticleGroup(id, texture, verticesType, std::move(behavior));
	}


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
		auto& behavior = particleGroup.behavior;
		uint32_t counter = 0;
		for (std::list<Particle>::iterator particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end();) {

			//寿命に達していたらグループから外す
			if ((*particleIterator).lifetime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup.particles.erase(particleIterator);
				continue;
			}
			float alpha1 =0.5;
			
			behavior->Update((*particleIterator), 1.0f / 60.0f, particleGroup.materialData, alpha1);
			
			//ローテート
			Matrix4x4 rotateMatrix = MyMath::MakeRotateMatrix((*particleIterator).transform.rotate);
			//ワールド行列を計算
			Matrix4x4 provisionalworldMatrix = MyMath::MakeScaleMatrix((*particleIterator).transform.scale) * rotateMatrix * MyMath::MakeTranslateMatrix((*particleIterator).transform.translate);
			//waorldViewProjection行列を計算
			Matrix4x4 worldViewProjetionMatrix = provisionalworldMatrix * viewMatrix * projectionMatrix;


			if (counter < kMaxInstanceCount) {
				particleGroup.instanceData[counter].WVP = worldViewProjetionMatrix;
				particleGroup.instanceData[counter].World = provisionalworldMatrix;
				particleGroup.instanceData[counter].color = particleIterator->color;
				//particleGroup.instanceData[counter].color.w = alpha1;
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


		dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &particleGroup.vertexBufferView);
		//マテリアルのCBufferの場所を設定
		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, particleGroup.materialResource->GetGPUVirtualAddress());
		// インスタンシングデータの SRV を設定
		srvManager_->SetGraficsRootDescriptorTable(2, particleGroup.materialdata.textureIndex);
		// テクスチャの SRV を設定
		srvManager_->SetGraficsRootDescriptorTable(1, particleGroup.srvIndex);
		//描画！
		dxCommon_->GetCommandList()->DrawInstanced(UINT(particleGroup.vertexCount), particleGroup.instanceCount, 0, 0);

	}

}

void ParticleMnager::CreateParticleGroup(const std::string name, const std::string textureFilePath, VerticesType verticesTypeValue, std::unique_ptr<IParticleBehavior> behavior)
{
	//登録済みなら早期リターン
	if (particleGroups.contains(name)) {
		return;
	}

	//パーティクルグループを作成コンテナに登録
	ParticleGroup particleGroup;
	particleGroups.insert(std::make_pair(name, std::move(particleGroup)));//名前をキーにして登録

	//パーティクルグループのマテリアルデータを初期化
	//マテリアル
	particleGroups.at(name).materialResource = dxCommon_->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	particleGroups.at(name).materialData = nullptr;
	particleGroups.at(name).materialResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).materialData));
	//色
	particleGroups.at(name).materialData->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	particleGroups.at(name).materialData->enableLighting = false;//有効にするか否か
	particleGroups.at(name).materialData->uvTransform = particleGroups.at(name).materialData->uvTransform.MakeIdentity4x4();

	//頂点タイプの設定
	SetGroupVerticesType(name,verticesTypeValue);

	//テクスチャの設定
	SetGroupTexture(name, textureFilePath);

	//最大インスタンスカウント
	uint32_t MaxInstanceCount = kMaxInstanceCount;
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

	// Behavior 登録
	particleGroups.at(name).behavior = std::move(behavior);


	uint32_t defaultCount = 10;
	float    defaultLifetime = 1.0f;
	particleGroups.at(name).defaultCount = defaultCount;
	particleGroups.at(name).defaultLifetime = defaultLifetime;

}

void ParticleMnager::Emit(const std::string& name, const EulerTransform transform)
{
	//パーティクルグループが存在するかチェックしてassert
	assert(particleGroups.contains(name));

	for (uint32_t i = 0; i < particleGroups.at(name).defaultCount; ++i) {
		particleGroups.at(name).particles.push_back(
			particleGroups.at(name).behavior->Create(
				randomEngine, transform, particleGroups.at(name).defaultLifetime));
	}

}

void ParticleMnager::Emit(const std::string& name, const EulerTransform transform, uint32_t count, float lifetime)
{
	//パーティクルグループが存在するかチェックしてassert
	assert(particleGroups.contains(name));
	for (uint32_t i = 0; i < count; ++i) {
		particleGroups.at(name).particles.push_back(
			particleGroups.at(name).behavior->Create(
				randomEngine, transform, lifetime));
	}
}



std::vector<VertexData> ParticleMnager::MakeRingVertices(uint32_t  RingDivide, float outerRadius, float innerRadius)
{

	std::vector<VertexData> ringVertices;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(RingDivide);

	for (uint32_t index = 0; index < RingDivide; ++index) {
		// 現在と次の角度
		float angle = index * radianPerDivide;
		float nextAngle = ((index + 1) % RingDivide) * radianPerDivide;

		// sin, cos
		float sin = std::sinf(angle);
		float cos = std::cosf(angle);
		float sinnext = std::sinf(nextAngle);
		float cosnext = std::cosf(nextAngle);

		// UV (ここもwrapを考慮)
		float u = (static_cast<float>(index) / RingDivide);
		float unext = (static_cast<float>(index + 1) / RingDivide);

		VertexData v[] = {
			{ {-sin * outerRadius,  cos * outerRadius,  0.0f, 1.0f},     {u,     0.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sin * innerRadius,  cos * innerRadius,  0.0f, 1.0f},     {u,     1.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sinnext * outerRadius, cosnext * outerRadius, 0.0f, 1.0f}, {unext, 0.0f}, {0.0f, 0.0f, 1.0f} },

			{ {-sinnext * outerRadius, cosnext * outerRadius, 0.0f, 1.0f}, {unext, 0.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sin * innerRadius,  cos * innerRadius,  0.0f, 1.0f},     {u,     1.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sinnext * innerRadius, cosnext * innerRadius, 0.0f, 1.0f}, {unext, 1.0f}, {0.0f, 0.0f, 1.0f} }
		};

		for (const auto& vert : v) {
			ringVertices.push_back(vert);
		}
	}

	return ringVertices;

}

std::vector<VertexData> ParticleMnager::MakeCylinderVertices(uint32_t cylinderDivide, float topRadius, float bottomRadius, float height)
{
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(cylinderDivide);

	std::vector<VertexData> cylinderVertices;

	for (uint32_t index = 0; index < cylinderDivide; ++index) {

		float sin = std::sinf(index * radianPerDivide);
		float cos = std::cosf(index * radianPerDivide);
		float sinnext = std::sinf((index + 1) * radianPerDivide);
		float cosnext = std::cosf((index + 1) * radianPerDivide);
		float u = float(index) / float(cylinderDivide);
		float unext = float(index + 1) / float(cylinderDivide);

		VertexData v[] = {
			{{-sin * topRadius,height,cos * topRadius,1.0f},				{u,0.0f} ,		{-sin,0.0f,cos}},
			{{-sinnext * topRadius,height,cosnext * topRadius,1.0f},		{unext,0.0f},	{-sinnext,0.0f,cosnext}},
			{{-sin * bottomRadius,0.0f,cos * bottomRadius,1.0f},			{u,1.0f} ,		{-sin,0.0f,cos}},
			{{-sinnext * topRadius,height,cosnext * topRadius,1.0f},		{unext,0.0f},	{-sinnext,0.0f,cosnext}},
			{{-sinnext * bottomRadius,0.0f,cosnext * bottomRadius,1.0f},	{unext,1.0f},	{-sinnext,0.0f,cosnext}},
			{{-sin * bottomRadius,0.0f,cos * bottomRadius,1.0f},{u,1.0f} ,	{-sin,0.0f,cos}}

		};
		for (const auto& vert : v) {
			cylinderVertices.push_back(vert);
		}

	}
	return cylinderVertices;
}

std::vector<VertexData> ParticleMnager::MakeQuadVertices()
{
	//クワッドの頂点情報を作成
	std::vector<VertexData> vertices;
	vertices = {
			{{-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	};
	return vertices;
}

std::vector<VertexData> ParticleMnager::MakeTriangleVertices()
{
	//三角形の頂点情報を作成
	std::vector<VertexData> vertices;
	vertices = {
			{{-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.0f,  0.5f, 0.0f, 1.0f}, {0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	};
	return vertices;


}

void ParticleMnager::SetGroupTexture(const std::string& groupName, const std::string& textureFilePath)
{
	assert(particleGroups.contains(groupName));
	auto& group = particleGroups.at(groupName);

	// パスを登録
	group.materialdata.textureFilePath = textureFilePath;

	// テクスチャ読み込み（既に読まれていてもOKな設計ならそのまま）
	TextureManager::GetInstance()->LoadTexture(textureFilePath);

	// インデックス更新
	group.materialdata.textureIndex =
		TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

}

void ParticleMnager::SetGroupVerticesType(const std::string& groupName, VerticesType verticesType)
{
	//グループが存在するかチェック
	assert(particleGroups.contains(groupName));
	auto& group = particleGroups.at(groupName);
	//頂点タイプを設定
	group.verticesType = verticesType;

	//頂点データを作成
	//タイプ別で頂点データを作成
	std::vector<VertexData> vertices;
	switch (verticesType) {
	case VerticesType::Quad:     vertices = MakeQuadVertices();     break;//四角
	case VerticesType::Ring:     vertices = MakeRingVertices();     break;//リング
	case VerticesType::Cylinder: vertices = MakeCylinderVertices(); break;//シリンダー
	case VerticesType::Triangle: vertices = MakeTriangleVertices(); break;//三角形
	}
	//頂点数を設定
	group.vertexCount = static_cast<uint32_t>(vertices.size());
	// GPUリソース作成	
	group.vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * vertices.size());
	// リソースアドレスを設定
	group.vertexBufferView.BufferLocation = group.vertexResource->GetGPUVirtualAddress();
	// サイズとストライドを設定
	group.vertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices.size());
	// Stride
	group.vertexBufferView.StrideInBytes = sizeof(VertexData);

	// GPUにデータ転送
	VertexData* vertexData = nullptr;
	// マップ
	group.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// コピー
	std::memcpy(vertexData, vertices.data(), sizeof(VertexData) * vertices.size());
	// アンマップ
	group.vertexResource->Unmap(0, nullptr);

}

void ParticleMnager::SetBehavior(const std::string& groupName, std::unique_ptr<IParticleBehavior> behavior)
{
	//グループが存在するかチェック
	assert(particleGroups.contains(groupName) && "ParticleGroup does not exist!");

	// ムーブ代入でユニークポインタを保持させる
	particleGroups.at(groupName).behavior = std::move(behavior);
}

