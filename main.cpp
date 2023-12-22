#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <map>
#include <math.h>
#include <set>

using namespace std;

#define LEFT_MIDDLE 2000
#define RIGHT_MIDDLE 8000

#define TOP_LIMIT 2500
#define TOP_MIDDLE 3750
#define MID_LIMIT 5000
#define MID_MIDDLE 6250
#define BOTTOM_LIMIT 7500
#define BOTTOM_MIDDLE 8750

#define LOW_LIGHT_RADIUS 800
#define BIG_LIGHT_RADIUS 2000
#define EMERGENCY_RADIUS 500
#define DANGER_RADIUS 2300

#define SCAN_SAVE 490

class AAction
{
public:
	virtual string extractString() = 0;
	virtual ~AAction() = default;
};

class MoveAction : public AAction
{
	int x;
	int y;
	bool bigLight;
	string msg;

public:
	MoveAction(int x, int y, bool bigLight = false) : x(x), y(y), bigLight(bigLight) {}
	MoveAction(int x, int y, bool bigLight, string msg) : x(x), y(y), bigLight(bigLight), msg(msg) {}
	string extractString()
	{
		string s = "MOVE " + to_string(x) + " " + to_string(y);
		s += bigLight ? " 1" : " 0";
		if (msg.length() > 0)
			s += " " + msg;
		return s;
	}
};

class WaitAction : public AAction
{
	bool bigLight;
	string msg;

public:
	WaitAction(bool bigLight = false) : bigLight(bigLight) {}
	WaitAction(bool bigLight, string msg) : bigLight(bigLight), msg(msg) {}
	string extractString()
	{
		string s = "WAIT";
		s += bigLight ? " 1" : " 0";
		if (msg.length() > 0)
			s += " " + msg;
		return s;
	}
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
		for (auto &a : actions)
		{
			cout << a->extractString() << endl;
		}
		actions.clear();
	}
};

typedef enum
{
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
} RadarDirection;

class Creature
{
public:
	int id;
	int color;
	int type;
	int x;
	int y;
	int dx;
	int dy;
	bool visible;
	bool savedByMe;
	bool savedByOpp;
	bool scannedByMe;
	bool dead;

	Creature(int id, int color, int type)
	{
		this->id = id;
		this->color = color;
		this->type = type;
		this->x = 0;
		this->y = 0;
		this->dx = 0;
		this->dy = 0;
		this->visible = false;
		this->savedByMe = false;
		this->savedByOpp = false;
		this->scannedByMe = false;
		this->dead = false;
	}

	Creature(const Creature &c) { *this = c; }

	Creature &operator=(const Creature &c)
	{
		id = c.id;
		color = c.color;
		type = c.type;
		x = c.x;
		y = c.y;
		dx = c.dx;
		dy = c.dy;
		visible = c.visible;
		savedByMe = c.savedByMe;
		savedByOpp = c.savedByOpp;
		scannedByMe = c.scannedByMe;
		dead = c.dead;
		return *this;
	}

	bool operator==(const Creature &c)
	{
		return id == c.id;
	}

	void update(int x, int y, int dx, int dy, bool visible)
	{
		this->x = x;
		this->y = y;
		this->dx = dx;
		this->dy = dy;
		this->visible = visible;
	}

	int distanceTo(int x, int y)
	{
		return abs(this->x - x) + abs(this->y - y);
	}

	int distanceTo(Creature &c)
	{
		return distanceTo(c.x, c.y);
	}

	~Creature() {}
};

typedef enum
{
	MY_DRONE,
	OPP_DRONE,
} DroneOwner;

class Drone
{
public:
	int id;
	int x;
	int y;
	int emergency;
	int battery;
	DroneOwner owner;
	ActionManager &actionManager;
	int moveX;
	int moveY;
	bool bigLight;
	int scanCount;
	list<Creature *> scans;
	map<int, RadarDirection> radarBlips;
	string actionMessage;

