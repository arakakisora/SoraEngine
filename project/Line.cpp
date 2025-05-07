#include "Line.h"
#include "LineCommon.h"




void Line::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	
	LineCommon::GetInstance()->DrawLine(start, end, color);
	

}

void Line::DrawLienAABB(const Vector3& min, const Vector3& max, const Vector4& color)
{
	
}


void Line::DrawGrid(float Gridhalfwidth, uint32_t Subdivision)
{
	

}


