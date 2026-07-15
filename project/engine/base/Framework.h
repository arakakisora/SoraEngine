#pragma once
#include <numbers>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include"D3DResourceLeakChecker.h"
#include "Logger.h"
#include "SpriteCommon.h"


#include "Object3DCommon.h"

#include "RenderingData.h"

#include "ModelManager.h"
#include "TextureManager.h"
#include"ImGuiManager.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI
#include "Audio.h"
#include "SrvManager.h"
#include "SceneManager.h"
#include <SceneFactory.h>
#include "OffscreenRenderManager.h"

#include "Linecommon.h"
#include "Line.h"

#include "SkyBoxCommon.h"

/// <summary>
/// フレームワーク基底クラス
/// </summary>
class Framework
{
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~Framework() = default;

	/// <summary>
	/// ゲームの初期化
	/// </summary>
	virtual void Initialize();
	/// <summary>
	/// 終了
	/// </summary>
	virtual void Finalize();
	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update();
	/// <summary>
	/// 描画
	/// </summary>
	virtual void Draw() = 0;
	/// <summary>
	/// ゲームの実行
	/// </summary>
	void Run();
	/// <summary>
	//ゲーム終了フラグの取得
	/// </summary>
	/// <returns>ゲーム終了フラグ</returns>
	virtual bool IsEndRequest()const { return endRequst_; }

public:

	//ゲーム終了フラグ	
	bool endRequst_ = false;

	//WinAppのポインタ
	std::unique_ptr<WinApp> winApp;
	//DirectXCommonのポインタ
	std::unique_ptr<DirectXCommon> dxCommon;
	//SrvManagerのポインタ
	std::unique_ptr<SrvManager> srvManager;
	//ImGuiManagerのポインタ
	std::unique_ptr<ImGuiManager> imGuiManager;
	//SceneManagerのポインタ
	std::unique_ptr<AbstractSceneFactory> sceneFactory;
	std::unique_ptr<OffscreenRenderManager> offscreenRenderManager;


};

