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
	void Initialize(SpriteCommon* spriteCommon);
	void Update();
	void Draw();

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


	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };


};

