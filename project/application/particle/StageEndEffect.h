#pragma once
#include <memory>
#include "Object3D.h"

enum class EndPhase {
    Opening,   // 開く
    PullIn,    // 吸い込む
    Closing,   // 閉じる
    Finished
};

class StageEndEffect
{
public:
    void Initialize(Object3D* player, const Vector3& gatePos);
    void Begin();
    void Update(float dt);
    void Draw();
    bool IsFinished() const { return phase_ == EndPhase::Finished; }

private:
    std::unique_ptr<Object3D> gateL_;
    std::unique_ptr<Object3D> gateR_;
    Object3D* player_ = nullptr;

    EndPhase phase_ = EndPhase::Opening;
    float timer_ = 0.0f;

    Vector3 basePos_;
    Vector3 playerEndPos_;
    
};
