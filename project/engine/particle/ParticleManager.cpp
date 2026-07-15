#include "ParticleManager.h"
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
#include <ChargeBehavior.h>
#include "ParticleEditor.h"

#include <filesystem>
#include <BarrierBreakBehavior.h>
#include <BarrierRingBehavior.h>


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
			return std::make_unique<ChargeBehavior>();
		}
		if (name == "ExhaustGas") {
			return std::make_unique<ExhaustGasBehavior>();
		}
		if (name == "BarrierBreak") {
			return std::make_unique<BarrierBreakBehavior>();
		}
		if (name == "BarrierRing") {
			return std::make_unique<BarrierRingBehavior>();
		}

		// ここに新しいビヘイビアを追加していく
		// if (name == "Falling") { return std::make_unique<FallingBehavior>(); }

		assert(false && "Unknown behaviorType in CreateBehaviorByName()");
		return nullptr;
	}
}

//シングルトンインスタンスの取得
ParticleManager* ParticleManager::instance_ = nullptr;
ParticleManager* ParticleManager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new ParticleManager();
	}
	return instance_;


}

void ParticleManager::ImguiDrawEditor()
{
#ifdef USE_IMGUI
	//Editorが生成されていたら描画
	editor_->DrawImguiEditor();
#endif

}

void ParticleManager::Initialize(DirectXCommon* dxcommn, SrvManager* srvmaneger)
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


	//カメラとモデルのTransform変数
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	//worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	//ビルボード行列の作成
	backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);

#ifdef USE_IMGUI
	//Editor を生成
	editor_ = std::make_unique<ParticleEditor>(this);
#endif
}


void ParticleManager::LoadFromJson(const std::string& filepath)
{
	// ファイルを開く
	std::ifstream ifs(filepath);
	if (!ifs) {
		// 見つからない場合 assert 
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
		std::string fullTexturePath = "Resources/ParticleTexture/" + texture;
		//CreateParticleGroup を呼んで登録
		CreateParticleGroup(id, fullTexturePath, verticesType, std::move(behavior));
		particleGroups[id].defaultCount = e.value("defaultCount", 100);
		particleGroups[id].defaultLifetime = e.value("defaultLifetime", 1.0f);
		particleGroups[id].isInfinite = e.value("isInfiniteLifetime", false);

		// 色
		if (e.contains("color") && e["color"].is_array() && e["color"].size() == 4) {
			particleGroups[id].defaultColor = {
				e["color"][0].get<float>(),
				e["color"][1].get<float>(),
				e["color"][2].get<float>(),
				e["color"][3].get<float>()
			};
		}



	}


}

void ParticleManager::SaveToJson(const std::string& filepath)
{
	json root;
	root["effects"] = json::array();

	for (const auto& [name, group] : particleGroups) {
		json effectJson;
		effectJson["id"] = name;

		// ビヘイビアの種類を文字列で保存
		std::string behaviorTypeStr = "Unknown";
		if (dynamic_cast<ExplosionBehavior*>(group.behavior.get())) {
			behaviorTypeStr = "Explosion";
		} else if (dynamic_cast<ChargeBehavior*>(group.behavior.get())) {
			behaviorTypeStr = "Charge";
		} else if (dynamic_cast<ExhaustGasBehavior*>(group.behavior.get())) {
			behaviorTypeStr = "ExhaustGas";
		} else if (dynamic_cast<BarrierBreakBehavior*>(group.behavior.get())) {
			behaviorTypeStr = "BarrierBreak";
		} else if (dynamic_cast<BarrierRingBehavior*>(group.behavior.get())) {
			behaviorTypeStr = "BarrierRing";
		}
		effectJson["behaviorType"] = behaviorTypeStr;

		// 頂点形状を文字列で保存
		std::string meshStr = "Quad";
		switch (group.verticesType) {
		case VerticesType::Ring:     meshStr = "Ring"; break;
		case VerticesType::Cylinder: meshStr = "Cylinder"; break;
		case VerticesType::Triangle: meshStr = "Triangle"; break;
		case VerticesType::Quad:     meshStr = "Quad"; break;
		}
		effectJson["mesh"] = meshStr;

		// テクスチャのファイルパスを保存
		effectJson["texture"] = std::filesystem::path(group.materialdata.textureFilePath).filename().string();
		// カウント
		effectJson["defaultCount"] = group.defaultCount;
		//ライフム
		effectJson["defaultLifetime"] = group.defaultLifetime;
		effectJson["isInfiniteLifetime"] = group.isInfinite;// 無限寿命かどうか
		// 色
		effectJson["color"] = {
			group.defaultColor.x,
			group.defaultColor.y,
			group.defaultColor.z,
			group.defaultColor.w
		};
		// ループ設定
		effectJson["isLoop"] = group.isLoop;
		//ループ間隔
		effectJson["loopInterval"] = group.loopInterval;
		// JSONの配列に追加
		root["effects"].push_back(effectJson);

	}
	std::ofstream ofs(filepath);
	ofs << root.dump(4); // インデント幅4で整形して保存

}

void ParticleManager::Finalize()
{

	delete instance_;
	instance_ = nullptr;


}


