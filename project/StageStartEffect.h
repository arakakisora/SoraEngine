#pragma once
#include <memory>
#include "Object3D.h"

class StageStartEffect {
public:
    void Initialize(Object3D* player);
    void Update();
    void Draw();
    bool IsFinished() const { return isFinished_; }

private:
    std::unique_ptr<Object3D> gateLeft_;
    std::unique_ptr<Object3D> gateRight_;
    Object3D* player_;

    float timer_ = 0.0f;
    bool isFinished_ = false;
};
