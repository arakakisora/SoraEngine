#include "ParticleBehaviorFactory.h"
#include <ChargeBehabiaor.h>
#include "BarrierBreakBehavior.h"
#include "BarrierRingBehavior.h"

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
    if (name == "BarrierBreak") {
        return std::make_unique<BarrierBreakBehavior>();
    }
    if (name == "BarrierRing") {
        return std::make_unique<BarrierRingBehavior>();
	}

	// ここに新しいビヘイビアを追加していく

    assert(false && "Unknown behaviorType in ParticleBehaviorFactory::Create()");
    return nullptr;
}

std::vector<std::string> ParticleBehaviorFactory::GetBehaviorNames()
{
    return {
        "Explosion",
        "Charge",
        "ExhaustGas",
        "BarrierBreak",
		"BarrierRing"
    };
}
