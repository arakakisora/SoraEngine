#include "Line.h"
#include "LineCommon.h"
#include "MyMath.h"
#include <CameraManager.h>

void Line::Initialize(LineCommon* lineCommon)
{
	lineCommon_ = lineCommon;

	VertexDataLine vertices[] = {
	   {{0, 0, 0, 1}}, // 始点
	   {{0, 1, 0, 1}}  // 終点
	   // Replace the following line:  
	   // memcpy(nullptr, vertices, sizeof(vertices));  


	};


	//モデル用のVetexResouceを作成
	vertexResource_ = lineCommon_->GetDxCommon()->CreateBufferResource(sizeof(vertices));
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(vertices));
	vertexBufferView_.StrideInBytes = sizeof(VertexDataLine);
	void* mapped = nullptr;
	vertexResource_->Map(0, nullptr, &mapped);
	memcpy(mapped, vertices, sizeof(vertices));

	//srvのインデックスを確保
	instanceSrvIndex_ = lineCommon_->GetSrvmanager()->Allocate();
}

void Line::Update()
{
	if (instanceDatas_.empty()) return;

	size_t bufferSize = sizeof(LineInstanceData) * instanceDatas_.size();
	auto* dxCommon = lineCommon_->GetDxCommon();

	instanceResource_ = dxCommon->CreateBufferResource(bufferSize);
	void* mapped = nullptr;
	instanceResource_->Map(0, nullptr, &mapped);
	memcpy(mapped, instanceDatas_.data(), bufferSize);

	lineCommon_->GetSrvmanager()->CreateSRVforStructuredBuffer(
		instanceSrvIndex_,
		instanceResource_.Get(),
		(UINT)instanceDatas_.size(),
		sizeof(LineInstanceData)
	);




}

void Line::Draw()
{
	if (instanceDatas_.empty()) return;

	lineCommon_->CommonDraw();
	lineCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	lineCommon_->GetSrvmanager()->SetGraficsRootDescriptorTable(0, instanceSrvIndex_);
	lineCommon_->GetDxCommon()->GetCommandList()->DrawInstanced(2, (UINT)instanceDatas_.size(), 0, 0);

	instanceDatas_.clear();
	


}

void Line::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	
	
	Camera* activeCamera = CameraManager::GetInstance()->GetActiveCamera();


	Vector3 direction = end - start;//ベクトルの差分
	float length = direction.Length();//長さを取得
	if (length <= 0.0001f) { return; }
	Vector3 normalizeDirection = direction.Normalize();//正規化

	// スケール（ラインの長さ分）
	Matrix4x4 scaleMat = MyMath::MakeScaleMatrix({ 1.0f, length, 1.0f });
	// 回転（Y軸を方向ベクトルに合わせる）
	Matrix4x4 rotateMat = MyMath::DirectionToDirection({ 0, 1, 0 }, normalizeDirection);
	// 移動（開始位置へ）
	Matrix4x4 translateMat = MyMath::MakeTranslateMatrix(start);

	// 最終ワールド行列
	Matrix4x4 world = MyMath::Multiply(MyMath::Multiply(scaleMat, rotateMat), translateMat);
	Matrix4x4 viewProjection = activeCamera->GetProjextionMatrix();
	Matrix4x4 worldViewProjectionMatrix = world * viewProjection;

	LineInstanceData instance{};
	instance.World = world;
	instance.WVP = worldViewProjectionMatrix;
	instance.color = color;

	instanceDatas_.push_back(instance);

}
