#include "Actor.h"
#include "StudentWorld.h"
#include "GameWorld.h"
#include <vector>

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

void IceMan::doSomething() {
    if (!isAlive()) return;

    StudentWorld* world = getWorld();
    if (!world)
        return;

    int key;
    if (GameController::getInstance().getLastKey(key)) {
        int newX = getX();
        int newY = getY();
        switch (key) {
        case KEY_PRESS_LEFT:
            if (newX > 0 && !world->isBoulderAt(newX - 1, getY())) {
                setDirection(left);
                newX--;
            }
            break;
        case KEY_PRESS_RIGHT:
            if (newX < VIEW_WIDTH - 1 && !world->isBoulderAt(newX + 1, getY())) {
                setDirection(right);
                newX++;
            }
            break;
        case KEY_PRESS_UP:
            if (newY < VIEW_HEIGHT - 1 && !world->isBoulderAt(getX(), newY + 1)) {
                setDirection(up);
                newY++;
            }
            break;
        case KEY_PRESS_DOWN:
            if (newY > 0 && !world->isBoulderAt(getX(), newY - 1)) {
                setDirection(down);
                newY--;
            }
            break;
        case KEY_PRESS_ESCAPE:
            takeDamage(100);
            return;
        case KEY_PRESS_SPACE:
            if (world->get_water() > 0) {
                world->squirtCharge(getX(), getY(), 3);
            }
            break;
        case 'z': case'Z':
            if (world->get_sonar() > 0) {
                world->sonarCharge(getX(), getY(), 12);
            }
            break;
        }

        if (newX != getX() || newY != getY()) {
            moveTo(newX, newY);
            for (int x = newX; x < newX + 4; ++x) {
                for (int y = newY; y < newY + 4; ++y) {
                    world->removeIce(x, y);
                }
            }
        }
    }
}

void IceMan::takeDamage(int amt) {
    setHitPoints(getHitPoints() - amt);
    if (getHitPoints() <= 0) {
        setDead();
        getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
    }
}

void RegularProtester::doSomething() {
    if (!isAlive())
        return;

    if (getRestingTicksLeft() > 0) {
        rest();
        return;
    }

    if (getState() == "leaving") {
        if (getX() == 60 && getY() == 60) {
            setDead();
            return;
        }

        shortestPath();
        nextMove();
        return;
    }

    StudentWorld* world = getWorld();
    for (auto* actor : world->getActors()) {
        if (actor->isIceman()) {
            if (isWithinRadius(actor->getX(), actor->getY(), 4)) {
                Direction dir = getDirection();
                if ((dir == left && getX() >= actor->getX()) ||
                    (dir == right && getX() <= actor->getX()) ||
                    (dir == up && getY() <= actor->getY()) ||
                    (dir == down && getY() >= actor->getY())) {
                    if (getTicksSinceShout() > 15) {
                        world->playSound(SOUND_PROTESTER_YELL);
                        actor->takeDamage(2);
                        resetTicksSinceShout();
                        return;
                    }
                }
            }
        }
    }
    resting_shouted++;
}

void RegularProtester::shortestPath() {
    const int exitX = 60;
    const int exitY = 60;

    if (getX() == exitX && getY() == exitY) {
        setDead();
        return;
    }

    struct Point {
        int x, y;
        Point(int x, int y) : x(x), y(y) {}
    };

    struct Node {
        Point pt;
        int dist;
        Node(int x, int y, int dist) : pt(x, y), dist(dist) {}
    };

    std::queue<Node> q;
    const int n = 64;
    bool visited[n][n] = { false };

    int rowNum[] = { -1, 0, 0, 1 };
    int colNum[] = { 0, -1, 1, 0 };

    q.push(Node(exitX, exitY, 0));
    visited[exitX][exitY] = true;

    StudentWorld* world = getWorld();

    while (!q.empty()) {
        Node curr = q.front();
        Point pt = curr.pt;

        q.pop();

        for (int i = 0; i < 4; i++) {
            int row = pt.x + rowNum[i];
            int col = pt.y + colNum[i];

            if (row >= 0 && row < n && col >= 0 && col < n && !visited[row][col]) {
                if (!world->isIceAt(row, col) && !world->isBoulderAt(row, col)) {
                    visited[row][col] = true;
                    q.push(Node(row, col, curr.dist + 1));

                    if (row == getX() && col == getY()) {
                        if (rowNum[i] == -1) setDirection(up);
                        else if (rowNum[i] == 1) setDirection(down);
                        else if (colNum[i] == -1) setDirection(left);
                        else if (colNum[i] == 1) setDirection(right);
                        return;
                    }
                }
            }
        }
    }
}

