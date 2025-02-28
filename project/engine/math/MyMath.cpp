#define NOMINMAX 
#include "MyMath.h"
#include <iostream>
#include <iomanip>
#include <algorithm>




Matrix4x4 MyMath::MakeTranslateMatrix(const Vector3& translate)

{
	Matrix4x4 ans;

	ans.m[0][0] = 1;
	ans.m[0][1] = 0;
	ans.m[0][2] = 0;
	ans.m[0][3] = 0;

	ans.m[1][0] = 0;
	ans.m[1][1] = 1;
	ans.m[1][2] = 0;
	ans.m[1][3] = 0;

	ans.m[2][0] = 0;
	ans.m[2][1] = 0;
	ans.m[2][2] = 1;
	ans.m[2][3] = 0;

	ans.m[3][0] = translate.x;
	ans.m[3][1] = translate.y;
	ans.m[3][2] = translate.z;
	ans.m[3][3] = 1;

	return ans;

}


Matrix4x4 MyMath::MakeScaleMatrix(const Vector3& scale)

{
	Matrix4x4 ans;

	ans.m[0][0] = scale.x;
	ans.m[0][1] = 0;
	ans.m[0][2] = 0;
	ans.m[0][3] = 0;

	ans.m[1][0] = 0;
	ans.m[1][1] = scale.y;
	ans.m[1][2] = 0;
	ans.m[1][3] = 0;

	ans.m[2][0] = 0;
	ans.m[2][1] = 0;
	ans.m[2][2] = scale.z;
	ans.m[2][3] = 0;

	ans.m[3][0] = 0;
	ans.m[3][1] = 0;
	ans.m[3][2] = 0;
	ans.m[3][3] = 1;
	return ans;

}


Vector3 MyMath::Transform(const Vector3& vector, const Matrix4x4& matrix)

{

	Vector3 ans;

	ans.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	ans.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	ans.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];

	assert(w != 0.0f);
	ans.x /= w;
	ans.y /= w;
	ans.z /= w;
	return ans;

}

Vector3 MyMath::Normlize(const Vector3& vector)
{

	float length = std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	return Vector3(vector.x / length, vector.y / length, vector.z / length);
}

//Add
Vector3 MyMath::Add(const Vector3& v, float scalar) {
	return Vector3(v.x + scalar, v.y + scalar, v.z + scalar);
}
Vector3 MyMath::Add(const Vector3& v1, const Vector3& v2) {
	return { v1.x + v2.x,v1.y + v2.y,v1.z + v2.z };
}

Matrix4x4 MyMath::MakeRotateXMatrix(float radian)

{

	Matrix4x4 ans;
	ans.m[0][0] = 1;
	ans.m[0][1] = 0;
	ans.m[0][2] = 0;
	ans.m[0][3] = 0;

	ans.m[1][0] = 0;
	ans.m[1][1] = std::cos(radian);;
	ans.m[1][2] = std::sin(radian);;
	ans.m[1][3] = 0;

	ans.m[2][0] = 0;
	ans.m[2][1] = -std::sin(radian);;
	ans.m[2][2] = std::cos(radian);;
	ans.m[2][3] = 0;

	ans.m[3][0] = 0;
	ans.m[3][1] = 0;
	ans.m[3][2] = 0;
	ans.m[3][3] = 1;

	return ans;

}


Matrix4x4 MyMath::MakeRotateYMatrix(float radian)

{

	Matrix4x4 ans;
	ans.m[0][0] = std::cos(radian);
	ans.m[0][1] = 0;
	ans.m[0][2] = -std::sin(radian);
	ans.m[0][3] = 0;

	ans.m[1][0] = 0;
	ans.m[1][1] = 1;
	ans.m[1][2] = 0;
	ans.m[1][3] = 0;

	ans.m[2][0] = std::sin(radian);;
	ans.m[2][1] = 0;
	ans.m[2][2] = std::cos(radian);;
	ans.m[2][3] = 0;

	ans.m[3][0] = 0;
	ans.m[3][1] = 0;
	ans.m[3][2] = 0;
	ans.m[3][3] = 1;

	return ans;

}


Matrix4x4 MyMath::MakeRotateZMatrix(float radian)

