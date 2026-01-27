#pragma once
#include "DirectXCommon.h"
#include "Camera.h"
#include "GraphicsPipeline.h"
#include "SrvManager.h"
#include <memory> // std::unique_ptr


/// <summary>
/// 3Dオブジェクト共通クラス
/// </summary>
class Object3DCommon
{
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	Object3DCommon() = default;
	~Object3DCommon() = default;
	Object3DCommon(const Object3DCommon&) = delete;
	Object3DCommon& operator=(const Object3DCommon&) = delete;
	//インスタンス
	static std::unique_ptr<Object3DCommon> instance_;
	
	static Object3DCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon,SrvManager*srvmanager);
	/// <summary>
	//終了
	/// </summary>
	void Finalize();
	/// <summary>
	//共通描画設定
	/// </summary>
	void CommonDraw();
	/// <summary>
	//スキニング共通描画設定
	/// </summary>
	void SkinNingCommonDraw();

	//アクセッサ
	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	//SrvManager
	SrvManager* GetSrvManager()const { return srvManager_; }

	
private:

	DirectXCommon* dxCommon_;
	SrvManager* srvManager_ = nullptr;

	Camera* defaultCamera = nullptr;

	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;
	std::unique_ptr<GraphicsPipeline> skinningGraphicsPipeline_;
};

