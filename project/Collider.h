#pragma once
#include "Mymath.h"
class Collider {
public:
    enum class Layer {
        Player,
        Enemy,
        PlayerBullet,
        EnemyBullet,
        // Map, Item など増やしたくなったらここに追加
    };

    virtual ~Collider() = default;

    // 当たり判定用のAABBを返す
    virtual AABB GetAABB() const = 0;

    // 当たった時に呼ばれるコールバック
    virtual void OnCollision(Collider* other) = 0;

    Layer GetLayer() const { return layer_; }

protected:
    Collider(Layer layer) : layer_(layer) {}
    Layer layer_;
};

