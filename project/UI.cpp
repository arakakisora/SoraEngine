#include "UI.h"
#include "SpriteCommon.h"

void UI::Initialize()
{
	spaceSprite_.Initialize(SpriteCommon::GetInstance(), "Resources/space.png");
	//spaceSprite_.SetSize({ 64,64 });
	spaceSprite_.SetPosition({ 100,100 });
	mouseSprite_.Initialize(SpriteCommon::GetInstance(), "Resources/mouse.png");
	//mouseSprite_.SetSize({ 64,64 });
	mouseSprite_.SetPosition({ 200,100 });
}

