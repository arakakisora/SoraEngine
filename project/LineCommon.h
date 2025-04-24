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
//#include <Windows.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <Camera.h>
#include <vector>

class LineCommon
{
public:
	static LineCommon* GetInstance();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon,SrvManager *srvManager);
	//終了
	void Finalize();
	//共通描画設定
	void CommonDraw();

	//更新
	void Update();
	//描画
	void Draw();

	void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);
	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	//SrvMnager
	SrvManager* GetSrvmanager()const { return srvManager_; }


private:
	LineCommon() = default;
	~LineCommon() = default;
	LineCommon(const LineCommon&) = delete;
	LineCommon& operator=(const LineCommon&) = delete;
private:
	//インスタンス
	static LineCommon* instance_;
	DirectXCommon* dxCommon_;
	SrvManager* srvManager_;
	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	uint32_t instanceSrvIndex_ = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_;
	std::vector<LineInstanceData> instanceDatas_;


};

