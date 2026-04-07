#pragma once
#include <vector>
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <random>
#include "Model.h"
#include "GraphicsPipeline.h"
#include "IParticleBehavior.h"
#include <memory>




enum class VerticesType
{
	Ring,
	Cylinder,
	Quad,
	Triangle,

};


struct Particle {

	EulerTransform transform;
	Vector3 Velocity;
	float lifetime;
	float currentTime;
	Vector3 center; // ← 発生中心を記録

	Vector4 color = { 0,0,0 };


};

struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;

};

class ParticleEditor;
class ParticleManager
{
	friend class ParticleEditor;

	struct ParticleGroup
	{
		MaterialData materialdata;
		//particleのリスト
		std::list<Particle> particles;
		std::unique_ptr<IParticleBehavior> behavior;

		//insutansing用のsrvインデックス
		uint32_t srvIndex;
		//insutansing用のリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource;
		//insutance数
		uint32_t instanceCount;
		//insutanceのデータ
		ParticleForGPU* instanceData = nullptr;
		//頂点
		uint32_t vertexCount = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
		//VBV
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
		//マテリアルにデータを書き込む	
		Material* materialData = nullptr;
		//std::string textureFilePath_;
		uint32_t defaultCount;
		float defaultLifetime;
		VerticesType verticesType = VerticesType::Quad;//頂点の種類

		Vector4 defaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		bool isLoop = false;
		float loopInterval = 1.0f;
		float loopTimer = 0.0f;
		

	};
public:

	static ParticleManager* GetInstance();



private:
	// コンストラクタをプライベートにする
	ParticleManager() = default;
	~ParticleManager() = default;
	// コピーコンストラクタと代入演算子を削除する
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

public:

	void ImguiDrawEditor();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxcommn"></param>
	/// <param name="srvmaneger"></param>
	void Initialize(DirectXCommon* dxcommn, SrvManager* srvmaneger);
	/// <summary>
	/// JSONファイルからパーティクル設定を読み込み
	/// </summary>
	/// <param name="filepath"></param>
	void LoadFromJson(const std::string& filepath);
	/// <summary>
	/// JSONファイルにパーティクル設定を保存
	/// </summary>
	/// <param name="filepath"></param>
	void SaveToJson(const std::string& filepath);
	/// <summary>
	/// 終了処理
	///< / summary>
	void Finalize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	/// <summary>
	/// パーティクルグループの作成
	/// </summary>
	/// <param name="name"></param>
	/// <param name="textureFilePath"></param>
	/// <param name="verticesType"></param>
	/// <param name="behavior"></param>
	void CreateParticleGroup(const std::string name, const std::string textureFilePath, VerticesType verticesType = VerticesType::Quad, std::unique_ptr<IParticleBehavior> behavior = nullptr);
	/// <summary>
	/// パーティクルの放出
	/// </summary>
	/// <param name="name"></param>
	/// <param name="transform"></param>
	/// <param name="count"></param>
	/// <param name="lifetime"></param>
	void Emit(const std::string& name, const EulerTransform transform);
	//void Emit(const std::string& name, const EulerTransform transform, uint32_t count, float lifetime);
	void EmitAtCamera(const std::string& name);

	//リングの頂点情報を作成
	std::vector<VertexData> MakeRingVertices(uint32_t RingDivide = 32, float outerRadius = 1.0f, float innerRadius = 0.2f);
	//シリンダーの頂点情報を作成
	std::vector<VertexData> MakeCylinderVertices(uint32_t cylinderDivide = 32, float topRadius = 1.0f, float bottomRadius = 1.0f, float height = 2.0f);
	//クワッドの頂点情報を作成
	std::vector<VertexData> MakeQuadVertices();
	//三角形
	std::vector<VertexData> MakeTriangleVertices();

	//テクスチャ設定
	void SetGroupTexture(const std::string& groupName, const std::string& textureFilePath);
	//頂点タイプ設定
	void SetGroupVerticesType(const std::string& groupName, VerticesType verticesType);

	
	/// <summary>
	// Behavior設定（明示的に設定する用）
	/// </summary>
	/// <param name="groupName"></param>
	/// <param name="behavior"></param>
	void SetBehavior(const std::string& groupName, std::unique_ptr<IParticleBehavior> behavior);
	//最大インスタンス数
	static constexpr uint32_t kMaxInstanceCount = 1000;

private:


	//インスタンス
	static ParticleManager* instance_;
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;

	Model* model_ = nullptr;
	//乱数生成エンジン
	std::mt19937 randomEngine;

	//SRT
	EulerTransform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;

	std::unordered_map<std::string, ParticleGroup> particleGroups;

	//ビルボード行列
	Matrix4x4 backToFrontMatrix;

	float scrollX = 0.0f; // グローバル or メンバ変数として定義しておく

	// 頂点の種類
	VerticesType verticesType = VerticesType::Quad;



	

#ifdef USE_IMGUI
	
	std::unique_ptr<ParticleEditor> editor_;
#endif

};