	Drone(int id, int x, int y, int emergency, int battery, DroneOwner owner, ActionManager &actionManager) : actionManager(actionManager)
	{
		this->id = id;
		this->x = x;
		this->y = y;
		this->emergency = emergency;
		this->battery = battery;
		this->owner = owner;
		this->moveX = -1;
		this->moveY = -1;
		this->bigLight = false;
		this->scanCount = 0;
		this->actionMessage = "";
	}

	Drone(const Drone &d) : actionManager(d.actionManager) { *this = d; }

	Drone &operator=(const Drone &d)
	{
		id = d.id;
		x = d.x;
		y = d.y;
		emergency = d.emergency;
		battery = d.battery;
		owner = d.owner;
		actionManager = d.actionManager;
		moveX = d.moveX;
		moveY = d.moveY;
		bigLight = d.bigLight;
		scanCount = d.scanCount;
		scans = d.scans;
		radarBlips = d.radarBlips;
		actionMessage = d.actionMessage;
		return *this;
	}

	bool operator==(const Drone &d)
	{
		return id == d.id;
	}

	void update(int x, int y, int emergency, int battery)
	{
		this->x = x;
		this->y = y;
		this->emergency = emergency;
		this->battery = battery;
		this->moveX = -1;
		this->moveY = -1;
		this->bigLight = false;
		this->actionMessage = "";
		this->scans.clear();
		this->scanCount = 0;
	}

	void registerScan(Creature &c)
	{
		if (find(scans.begin(), scans.end(), &c) != scans.end())
			return;
		if (c.savedByMe)
			return;
		this->scans.push_back(&c);
		this->scanCount = scans.size();
	}

	void move(int x, int y, string msg = "")
	{
		this->actionMessage = msg;
		this->moveX = x;
		this->moveY = y;
	}

	void move(Creature &c, string msg = "")
	{
		move(c.x, c.y, msg);
	}

	void move(RadarDirection dir, string msg = "")
	{
		switch (dir)
		{
		case TOP_LEFT:
			move(min(x - 300, 0), min(y - 300, 0), msg);
			break;
		case TOP_RIGHT:
			move(max(x + 300, 10000), min(y - 300, 0), msg);
			break;
		case BOTTOM_LEFT:
			move(min(x - 300, 0), max(y + 350, 10000), msg);
			break;
		case BOTTOM_RIGHT:
			move(max(x + 300, 10000), max(y + 350, 10000), msg);
			break;
		}
	}

	void wait(string msg = "")
	{
		this->actionMessage = msg;
		this->moveX = -1;
		this->moveY = -1;
	}

	void setBigLight()
	{
		this->bigLight = true;
	}

	void setLowLight()
	{
		this->bigLight = false;
	}

	void toggleLight()
	{
		this->bigLight = !this->bigLight;
	}

	void registerActions()
	{
		if (moveX != -1 && moveY != -1)
			actionManager.addAction(new MoveAction(moveX, moveY, bigLight, actionMessage));
		else
		{
			actionManager.addAction(new WaitAction(bigLight, actionMessage));
		}
	}

	int distanceTo(int x, int y)
	{
		return abs(this->x - x) + abs(this->y - y);
	}

	int distanceTo(Creature &c)
	{
		return distanceTo(c.x, c.y);
	}

	int distanceTo(Drone &d)
	{
		return distanceTo(d.x, d.y);
	}

	int distanceTo(Creature *c)
	{
		return distanceTo(c->x, c->y);
	}

	pair<int, int> getTarget()
	{
		if (moveX != -1 && moveY != -1)
			return make_pair(moveX, moveY);
		else
			return make_pair(x, y);
	}

	~Drone() {}
};

class Game
{
public:
	int turn;
	int creatureCount;
	list<Creature> creatures;
	int myScore;
	int oppScore;
	int mySavedScanCount;
	list<Creature *> mySavedScans;
	int oppSavedScanCount;
	list<Creature *> oppSavedScans;
	int myDroneCount;
	list<Drone> myDrones;
	int oppDroneCount;
	list<Drone> oppDrones;
	int visibleCreatureCount;
	list<Creature *> visibleCreatures;
	int myScanCount;
	list<Creature *> myScans;
	list<Creature *> monsters;

