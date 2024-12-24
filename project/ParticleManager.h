#pragma once

#include "DirectXCommon.h"
#include "SrvManager.h"
#include <random>
#include "Model.h"
#include <ModelManager.h>
#include "Object3D.h"
#include "GraphicsPipeline.h"

//パーティクル
struct Particle {

	Transform transform;
	Vector3 Velocity;
	Vector4 color;
	float lifetime;
	float currentTime;

};
//パーティクルGPU
struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;

};

//使用するテクスチャごとに、パーティクルグループにしてまとめる
//DroawCallは1ぐループごとに発行する
struct ParticleGroup {
	//マテリアルデータ（テクスチャファイルパスとテクスチャ用SRVインデックス）
	MaterialData material;
	//パーティクルのリスト
	std::list<Particle> particles;
	//インスタンシングデータ用SRVインデックス	
	uint32_t instanceIndex;
	//インスタンシングリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource;
	//インスタンス数
	uint32_t instanceCount;
	//インスタンスデータを書き込むためのポインタ
	ParticleForGPU* instanceData = nullptr;
	
	

};
//加速度フィールド
struct AccelerationField {

	Vector3 acceleration;//加速度	
	AABB area;//範囲

};

class ParticleManager
{
	// シングルトンクラス
	static ParticleManager* instance;
	ParticleManager() = default;
	~ParticleManager() = default;
	ParticleManager(ParticleManager&) = default;
	ParticleManager& operator=(ParticleManager&) = delete;

public:
	// シングルトンインスタンス
	static ParticleManager* GetInstance();
	// 終了
	void Finalize();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon*dxcmmon,SrvManager*srvmnager, const std::string& filepath);
	void CreateParticleGroup(const std::string& groupName, const std::string& textureFilePath);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(Camera* camera);
	
	void SetModel(const std::string& filepath);
	
private:
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	//乱数エンジン
	std::mt19937 randomEngine;
	Model* model_ = nullptr;
	//vertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//グラフィックスパイプライン
	GraphicsPipeline* graphicsPipeline = nullptr;
	//パーティクルグループ
	std::unordered_map<std::string, ParticleGroup> particleGroups;
	//ビルボード行列
	Matrix4x4 backToFrontMatrix;

	//加速度フィールド
	AccelerationField accelerationFienld;

	//デルタタイム
	const float kDeletaTime = 1.0f / 60.f;

};

