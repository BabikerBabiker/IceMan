#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameWorld.h"
#include "GameController.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <queue>
#include <vector>
#include <algorithm>
class StudentWorld;

class Actor : public GraphObject {
public:
    Actor(int col, int row, int id, Direction d, float size, unsigned int depth, StudentWorld* world)
        : GraphObject(id, col, row, d, size, depth), sw(world), is_alive(true)
    {
    }

    StudentWorld* getWorld() const {
        return sw;
    }

    bool isAlive() const {
        return is_alive;
    }

    virtual void setDead() {
        is_alive = false;
    }

    virtual bool isProtester() const {
        return false;
    }

    virtual bool isIceman() const { return false; }

    virtual bool isBarrel() const { return false; }

    virtual bool isNugget() const { return false; }

    virtual void takeDamage(int amt) {}

    virtual void Bribed() {}

    virtual std::string getState() const { return ""; }

    bool isWithinRadius(int x, int y, int radius) const {
        int deltaX = getX() - x;
        int deltaY = getY() - y;
        return deltaX * deltaX + deltaY * deltaY <= radius * radius;
    }

    virtual void doSomething() = 0;

private:
    StudentWorld* sw;
    bool is_alive;
};

class Ice : public Actor {
public:
    Ice(int x, int y, StudentWorld* world)
        : Actor(x, y, IID_ICE, right, .25, 3, world)
    {
        setVisible(true);
    }

    virtual ~Ice() {}
    virtual void doSomething() override {}
};

class Annoyed : public Actor {
public:
    Annoyed(int col, int row, int id, Direction d, float size, unsigned int depth, StudentWorld* world, int hitPoints)
        : Actor(col, row, id, d, size, depth, world), hp(hitPoints)
    {
        setVisible(true);
    }

    virtual void giveUp() = 0;

    virtual void takeDamage(int amt) {
        hp -= amt;
        if (hp <= 0)
            giveUp();
    }

    int getHitPoints() const {
        return hp;
    }

    void setHitPoints(int hitPoints) {
        hp = hitPoints;
    }

    virtual void setDead() {
        Actor::setDead();
        setHitPoints(0);
    }

private:
    int hp;
};

class IceMan : public Annoyed {
public:
    IceMan(int x, int y, StudentWorld* world)
        : Annoyed(x, y, IID_PLAYER, right, 1.0, 0, world, 100), sonar(1), gold(0), water(5)
    {
        setHitPoints(100);
    }

    virtual ~IceMan() {}
    void addGold() { gold++; }
    int getGold() const { return gold; }

    void addSonar() { sonar++; }
    int getSonar() const { return sonar; }

    void addWater() { water += 5; }
    int getSquirts() const { return water; }

    virtual void doSomething() override;
    virtual bool isIceman() const override { return true; }

    virtual void giveUp() {}
    void takeDamage(int amt);

    virtual void setDead() override {
        Annoyed::setDead();
    }

private:
    int sonar;
    int gold;
    int water;
};

class RegularProtester : public Annoyed {
public:
    RegularProtester(int x, int y, StudentWorld* world)
        : Annoyed(x, y, IID_PROTESTER, left, 1, 0, world, 5), ticks_wait(0), state(""), resting_left(0), resting_shouted(0), resting_turned(0), steps_inDir(0)
    {
        setVisible(true);
    }

    void shortestPath();

    virtual void doSomething() override;

    virtual bool isProtester() const override { return true; }

    int getRestingTicksLeft() const { return resting_left; }
    void rest() { resting_left--; }

    int getTicksSinceShout() const { return resting_shouted; }

    void nextMove() {
        resting_turned++;
        resting_shouted++;
        resting_left = ticks_wait;
    }

    void resetDir() { steps_inDir = 0; }

    void decSteps() { steps_inDir--; }

    int getSteps() const { return steps_inDir; }
    void resetSteps() { steps_inDir = 0; }

    void newDir() { steps_inDir = rand() % 53 + 8; }

    int getTicksSinceTurned() const { return resting_turned; }

    void resetTicksSinceTurned() { resting_turned = 0; }

    void resetTicksSinceShout() { resting_shouted = 0; }

    virtual void giveUp() { setDead(); }

    virtual bool isHardcoreProtester() const { return false; }

    void setState(const std::string& newState) {
        state = newState;
        resting_left = 0;
    }

    virtual std::string getState() const override { return state; }

private:
    int ticks_wait;
    std::string state;
    int resting_left;
    int resting_shouted;
    int resting_turned;
    int steps_inDir;
};

class Boulder : public Actor {
public:
    Boulder(int x, int y, StudentWorld* world)
        : Actor(x, y, IID_BOULDER, down, 1.0, 1, world), state(0), tick(0)
    {
        setVisible(true);
    }

    virtual void doSomething() override;

private:
    int state;  // negative = falling; 0 = still; 1 = waiting
    int tick;
};

class GoldNugget : public Actor {
public:
    GoldNugget(int x, int y, StudentWorld* world, bool visible = true)
        : Actor(x, y, IID_GOLD, right, 1.0, 2, world)
    {
    }

    virtual void doSomething() override;

    bool isNugget() const override { return true; }
};

class Barrel : public Actor {
public:
    Barrel(int x, int y, StudentWorld* world)
        : Actor(x, y, IID_BARREL, right, 1.0, 2, world) {}

    virtual bool isBarrel() const override { return true; }
    
    virtual void doSomething() override;
private:

};

class SonarKit : public Actor {
public:
    SonarKit(int x, int y, StudentWorld* world)
        : Actor(x, y, IID_SONAR, right, 1.0, 2, world) {
        setVisible(true);
        lifetime = 0;
    }

    virtual void doSomething() override;
private:
    int lifetime;
};

class WaterPool : public Actor {
public:
    WaterPool(int x, int y, StudentWorld* world)
        : Actor(x, y, IID_WATER_POOL, right, 1.0, 2, world) {
        setVisible(true);
        lifetime = 0;
    }

    virtual void doSomething() override;
private:
    int lifetime;
};

class Squirt : public Actor {
public:
    Squirt(int x, int y, StudentWorld* world) : Actor(x, y, IID_WATER_SPURT, right, 1.0, 2, world) {
        setVisible(true);
        travelDistance = 0;
    }

    virtual void doSomething() override;
private:
    int travelDistance;
};
#endif // ACTOR_H_