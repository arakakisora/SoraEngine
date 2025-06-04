#pragma once
#include <string>
#include "MyMath.h"
#include "RenderingData.h"
#include <functional>
enum class ShapeType { AABB };

struct CollisionObject {
	ShapeType shapeType;
	std::string tag;
	int id = 0;
	void* owner = nullptr; // オーナーオブジェクト（衝突後のコールバックなどで使う）
	AABB aabb;

	// 衝突時に呼ばれるコールバック関数（任意設定）
	std::function<void(const CollisionObject& other)> onCollision = nullptr;

};

class CollisionManager
{
public:
    // シングルトン取得
    static CollisionManager& GetInstance() {
        static CollisionManager instance;
        return instance;
    }

    // 全衝突オブジェクトをクリア
    void Clear();

    // オブジェクトを登録
    void RegisterObject(const std::string& tag, const AABB& aabb, void* owner = nullptr, int id = 0,
        std::function<void(const CollisionObject&)> onCollision = nullptr);

    // 衝突判定を実行
    void CheckCollisions();

private:
    CollisionManager() = default;
    std::vector<CollisionObject> collisionObjects_;

};

