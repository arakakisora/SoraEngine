#include "Line.h"
#include "LineCommon.h"




void Line::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	
	LineCommon::GetInstance()->DrawLine(start, end, color);
	

}

void Line::DrawGrid(float Gridhalfwidth, uint32_t Subdivision)
{
	const float kGridEvery = (Gridhalfwidth * 2.0f) / float(Subdivision);

	//奥から手前への線を順番に引いていく
	for (uint32_t xIndex = 0; xIndex <= Subdivision; ++xIndex) {
		float x = -Gridhalfwidth + (xIndex * kGridEvery);
		Vector3 start{ x,0.0f,-Gridhalfwidth };
		Vector3 end{ x,0.0f,Gridhalfwidth };

		LineCommon::GetInstance()->DrawLine(start, end, {1,0,0,1});
		

	}

}


