#include "StudentWorld.h"
#include <string>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <algorithm>
using namespace std;

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

int StudentWorld::init() {
	Actors.clear();
	num_barrels = 0;
	num_gold = 0;
	num_sonar = 1;
	num_water = 5;
	num_protesters = 0;
	protesterSpawnCounter = 0;
	setDisplayText();

	Iceman = new IceMan(30, 60, this);
	Actors.push_back(Iceman);

	for (int x = 0; x < VIEW_WIDTH; ++x) {
		for (int y = 0; y < 60; ++y) {
			if (!(x >= 30 && x <= 33 && y >= 4)) {
				IceField[x][y] = new Ice(x, y, this);
			}
			else {
				IceField[x][y] = nullptr;
			}
		}
	}

	int current_level_number = getLevel();
	int B = min(current_level_number / 2 + 2, 9);
	int L = min(2 + current_level_number, 21);
	int G = max(5 - current_level_number / 2, 2);

	srand(static_cast<unsigned int>(time(0)));

	for (int i = 0; i < B; ++i) {
		int x, y;
		bool boulder_placed = false;
		while (!boulder_placed) {
			x = rand() % 61;
			y = rand() % 37 + 15;
			if ((x < 26 || x > 37) && !overlap(x, y)) {
				Boulder* boulder = new Boulder(x, y, this);
				Actors.push_back(boulder);
				for (int bx = x; bx < x + 4 && bx < 60; ++bx) {
					for (int by = y; by < y + 4 && by < 60; ++by) {
						removeIce(bx, by);
					}
				}
				boulder_placed = true;
			}
		}
	}

	for (int i = 0; i < L; ++i) {
		int x, y;
		bool barrel_placed = false;
		while (!barrel_placed) {
			x = rand() % 61;
			y = rand() % 57;
			if ((x < 26 || x > 37) && !overlap(x, y)) {
				Barrel* barrel = new Barrel(x, y, this);
				Actors.push_back(barrel);
				num_barrels++;
				barrel_placed = true;
			}
		}
	}

	for (int i = 0; i < G; ++i) {
		int x, y;
		bool nugget_placed = false;
		while (!nugget_placed) {
			x = rand() % 61;
			y = rand() % 57;
			if ((x < 26 || x > 37) && !overlap(x, y)) {
				GoldNugget* goldNugget = new GoldNugget(x, y, this);
				Actors.push_back(goldNugget);
				nugget_placed = true;
			}
		}
	}

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() {
	setDisplayText();
	spawnProtester();

	for (auto* actor : Actors) {
		if (actor->isAlive()) {
			actor->doSomething();

			if (!Iceman->isAlive()) {
				decLives();
				return GWSTATUS_PLAYER_DIED;
			}

			if (num_barrels == 0) {
				playSound(SOUND_FINISHED_LEVEL);
				return GWSTATUS_FINISHED_LEVEL;
			}
		}
	}

	auto it = Actors.begin();
	while (it != Actors.end()) {
		if (!(*it)->isAlive()) {
			delete* it;
			it = Actors.erase(it);
		}
		else {
			++it;
		}
	}

	if (!Iceman->isAlive()) {
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	if (num_barrels == 0) {
		playSound(SOUND_FINISHED_LEVEL);
		return GWSTATUS_FINISHED_LEVEL;
	}

	addRandomGoodie();

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp() {
	for (auto* actor : Actors) {
		delete actor;
	}

	Actors.clear();

	for (int x = 0; x < VIEW_WIDTH; ++x) {
		for (int y = 0; y < 60; ++y) {
			if (IceField[x][y] != nullptr) {
				delete IceField[x][y];
				IceField[x][y] = nullptr;
			}
		}
	}
}

bool StudentWorld::overlap(int x, int y) const {
	for (const auto* actor : Actors) {
		int ax = actor->getX();
		int ay = actor->getY();
		if (sqrt((x - ax) * (x - ax) + (y - ay) * (y - ay)) <= 6.0) {
			return true;
		}
	}
	return false;
}

void StudentWorld::removeIce(int x, int y) {
	if (x >= 0 && x < VIEW_WIDTH && y >= 0 && y < VIEW_HEIGHT) {
		if (IceField[x][y] != nullptr) {
			playSound(SOUND_DIG);
			delete IceField[x][y];
			IceField[x][y] = nullptr;
		}
	}
}

void StudentWorld::setDisplayText() {
	int level = get_lvl();
	int lives = get_lives();
	int health = get_health();
	int water = get_water();
	int gold = get_gold();
	int oilLeft = get_oil();
	int sonar = get_sonar();
	int score = get_score();

	std::ostringstream oss;
	oss << "Lvl: " << std::setw(2) << level
		<< "  Lives: " << lives
		<< "  Hlth: " << std::setw(3) << health << '%'
		<< "  Wtr: " << std::setw(2) << water
		<< "  Gld: " << std::setw(2) << gold
		<< "  Oil Left: " << std::setw(2) << oilLeft
		<< "  Sonar: " << std::setw(2) << sonar
		<< "  Scr: " << std::setfill('0') << std::setw(6) << score;

	setGameStatText(oss.str());
}

bool StudentWorld::isBoulderAt(int x, int y) const {
	for (const auto& actor : Actors) {
		if (Boulder* boulder = dynamic_cast<Boulder*>(actor)) {
			if (x >= boulder->getX() && x < boulder->getX() + 4 &&
				y >= boulder->getY() && y < boulder->getY() + 4) {
				return true;
			}
		}
	}
	return false;
}

GraphObject::Direction StudentWorld::squirtDir()
{
	for (const auto* actor : Actors) {
		if (actor->isIceman()) {
			return actor->getDirection();
		}
	}
	return GraphObject::Direction();
}

bool StudentWorld::isIceAt(int x, int y) const {
	return x >= 0 && x < VIEW_WIDTH && y >= 0 && y < 60 && IceField[x][y] != nullptr;
}

bool StudentWorld::isPlayerInRadius(int x, int y, int radius) const {
	return (Iceman->getX() - x) * (Iceman->getX() - x) + (Iceman->getY() - y) * (Iceman->getY() - y) <= radius * radius;
}

void StudentWorld::annoyPlayer(int points) {
	Iceman->takeDamage(points);
}

std::vector<Actor*> StudentWorld::getActors() const {
	return Actors;
}

void StudentWorld::sonarCharge(int x, int y, int radius) {
	for (auto* actor : Actors) {
		if (actor->isWithinRadius(x, y, radius)) {
			actor->setVisible(true);
			playSound(SOUND_SONAR);
		}
	}
	num_sonar--;
}

void StudentWorld::squirtCharge(int x, int y, int radius) {
	for (const auto* actor : Actors) {
		if (actor->isIceman()) {
			Squirt* squirt = new Squirt(actor->getX(), actor->getY(), this);
			Actors.push_back(squirt);
			playSound(SOUND_PLAYER_SQUIRT);
		}
	}
	num_water--;
}

void StudentWorld::addRandomGoodie() {
	int G = getLevel() * 25 + 300;
	if (rand() % G == 0) {
		if (rand() % 5 == 0) {
			SonarKit* sonarKit = new SonarKit(0, 60, this);
			Actors.push_back(sonarKit);
		}
		else {
			int x, y;
			bool waterPlaced = false;
			while (!waterPlaced) {
				x = rand() % 61;
				y = rand() % 61;
				if (!isBoulderAt(x, y) && !isIceAt(x, y)) {
					WaterPool* waterPool = new WaterPool(x, y, this);
					Actors.push_back(waterPool);
					waterPlaced = true;
				}
			}
		}
	}
}

int StudentWorld::GoodieT() const {
	return max(100, (300 - 10 * get_lvl()));
}

void StudentWorld::spawnProtester() {
	int T = std::max(25, 20 - get_lvl());

	protesterSpawnCounter++;

	if (protesterSpawnCounter >= T) {
		RegularProtester* protester = new RegularProtester(60, 60, this);
		Actors.push_back(protester);

		protesterSpawnCounter = 0;
	}
	else return;
}