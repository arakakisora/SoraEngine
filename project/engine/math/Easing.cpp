#include "Easing.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace Easing {
    float Linear(float t) {
        return t;
    }

    float EaseInQuad(float t) {
        return t * t;
    }
    
    float EaseOutQuad(float t) {
        return t * (2 - t);
    }

    float EaseInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : 1.0f - static_cast<float>(pow(-2.0f * t + 2.0f, 2)) / 2.0f;
    }

    float EaseInCubic(float t) {
        return t * t * t;
    }

    float EaseOutCubic(float t) {
        t -= 1.0f;
        return t * t * t + 1.0f;
    }

    float EaseInOutCubic(float t) {
        return t < 0.5f
            ? 4 * t * t * t
            : (t - 1.0f) * (2 * t - 2) * (2 * t - 2) + 1.0f;
    }

    float Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    float EaseLerp(float a, float b, float t, EasingFunction easingFunc) {
        return Lerp(a, b, easingFunc(t));
    }

    Vector2 EaseLerp(const Vector2& a, const Vector2& b, float t, EasingFunction easingFunc)
    {
        float e = easingFunc(t);
        return {
            Lerp(a.x, b.x, e),
            Lerp(a.y, b.y, e)
        };
    }

    Vector3 EaseLerp(const Vector3& a, const Vector3& b, float t, EasingFunction easingFunc)
    {
        float e = easingFunc(t);
        return {
            Lerp(a.x, b.x, e),
            Lerp(a.y, b.y, e),
            Lerp(a.z, b.z, e)
        };
    }

	float EaseInOutBack(float t) {
		const float c1 = 1.70158f;
		const float c2 = c1 * 1.525f;

		if (t < 0.5f) {
			return (powf(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2;
		} else {
			return (powf(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
		}
	}

    Vector3 SmoothLerp(const Vector3& a, const Vector3& b, float t) {
        Vector3 ans;
        ans.x = a.x + t * (b.x - a.x);
        ans.y = a.y + t * (b.y - a.y);
        ans.z = a.z + t * (b.z - a.z);
        return ans;
    }

    float EaseOutElastic(float t, float b, float c, float d, float amplitudeScale) {
        if (t == 0) return b;
        if ((t /= d) == 1) return b + c;
        float p = d * 0.1f; // 伸縮の周期
        float a = c * amplitudeScale; // 振幅をスケールダウン
        float s = p / 4.0f;
        return a * powf(2.0f, -10.0f * t) * sinf((t * d - s) * (2.0f * static_cast<float>(M_PI)) / p) + c + b;
    }

    float EaseOutBounce(float time, float start, float change, float duration) {
        float t = time / duration;
        if (t < (1 / 2.75f)) {
            return change * (7.5625f * t * t) + start;
        } else if (t < (2 / 2.75f)) {
            t -= (1.5f / 2.75f);
            return change * (7.5625f * t * t + 0.75f) + start;
        } else if (t < (2.5 / 2.75)) {
            t -= (2.25f / 2.75f);
            return change * (7.5625f * t * t + 0.9375f) + start;
        } else {
            t -= (2.625f / 2.75f);
            return change * (7.5625f * t * t + 0.984375f) + start;
        }
    }
}