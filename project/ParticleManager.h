#pragma once
#include <DirectXCommon.h>
#include "SrvManager.h"
#include <random>
#include "GraphicsPipeline.h"
#include <RenderingData.h>
#include "Camera.h"
#include"Model.h"

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
//エミッタ
struct Emitter {
	Transform transform;//エミッタのtransform
	uint32_t count;//発生数
	float frequency;//発生頻度
	float frequencyTime;//頻度用時刻	
};
//加速度フィールド
struct AccelerationField {

	Vector3 acceleration;//加速度	
	AABB area;//範囲

};


struct ParticleGroup {

	MaterialData* materialData;
	std::list<Particle> particles;
	uint32_t srvIndex;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResouce;
	uint32_t kNumMaxInstance;
	ParticleForGPU* instancingData;

};





class ParticleManager
{
	//シングルトン
	static ParticleManager* instance;
	ParticleManager() = default;//コンストラクタ
	~ParticleManager() = default;//デストラクタ
	ParticleManager(ParticleManager&) = default;//コピーコンストラクタ
	ParticleManager& operator=(ParticleManager&) = delete;//代入演算子

public:
	//シングルトンインスタンスの取得
	static ParticleManager* GetInstans();
	//終了
	void Finalize();

	/// <summary>
    /// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxcommon, SrvManager* srvmnager,Camera*camera);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	///	</summary>
	void Draw(const std::string& name);

	//パーティクルの作成
	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath, uint32_t maxInstance, const std::string& modelfilepath);

	void Emit(const std::string& name, const Vector3&position,uint32_t count);


private:

	//DirectXCommon
	DirectXCommon* dxcommon_ = nullptr;
	//SRVマネージャ
	SrvManager* srvmnager_ = nullptr;
	//カメラ
	Camera* camera_ = nullptr;
	//乱数エンジン
	std::mt19937 randomEngine;
	//グラフィックスパイプライン
	GraphicsPipeline* graphicsPipeline = nullptr;

	//vertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{ };
	//パーティクルグループ
	std::unordered_map<std::string, ParticleGroup> particleGroups;
	//ビルボード行列
	Matrix4x4 backToFrontMatrix;
	//加速度フィールド
	AccelerationField accelerationFienld;
	//デルタタイム
	const float kDeletaTime = 1.0f / 60.f;
	
	Model* model_ = nullptr;

	void SetModel(const std::string& filepath);

};

