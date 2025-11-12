#pragma once

#include "MyMath.h"
#include "RenderingData.h"

class LineCommon;
/// <summary>
/// ライン描画クラス
/// </summary>
class Line
{
public:

	/// <summary>
	//ラインを描画する
	/// </summary>
	/// <param name="start">始点</param>
	/// <param name="end">終点</param>
	/// <param name="color">色</param>
	void Draw(const Vector3& start, const Vector3& end, const Vector4& color);
	/// <summary>
	/// AABBを描画する
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <param name="color"></param>
	void DrawAABB(const Vector3& min, const Vector3& max, const Vector4& color);
	/// <summary>
	/// centerを中心に半径radiusのAABBを描画する
	/// </summary>
	/// <param name="center"></param>
	/// <param name="radius"></param>
	/// <param name="color"></param>
	void DrawAABBVector3(Vector3 center, float radius, Vector4 color);
	/// <summary>
	/// centerを中心にグリッドを描画する
	/// </summary>
	/// <param name="center"></param>
	/// <param name="Gridhalfwidth"></param>
	/// <param name="Subdivision"></param>
	void DrawGrid(Vector3 center, float Gridhalfwidth = 2.0, uint32_t Subdivision = 50);
	/// <summary>
	/// centerを中心に半径radiusの球を描画する
	/// </summary>
	/// <param name="center"></param>
	/// <param name="radius"></param>
	/// <param name="color"></param>
	void DrawSphere(const Vector3& center, float radius, const Vector4& color);
	/// <summary>
	/// skeletonを描画する
	/// </summary>
	/// <param name="skeleton"></param>
	/// <param name="skeletonPose"></param>
	/// <param name="worldMatrix"></param>
	/// <param name="color"></param>
	void DrawSkeleton(const Skeleton& skeleton, const std::vector<Matrix4x4>& skeletonPose, const Matrix4x4& worldMatrix, const Vector4& color = { 1.0f,1.0f,1.0f,1.0f });


};

