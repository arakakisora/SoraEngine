#pragma once
#include <memory>
#include <string>
#include "IParticleBehavior.h"

class ParticleBehaviorFactory
{
	/// <summary>
	/// ビヘイビアの名前から生成
	/// </summary>
	/// <param name="behaviorName"></param>
	/// <returns></returns>
	static std::unique_ptr<IParticleBehavior> Create(const std::string& behaviorName);
	
	/// <summary>
	/// 利用可能なビヘイビア名一覧を取得
	/// </summary>
	/// <returns></returns>
	static std::vector<std::string> GetBehaviorNames();

};

