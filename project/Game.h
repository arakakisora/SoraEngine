#include <string>
#include <format>

#include <windows.h>


#pragma comment(lib,"dxguid.lib")

#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include"MyMath.h"
//#include "RenderingPipeline.h"





#include "Framework.h"

class Game :public Framework
{
public:

	//ゲームの初期化
	void Initialize()override;
	//終了
	void Finalize()override;
	//更新
	void Update()override;
	//描画
	void Draw()override;

	

private:

	

	



	//スプライトの初期化
	std::vector<Sprite*>sprites;
	Object3D* object3D = nullptr;
	Object3D* object3D2nd = nullptr;
	SoundData sounddata1 = Audio::GetInstance()->SoundLoadWave("Resources/gamePlayBGM.wav");

	//wvpData用のTransform変数を作る
	Transform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	Transform transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	bool useMonsterBall = true;

	bool bgm = false;

	float rotation{ 0 };



};

