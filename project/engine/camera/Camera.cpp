#include "Camera.h"
#include "MyMath.h"
#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG
#include "Object3D.h"

Camera::Camera()
{
	//カメラ用のTransformを作る
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{ 0.0f,0.0f,-5.0f} };

	fovY = 0.45f;
	aspectRatio = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
	nearCilp = 0.1f;
	farClip = 100.0f;
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearCilp, farClip);
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = worldMatrix.Inverse();
	viewProjectionMatrix = viewMatrix * projectionMatrix;

}

void Camera::Update()
{
	// フォローモードが有効でターゲットが設定されている場合、ターゲットの位置にカメラを補間移動させる
	if (followMode && followTarget) {
		const EulerTransform& targetTransform = followTarget->GetTransform();
		Vector3 targetPosition = targetTransform.translate + followOffset;
		// 補間率
		if (useFollowBounds_) {
			targetPosition.x = std::clamp(targetPosition.x, followMin_.x, followMax_.x);
			targetPosition.y = std::clamp(targetPosition.y, followMin_.y, followMax_.y);
			targetPosition.z = std::clamp(targetPosition.z, followMin_.z, followMax_.z);
		}
		transform.translate = MyMath::Lerp(transform.translate, targetPosition, interpolationRate);
	}
	// カメラの行列を再計算
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearCilp, farClip);
	// カメラのワールド行列を計算
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	// ビュー行列とビュー射影行列を計算
	viewMatrix = worldMatrix.Inverse();
	// ビュー射影行列を計算
	viewProjectionMatrix = viewMatrix * projectionMatrix;
#ifdef _DEBUG
	ImGui::Begin("Camera");
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{

		ImGui::DragFloat3("*CameraRotate", &transform.rotate.x, 0.01f);
		ImGui::DragFloat3("*CameraTranslate", &transform.translate.x, 0.01f);


	}
	ImGui::End();
#endif // DEBUG_


}



void Camera::SetFollowTarget(Object3D* obj, const Vector3& offset) {
	followTarget = obj;
	followOffset = offset;
}

void Camera::SetFollowMode(bool enable) {
	followMode = enable;
}