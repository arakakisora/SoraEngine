#include "Camera.h"
#include "MyMath.h"
#include <imgui.h>
#include "Object3D.h"

Camera::Camera()
{
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{ 0.0f,0.0f,-5.0f} };
	
	fovY = 0.45f;
	aspectRatio = float(WinApp::kClientWindth) / float(WinApp::kClientHeight);
	nearCilp = 0.1f;
	farClip = 100.0f;
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearCilp, farClip);
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = worldMatrix.Inverse();
	viewProjectionMatrix = viewMatrix * projectionMatrix;

}

void Camera::Update()
{

	if (followMode && followTarget) {
		const Transform& targetTransform = followTarget->GetTransform();
		Vector3 targetPosition = targetTransform.translate + followOffset;
		transform.translate = MyMath::Lerp(transform.translate, targetPosition, interpolationRate);
	}

	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearCilp, farClip);
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = worldMatrix.Inverse();
	viewProjectionMatrix = viewMatrix * projectionMatrix;
#ifdef DEBUG_

	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		

		//ImGui::DragFloat3("*ModelScale", &transform.scale.x, 0.01f);
		ImGui::DragFloat3("*CameraRotate", &transform.rotate.x, 0.01f);
		ImGui::DragFloat3("*CameraTransrate", &transform.translate.x, 0.01f);

		
	}
#endif // DEBUG_


}

void Camera::SetFollowTarget(Object3D *obj, const Vector3& offset) {
	followTarget = obj;
	followOffset = offset;
}

void Camera::SetFollowMode(bool enable) {
	followMode = enable;
}