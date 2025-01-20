#include "MyMath.h"
#include <numbers>
#include <imgui.h>



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

Vector3 MyMath::Normalize(const Vector3& vector)
{
	
	float length = std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	return { vector.x / length, vector.y / length, vector.z / length };
}

float MyMath::Dot(const Vector3& lhs, const Vector3& rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

Vector3 MyMath::Cross(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3{
		lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.x * rhs.y - lhs.y * rhs.x
	};
	
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

	Matrix4x4 rotateXYZ = MakeRotateXMatrix(rotate.x)* MakeRotateYMatrix(rotate.y)* MakeRotateZMatrix(rotate.z);
	return MakeScaleMatrix(scale)* rotateXYZ* MakeTranslateMatrix(translate);
	
}

float MyMath::Cot(float theta)
{
	return 1 / std::tan(theta);
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

Vector3 Reflect(const Vector3& input, const Vector3& normal)
{
    return input - normal * 2 * MyMath::Dot(input, normal);
}

Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle)
{
	Matrix4x4 ans;

	float cosA = cosf(angle);
	float sinA = sinf(angle);
	float oneMinusCosA = 1.0f - cosA;

	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

	// Row 0
	ans.m[0][0] = x * x * oneMinusCosA + cosA;
	ans.m[0][1] = x * y * oneMinusCosA + z * sinA;
	ans.m[0][2] = x * z * oneMinusCosA - y * sinA;
	ans.m[0][3] = 0.0f;

	// Row 1
	ans.m[1][0] = y * x * oneMinusCosA - z * sinA;
	ans.m[1][1] = y * y * oneMinusCosA + cosA;
	ans.m[1][2] = y * z * oneMinusCosA + x * sinA;
	ans.m[1][3] = 0.0f;

	// Row 2
	ans.m[2][0] = z * x * oneMinusCosA + y * sinA;
	ans.m[2][1] = z * y * oneMinusCosA - x * sinA;
	ans.m[2][2] = z * z * oneMinusCosA + cosA;
	ans.m[2][3] = 0.0f;

	// Row 3
	ans.m[3][0] = 0.0f;
	ans.m[3][1] = 0.0f;
	ans.m[3][2] = 0.0f;
	ans.m[3][3] = 1.0f;

	return ans;
}


Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to)
{
	Vector3 fromNormalized = MyMath::Normalize(from);
	Vector3 toNormalized = MyMath::Normalize(to);
	float dotProduct = MyMath::Dot(fromNormalized, toNormalized);

	if (dotProduct > 0.9999f) {
		return Matrix4x4{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}
	else if (dotProduct < -0.9999f) {
		Vector3 orthogonalAxis = (fabs(fromNormalized.x) > fabs(fromNormalized.z)) ? Vector3{ -fromNormalized.y, fromNormalized.x, 0.0f } : Vector3{ 0.0f, -fromNormalized.z, fromNormalized.y };
		orthogonalAxis = MyMath::Normalize(orthogonalAxis);
		return MakeRotateAxisAngle(orthogonalAxis, std::numbers::pi_v<float>);
	}

	Vector3 axis = MyMath::Cross(fromNormalized, toNormalized);
	float angle = acos(dotProduct);
	return MakeRotateAxisAngle(axis, angle);
}

Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs)
{
	return {
		lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
		lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x,
		lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w
	};
}

Quaternion IdentityQuaternion()
{
	return { 0.0f, 0.0f, 0.0f, 1.0f };
}

Quaternion Conjugate(const Quaternion& quaternion)
{
	return { -quaternion.x, -quaternion.y, -quaternion.z, quaternion.w };
}

float Norm(const Quaternion& quaternion)
{
	return std::sqrt(
		quaternion.w * quaternion.w +
		quaternion.x * quaternion.x +
		quaternion.y * quaternion.y +
		quaternion.z * quaternion.z
	);
}

Quaternion Normalize(const Quaternion& quaternion)
{
	float norm = Norm(quaternion);
	return {
		quaternion.x / norm,
		quaternion.y / norm,
		quaternion.z / norm,
		quaternion.w / norm
	};
}

Quaternion Inverse(const Quaternion& quaternion)
{
	Quaternion conjugate = Conjugate(quaternion);
	float normSquared = Norm(quaternion) * Norm(quaternion);
	return {
		conjugate.x / normSquared,
		conjugate.y / normSquared,
		conjugate.z / normSquared,
		conjugate.w / normSquared
	};
}

void QuaternionImGuiText(const Quaternion& quaternion, const char* label)
{
	// クォータニオンの各成分を ImGui テキストで表示
	ImGui::Text("x: %.02f", quaternion.x);
	ImGui::SameLine();
	ImGui::Text("y: %.02f", quaternion.y);
	ImGui::SameLine();
	ImGui::Text("z: %.02f", quaternion.z);
	ImGui::SameLine();
	ImGui::Text("w: %.02f", quaternion.w);

	// ラベルを表示
	ImGui::SameLine();
	ImGui::Text("%s", label);
}










