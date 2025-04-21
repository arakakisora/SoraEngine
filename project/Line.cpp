#include "Line.h"
#include "LineCommon.h"

void Line::Initialize(LineCommon* lineCommon)
{
	lineCommon_ = lineCommon;
	//モデル用のVetexResouceを作成
	vertexResource_ = lineCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexDataLine) * 2);
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexDataLine) * 2);

	memccpy(vertexResource_->Map(0, nullptr), reinterpret_cast<void**>(&VertexDataLine), sizeof(VertexDataLine) * 2);


	//モデルオブジェクト
	//モデル用のVetexResouceを作成
	vertexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());


}

void Line::Update()
{
}

void Line::Draw()
{
}