{
	Matrix4x4 ans;
	ans.m[0][0] = std::cos(radian);
	ans.m[0][1] = std::sin(radian);
	ans.m[0][2] = 0;
	ans.m[0][3] = 0;

	ans.m[1][0] = -std::sin(radian);
	ans.m[1][1] = std::cos(radian);
	ans.m[1][2] = 0;
	ans.m[1][3] = 0;

	ans.m[2][0] = 0;
	ans.m[2][1] = 0;
	ans.m[2][2] = 1;
	ans.m[2][3] = 0;

	ans.m[3][0] = 0;
	ans.m[3][1] = 0;
	ans.m[3][2] = 0;
	ans.m[3][3] = 1;


	return ans;

}


Matrix4x4 MyMath::MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate)
{

	Matrix4x4 rotateXYZ = MakeRotateXMatrix(rotate.x) * MakeRotateYMatrix(rotate.y) * MakeRotateZMatrix(rotate.z);
	return MakeScaleMatrix(scale) * rotateXYZ * MakeTranslateMatrix(translate);

}

float MyMath::Cot(float theta)
{
	return 1 / std::tan(theta);
}

float MyMath::Dot(const Vector3& v1, const Vector3& v2)
{

	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float MyMath::Dot(const Vector3& v1, const float& num)
{

	return v1.x * num + v1.y * num + v1.z * num;
}

float MyMath::Length(const Vector3& v)
{
	float ans;

	ans = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return ans;
}

Matrix4x4 MyMath::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearCilp, float farClip)
{

	Matrix4x4 ans;

	ans.m[0][0] = Cot(fovY / 2) / aspectRatio;
	ans.m[0][1] = 0;
	ans.m[0][2] = 0;
	ans.m[0][3] = 0;

	ans.m[1][0] = 0;
	ans.m[1][1] = Cot(fovY / 2);
	ans.m[1][2] = 0;
	ans.m[1][3] = 0;

	ans.m[2][0] = 0;
	ans.m[2][1] = 0;
	ans.m[2][2] = nearCilp / (farClip - nearCilp);
	ans.m[2][3] = 1;

	ans.m[3][0] = 0;
	ans.m[3][1] = 0;
	ans.m[3][2] = -(nearCilp + nearCilp) / (farClip - nearCilp);
	ans.m[3][3] = 0;

	return ans;

}
Matrix4x4 MyMath::MakeOrthographicMatrix(float left, float top, float right, float bottm, float nearCip, float farCip)
{

	Matrix4x4 ans;

	ans.m[0][0] = 2 / (right - left);
	ans.m[0][1] = 0;
	ans.m[0][2] = 0;
	ans.m[0][3] = 0;

	ans.m[1][0] = 0;
	ans.m[1][1] = 2 / (top - bottm);
	ans.m[1][2] = 0;
	ans.m[1][3] = 0;

	ans.m[2][0] = 0;
	ans.m[2][1] = 0;
	ans.m[2][2] = 1 / (farCip - nearCip);
	ans.m[2][3] = 0;

	ans.m[3][0] = (left + right) / (left - right);
	ans.m[3][1] = (top + bottm) / (bottm - top);
	ans.m[3][2] = nearCip / (nearCip - farCip);
	ans.m[3][3] = 1;

	return ans;

}

Matrix4x4 MyMath::MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth)
{
	Matrix4x4 ans;



	ans.m[0][0] = width / 2;
	ans.m[0][1] = 0;
	ans.m[0][2] = 0;
	ans.m[0][3] = 0;

	ans.m[1][0] = 0;
	ans.m[1][1] = -(height / 2);
	ans.m[1][2] = 0;
	ans.m[1][3] = 0;

	ans.m[2][0] = 0;
	ans.m[2][1] = 0;
	ans.m[2][2] = maxDepth - minDepth;
	ans.m[2][3] = 0;

	ans.m[3][0] = left + (width / 2);
	ans.m[3][1] = top + (height / 2);
	ans.m[3][2] = minDepth;
	ans.m[3][3] = 1;




	return ans;

}

//単位行列
Matrix4x4 MyMath::MakeIdentity4x4() {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	return result;
}

Matrix4x4 MyMath::Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			for (int i = 0; i < 4; ++i) {
				result.m[row][column] += m1.m[row][i] * m2.m[i][column];
			}
		}
	}
	return result;
}

Vector3 MyMath::Project(const Vector3& v1, const Vector3& v2)
{
	float t = Dot(v1, v2) / Dot(v2, v2);
	Vector3 tb = { v2.x * t,v2.y * t,v2.z * t };
	return tb;
}