	ActionManager actionManager;
	Game() : turn(0), creatureCount(0) { initParse(); }
	Game(const Game &g) { *this = g; }
	~Game() {}
	Game &operator=(const Game &g)
	{
		turn = g.turn;
		creatureCount = g.creatureCount;
		creatures = g.creatures;
		myScore = g.myScore;
		oppScore = g.oppScore;
		mySavedScanCount = g.mySavedScanCount;
		mySavedScans = g.mySavedScans;
		oppSavedScanCount = g.oppSavedScanCount;
		oppSavedScans = g.oppSavedScans;
		myDroneCount = g.myDroneCount;
		myDrones = g.myDrones;
		oppDroneCount = g.oppDroneCount;
		oppDrones = g.oppDrones;
		visibleCreatureCount = g.visibleCreatureCount;
		visibleCreatures = g.visibleCreatures;
		myScanCount = g.myScanCount;
		myScans = g.myScans;
		monsters = g.monsters;
		return *this;
	}

	void initParse()
	{
		cin >> creatureCount;
		cin.ignore();
		for (int i = 0; i < creatureCount; i++)
		{
			int creature_id;
			int color;
			int type;
			cin >> creature_id >> color >> type;
			cin.ignore();
			creatures.push_back(Creature(creature_id, color, type));
			if (type < 0)
				monsters.push_back(&creatures.back());
		}
	}

	Creature &getCreatureById(int id)
	{
		for (auto &c : creatures)
		{
			if (c.id == id)
				return c;
		}
	}

	Drone &getDroneById(int id)
	{
		for (auto &d : myDrones)
		{
			if (d.id == id)
				return d;
		}
		for (auto &d : oppDrones)
		{
			if (d.id == id)
				return d;
		}
	}

	void turnParse()
	{
		cin >> myScore;
		cin.ignore();
		cin >> oppScore;
		cin.ignore();

		for (auto &c : creatures)
			c.scannedByMe = false;

		cin >> mySavedScanCount;
		cin.ignore();
		for (int i = 0; i < mySavedScanCount; i++)
		{
			int creature_id;
			cin >> creature_id;
			cin.ignore();
			Creature &c = getCreatureById(creature_id);
			c.savedByMe = true;
			mySavedScans.push_back(&c);
		}

		cin >> oppSavedScanCount;
		cin.ignore();
		for (int i = 0; i < oppSavedScanCount; i++)
		{
			int creature_id;
			cin >> creature_id;
			cin.ignore();
			Creature &c = getCreatureById(creature_id);
			c.savedByOpp = true;
			oppSavedScans.push_back(&c);
		}

		cin >> myDroneCount;
		cin.ignore();
		for (int i = 0; i < myDroneCount; i++)
		{
			int drone_id;
			int drone_x;
			int drone_y;
			int emergency;
			int battery;
			cin >> drone_id >> drone_x >> drone_y >> emergency >> battery;
			cin.ignore();
			if (turn == 0)
				myDrones.push_back(Drone(drone_id, drone_x, drone_y, emergency, battery, MY_DRONE, this->actionManager));
			else
				getDroneById(drone_id).update(drone_x, drone_y, emergency, battery);
		}

		cin >> oppDroneCount;
		cin.ignore();
		for (int i = 0; i < oppDroneCount; i++)
		{
			int drone_id;
			int drone_x;
			int drone_y;
			int emergency;
			int battery;
			cin >> drone_id >> drone_x >> drone_y >> emergency >> battery;
			cin.ignore();
			if (turn == 0)
				oppDrones.push_back(Drone(drone_id, drone_x, drone_y, emergency, battery, OPP_DRONE, this->actionManager));
			else
				getDroneById(drone_id).update(drone_x, drone_y, emergency, battery);
		}

		int drone_scan_count;
		cin >> drone_scan_count;
		cin.ignore();
		myScans.clear();
		for (int i = 0; i < drone_scan_count; i++)
		{
			int drone_id;
			int creature_id;
			cin >> drone_id >> creature_id;
			cin.ignore();
			Creature &c = getCreatureById(creature_id);
			Drone &d = getDroneById(drone_id);
			if (d.owner == MY_DRONE)
			{
				c.scannedByMe = true;
				if (find(myScans.begin(), myScans.end(), &c) == myScans.end())
				{
					myScans.push_back(&c);
				}
			}
			d.registerScan(c);
		}
		myScanCount = myScans.size();

		for (auto &c : creatures)
			c.visible = false;
		cin >> visibleCreatureCount;
		cin.ignore();
		visibleCreatures.clear();
		for (int i = 0; i < visibleCreatureCount; i++)
		{
			int creature_id;
			int creature_x;
			int creature_y;
			int creature_vx;
			int creature_vy;
			cin >> creature_id >> creature_x >> creature_y >> creature_vx >> creature_vy;
			cin.ignore();
			// cerr << "Creature " << creature_id << " (" << getCreatureById(creature_id).type << ") is visible" << endl;
			Creature &c = getCreatureById(creature_id);
			c.update(creature_x, creature_y, creature_vx, creature_vy, true);
			visibleCreatures.push_back(&c);
		}

		int radar_blip_count;
		cin >> radar_blip_count;
		cin.ignore();
		vector<int> aliveCreatures;
		for (int i = 0; i < radar_blip_count; i++)
		{
			int drone_id;
			int creature_id;
			string radar;
			cin >> drone_id >> creature_id >> radar;
			cin.ignore();
			Drone &d = getDroneById(drone_id);
			RadarDirection dir;
			if (radar == "TL")
				dir = TOP_LEFT;
			else if (radar == "TR")
				dir = TOP_RIGHT;
			else if (radar == "BL")
				dir = BOTTOM_LEFT;
			else if (radar == "BR")
				dir = BOTTOM_RIGHT;
			d.radarBlips[creature_id] = dir;
			aliveCreatures.push_back(creature_id);
		}

		for (auto &c : creatures)
		{
			if (find(aliveCreatures.begin(), aliveCreatures.end(), c.id) == aliveCreatures.end())
				c.dead = true;
		}

		for (auto &c : creatures)
		{
			if (c.savedByMe)
				c.scannedByMe = true;
		}
	}

