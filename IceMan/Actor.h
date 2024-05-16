#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
class Actor {
private:
	int m_row;
	int m_col;
	bool m_active;
public:
	//Constructor
	Actor(int r, int c);

	//Accessors
	int row() const;
	int col() const;
	bool isActive() const;

};

#endif // ACTOR_H_
