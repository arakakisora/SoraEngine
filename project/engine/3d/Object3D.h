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
	
	//ディレクションライト
	void SetDirectionalLight(const DirectionalLight& directionalLight) { *directionalLightData = directionalLight; }
	DirectionalLight GetDirectionalLight() { return *directionalLightData; }
	//ディレクションライトの向き
	void SetDirectionalLightDirection(const Vector3& direction) { directionalLightData->direction = direction; }
	//ディレクションライトの色
	void SetDirectionalLightColor(const Vector4& color) { directionalLightData->color = color; }
	//ディレクションライトの強さ
	void SetDirectionalLightIntensity(float intensity) { directionalLightData->intensity = intensity; }
	//ライトオンオフ
	void SetDirectionalLightEnable(bool enable) { directionalLightData->enable = enable; }

	//ポイントライト
	void SetPointLight(const PointLight& pointLight) { *pointLightData = pointLight; }
	PointLight GetPointLight() { return *pointLightData; }
	//ポイントライトの位置
	void SetPointLightPosition(const Vector3& position) { pointLightData->position = position; }
	//ポイントライトの色
	void SetPointLightColor(const Vector4& color) { pointLightData->color = color; }
	//ポイントライトの強さ
	void SetPointLightIntensity(float intensity) { pointLightData->intensity = intensity; }
	//ポイントライトの半径
	void SetPointLightRadius(float radius) { pointLightData->radius = radius; }
	float GetPointLightRadius() { return pointLightData->radius; }
	//ポイントライトの減衰率
	void SetPointLightDecay(float decay) { pointLightData->decay = decay; }
	float GetPointLightDecay() { return pointLightData->decay; }
	//ポイントライトのオンオフ
	void SetPointLightEnable(bool enable) { pointLightData->enable = enable; }


	//ライトのオンオフ
	void SetLighting(bool enable) { enableLighting = enable; }
	
	

	void SetColor(const Vector4& color) { color_ = color; }
	Vector4 GetColor() const { return color_; }
	

	

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

	//ポイントライト
	//ポイントライト用のリソースを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource;
	PointLight* pointLightData = nullptr;

	//SRT
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;


	//ライトのオンオフ
	bool enableLighting = true;

	Camera* camera = nullptr;
	//カメラforGPU
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;//カメラのデータを送るためのリソース
	CaMeraForGpu* cameraForGpu = nullptr;//カメラのデータをGPUに送るための構造体

private:
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルトは白




};

