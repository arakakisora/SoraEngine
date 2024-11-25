#pragma once
#include "RenderingData.h"
class Camera
{
public:

/// <summary>
/// 更新
/// </summary>
	void Update();

private:
	//カメラ用のTransformを作る
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;

	

};

