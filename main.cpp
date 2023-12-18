#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class AAction
{
public:
	virtual string extractString() = 0;
	virtual ~AAction() = default;
};

class ActionManager
{
public:
	vector<AAction *> actions;
	ActionManager(){};
	void addAction(AAction *action)
	{
		actions.push_back(action);
	}
	void execute()
	{
		if (actions.size() > 0)
		{
		}
		else
		{
		}
		actions.clear();
	}
};

class Game
{
public:
	int turn;
	ActionManager actionManager;
	Game() : turn(0) { initParse(); }
	Game(const Game &g) { *this = g; }
	~Game() {}
	Game &operator=(const Game &g)
	{
		turn = g.turn;
		return *this;
	}

	void initParse()
	{
	}

	void turnParse()
	{
	}

	void play()
	{
		turnParse();
		turn++;
		routine();
		actionManager.execute();
	}

	void routine()
	{
	}
};

int main()
{
	Game game;
	while (1)
	{
		game.play();
	}
	return 0;
}