#include "Object3DCommon.h"
#include "Object3D.h"
#include "MyMath.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "CameraManager.h"





void Object3D::Initialize(Object3DCommon* object3DCommon)
{
	//引数で受け取って、メンバ変数に記録する
	object3DCommon_ = object3DCommon;

	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = object3DCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixData));
	//単位行列を書き込む

	transformaitionMatrixData->WVP = transformaitionMatrixData->WVP.MakeIdentity4x4();
	transformaitionMatrixData->World = transformaitionMatrixData->World.MakeIdentity4x4();


	//平行光源
	//平行光源用のResoureceを作成
	directionalLightResource = object3DCommon_->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,1.0f };
	directionalLightData->intensity = 1.0f;



	//カメラとモデルのTrandform変数
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	
	 
	




}

void Object3D::Update()
{

	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Camera* activeCamera = CameraManager::GetInstans()->GetActiveCamera();
	
	if (activeCamera) {
		const Matrix4x4& viewProjectionMatrix = activeCamera->GetViewprojectionMatrix();
		worldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;
	}
	else {

		worldViewProjectionMatrix = worldMatrix;
	}

	transformaitionMatrixData->WVP = worldViewProjectionMatrix;
	transformaitionMatrixData->World = worldMatrix;
}

void Object3D::Draw()
{


	object3DCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	//平行光源Cbufferの場所を設定
	object3DCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	//3Dモデルが割り当てられているなら描画する
	if (model_) {
		model_->Draw();
	}

}

void Object3D::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstans()->FindModel(filepath);
}









