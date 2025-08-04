#pragma once
#include "Object3D.h"
#include "MyMath.h"
#


class testplayer
{
public:
	~testplayer();
	void Initialize(EulerTransform playerTransform);
	void Update();
	void Draw();
private:
	Object3D *object3D=nullptr; 


};

