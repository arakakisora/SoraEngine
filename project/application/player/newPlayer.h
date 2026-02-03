
#include "assert.h"
#include <algorithm>
#include <numbers>
#include "MyMath.h"

#include "RenderingData.h"

#include "Object3D.h"
#include <PlayerBullet.h>
#include <ParticleEmitter.h>
#include "StageStartEffect.h"
#include "Collider.h"
#include "Line.h"            // 追加: ライン描画
#include <memory>           // 追加: std::unique_ptr を使用


enum class LRDirecion {
	kright,
	kLeft,
};

struct CollisionMapInfo {

	bool ceiling = false; // 天井衝突
	bool landing = false; // 着地
	bool hitWall = false; // 壁接触
	Vector3 move;         // 移動量
};

enum Corner {
	kRightBottom,
	kLeftBottom,
	kRightTop,
	kLeftTop,
	kNumCorner // 要素数
};

enum class WeaponType {
	Gatling,
	Cannon,
};

class Enemy;
class MapChipField;

class newPlayer :public Collider {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="position"></param>
	void Initialize(const Vector3& position);

	/// <summary>
	// 更新
	/// </summary>
	void Update();

	/// <summary>
	// 描画
	/// </summary>
	void Draw();

	/// <summary>	
	/// 自機の動き
	/// </summary>
	void Move(); // 自機の動き


private:
	std::unique_ptr<Object3D> object3D_;// プレイヤーの3Dオブジェクト
	Vector3 playerPosition_ = {};// プレイヤーの位置
	Vector3 velocity_ = {};                          // 速度
	Vector2 acceleration_ = {};                      // 加速度

	static inline const float kjumpPower = 0.01f;  // 定数加速度

	// 当たり判定
	MapChipField* mapChipField_ = nullptr;
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 1.0f;
	static inline const float kAttenuationLanding = 0.5f;
	static inline const float kCollisionEpsilon = 0.1f;
	static inline const float kAttenuationWall = 0.1f;


};