void ParticleManager::Update()
{
	//カメラからビュープロジェクション行列を取得
	//ビルボード行列の計算
	Matrix4x4 billboardMatrix = backToFrontMatrix * CameraManager::GetInstance()->GetActiveCamera()->GetWorldMatrix();
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	//ビルボード行列を使ってビルボード行列を計算
	Matrix4x4 viewMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetViewMatrix();
	Matrix4x4 projectionMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetProjectionMatrix();



	//全パーティクル	グループ内の全パーティクルについて二重処理する
	for (auto& [name, particleGroup] : particleGroups) {
		auto& behavior = particleGroup.behavior;
		uint32_t counter = 0;
		if (particleGroup.isLoop) {
			particleGroup.loopTimer += 1.0f / 60.0f;

			if (particleGroup.loopTimer >= particleGroup.loopInterval) {


				/*EulerTransform previewTransform = editor_->MakePreviewTransform();
				Emit(name, previewTransform);*/

				particleGroup.loopTimer = 0.0f;
			}
		}

		for (std::list<Particle>::iterator particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end();) {

			if (!(*particleIterator).isInfiniteLifetime) {
				if ((*particleIterator).lifetime <= (*particleIterator).currentTime) {
					particleIterator = particleGroup.particles.erase(particleIterator);
					continue;
				}
			}
			float alpha1 = 0.5;

			behavior->Update((*particleIterator), particleGroup.materialData, alpha1);

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

void ParticleManager::Draw()
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

void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilePath, VerticesType verticesTypeValue, std::unique_ptr<IParticleBehavior> behavior)
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

	particleGroups.at(name).isLoop = false;
	particleGroups.at(name).loopInterval = 1.0f;

	//頂点タイプの設定
	SetGroupVerticesType(name, verticesTypeValue);

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


	uint32_t defaultCount = 100;
	float    defaultLifetime = 1.0f;
	particleGroups.at(name).defaultCount = defaultCount;
	particleGroups.at(name).defaultLifetime = defaultLifetime;

}

void ParticleManager::Emit(const std::string& name, const EulerTransform transform)
{
	assert(particleGroups.contains(name));

	auto& group = particleGroups.at(name);

	for (uint32_t i = 0; i < group.defaultCount; ++i) {
		//Particleを作成
		float lifetime = group.defaultLifetime;
		if (group.isInfinite) {
			lifetime = 999999.0f; 
		}

		Particle particle = group.behavior->Create(
			randomEngine, transform, lifetime);
	
		particle.isInfiniteLifetime = group.isInfinite;
		particle.color = group.defaultColor;
		group.particles.push_back(particle);
	}

}

void ParticleManager::EmitFollowOne(const std::string& name, const EulerTransform& transform)
{


	assert(particleGroups.contains(name));

	auto& group = particleGroups.at(name);

	// まだ無ければ1個だけ作る
	if (group.particles.empty()) {
		float lifetime = group.isInfinite ? 999999.0f : group.defaultLifetime;

		Particle particle = group.behavior->Create(randomEngine, transform, lifetime);
		particle.isInfiniteLifetime = true; // 追従中は消えない
		particle.color = group.defaultColor;

		group.particles.push_back(particle);
	}

	// 既存の1個をプレイヤー位置へ更新
	Particle& p = group.particles.front();
	p.transform.translate = transform.translate;
	p.currentTime = 0.0f;
}

void ParticleManager::StopFollow(const std::string& name)
{
	if (!particleGroups.contains(name)) return;

	auto& group = particleGroups.at(name);
	group.particles.clear();

}



std::vector<VertexData> ParticleManager::MakeRingVertices(uint32_t  RingDivide, float outerRadius, float innerRadius)
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

std::vector<VertexData> ParticleManager::MakeCylinderVertices(uint32_t cylinderDivide, float topRadius, float bottomRadius, float height)
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

std::vector<VertexData> ParticleManager::MakeQuadVertices()
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

std::vector<VertexData> ParticleManager::MakeTriangleVertices()
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

void ParticleManager::SetGroupTexture(const std::string& groupName, const std::string& textureFilePath)
{
	assert(particleGroups.contains(groupName));
	auto& group = particleGroups.at(groupName);

	// パスを登録
	group.materialdata.textureFilePath = textureFilePath;
	//"Resources/ParticleTexture/"

	// テクスチャ読み込み（既に読まれていてもOKな設計ならそのまま）
	TextureManager::GetInstance()->LoadTexture(group.materialdata.textureFilePath);

	// インデックス更新
	group.materialdata.textureIndex =
		TextureManager::GetInstance()->GetTextureIndexByFilePath(group.materialdata.textureFilePath);

}

void ParticleManager::SetGroupVerticesType(const std::string& groupName, VerticesType verticesType)
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

void ParticleManager::SetBehavior(const std::string& groupName, std::unique_ptr<IParticleBehavior> behavior)
{
	//グループが存在するかチェック
	assert(particleGroups.contains(groupName) && "ParticleGroup does not exist!");

	// ムーブ代入でユニークポインタを保持させる
	particleGroups.at(groupName).behavior = std::move(behavior);
}

