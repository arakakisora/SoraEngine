#include "LineCommon.h"
#include "MyMath.h"
#include <CameraManager.h>

LineCommon* LineCommon::instance_ = nullptr;
LineCommon* LineCommon::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new LineCommon();
	}
	return instance_;
}

void LineCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	//パイプラインの生成
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateLine();


	

	VertexDataLine vertices[] = {
	   {{0, 0, 0, 1}}, // 始点
	   {{1, 0, 0, 1}}  // 終点
	   


	};


	//モデル用のVetexResouceを作成
	vertexResource_ = dxCommon_->CreateBufferResource(sizeof(vertices));
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(vertices));
	vertexBufferView_.StrideInBytes = sizeof(VertexDataLine);
	void* mapped = nullptr;
	vertexResource_->Map(0, nullptr, &mapped);
	memcpy(mapped, vertices, sizeof(vertices));

	//srvのインデックスを確保
	instanceSrvIndex_ = srvManager_->Allocate();

}
void LineCommon::Finalize()
{
	delete instance_;
	instance_ = nullptr;
}
void LineCommon::CommonDraw()
{
	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureLine());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateLine());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
}

void LineCommon::Update()
{

	if (instanceDatas_.empty()) return;
	size_t bufferSize = sizeof(LineInstanceData) * instanceDatas_.size();
	

	instanceResource_ = dxCommon_->CreateBufferResource(bufferSize);
	void* mapped = nullptr;
	instanceResource_->Map(0, nullptr, &mapped);
	memcpy(mapped, instanceDatas_.data(), bufferSize);

	srvManager_-> CreateSRVforStructuredBuffer(
		instanceSrvIndex_,
		instanceResource_.Get(),
		(UINT)instanceDatas_.size(),
		sizeof(LineInstanceData)
	);


}

void LineCommon::Draw()
{

	if (instanceDatas_.empty()) return;

	CommonDraw();
	dxCommon_ ->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	srvManager_->SetGraficsRootDescriptorTable(0, instanceSrvIndex_);
	dxCommon_->GetCommandList()->DrawInstanced(2, (UINT)instanceDatas_.size(), 0, 0);

	instanceDatas_.clear();



}

void LineCommon::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	Camera* activeCamera = CameraManager::GetInstance()->GetActiveCamera();


	Vector3 direction = end - start;//ベクトルの差分
	float length = direction.Length();//長さを取得
	if (length <= 0.0001f) { return; }
	Vector3 normalizeDirection = direction.Normalize();//正規化

	// スケール（ラインの長さ分）
	Matrix4x4 scaleMat = MyMath::MakeScaleMatrix({ length,1.0f , 1.0f });
	// 回転（Y軸を方向ベクトルに合わせる）
	Matrix4x4 rotateMat = MyMath::DirectionToDirection({ 1, 0, 0 }, normalizeDirection);
	// 移動（開始位置へ）
	Matrix4x4 translateMat = MyMath::MakeTranslateMatrix(start);

	Matrix4x4 world = translateMat * rotateMat * scaleMat;

	//// 最終ワールド行列
	//Matrix4x4 world =
	//	MyMath::Multiply(
	//		MyMath::Multiply(translateMat, rotateMat),
	//		scaleMat
	//	);

	Matrix4x4 viewProjection = activeCamera->GetViewprojectionMatrix();
	Matrix4x4 worldViewProjectionMatrix = world * viewProjection;

	LineInstanceData instance{};
	instance.World = world;
	instance.WVP = worldViewProjectionMatrix;
	instance.color = color;

	instanceDatas_.push_back(instance);


}
