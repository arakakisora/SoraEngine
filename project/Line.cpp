#include "Line.h"
#include "LineCommon.h"
#include <numbers>




void Line::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	
	LineCommon::GetInstance()->DrawLine(start, end, color);
	

}

void Line::DrawLienAABB(const Vector3& min, const Vector3& max, const Vector4& color)
{ // 8頂点を定義
    Vector3 p[8] = {
        { min.x, min.y, min.z }, // 0
        { max.x, min.y, min.z }, // 1
        { max.x, max.y, min.z }, // 2
        { min.x, max.y, min.z }, // 3
        { min.x, min.y, max.z }, // 4
        { max.x, min.y, max.z }, // 5
        { max.x, max.y, max.z }, // 6
        { min.x, max.y, max.z }  // 7
    };

    // 前面
    LineCommon::GetInstance()->DrawLine(p[0], p[1], color);
    LineCommon::GetInstance()->DrawLine(p[1], p[2], color);
    LineCommon::GetInstance()->DrawLine(p[2], p[3], color);
    LineCommon::GetInstance()->DrawLine(p[3], p[0], color);

    // 背面
    LineCommon::GetInstance()->DrawLine(p[4], p[5], color);
    LineCommon::GetInstance()->DrawLine(p[5], p[6], color);
    LineCommon::GetInstance()->DrawLine(p[6], p[7], color);
    LineCommon::GetInstance()->DrawLine(p[7], p[4], color);

    // 側面（前後の接続）
    LineCommon::GetInstance()->DrawLine(p[0], p[4], color);
    LineCommon::GetInstance()->DrawLine(p[1], p[5], color);
    LineCommon::GetInstance()->DrawLine(p[2], p[6], color);
    LineCommon::GetInstance()->DrawLine(p[3], p[7], color);
}


void Line::DrawGrid(float Gridhalfwidth, uint32_t Subdivision)
{
    assert(Subdivision > 0);

    const float step = (Gridhalfwidth * 2.0f) / Subdivision;
    const float start = -Gridhalfwidth;
    const float end = Gridhalfwidth;

    Vector4 lineColor = { 0.5f, 0.5f, 0.5f, 1.0f };  // 灰色

    // Z方向のライン（Xを固定してZを移動）
    for (uint32_t i = 0; i <= Subdivision; ++i) {
        float z = start + step * i;
        LineCommon::GetInstance()->DrawLine(
            { start, 0.0f, z },
            { end,   0.0f, z },
            lineColor
        );
    }

    // X方向のライン（Zを固定してXを移動）
    for (uint32_t i = 0; i <= Subdivision; ++i) {
        float x = start + step * i;
        LineCommon::GetInstance()->DrawLine(
            { x, 0.0f, start },
            { x, 0.0f, end },
            lineColor
        );
    }

}

void Line::DrawSphere(const Vector3& center, float radius, const Vector4& color)
{
    const uint32_t kSbdivision = 16;
    const float kLonEvery = 2 * std::numbers::pi_v<float> / kSbdivision;
    const float KLatEvery = std::numbers::pi_v<float> / kSbdivision;

    for (uint32_t latIndex = 0; latIndex < kSbdivision; ++latIndex) {
        float lat = -std::numbers::pi_v<float> / 2.0f + KLatEvery * latIndex;
        for (uint32_t lonIndex = 0; lonIndex < kSbdivision; ++lonIndex) {

            float lon = lonIndex * kLonEvery;
            Vector3 a = {
                radius * std::cosf(lat) * std::cosf(lon) + center.x,
                radius * std::sinf(lat) + center.y,
                radius * std::cosf(lat) * std::sinf(lon) + center.z
            };

            Vector3 b = {
                radius * std::cosf(lat + KLatEvery) * std::cosf(lon) + center.x,
                radius * std::sinf(lat + KLatEvery) + center.y,
                radius * std::cosf(lat + KLatEvery) * std::sinf(lon) + center.z
            };

            Vector3 c = {
                radius * std::cosf(lat) * std::cosf(lon + kLonEvery) + center.x,
                radius * std::sinf(lat) + center.y,
                radius * std::cosf(lat) * std::sinf(lon + kLonEvery) + center.z
            };


            // 線を描画（緯度線・経度線）
            LineCommon::GetInstance()->DrawLine(a, b, color); // 縦方向
            LineCommon::GetInstance()->DrawLine(a, c, color); // 横方向
           


        }


    }
	

}


