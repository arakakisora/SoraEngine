#pragma once
#include <Camera.h>
#include <unordered_map>
#include <string>
class CameraManager
{
	static CameraManager* instance;
	CameraManager() = default;
	~CameraManager() = default;
	CameraManager(CameraManager&) = default;
	CameraManager& operator=(CameraManager&) = delete;

public:
	//シングルトンインスタンスの取得
	static CameraManager* GetInstans();
	//終了
	void Finalize();

	//カメラの追加
	void AddCamera(const std::string& name, const Camera* camera);

	//カメラの削除
	void RemoveCamera(const std::string& name);

	//カメラの取得
	Camera* GetCamera(const std::string& name);

	// アクティブカメラの取得
	Camera* GetActiveCamera() {

		return GetCamera(activeCameraName);

	};

	// アクティブカメラの設定
	void SetActiveCamera(const std::string& name) {

		if (cameras.find(name) != cameras.end()) {
			activeCameraName = name;
		}

	};



private:
	//カメラデータ
	std::unordered_map<std::string, Camera> cameras;

	// アクティブカメラ名
	std::string activeCameraName;

};

