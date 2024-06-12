#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <vector>
#include <algorithm>

class Actor;
class IceMan;
class Ice;

class StudentWorld : public GameWorld {
public:
    StudentWorld(std::string assetDir) : GameWorld(assetDir), Iceman(nullptr) {
        for (int i = 0; i < VIEW_WIDTH; ++i) {
            for (int j = 0; j < VIEW_HEIGHT; ++j) {
                IceField[i][j] = nullptr;
                actor_pos[i][j] = 0;
            }
        }
    }

    virtual ~StudentWorld() {
        cleanUp();
    }

    virtual int init();
    virtual int move();
    virtual void cleanUp();
    bool overlap(int x, int y) const;
    void removeIce(int x, int y);
    void setDisplayText();
    bool isBoulderAt(int x, int y) const;
    GraphObject::Direction squirtDir();
    bool isIceAt(int x, int y) const;
    bool isPlayerInRadius(int x, int y, int radius) const;
    void annoyPlayer(int points);
    std::vector<Actor*> getActors() const;
    void sonarCharge(int x, int y, int radius);
    void squirtCharge(int x, int y, int radius);
    void addRandomGoodie();
    int GoodieT() const;
    void pickedUpBarrel() { num_barrels--; }
    void pickedUpNugget() { num_gold++; }
    void pickedUpSonar() { num_sonar+=2; }
    void pickedUpWater() { num_water += 5; }
    void spawnProtester();

    int get_lvl() const { return this->getLevel(); }
    int get_lives() const { return this->getLives(); }
    int get_health() const { return 100; }
    int get_water() const { return num_water; }
    int get_gold() const { return num_gold; }
    int get_oil() const { return num_barrels; }
    int get_sonar() const { return num_sonar; }
    int get_score() const { return this->getScore(); }
private:
    std::vector<Actor*> Actors;
    IceMan* Iceman;
    int num_protesters{};
    int protesterSpawnCounter{};
    int num_ice{};
    int num_boulders{};
    int num_gold{};
    int num_barrels{};
    int num_protestors{};
    int num_sonar{};
    int num_water{};
    int ticks_wait{};
    int ticks_since{};
    char actor_pos[64][64];
    Ice* IceField[64][64];
};

#endif // STUDENTWORLD_H_