	void play()
	{
		turnParse();
		turn++;
		routine();
		for (auto &d : myDrones)
			d.registerActions();
		actionManager.execute();
	}

	int countAliveFish()
	{
		int count = 0;
		for (auto &c : creatures)
		{
			if (c.type >= 0 && c.dead == false)
				count++;
		}
		return count;
	}

	int countAliveFish(int type)
	{
		int count = 0;
		for (auto &c : creatures)
		{
			if (c.type == type && c.dead == false)
				count++;
		}
		return count;
	}

	bool areAllFishScanned(int type)
	{
		for (auto &c : creatures)
		{
			if (c.type == type && !c.dead && !c.scannedByMe)
				return false;
		}
		return true;
	}

	bool areAllFishScanned()
	{
		for (int i = 0; i < 3; i++)
		{
			if (!areAllFishScanned(i))
				return false;
		}
		return true;
	}

	void protectionMode(Drone &d)
	{
		pair<int, int> originalTarget = d.getTarget();
		int originalTargetX = originalTarget.first;
		int originalTargetY = originalTarget.second;
		pair<double, double> originalVector = make_pair(originalTargetX - d.x, originalTargetY - d.y);
		double norm = sqrt(originalVector.first * originalVector.first + originalVector.second * originalVector.second);
		originalVector.first /= norm;
		originalVector.second /= norm;

		vector<Creature *> danger;
		for (auto &m : monsters)
		{
			if (!m->visible)
				continue;
			if (d.distanceTo(m) > DANGER_RADIUS)
				continue;
			danger.push_back(m);
		}
		if (danger.size() == 0)
			return;

		int minDistance = 1000000;
		pair<int, int> bestPosition = make_pair(-1, -1);
		for (int i = 0; i < 360; i++)
		{
			double angle = i * M_PI / 180;
			pair<double, double> rotatedVector = make_pair(originalVector.first * cos(angle) - originalVector.second * sin(angle), originalVector.first * sin(angle) + originalVector.second * cos(angle));
			pair<int, int> targetVector = make_pair(rotatedVector.first * 600, rotatedVector.second * 600);
			vector<double> split = {0.25, 0.5, 0.75, 1};
			bool danger = false;
			for (double s : split)
			{
				pair<int, int> partialTarget = make_pair(d.x + targetVector.first * s, d.y + targetVector.second * s);
				for (auto &m : monsters)
				{
					pair<int, int> partialMonster = make_pair(m->x + m->dx * s, m->y + m->dy * s);
					int dist = sqrt((partialTarget.first - partialMonster.first) * (partialTarget.first - partialMonster.first) + (partialTarget.second - partialMonster.second) * (partialTarget.second - partialMonster.second));
					if (dist < EMERGENCY_RADIUS)
					{
						danger = true;
						break;
					}
				}
			}
			if (danger)
				continue;
			int distance = sqrt((d.x + targetVector.first - originalTargetX) * (d.x + targetVector.first - originalTargetX) + (d.y + targetVector.second - originalTargetY) * (d.y + targetVector.second - originalTargetY));
			if (distance < minDistance)
			{
				minDistance = distance;
				bestPosition = make_pair(d.x + targetVector.first, d.y + targetVector.second);
			}
		}
		if (bestPosition.first != -1 && bestPosition.second != -1)
		{
			d.move(bestPosition.first, bestPosition.second, "Ahhh");
		}
	}

