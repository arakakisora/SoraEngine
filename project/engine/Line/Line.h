#pragma once

#include "MyMath.h"




class LineCommon;
class Line
{
public:


	//ラインを描画する
	void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);

	void DrawLienAABB(const Vector3& min, const Vector3& max, const Vector4& color);

	void DrawGrid(float Gridhalfwidth = 2.0, uint32_t Subdivision = 50);

	void DrawSphere(const Vector3& center, float radius, const Vector4& color);


};