Vector3 MyMath::ClosestPoint(const Vector3& point, const Segment& segment)
{
	Vector3 v = point - segment.origin;
	float t = Dot(v, segment.diff) / Dot(segment.diff, segment.diff);
	t = std::clamp(t, 0.0f, 1.0f);

	Vector3 tb = { t * segment.diff.x, t * segment.diff.y,t * segment.diff.z };
	return Add(segment.origin, tb);
}

bool MyMath::IsCollision(const Sphere& s1, const Sphere& s2)
{
	float distance = Length(s2.center - s1.center);

	if (distance <= s1.radius + s2.radius) { return true; };

	return false;
}

bool MyMath::IsCollision(const Sphere& s1, const Plane& plane)
{
	float d = Dot(plane.normal, plane.distance);
	float k = fabs(Dot(plane.normal, s1.center) - d);
	if (k <= s1.radius) {
		return true;
	}
	return false;
}

bool MyMath::IsCollision(const Segment& segment, const Plane& plane)
{
	float dot = Dot(plane.normal, segment.diff);
	if (dot == 0.0f) {
		return false;
	}
	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;

	if (0 <= t && t <= 1) { return true; }
	return false;
}

bool MyMath::IsCollision(const Segment& segment, const Triangle& triangle)
{
	Vector3 v1 = triangle.vertices[0] - triangle.vertices[1];
	Vector3 v2 = triangle.vertices[2] - triangle.vertices[1];
	Vector3 v3 = triangle.vertices[0] - triangle.vertices[2];
	Plane plane;
	plane.normal = Normlize(v1.Cross(v2));
	plane.distance = Dot(plane.normal, triangle.vertices[0]);

	float dot = Dot(plane.normal, segment.diff);
	if (dot == 0.0f) {
		return false;
	}
	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;
	Vector3 p = segment.origin + segment.diff * t;
	Vector3 v1p = p - triangle.vertices[1];
	Vector3 v2p = p - triangle.vertices[2];
	Vector3 v0p = p - triangle.vertices[0];

	Vector3 cross01 = v1.Cross(v1p);
	Vector3 cross12 = v2.Cross(v2p);
	Vector3 cross20 = v3.Cross(v0p);
	if (0 <= t && t <= 1) {

		if (Dot(cross01, plane.normal) >= 0.0f &&
			Dot(cross12, plane.normal) >= 0.0f &&
			Dot(cross20, plane.normal) >= 0.0f) {

			return true;


		}


	}
	return false;
}

bool MyMath::IsCollision(const AABB& aabb1, const AABB& aabb2)
{
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) &&
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) &&
		(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) {

		return true;


	}

	return false;
}

bool MyMath::IsCollision(const AABB& aabb, const Sphere& sphere)
{
	// 最近接点を求める
	Vector3 closesetPint{
		{std::clamp(sphere.center.x,aabb.min.x,aabb.max.x)},
		{std::clamp(sphere.center.y,aabb.min.y,aabb.max.y)},
		{std::clamp(sphere.center.z,aabb.min.z,aabb.max.z)},
	};
	//最近接点と球の中心との距離を求める
	float distance = Length(closesetPint - sphere.center);
	//距離が半径よりも小さければ衝突
	if (distance <= sphere.radius) {
		return true;
	}
	return false;
}

bool MyMath::IsCollision(const AABB& aabb, const Segment& segment)
{
	Vector3 Tmin{

		{(aabb.min.x - segment.origin.x) / segment.diff.x},
		{(aabb.min.y - segment.origin.y) / segment.diff.y},
		{(aabb.min.z - segment.origin.z) / segment.diff.z},
	};

	Vector3 Tmax{
		{(aabb.max.x - segment.origin.x) / segment.diff.x},
		{(aabb.max.y - segment.origin.y) / segment.diff.y},
		{(aabb.max.z - segment.origin.z) / segment.diff.z},
	};

	Vector3 tNear{

		{std::min(Tmin.x,Tmax.x)},
		{std::min(Tmin.y,Tmax.y)},
		{std::min(Tmin.z,Tmax.z)}
	};
	Vector3 tFar{

		{std::max(Tmin.x,Tmax.x)},
		{std::max(Tmin.y,Tmax.y)},
		{std::max(Tmin.z,Tmax.z)}
	};

	float tmin = std::max(std::max(tNear.x, tNear.y), tNear.z);
	float tmax = std::min(std::min(tFar.x, tFar.y), tFar.z);
	if (tmin <= tmax) {
		if (tmin * tmax < 0.0f)
		{
			return true;
		}
	}
	return false;
}










