#pragma once
#include "RenderingData.h"
#include "MyMath.h"
#include "Vector3.h"
#include "Object3D.h"


struct PayerData{
	//プレイヤーの位置
	Vector3 position;
	//playerの速度
	Vector3 Velocity;
	//playerの回転
	Vector3 rotation;
	//playerのスケール
	Vector3 scale;
	//playerの加速度
	Vector3 acceleration;
};

class Player
{
public:
	Player();
	~Player();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3D* Object3D);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	//playerobject
	Object3D* object3D_=nullptr;
	//playerのデータ
	PayerData playerData;
	//
	Input* input_;




};

