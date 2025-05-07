#include "Line.h"
#include "LineCommon.h"




void Line::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	
	LineCommon::GetInstance()->DrawLine(start, end, color);
	

}

void Line::DrawLienAABB(const Vector3& min, const Vector3& max, const Vector4& color)
{
	// AABBの8つの頂点を計算
	Vector3 vertices[8] = {
		{ min.x, min.y, min.z },
		{ max.x, min.y, min.z },
		{ min.x, max.y, min.z },
		{ max.x, max.y, min.z },
		{ min.x, min.y, max.z },
		{ max.x, min.y, max.z },
		{ min.x, max.y, max.z },
		{ max.x, max.y, max.z }
	};
	// AABBの12本のエッジを描画
	for (int i = 0; i < 4; ++i) {
		LineCommon::GetInstance()->DrawLine(vertices[i], vertices[(i + 1) % 4], color); // 底面
		LineCommon::GetInstance()->DrawLine(vertices[i + 4], vertices[((i + 1) % 4) + 4], color); // 上面
		LineCommon::GetInstance()->DrawLine(vertices[i], vertices[i + 4], color); // 垂直線
	}
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


