#pragma once
#include <memory>
#include "Object3D.h"

class StageStartEffect {
public:
    void Initialize(Object3D* player, const Vector3& basePos);
    void Begin();                  // 再生開始
    void Update(float dt);
    void Draw();                   // ←ゲートのみ描画
    bool IsFinished() const { return isFinished_; }

private:
    std::unique_ptr<Object3D> gateLeft_;
    std::unique_ptr<Object3D> gateRight_;
    Object3D* player_ = nullptr;
    Vector3 basePos_{};            // プレイヤー基準位置
    float timer_ = 0.0f;
    bool isFinished_ = false;
   
    Vector3 playerStartPos_{};
    float   baseY_ = 0.0f;      // Y固定用
    float   moveDistZ_ = 2.0f;  // 奥→手前の距離

};
