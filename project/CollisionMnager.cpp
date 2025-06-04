#include "CollisionMnager.h"
#include "MyMath.h"

void CollisionManager::Clear() {
    collisionObjects_.clear();
}

void CollisionManager::RegisterObject(const std::string& tag, const AABB& aabb, void* owner, int id, std::function<void(const CollisionObject&)> onCollision) {
    CollisionObject obj;
    obj.shapeType = ShapeType::AABB;
    obj.tag = tag;
    obj.owner = owner;
    obj.aabb = aabb;
    obj.id = id;
    obj.onCollision = onCollision;
    collisionObjects_.push_back(obj);
}

void CollisionManager::CheckCollisions() {
    for (size_t i = 0; i < collisionObjects_.size(); ++i) {
        for (size_t j = i + 1; j < collisionObjects_.size(); ++j) {
            auto& a = collisionObjects_[i];
            auto& b = collisionObjects_[j];
            if (a.shapeType == ShapeType::AABB && b.shapeType == ShapeType::AABB) {
                if (MyMath::IsCollision(a.aabb, b.aabb)) {
                    if (a.onCollision) a.onCollision(b);
                    if (b.onCollision) b.onCollision(a);
                }
            }
        }
    }
}


