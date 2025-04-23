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
//#include <Windows.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <Camera.h>
#include <vector>




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

	void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);

private:
	LineCommon* lineCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	uint32_t instanceSrvIndex_ = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_;
	std::vector<LineInstanceData> instanceDatas_;





};

