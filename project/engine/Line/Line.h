#pragma once

#include "MyMath.h"
#include <Object3D.h>




class LineCommon;
class Line
{
public:


	//ラインを描画する
	void Draw(const Vector3& start, const Vector3& end, const Vector4& color);

	void DrawAABB(const Vector3& min, const Vector3& max, const Vector4& color);

	void DrawAABBVector3( Vector3 center, float radius, Vector4 color);
	

	void DrawGrid(Vector3 center,float Gridhalfwidth = 2.0, uint32_t Subdivision = 50);

	void DrawSphere(const Vector3& center, float radius, const Vector4& color);


};

