#include "Matrix4x4.h"
#include "Vector3.h"
#pragma once
#include <assert.h>

#include <cmath>
#include <stdio.h>
#include "4x4Matrixcalculation.h"

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
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