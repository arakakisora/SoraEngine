#include "CameraManager.h"
#include <cassert>

CameraManager* CameraManager::instance = nullptr;

CameraManager* CameraManager::GetInstans()
{
    if (!instance) {
        instance = new CameraManager();
    }
    
	return instance;
}

void CameraManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void CameraManager::AddCamera(const std::string& name, const Camera* camera)
{
    assert(cameras.find(name) == cameras.end() && "Camera with the same name already exists!");
    cameras[name] = *camera; // Dereference the pointer to store the Camera object
    // 最初のカメラをアクティブに設定
    if (activeCameraName.empty()) {
        activeCameraName = name;
    }
}

void CameraManager::RemoveCamera(const std::string& name) {
    if (cameras.erase(name) > 0 && activeCameraName == name) {
        // アクティブカメラが削除された場合、他のカメラをアクティブに設定
        if (!cameras.empty()) {
            activeCameraName = cameras.begin()->first;
        }
        else {
            activeCameraName.clear();
        }
    }
}

Camera* CameraManager::GetCamera(const std::string& name) {
    auto it = cameras.find(name);
    if (it != cameras.end()) {
        return &(it->second);
    }
    return nullptr;
}




