#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "MyMath.h"
#include "RenderingData.h"
#include "Model.h"
#include "Camera.h"
#include "Line.h"

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

	void SkeletonUpdate( Skeleton& skeleton);
	void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();



	void SetModel(Model* model) { model_ = model; }
	void SetModel(const std::string& filepath);

	//transrat
	void SetTransform(const EulerTransform& transform) { this->transform = transform; }
	EulerTransform GetTransform() { return transform; }

	

	//スケール
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	//回転
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	//位置
	void SetTranslate(const Vector3& transrate) { transform.translate = transrate; }
	//カメラ
	//void SetCamera(Camera* camera) { this->camera = camera; }
	////デフォルトカメラ

	

	void SetColor(const Vector4& color) { color_ = color; }
	Vector4 GetColor() const { return color_; }

	//アニメーション
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);
	



private:
	Object3DCommon* object3DCommon_ = nullptr;//Object3DCommonのポインタ

	Model* model_ = nullptr;//モデルのポインタ

	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	//データを書き込む

	TransformationMatrix* transformaitionMatrixData = nullptr;


	

	//SRT
	EulerTransform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;

	//カメラ
	Camera* camera = nullptr;
	//カメラforGPU
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;//カメラのデータを送るためのリソース
	CaMeraForGpu* cameraForGpu = nullptr;//カメラのデータをGPUに送るための構造体
	//アニメーション
	float animationTime = 0.0f;


private:
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルトは白
	Line line_; // Lineクラスのポインタ
	std::vector<Matrix4x4> skeletonPose_;



};

