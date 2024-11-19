#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "RenderingData.h"
#include "Model.h"

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
	

	void Setmodel(Model* model) { model_ = model; }

private:
	Object3DCommon* object3DCommon_ = nullptr;

	Model* model_ = nullptr;

	

	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	//データを書き込む
	TransformationMatrix* transformaitionMatrixData=nullptr;

	//平行光源
	//平行光源用のResoureceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	DirectionalLight* directionalLightData = nullptr;

	//SRT
	Transform transform;
	//カメラ用のTransformを作る
	Transform cameraTransform;

};

