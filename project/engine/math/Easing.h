#pragma once
#include <cmath>
#include <functional>
#include "Vector3.h"
#include "Vector2.h"

namespace Easing {
    // イージング関数の型（float t[0〜1] を受け取って float を返す）
    using EasingFunction = std::function<float(float)>;

    // 各種イージング関数
    float Linear(float t);
    float EaseInQuad(float t);
    float EaseOutQuad(float t);
    float EaseInOutQuad(float t);
    float EaseInCubic(float t);
    float EaseOutCubic(float t);
    float EaseInOutCubic(float t);
    float EaseInOutBack(float t);
    float EaseOutElastic(float t, float b, float c, float d, float amplitudeScale = 0.3f);
    float EaseOutBounce(float time, float start, float change, float duration);
    // 線形補間（Lerp）
    float Lerp(float a, float b, float t);
    // EaseOutSine
    float EaseOutSine(float t);

    // イージング付き補間：EasingLerp
    float EaseLerp(float a, float b, float t, EasingFunction easingFunc);

    // Vector2のイージング付き補間
    Vector2 EaseLerp(const Vector2& a, const Vector2& b, float t, EasingFunction easingFunc);

    // Vector3のイージング付き補間
    Vector3 EaseLerp(const Vector3& a, const Vector3& b, float t, EasingFunction easingFunc);

    // 2つのベクトル a と b を、補間率 t（0.0 ～ 1.0）に基づいて線形補間（Lerp）
    Vector3 SmoothLerp(const Vector3& a, const Vector3& b, float t);
};