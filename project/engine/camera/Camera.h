#pragma once
#include "RenderingData.h"
#include <WinApp.h>

class Object3D;
/// <summary>
/// カメラ
/// </summary>
class Camera
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Camera();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// フォローモード用ターゲットセット
	/// </summary>
	/// <param name="obj"></param>
	/// <param name="offset"></param>
	void SetFollowTarget(Object3D* obj, const Vector3& offset);
	/// <summary>
	/// フォローモードセット
	/// </summary>
	/// <param name="enable"></param>
	void SetFollowMode(bool enable);
	/// <summary>
	/// スケールセット
	/// </summary>
	/// <param name="scale"></param>
	void SetRotate(const Vector3& rotate) { this->transform.rotate = rotate; }
	/// <summary>
	/// トランスレートセット
	/// </summary>
	/// <param name="translate"></param>
	void SetTranslate(const Vector3& translate) { this->transform.translate = translate; }
	/// <summary>
	/// FovYセット
	/// </summary>
	/// <param name="fovy"></param>
	void SetFovY(const float& fovy) { this->fovY = fovy; }
	/// <summary>
	/// アスペクト比セット
	/// </summary>
	/// <param name="aspectRation"></param>
	void SetAspectRatio(const float& aspectRation) { this->aspectRatio = aspectRation; }
	/// <summary>
	/// ニアクリップセット
	/// </summary>
	/// <param name="nearClip"></param>
	void SetNearClip(const float& nearClip) { this->nearCilp = nearClip; }
	/// <summary>
	/// ファークリップセット
	/// </summary>
	/// <param name="farClipValue"></param>
	void SetFarClip(const float& farClipValue) { this->farClip = farClipValue; }
	/// <summary>
	/// ワールド、ビュー、プロジェクション行列取得
	/// </summary>
	/// <returns></returns>
	const Matrix4x4& GetWorldMatrix()const { return worldMatrix; }
	/// <summary>
	/// ビュー行列取得
	/// </summary>
	/// <returns></returns>
	const Matrix4x4& GetViewMatrix()const { return viewMatrix; }
	/// <summary>
	/// プロジェクション行列取得
	/// </summary>
	/// <returns></returns>
	const Matrix4x4& GetProjectionMatrix()const { return projectionMatrix; }
	/// <summary>
	/// ビュープロジェクション行列取得
	/// </summary>
	/// <returns></returns>
	const Matrix4x4& GetViewProjectionMatrix()const { return viewProjectionMatrix; }
	/// <summary>
	/// トランスフォーム取得
	/// </summary>
	/// <returns></returns>
	const EulerTransform& GetTransform()const { return transform; }

	/// <summary>
	/// フォローボーダー有効化
	/// </summary>
	/// <param name="enable"></param>
	void SetFollowBoundsEnabled(bool enable) { useFollowBounds_ = enable; }
	/// <summary>
	/// フォローボーダーセット
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	void SetFollowBounds(const Vector3& min, const Vector3& max) { followMin_ = min; followMax_ = max; }

	Matrix4x4 GetprojectionMatrix() const { return projectionMatrix; }
	

private:

	//カメラ用のTransformを作る
	EulerTransform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 viewProjectionMatrix;


	float fovY = 0.45f;
	float aspectRatio = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
	float nearCilp = 0.1f;
	float farClip = 100.0f;

	Object3D* followTarget = nullptr;
	Vector3 followOffset = { 0.0f, 0.0f, -15.0f };
	bool followMode = false;
	static inline const float interpolationRate = 0.5f; // 補間率

	bool useFollowBounds_ = false;
	Vector3 followMin_ = { -99999.0f, -99999.0f, -99999.0f };
	Vector3 followMax_ = { 99999.0f,  99999.0f,  99999.0f };
};

