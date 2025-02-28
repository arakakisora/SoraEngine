#pragma once
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Vector2.h"
#pragma once
#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
struct Sphere {
	Vector3 center;
	float radius;
};

struct Line {
	Vector3 origin;
	Vector3 diff;
};

struct Ray {

	Vector3 origin;
	Vector3 diff;
};

struct Segment {

	Vector3 origin;
	Vector3 diff;
};

struct Plane {

	Vector3 normal;
	float distance;

};

struct Triangle {

	Vector3 vertices[3];
};

struct AABB {

	Vector3 min;
	Vector3 max;
};

struct Quaternion {
	float w, x, y, z;
};

namespace MyMath {
	//回転
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
	//拡大
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);
	//同時座標変換
	Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
	Vector3 Normlize(const Vector3& vector);

	//Add
	Vector3 Add(const Vector3& v, float scalar);
	Vector3 Add(const Vector3& v1, const Vector3& v2);

	//回転X
	Matrix4x4 MakeRotateXMatrix(float radian);
	//回転Y
	Matrix4x4 MakeRotateYMatrix(float radian);
	//回転Z
	Matrix4x4 MakeRotateZMatrix(float radian);

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

	float Cot(float theta);
	//ドット積
	float Dot(const Vector3& v1, const Vector3& v2);
	float Dot(const Vector3& v1, const float& num);
	float Length(const Vector3& v);


	//透視投影行列
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearCilp, float farClip);

	//正射影行列
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottm, float nearCip, float farCip);
	//ビューポート変換行列
	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
	//単位行列
	Matrix4x4 MakeIdentity4x4();
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	//最近接点
	Vector3 Project(const Vector3& v1, const Vector3& v2);
	Vector3 ClosestPoint(const Vector3& point, const Segment& segment);
	//当たり判定
	bool IsCollision(const Sphere& s1, const Sphere& s2);
	bool IsCollision(const Sphere& s1, const Plane& plane);
	bool IsCollision(const Segment& segment, const Plane& plane);
	bool IsCollision(const Segment& segment, const Triangle& triangle);
	bool IsCollision(const AABB& aabb1, const AABB& aabb2);
	bool IsCollision(const AABB& aabb, const Sphere& sphere);
	bool IsCollision(const AABB& aabb, const Segment& segment);

}














