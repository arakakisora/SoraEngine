#pragma once
class Player;

class Icommand
{

public:
	
	virtual ~Icommand();
	virtual void Exec(Player& player) = 0;
};

class MoveRightCommand :public Icommand
{
public:
	void Exec(Player& player) override;

};

class MoveLeftCommand :public Icommand
{
public:
	void Exec(Player& player) override;
};

class InputHandler
{
public:
	Icommand* HandleInput();	
	void AssignMoveLeftCommand2PresskeyA();
	void AssignMoveRightCommand2PresskeyD();

private:
	Icommand* PressKeyA_ = nullptr;
	Icommand* PressKeyD_ = nullptr;


};


