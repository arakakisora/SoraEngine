#pragma once
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Vector2.h"
#include "Quaternion.h"
#pragma once
#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
//球
struct Sphere {
	Vector3 center;
	float radius;
};
//光線
struct Ray {

	Vector3 origin;
	Vector3 diff;
};
//線分
struct Segment {

	Vector3 origin;
	Vector3 diff;
};
//平面
struct Plane {

	Vector3 normal;
	float distance;

};
//三角形
struct Triangle {

	Vector3 vertices[3];
};
//軸平行境界ボックス
struct AABB {

	Vector3 min;
	Vector3 max;
};


/// <summary>
/// 独自数学ライブラリ
/// </summary>
namespace MyMath {
	/// <summary>
	/// 平行移動
	/// </summary>
	/// <param name="translate"></param>
	/// <returns></returns>
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
	/// <summary>
	/// 拡縮
	/// </summary>
	/// <param name="scale"></param>
	/// <returns></returns>
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);
	/// <summary>
	/// ベクトルの変換
	/// </summary>
	/// <param name="vector"></param>
	/// <param name="matrix"></param>
	/// <returns></returns>
	Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
	/// <summary>
	/// ベクトルの正規化
	/// </summary>
	/// <param name="vector"></param>
	/// <returns></returns>
	Vector3 Normalize(const Vector3& vector);
	/// <summary>
	/// ベクトルの加算
	/// </summary>
	/// <param name="v"></param>
	/// <param name="scalar"></param>
	/// <returns></returns>
	Vector3 Add(const Vector3& v, float scalar);
	/// <summary>
	/// ベクトルの加算
	/// </summary>
	/// <param name="v1"></param>
	/// <param name="v2"></param>
	/// <returns></returns>
	Vector3 Add(const Vector3& v1, const Vector3& v2);
	/// <summary>
	/// ベクトルの外積
	/// </summary>
	/// <param name="v1"></param>
	/// <param name="v2"></param>
	/// <returns></returns>
	Vector3 Cross(const Vector3& v1, const Vector3& v2);
	/// <summary>
	/// 回転X
	/// </summary>
	/// <param name="radian"></param>
	/// <returns></returns>
	Matrix4x4 MakeRotateXMatrix(float radian);
	/// <summary>
	/// 回転Y
	/// </summary>
	/// <param name="radian"></param>
	/// <returns></returns>
	Matrix4x4 MakeRotateYMatrix(float radian);
	/// <summary>
	/// 回転Z
	/// </summary>
	/// <param name="radian"></param>
	/// <returns></returns>
	Matrix4x4 MakeRotateZMatrix(float radian);
	/// <summary>
	//回転XYZ
	/// </summary>
	/// <param name="rotate"></param>
	Matrix4x4 MakeRotateMatrix(const Vector3& rotate);
	/// <summary>
	/// アフィン変換行列の作成
	/// </summary>
	/// <param name="scale"></param>
	/// <param name="rotate"></param>
	/// <param name="translate"></param>
	/// <returns></returns>
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

	/// <summary>
	/// コサイン
	/// </summary>
	/// <param name="theta"></param>
	/// <returns></returns>
	float Cot(float theta);
	/// <summary>
	//ドット積
	/// </summary>
	float Dot(const Vector3& v1, const Vector3& v2);
	float Dot(const Vector3& v1, const float& num);
	float Dot(const Quaternion& q1, const Quaternion& q2);
	float Length(const Vector3& v);


	//Lerp
	Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);
	//Slerp
	Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t);
	float fLerp(float v1, float v2, float t);

	//透視投影行列
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearCilp, float farClip);

	//正射影行列
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottm, float nearCip, float farCip);
	//ビューポート変換行列
	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
	//単位行列
	Matrix4x4 MakeIdentity4x4();
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	Matrix4x4 Transpose(const Matrix4x4& matrix);


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

	//ベクトルを求める関数
	Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle);
	//ある方向をある方向に向ける回転行列
	Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to);

	//クオタニオン
	//クオタニオンを行列に変換
	Matrix4x4 MakeRotationMatrix(const Quaternion& Quaternion);
	//クオタニオンを回転行列に変換
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Quaternion& Quaternion, const Vector3& translate);
	//球面線形補間
	Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);

	//クオタニオンの正規化
	Quaternion NormalizeQuaternion(const Quaternion& quaternion);

	Vector3 GetTranslate( const Matrix4x4& matrix);

	//debugテキスト
	void MatrixImGuiText(const Matrix4x4& matrix, const char* label);
	void QuaternionImGuiText(const Quaternion& quaternion, const char* label);
	void Vector3ImGuiText(const Vector3& vector, const char* label);

	

	Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

	// SmoothStep関数の追加
	inline float SmoothStep(float edge0, float edge1, float x) {
        // Clamp x to [0, 1]
        x = (x - edge0) / (edge1 - edge0);
        x = x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
        return x * x * (3.0f - 2.0f * x);
    }

	static bool IsNearlyZero2D(const Vector3& v, float eps = 1e-4f) {
		return (std::abs(v.x) < eps) && (std::abs(v.y) < eps);
	}
}



























