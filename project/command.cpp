#include "command.h"
#include "Player.h"
#include "Input.h"

void MoveRightCommand::Exec(Player& player)
{
	player.MoveRight();
}

void MoveLeftCommand::Exec(Player& player)
{
	player.MoveLeft();
}

Icommand* InputHandler::HandleInput()
{
	if (Input::GetInstans()->PushKey(DIK_D))
	{
		return PressKeyD_;
	}

	if (Input::GetInstans()->PushKey(DIK_A))
	{
		return PressKeyA_;
	}

	

	return nullptr;
}

void InputHandler::AssignMoveLeftCommand2PresskeyA()
{
	Icommand* command= new MoveLeftCommand();
	PressKeyA_ = command;
}

void InputHandler::AssignMoveRightCommand2PresskeyD()
{
	Icommand* command = new MoveRightCommand();
	PressKeyD_ = command;
}

Icommand::~Icommand()
{
}
