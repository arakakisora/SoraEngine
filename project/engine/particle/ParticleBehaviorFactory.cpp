#include "ParticleBehaviorFactory.h"
#include <ChargeBehabiaor.h>

std::unique_ptr<IParticleBehavior> ParticleBehaviorFactory::Create(const std::string& name)
{
	// 文字列 → IParticleBehavior 派生クラス 生成
    if (name == "Explosion") {
		//explosionビヘイビアを生成して返す
        return std::make_unique<ExplosionBehavior>();
    }
    if (name == "Charge") {
		//chargeビヘイビアを生成して返す
        return std::make_unique<ChargeBehabiaor>();
    }
    if (name == "ExhaustGas") {
		//exhaustgasビヘイビアを生成して返す
        return std::make_unique<ExhaustGasBehavior>();
    }
	// ここに新しいビヘイビアを追加していく

    assert(false && "Unknown behaviorType in ParticleBehaviorFactory::Create()");
    return nullptr;
}
