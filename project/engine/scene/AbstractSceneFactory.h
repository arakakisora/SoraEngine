#pragma once

#include "BaseScene.h"
#include <string>
#include <memory>

class AbstractSceneFactory
{

public:
	virtual std::unique_ptr<BaseScene> CreateScene(const std::string& Scenename) = 0;
	virtual ~AbstractSceneFactory() = default;

};

