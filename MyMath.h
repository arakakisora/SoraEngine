#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"
#pragma once
#include <assert.h>
#include <cmath>
#include <stdio.h>
#include "Vector4.h"
#include "Vector2.h"

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct VertexData {

	Vector4 position;
	Vector2 texcoord;


};

//回転
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
//拡大
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
//同時座標変換
Vector3 TransformVector3(const Vector3& vector, const Matrix4x4& matrix);

//回転X
Matrix4x4 MakeRotateXMatrix(float radian);
//回転Y
Matrix4x4 MakeRotateYMatrix(float radian);
//回転Z
Matrix4x4 MakeRotateZMatrix(float radian);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);



//スクリーンプリントf

//足し算
Matrix4x4 Add(const Matrix4x4& mt1, const Matrix4x4& mt2);
//引き算
Matrix4x4 Subtract(const Matrix4x4& mt1, const Matrix4x4& mt2);

//掛け算
Matrix4x4 Multiply(const Matrix4x4& mt1, const Matrix4x4& mt2);

//逆行列
Matrix4x4 Inverse(const Matrix4x4& m);



Matrix4x4 Transpose(const Matrix4x4& mt1);
//0行列
Matrix4x4 MekeIdentity4x4();

//クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2);

Matrix4x4 MakeIdentity4x4();



//void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
//	const float kGridHalfWidth = 2.0f;
//	const uint32_t kSubdivision = 10;
//	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);
//	//奥から手前への線を順番に引いていく
//	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
//		float x = -kGridHalfWidth + (xIndex * kGridEvery);
//		unsigned int color = 0xAAAAAAFF;
//		Vector3 start{ x,0.0f,-kGridHalfWidth };
//		Vector3 end{ x,0.0f,kGridHalfWidth };
//
//		Vector3 startScreen = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
//		Vector3 endScrenn = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);
//
//		if (x == 0.0f) {
//			color = BLACK;
//		}
//		Novice::DrawLine(int(startScreen.x),int(startScreen.y),int(endScrenn.x))
//
//	}
//
//}
