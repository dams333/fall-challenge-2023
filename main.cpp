#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <map>
#include <math.h>
#include <set>

using namespace std;

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

typedef enum {
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

	Creature(int id, int color, int type) {
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

typedef enum {
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

	Drone(int id, int x, int y, int emergency, int battery, DroneOwner owner,ActionManager &actionManager):actionManager(actionManager) {
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

	Drone(const Drone &d): actionManager(d.actionManager) { *this = d; }

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

		list<Creature *> toRemove;
		for (auto &c : scans)
		{
			if (c->savedByMe)
			{
				toRemove.push_back(c);
				continue;
			}
		}
		for (auto &c : toRemove)
		{
			scans.remove(c);
		}
		this->scanCount = scans.size();
	}

	void registerScan(Creature &c)
	{
		if (find(scans.begin(), scans.end(), &c) != scans.end())
			return;
		this->scanCount++;
		this->scans.push_back(&c);
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
			move(min(x - 300, 0), max(y + 300, 10000), msg);
			break;
		case BOTTOM_RIGHT:
			move(max(x + 300, 10000), max(y + 300, 10000), msg);
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
		cin >> creatureCount; cin.ignore();
		for (int i = 0; i < creatureCount; i++) {
			int creature_id;
			int color;
			int type;
			cin >> creature_id >> color >> type; cin.ignore();
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
		cin >> myScore; cin.ignore();
		cin >> oppScore; cin.ignore();
		
		cin >> mySavedScanCount; cin.ignore();
		for (int i = 0; i < mySavedScanCount; i++) {
			int creature_id;
			cin >> creature_id; cin.ignore();
			Creature &c = getCreatureById(creature_id);
			c.savedByMe = true;
			mySavedScans.push_back(&c);
		}

		cin >> oppSavedScanCount; cin.ignore();
		for (int i = 0; i < oppSavedScanCount; i++) {
			int creature_id;
			cin >> creature_id; cin.ignore();
			Creature &c = getCreatureById(creature_id);
			c.savedByOpp = true;
			oppSavedScans.push_back(&c);
		}

		cin >> myDroneCount; cin.ignore();
		for (int i = 0; i < myDroneCount; i++) {
			int drone_id;
			int drone_x;
			int drone_y;
			int emergency;
			int battery;
			cin >> drone_id >> drone_x >> drone_y >> emergency >> battery; cin.ignore();
			if (turn == 0)
				myDrones.push_back(Drone(drone_id, drone_x, drone_y, emergency, battery, MY_DRONE, this->actionManager));
			else
				getDroneById(drone_id).update(drone_x, drone_y, emergency, battery);
		}

		cin >> oppDroneCount; cin.ignore();
		for (int i = 0; i < oppDroneCount; i++) {
			int drone_id;
			int drone_x;
			int drone_y;
			int emergency;
			int battery;
			cin >> drone_id >> drone_x >> drone_y >> emergency >> battery; cin.ignore();
			if (turn == 0)
				oppDrones.push_back(Drone(drone_id, drone_x, drone_y, emergency, battery, OPP_DRONE, this->actionManager));
			else
				getDroneById(drone_id).update(drone_x, drone_y, emergency, battery);
		}

		int drone_scan_count;
        cin >> drone_scan_count; cin.ignore();
        for (int i = 0; i < drone_scan_count; i++) {
            int drone_id;
            int creature_id;
            cin >> drone_id >> creature_id; cin.ignore();
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
        cin >> visibleCreatureCount; cin.ignore();
		visibleCreatures.clear();
        for (int i = 0; i < visibleCreatureCount; i++) {
            int creature_id;
            int creature_x;
            int creature_y;
            int creature_vx;
            int creature_vy;
            cin >> creature_id >> creature_x >> creature_y >> creature_vx >> creature_vy; cin.ignore();
			Creature &c = getCreatureById(creature_id);
			c.update(creature_x, creature_y, creature_vx, creature_vy, true);
			visibleCreatures.push_back(&c);
        }

		int radar_blip_count;
        cin >> radar_blip_count; cin.ignore();
		vector<int> aliveCreatures;
        for (int i = 0; i < radar_blip_count; i++) {
            int drone_id;
            int creature_id;
            string radar;
            cin >> drone_id >> creature_id >> radar; cin.ignore();
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

	void routine()
	{
		Creature *old_target = nullptr;
		cerr << "Scanned: " << myScanCount << " / " << countAliveFish() << endl;
		vector<int> levels;
		for (auto &c : creatures)
		{
			if (c.dead)
				continue;
			if (c.type < 0)
				continue;
			levels.push_back(c.type);
		}
		sort(levels.begin(), levels.end());
		int targetLevel = levels.size() > 0 ? *levels.begin() : -1;
		for (auto &d : myDrones)
		{
			if (targetLevel != -1)
			{
				bool visible = false;
				bool cancel = false;
				Creature *target = nullptr;
				for (auto &c : visibleCreatures)
				{
					if (c->dead)
						continue;
					if (c->type < 0)
					{
						if (d.distanceTo(*c) < 1000)
						{
							float x = d.x - c->x;
							float y = d.y - c->y;
							float norm = sqrt(x * x + y * y);
							x = x / norm * 900;
							y = y / norm * 300;
							d.move(d.x + x, d.y + y, "BOUH  " + to_string(c->id));
							d.setLowLight();
							cancel = true;
							break;
						}
					}
					if (c->scannedByMe)
						continue;
					if (old_target == c)
						continue;
					visible = true;
					if (target == nullptr)
						target = c;
					else if (d.distanceTo(*c) < d.distanceTo(*target))
						target = c;
				}
				if (cancel)
					continue;
				if (target == nullptr)
				{
					for (auto &c : creatures)
					{
						if (c.dead)
							continue;
						if (c.type != targetLevel)
							continue;
						if (c.scannedByMe)
							continue;
						bool danger = false;
						for (auto &m : monsters)
						{
							if (d.radarBlips[m->id] == d.radarBlips[c.id])
							{
								danger = true;
								break;
							}
						}
						if (danger)
							continue;
						if (old_target == &c)
							continue;
						if (target == nullptr)
							target = &c;
					}
				}
				if (target == nullptr)
				{
					for (auto &c : creatures)
					{
						if (c.dead)
							continue;
						if (c.type < 0)
							continue;
						if (c.scannedByMe)
							continue;
						bool danger = false;
						for (auto &m : visibleCreatures)
						{
							if (m->type >= 0)
								continue;
							if (d.radarBlips[m->id] == d.radarBlips[c.id])
							{
								danger = true;
								break;
							}
						}
						if (danger)
							continue;
						if (old_target == &c)
							continue;
						if (target == nullptr)
							target = &c;
					}
				}
				if (target != nullptr)
				{
					old_target = target;
					d.move(d.radarBlips[target->id], to_string(targetLevel) + " | " + (visible ? "VI  " : "NV  ") + to_string(target->id));
				}
			}
			else
			{
				d.setLowLight();
				int targetX = d.x;
				for (auto &c : visibleCreatures)
				{
					if (c->type < 0)
					{
						if (c->y > d.y) {
							if (abs(c->x - d.x) < 500) {
								targetX = c->x + (c->x - d.x);
							}
						}
					}
				}
				d.move(targetX, 0, "The cake is a lie");
			}
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