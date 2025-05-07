#pragma once
#include <vector>
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <random>
#include "Model.h"
#include "GraphicsPipeline.h"

struct Particle {

	Transform transform;
	Vector3 Velocity;
	float lifetime;
	float currentTime;

	Vector4 color;


	


};

struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;

};

class ParticleMnager
{
	struct ParticleGroup
	{
		MaterialData materialdata;
		//particleのリスト
		std::list<Particle> particles;
		//insutansing用のsrvインデックス
		uint32_t srvIndex;
		//insutansing用のリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource;
		//insutance数
		uint32_t instanceCount;
		//insutanceのデータ
		ParticleForGPU* instanceData = nullptr;

	};
public:

	static ParticleMnager* GetInstance();
	
private:
	// コンストラクタをプライベートにする
	ParticleMnager() = default;
	~ParticleMnager() = default;
	// コピーコンストラクタと代入演算子を削除する
	ParticleMnager(const ParticleMnager&) = delete;
	ParticleMnager& operator=(const ParticleMnager&) = delete;


public:

	//初期化
	void Initialize(DirectXCommon* dxcommn,SrvManager*srvmaneger);



	/// <summary>
	/// 終了処理
	///< / summary>
	void Finalize();
	void Update();
	void Draw();

	void CreateParticleGroup(const std::string name,const std::string textureFilePath, std::string modelFilePath);

	void Emit(const std::string& name, const Vector3 position, uint32_t count);

	void SetModel(const std::string& filepath);

	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);
	

private:


	//インスタンス
	static ParticleMnager* instance_;
	DirectXCommon* dxCommon_=nullptr;
	SrvManager* srvManager_ = nullptr;



	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;

	Model* model_ = nullptr;

	std::mt19937 randomEngine;


	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	//VBV
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	
	//SRT
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;

	std::unordered_map<std::string, ParticleGroup> particleGroups;

	//ビルボード行列
	Matrix4x4 backToFrontMatrix;

	//modelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//マテリアルにデータを書き込む	
	Material* materialData = nullptr;
	//std::string textureFilePath_;
	
};


