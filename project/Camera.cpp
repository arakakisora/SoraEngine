#include "Camera.h"
#include "MyMath.h"

void Camera::Update()
{

	worldMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	viewMatrix = Inverse(cameraMatrix);


}
