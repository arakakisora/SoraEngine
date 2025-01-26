#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "RenderingData.h"
#include "Model.h"
#include "Camera.h"

class Object3DCommon;
class Object3D
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3DCommon* object3DCommon);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();


	void SetModel(Model* model) { model_ = model; }
	void SetModel(const std::string& filepath);

	//transrat
	void SetTransform(const Transform& transform) { this->transform = transform; }
	Transform GetTransform() { return transform; }

	//スケール
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	//回転
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	//位置
	void SetTranslate(const Vector3& transrate) { transform.translate = transrate; }
	//カメラ
	//void SetCamera(Camera* camera) { this->camera = camera; }
	////デフォルトカメラ
	
	//ライト
	void SetDirectionalLight(const DirectionalLight& directionalLight) { *directionalLightData = directionalLight; }
	DirectionalLight GetDirectionalLight() { return *directionalLightData; }
	//ライトの向き
	void SetDirectionalLightDirection(const Vector3& direction) { directionalLightData->direction = direction; }
	//ライトの色
	void SetDirectionalLightColor(const Vector4& color) { directionalLightData->color = color; }
	//ライトの強さ
	void SetDirectionalLightIntensity(float intensity) { directionalLightData->intensity = intensity; }

	//ライトのオンオフ
	void SetLighting(bool enable) { enableLighting = enable; }
	


	

	

private:
	Object3DCommon* object3DCommon_ = nullptr;

	Model* model_ = nullptr;


	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	//データを書き込む

	TransformationMatrix* transformaitionMatrixData = nullptr;


	//平行光源
	//平行光源用のResoureceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	DirectionalLight* directionalLightData = nullptr;

	//SRT
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;

	Camera* camera = nullptr;

	//ライトのオンオフ
	bool enableLighting = true;

	//カメラforGPU
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;//カメラのデータを送るためのリソース
	CaMeraForGpu* cameraForGpu = nullptr;//カメラのデータをGPUに送るための構造体



};

