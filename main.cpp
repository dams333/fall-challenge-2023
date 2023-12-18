#include <iostream>
#include <string>
#include <vector>
#include <list>
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

	Creature(int id, int color, int type) {
		this->id = id;
		this->color = color;
		this->type = type;
		this->x = 0;
		this->y = 0;
		this->dx = 0;
		this->dy = 0;
		this->visible = false;
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
		return *this;
	}

	void update(int x, int y, int dx, int dy, bool visible)
	{
		this->x = x;
		this->y = y;
		this->dx = dx;
		this->dy = dy;
		this->visible = visible;
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

	Drone(int id, int x, int y, int emergency, int battery, DroneOwner owner) {
		this->id = id;
		this->x = x;
		this->y = y;
		this->emergency = emergency;
		this->battery = battery;
		this->owner = owner;
	}

	Drone(const Drone &d) { *this = d; }

	Drone &operator=(const Drone &d)
	{
		id = d.id;
		x = d.x;
		y = d.y;
		emergency = d.emergency;
		battery = d.battery;
		owner = d.owner;
		return *this;
	}

	void update(int x, int y, int emergency, int battery)
	{
		this->x = x;
		this->y = y;
		this->emergency = emergency;
		this->battery = battery;
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
	int myScanCount;
	list<Creature *> myScans;
	int oppScanCount;
	list<Creature *> oppScans;
	int myDroneCount;
	list<Drone> myDrones;
	int oppDroneCount;
	list<Drone> oppDrones;
	int visibleCreatureCount;
	list<Creature *> visibleCreatures;

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
		myScanCount = g.myScanCount;
		myScans = g.myScans;
		oppScanCount = g.oppScanCount;
		oppScans = g.oppScans;
		myDroneCount = g.myDroneCount;
		myDrones = g.myDrones;
		oppDroneCount = g.oppDroneCount;
		oppDrones = g.oppDrones;
		visibleCreatureCount = g.visibleCreatureCount;
		visibleCreatures = g.visibleCreatures;
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
		
		cin >> myScanCount; cin.ignore();
		for (int i = 0; i < myScanCount; i++) {
			int creature_id;
			cin >> creature_id; cin.ignore();
			myScans.push_back(&getCreatureById(creature_id));
		}

		cin >> oppScanCount; cin.ignore();
		for (int i = 0; i < oppScanCount; i++) {
			int creature_id;
			cin >> creature_id; cin.ignore();
			oppScans.push_back(&getCreatureById(creature_id));
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
				myDrones.push_back(Drone(drone_id, drone_x, drone_y, emergency, battery, MY_DRONE));
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
				oppDrones.push_back(Drone(drone_id, drone_x, drone_y, emergency, battery, OPP_DRONE));
			else
				getDroneById(drone_id).update(drone_x, drone_y, emergency, battery);
		}

		//TODO
		int drone_scan_count;
        cin >> drone_scan_count; cin.ignore();
        for (int i = 0; i < drone_scan_count; i++) {
            int drone_id;
            int creature_id;
            cin >> drone_id >> creature_id; cin.ignore();
        }

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

		//TODO
		int radar_blip_count;
        cin >> radar_blip_count; cin.ignore();
        for (int i = 0; i < radar_blip_count; i++) {
            int drone_id;
            int creature_id;
            string radar;
            cin >> drone_id >> creature_id >> radar; cin.ignore();
        }
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
		cout << "WAIT 1" << endl;
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