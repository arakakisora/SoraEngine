#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

#include <assert.h>
#include <cmath>
#include <stdio.h>
//#include <Windows.h>
#include <wrl/client.h>
#include <d3d12.h>



class SpriteCommon;
class Sprite
{

	struct VertexData {

		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;

	};

	struct Material {

		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	struct TransformationMatrix
	{
		Matrix4x4 WVP;
		Matrix4x4 World;

	};

	struct Transform {
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon,std::string textureFilePath);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	//Getter
	const Vector2& GetSize()const { return size; }
	const Vector2& GetPosition()const { return position; }
	const float& GetRotation()const { return rotation; }
	const Vector4& GetColor()const { return materialData->color; }
	//Setter
	void SetSize(const Vector2& size) { this->size= size; }
	void SetPosition(const Vector2& position) { this->position = position; }
	void SetRotation(const float& rotation) { this->rotation = rotation; }
	void setColor(const Vector4& color) { materialData->color=color; }

private:
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
	Material* materialData = nullptr;
	TransformationMatrix* transformaitionMatrixData = nullptr;

	////vetexResourceSprite頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//IndexBufferSprite頂点バッファーを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	//Transform
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	//設定用SRT
	Vector2 size = { 640.0f,360.0f };
	Vector2 position = { 0.0f,0.0f };
	float rotation = 0.0f;

	uint32_t textureIndex = 0;

};

