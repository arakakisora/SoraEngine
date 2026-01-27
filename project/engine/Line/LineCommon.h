#pragma once
#include "DirectXCommon.h"
#include "GraphicsPipeline.h"
#include "SrvManager.h"

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "RenderingData.h"

#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <string>
#include <wrl/client.h>
#include <d3d12.h>
#include <Camera.h>
#include <vector>

struct VertexDataLine
{
	Vector3 position;

};

struct LineInstanceData {
	Vector3 start;
	Vector3 end;
	Vector4 color;
};

struct CameraBufferforGpu {
	Matrix4x4 view;
	Matrix4x4 projection;

};
/// <summary>
/// ライン描画共通クラス
/// </summary>
class LineCommon
{
public:
	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static LineCommon* GetInstance();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();
	/// <summary>
	//共通描画設定
	/// </summary>
	void CommonDraw();
	/// <summary>
	//更新
	/// </summary>
	void Update();
	/// <summary>
	//描画
	/// </summary>
	void Draw();
	/// <summary>
	/// ライン描画
	/// </summary>
	/// <param name="start"></param>
	/// <param name="end"></param>
	/// <param name="color"></param>
	void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);
	/// <summary>
	// DXCommon
	/// </summary>
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	/// <summary>
	//SrvMnager
	/// </summary>
	///<returns></returns>
	SrvManager* GetSrvmanager()const { return srvManager_; }

	/// <summary>
	/// コンストラクタ
	/// </summary>
	LineCommon() = default;
	~LineCommon() = default;
	LineCommon(const LineCommon&) = delete;
	LineCommon& operator=(const LineCommon&) = delete;

private:
	// インスタンス（staticに変更）
	static std::unique_ptr<LineCommon> instance_;

	DirectXCommon* dxCommon_;
	SrvManager* srvManager_;
	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	uint32_t instanceSrvIndex_ = UINT32_MAX;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_;

	LineInstanceData instance = {
		.start = {0.0f, 0.0f, 0.0f},
		.end = {0.0f, 1.0f, 1.0f},
		.color = {1.0f, 0.0f, 0.0f, 1.0f}
	};

	std::vector<VertexDataLine>linevertices = {
		{{0.0f,0.0f,0.0f}},
		{{1.0f,0.0f,0.0f}},
	};
	std::vector<LineInstanceData> instances_;

	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;//カメラのデータを送るためのリソース
	CameraBufferforGpu* camerabuffer = nullptr;//カメラのデータをGPUに送るための構造体
};

