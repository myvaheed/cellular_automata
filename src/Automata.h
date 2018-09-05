#pragma once


#include <stdexcept>
#include <iostream>
#include "omp.h"

class Cell {
public:
	int x = 0;
	int y = 0;
//питательность/уровень активатора
	float energy = 0;

	static float MAX_ENERGY;

	int countNeighbrs = 0;
	const int MAX_NEIGHBRS = 8;
	Cell** neighbrs = 0;

	//живой-неживой/возбужденный/микроорганизм
	bool isAlive = false;

	void init(int x, int y) {
		isAlive = false;
		neighbrs = new Cell*[MAX_NEIGHBRS];
		this->x = x;
		this->y = y;
	}

	void addNeighb(Cell* automata) {
		if (countNeighbrs > MAX_NEIGHBRS)
			throw std::runtime_error(" > maxNeigbrs");
		neighbrs[countNeighbrs++] = automata;
	}

	void getNeigbrs(Cell** &neighbrs, int& countNeighbrs) {
		neighbrs = this->neighbrs;
		countNeighbrs = this->countNeighbrs;
	}
	~Cell() {
		delete[] neighbrs;
	}
};