void Boulder::doSomething() {
    if (!isAlive()) return;

    StudentWorld* world = getWorld();

    switch (state) {
    case 0: { // Stable state
        bool iceBelow = false;
        for (int x = getX(); x < getX() + 4; ++x) {
            for (int y = getY() - 1; y >= getY() - 1 && y < getY(); ++y) {
                if (world->isIceAt(x, y)) {
                    iceBelow = true;
                    break;
                }
            }
            if (iceBelow) break;
        }
        if (!iceBelow) {
            state = 1; // Transition to waiting state
            tick = 0;
        }
        break;
    }
    case 1: { // Waiting state
        tick++;
        if (tick >= 30) {
            state = -1; // Transition to falling state
            world->playSound(SOUND_FALLING_ROCK);
        }
        break;
    }
    case -1: { // Falling state
        int newX = getX();
        int newY = getY() - 1;
        if (newY < 0 || world->isIceAt(newX, newY) || world->isBoulderAt(newX, newY)) {
            setDead();
            return;
        }
        moveTo(newX, newY);

        for (auto* actor : world->getActors()) {
            if (actor->isAlive() && actor->isWithinRadius(newX, newY, 3)) {
                if (actor->isIceman()) {
                    world->annoyPlayer(100);
                    setDead();
                }
                else if (actor->isProtester()) {
                    actor->takeDamage(100);
                    setDead();
                }
            }
        }
        break;
    }
    }
}

void Barrel::doSomething() {
    if (!isAlive()) return;

    StudentWorld* world = getWorld();

    if (!isVisible() && world->isPlayerInRadius(getX(), getY(), 4.0)) {
        setVisible(true);
        return;
    }

    if (isVisible() && world->isPlayerInRadius(getX(), getY(), 3.0)) {
        setDead();
        world->playSound(SOUND_FOUND_OIL);
        world->increaseScore(1000);
        world->pickedUpBarrel();
    }
}

void GoldNugget::doSomething() {
    if (!isAlive()) return;

    StudentWorld* world = getWorld();

    if (!isVisible() && world->isPlayerInRadius(getX(), getY(), 4.0)) {
        setVisible(true);
        return;
    }

    if (isVisible() && world->isPlayerInRadius(getX(), getY(), 3.0)) {
        setDead();
        world->playSound(SOUND_GOT_GOODIE);
        world->increaseScore(10);
        world->pickedUpNugget();
    }
}

void SonarKit::doSomething() {
    if (!isAlive())
        return;

    StudentWorld* world = getWorld();
    lifetime = world->GoodieT();

    for (auto* actor : world->getActors()) {
        if (actor->isIceman()) {
            if (isWithinRadius(actor->getX(), actor->getY(), 3)) {
                setDead();
                world->playSound(SOUND_GOT_GOODIE);

                world->pickedUpSonar();

                world->increaseScore(75);

                return;
            }

            lifetime--;
            if (lifetime <= 0) {
                setDead();
            }
        }
    }
}

void WaterPool::doSomething() {
    if (!isAlive())
        return;

    StudentWorld* world = getWorld();
    lifetime = world->GoodieT();

    for (auto* actor : world->getActors()) {
        if (actor->isIceman()) {
            if (isWithinRadius(actor->getX(), actor->getY(), 3)) {
                setDead();
                world->playSound(SOUND_GOT_GOODIE);

                world->pickedUpWater();

                world->increaseScore(75);

                return;
            }

            lifetime--;
            if (lifetime <= 0) {
                setDead();
            }
        }
    }
}

void Squirt::doSomething() {
    if (!isAlive()) {
        return;
    }

    StudentWorld* world = getWorld();
    Squirt::setDirection(world->squirtDir());
    if (world->get_water() <= 0)
    {
        setDead();
        return;
    }


    for (auto* actor : world->getActors()) {
        if (actor->isIceman()) {
            if (actor->getDirection() == right) {
                Squirt::setDirection(right);
                if (world->isBoulderAt(getX() + 1, getY()) || world->isIceAt(getX() + 1, getY())) {
                    setDead();
                    return;
                }
                moveTo(getX() + 1, getY());
            }
            else if (actor->getDirection() == left) {
                Squirt::setDirection(left);
                if (world->isBoulderAt(getX() - 1, getY()) || world->isIceAt(getX() - 1, getY())) {
                    setDead();
                    return;
                }
                moveTo(getX() - 1, getY());
            }
            else if (actor->getDirection() == up) {
                Squirt::setDirection(up);
                if (world->isBoulderAt(getX(), getY() + 1) || world->isIceAt(getX(), getY() + 1)) {
                    setDead();
                    return;
                }
                moveTo(getX(), getY() + 1);
            }
            else if (actor->getDirection() == down) {
                Squirt::setDirection(down);
                if (world->isBoulderAt(getX(), getY() - 1) || world->isIceAt(getX(), getY() - 1)) {
                    setDead();
                    return;
                }
                moveTo(getX(), getY() - 1);
            }
        }
    }

    if (travelDistance > 4) {
        setDead();
        return;
    }

    std::vector<Actor*> actors = world->getActors();
    for (auto* actor : actors) {
        if (RegularProtester* rp = dynamic_cast<RegularProtester*>(actor)) {
            if (rp->getX() == this->getX() + 3 || rp->getX() == this->getX() - 3 ||
                rp->getY() == this->getY() + 3 || rp->getY() == this->getY() - 3) {
                rp->takeDamage(2);
                this->setDead();
            }
        }
    }
    travelDistance++;
}