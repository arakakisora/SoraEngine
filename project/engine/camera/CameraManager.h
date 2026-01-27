#pragma once
#include <Camera.h>
#include <unordered_map>
#include <string>
#include <memory>
/// <summary>
/// カメラマネージャークラス
/// </summary>
class CameraManager
{

public:
	/// <summary>
	/// シングルトンインスタンス
	/// </summary>
	static std::unique_ptr <CameraManager> instance;
	CameraManager() = default;
	~CameraManager() = default;
	CameraManager(CameraManager&) = default;
	CameraManager& operator=(CameraManager&) = delete;
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	static CameraManager* GetInstance();
	/// <summary>
	// 終了
	/// </summary>
	void Finalize();
	/// <summary>
	//初期化
	/// </summary>
	void Initialize();
	/// <summary>
	//カメラの追加
	/// </summary>
	/// <param name="name"></param>
	/// <param name="camera"></param>
	void AddCamera(const std::string& name, const Camera* camera);
	/// <summary>
	//カメラの削除
	/// </summary>
	/// <param name="name"></param>
	void RemoveCamera(const std::string& name);
	/// <summary>
	//カメラの取得
	/// </summary>
	/// <param name="name"></param>
	Camera* GetCamera(const std::string& name);
	/// <summary>
	// アクティブカメラの取得
	/// </summary>
	Camera* GetActiveCamera();

	/// <summary>
	// アクティブカメラの設定
	/// </summary>
	/// <param name="name"></param>
	void SetActiveCamera(const std::string& name);



private:
	//カメラデータ
	std::unordered_map<std::string, Camera> cameras;

	// アクティブカメラ名
	std::string activeCameraName;

	//デフォルトカメラ

	//デフォルトカメラ
	Camera* defaultCamera = nullptr;


};

