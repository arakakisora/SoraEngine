#pragma once
#include "Vector4.h"
#include "Matrix4x4.h"
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;


struct VertexDataLine 
{
	Vector4 position;

};

struct LineInstanceData {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

class LineCommon;
class Line
{
public:
	//初期化
	void Initialize(LineCommon* lineCommon);
	//更新
	void Update();
	//描画
	void Draw();
	
private:
	LineCommon* lineCommon_ = nullptr;

	ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;





};

