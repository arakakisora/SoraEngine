#pragma once
#include "DirectXCommon.h"
#include "Camera.h"
#include "GraphicsPipeline.h"

class Object3DCommon
{
public:
	Object3DCommon();
	~Object3DCommon();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	//共通描画設定
	void CommonDraw();
	
	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	Camera* GetDefaultCamera()const { return defaultCamera; }

private:

	DirectXCommon* dxCommon_;
	Camera* defaultCamera = nullptr;
	GraphicsPipeline* graphicsPipeline_ = nullptr;
};

