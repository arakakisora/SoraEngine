#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "RenderingData.h"

#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <string>

#include <wrl/client.h>
#include <d3d12.h>
#include <Camera.h>



class SpriteCommon;
/// <summary>
/// スプライト
/// </summary>
class Sprite
{

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	//大きさ
	/// </summary>
	/// <returns></returns>
	const Vector2& GetSize()const { return size; }
	/// <summary>
	/// 大きさセット
	/// </summary>
	/// <param name="sizeValue"></param>
	void SetSize(const Vector2& sizeValue) { this->size = sizeValue; }
	///<summary>
	//位置
	///</summary>
	///<returns></returns>
	const Vector2& GetPosition()const { return position; }
	/// <summary>
	/// 位置セット
	/// </summary>
	/// <param name="positionValue"></param>
	void SetPosition(const Vector2& positionValue) { this->position = positionValue; }
	///<summary>
	//回転
	///</summary>
	///<returns></returns>
	const float& GetRotation()const { return rotation; }
	/// <summary>
	/// 回転セット
	/// </summary>
	/// <param name="rotationValue"></param>
	void SetRotation(const float& rotationValue) { this->rotation = rotationValue; }
	///<summary>
	//色
	///</summary>
	///<returns></returns>
	const Vector4& GetColor()const { return materialData->color; }
	/// <summary>
	/// 色セット
	/// </summary>
	/// <param name="colorValue"></param>
	void setColor(const Vector4& colorValue) { materialData->color = colorValue; }
	/// <summary>
	/// アンカーポイント取得
	/// </summary>
	/// <returns></returns>
	const Vector2& GetAnchorPoint()const { return anchorPoint_; }
	/// <summary>
	/// アンカーポイントセット
	/// </summary>
	/// <param name="anchorPoint"></param>
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }
	/// <summary>
	//左右フリップ
	/// </summary>
	/// <returns></returns>
	const bool& GetIsFlipX()const { return isFlipX_; }
	/// <summary>
	/// 左右フリップセット
	/// </summary>
	/// <param name="isFlipX"></param>
	void SetIsFlipX(const bool& isFlipX) { isFlipX_ = isFlipX; }
	/// <summary>
	//上下フリップ
	/// </summary>
	const bool& GetIsFlipY()const { return isFlipY_; }
	/// <summary>
	/// 上下フリップセット
	/// </summary>
	void SetIsFlipY(const bool& isFlipY) { isFlipY_ = isFlipY; }
	/// <summary>
	//テクスチャ左上
	/// </summary>
	const Vector2& GetTextureLeftTop()const { return textureLeftTop_; }
	/// <summary>
	/// テクスチャ左上セット
	/// </summary>
	/// <param name="textureLeftTop"></param>
	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }
	/// <summary>
	//テクスチャサイズ
	/// </summary>
	const Vector2& GetTextureSize()const { return textureSize_; }
	/// <summary>
	/// テクスチャサイズセット
	/// </summary>
	/// <param name="textureSize"></param>
	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }

private:

	std::string textureFilePath_;

	//テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

	SpriteCommon* spriteCommon_ = nullptr;

	//バッファリソース
	//Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vetexResource;
	//Sprite用のindexResourceを作成						
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	//Sprite用のマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//transformation用のリソースを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;

	//バッファー内のデータを示すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	MaterialSprite* materialData = nullptr;
	TransformationMatrixsprite* transformaitionMatrixData = nullptr;

	////vetexResourceSprite頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//IndexBufferSprite頂点バッファーを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	//Transform
	EulerTransform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	//設定用SRT
	Vector2 size = { 640.0f,360.0f };
	Vector2 position = { 0.0f,0.0f };
	float rotation = 0.0f;

	uint32_t textureIndex = 0;

	//アンカーポイント 中心位置を変えれる
	Vector2 anchorPoint_ = { 0.0f,0.0f };

	//フリップ
	bool isFlipX_ = false;//左右フリップ
	bool isFlipY_ = false;//上下フリップ

	//テクスチャ左上座標
	Vector2 textureLeftTop_ = { 0.0f,0.0f };
	//テクスチャ切り出しサイズ
	Vector2 textureSize_ = { 512.0f,512.0f };


	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 worldViewProjectionMatrix;


	Camera* camera = nullptr;
	//カメラforGPU
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;//カメラのデータを送るためのリソース
	CaMeraForGpu* cameraForGpu = nullptr;//カメラのデータをGPUに送るための構造体



};