	void cake(Drone &d)
	{
		if (areAllFishScanned())
			d.setBigLight();
		vector<Creature *> danger;
		for (auto &m : monsters)
		{
			if (!m->visible)
				continue;
			if (d.distanceTo(m) > DANGER_RADIUS)
				continue;
			if (m->y > d.y)
				continue;
			danger.push_back(m);
		}
		d.move(d.x, SCAN_SAVE, "The cake is a lie");
		if (danger.size() > 0)
			protectionMode(d);
	}

	int missingScan(int type)
	{
		int count = 0;
		for (auto &c : creatures)
		{
			if (c.type == type && !c.dead && !c.scannedByMe)
				count++;
		}
		return count;
	
	}

	vector<int> horizontalTarget;
	vector<bool> finished;
	void routine()
	{
		if (turn == 1)
		{
			Drone &d1 = myDrones.front();
			Drone &d2 = myDrones.back();
			if (d1.x < d2.x)
			{
				horizontalTarget.push_back(LEFT_MIDDLE);
				horizontalTarget.push_back(RIGHT_MIDDLE);
			}
			else
			{
				horizontalTarget.push_back(RIGHT_MIDDLE);
				horizontalTarget.push_back(LEFT_MIDDLE);
			}
			finished.push_back(false);
			finished.push_back(false);
		}

		int i = 0;
		Creature *oldTarget = nullptr;
		for (Drone &d : myDrones)
		{
			if ((d.y < TOP_LIMIT && d.scanCount > 0) || areAllFishScanned())
			{
				cake(d);
				continue;
			}

			if (d.y < BOTTOM_LIMIT && !finished[i])
			{
				d.move(horizontalTarget[i], BOTTOM_LIMIT + 100, "Glad0s");
				if (turn == 5 || turn == 7 || turn == 9 || turn == 11)
					d.setBigLight();
				else
					d.setLowLight();
			}
			else
			{
				finished[i] = true;
				d.setBigLight();
				int target = 2;
				while (true)
				{
					if (missingScan(target) > 1 || (missingScan(target) == 1 && oldTarget != nullptr && oldTarget->type != target))
						break;
					target--;
					if (target < 0)
						break;
				}
				cerr << "For bot " << d.id << " target is " << target << endl;
				if (target < 0)
				{
					cake(d);
					continue;
				}
				bool found = false;
				for (auto &c : creatures)
				{
					if (c.dead)
						continue;
					if (c.type != target)
						continue;
					if (c.scannedByMe)
						continue;
					d.move(d.radarBlips[c.id], "Wheatley");
					oldTarget = &c;
					found = true;
					break;
				}
				if (!found)
				{
					cake(d);
					continue;
				}
			}
			protectionMode(d);

			i++;
		}
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