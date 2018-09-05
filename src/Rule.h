#pragma once

#include "Automata.h"
#include "cmath"

class Rule {
public:
	virtual ~Rule() {}

	virtual void prerule(Cell** automata) = 0;
	//set automata's state by state of prevAutomata's state
	virtual void rule(Cell* automata, Cell* prevAutomata) = 0;
};

template<int N>
class GameLifeRule : public Rule {

public:

	GameLifeRule(Cell** automatas) {
		Cell::MAX_ENERGY = 1.0;
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				automatas[i][j].isAlive = rand() % 2 == 0 ? true : false;
			}
		}
	}

	void prerule(Cell** automata) {}

	void rule(Cell* automata, Cell* prevAutomata) {

		Cell** neighbrs;
		int countNeighbrs;
		prevAutomata->getNeigbrs(neighbrs, countNeighbrs);

		int countLivesAutomatas = 0;
		for (int i = 0; i < countNeighbrs; i++) {
			if (neighbrs[i]->isAlive) {
				countLivesAutomatas++;
			}
		}

		if (prevAutomata->isAlive) {
			if (countLivesAutomatas < 2 || countLivesAutomatas > 3)
				automata->isAlive = false;
			else
				automata->isAlive = true;
		}
		else {
			if (countLivesAutomatas == 3)
				automata->isAlive = true;
			else
				automata->isAlive = false;
		}
	}
};

template<int N>
class NeuralNetworkRule : public Rule {
	static const int REPOSE_STATE = 0;
	static const int RELAX_STATE = 1;
	static const int ACTIVE_STATE = 2;

	const int LEVEL_FOR_ACTIVATE = 3;
	const int MAX_RELAX_AGE = 8;
	const int MAX_ACTIVE_AGE = 5;

	int state[N][N]; // 0 - dead, 1 - sleep, 2 - active
	int activeAge[N][N];
	int relaxAge[N][N];

//для пульсирующей точки
	int iter = 0;

public:

	NeuralNetworkRule<N>(Cell** automatas) {
		//обязательно переобозначить MAX_ENERGY! - с ней работает Renderer для правильной отрисовки уровня "энергии" в клетке
		Cell::MAX_ENERGY = 1.0;
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				state[i][j] = REPOSE_STATE;
				activeAge[i][j] = 0;
				relaxAge[i][j] = 0;
			}
		}

		//double front
		int j = N / 8;
		for (int i = 0; i < N; i++) {
			automatas[i][j].isAlive = true;
			automatas[i][N - j].isAlive = true;
		}
	}

	void prerule(Cell** automata) {
		iter++;
	}

	void rule(Cell* automata, Cell* prevAutomata) {
		Cell** neighbrs;
		int countNeighbrs;
		prevAutomata->getNeigbrs(neighbrs, countNeighbrs);

		int x = prevAutomata->x;
		int y = prevAutomata->y;

		if (prevAutomata->isAlive) {
			//если значение активности(возбуждения) установлен юзером (через клик мышки)
			//а состояние не поменялось
			if (state[x][y] == REPOSE_STATE || state[x][y] == RELAX_STATE) {
				state[x][y] = ACTIVE_STATE;
				activeAge[x][y] = 0;
			}
			if (activeAge[x][y]++ > MAX_ACTIVE_AGE) {
				state[x][y] = RELAX_STATE;
				relaxAge[x][y] = 0;
			}
		} else {
			//если значение активности(возбуждения) установлен юзером (через клик мышки)
			//а состояние не поменялось
			if (state[x][y] == ACTIVE_STATE) {
				state[x][y] = REPOSE_STATE;
			}
			//чтоб была возможность обработать случай с RELAX_STATE в REPOSE_STATE в этом же шаге
			for (;;) {
				if (state[x][y] == REPOSE_STATE) {
					double activatorLevel = prevAutomata->energy;

					for (int i = 0; i < countNeighbrs; i++) {
						activatorLevel += neighbrs[i]->energy;
					}
					if (activatorLevel >= LEVEL_FOR_ACTIVATE) {
						state[x][y] = ACTIVE_STATE;
						activeAge[x][y] = 0;
					}
					break;
				} else {
					if (relaxAge[x][y]++ > MAX_RELAX_AGE) {
						state[x][y] = REPOSE_STATE;
						continue;
					}
					break;
				}
			}
		}

		switch (state[x][y]) {
		case REPOSE_STATE:

			automata->isAlive = false;
			automata->energy = std::max(prevAutomata->energy * 0.7, 0.);

			break;
		case RELAX_STATE:

			automata->isAlive = false;
			automata->energy = std::max(prevAutomata->energy * 0.7, 0.);

			break;
		case ACTIVE_STATE:

			automata->energy = 1;
			automata->isAlive = true;

			break;
		}

		//impulsator
		/*if (iter % 15 == 0) {
			if (((x >= N/8) && (x < (N/8 + 3))) && (y >= (N/4) && (y < (N/4) + 3))) {
				automata->isAlive = true;
			}
		}*/
	}
};

template<int N>
class OrgSystem : public Rule {

	const int CELL_ENERGY_MAX = 10;
	const int INC_ENERGY = 1;
	const int INC_FOOD = 5;
	const int DEC_FOOD = 2;
	const int MAX_FOOD = 35;
	const int MAX_AGE = 35;
	const int MATURATY_AGE = 3;
	const int PARTURATION_ENERGY = 3;

	float energyOfOrg[N][N];
	int ageOfOrg[N][N];

	//массив для хранения следующих по времени позиций автоматов
	bool preparedToNext[N][N];

	Cell** *automatas;
public:

	OrgSystem<N>(Cell** *automatas) : automatas(automatas) {
		Cell::MAX_ENERGY = CELL_ENERGY_MAX;
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				energyOfOrg[i][j] = 0;
				ageOfOrg[i][j] = 0;
				(*automatas)[i][j].energy = CELL_ENERGY_MAX;
			}
		}
	}
	void prerule(Cell** automatas) {
		//clear preparedToNext[][]
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++)
				preparedToNext[i][j] = false;
		}
	}


	void rule(Cell* automata, Cell* prevAutomata) {
		int x = prevAutomata->x;
		int y = prevAutomata->y;

		automata->energy = std::min((float)CELL_ENERGY_MAX, prevAutomata->energy + (float)INC_ENERGY);

		//если это не та ячейка, которая "забита" некоторым автоматом для прыжка в следующий такт времени
		if (!preparedToNext[x][y]) {
			//изначально предполагаем что клетка по любому будет мертвый(не будет живого организма)
			automata->isAlive = false;
		} else
			return;

		if (prevAutomata->isAlive) {
			if  (ageOfOrg[x][y]++ > MAX_AGE) {
				return;
			}

			float food = std::min(prevAutomata->energy, (float) INC_FOOD);
			energyOfOrg[x][y] = std::min(energyOfOrg[x][y] + food, (float) MAX_FOOD);
			automata->energy = prevAutomata->energy - food;

			energyOfOrg[x][y] = energyOfOrg[x][y] - DEC_FOOD;

			if (energyOfOrg[x][y] <= 0) {
				return;
			}

			Cell** neighbrs;
			int countNeighbrs;
			prevAutomata->getNeigbrs(neighbrs, countNeighbrs);

			int countFreeCells = 0;
			int* indicesOfFreeCells = new int[countNeighbrs];
			for (int i = 0; i < countNeighbrs; i++) {
				//if neigbor is free and not busy cell on the current iteration
				if (!neighbrs[i]->isAlive && !(*automatas)[neighbrs[i]->x][neighbrs[i]->y].isAlive) {
					indicesOfFreeCells[countFreeCells++] = i;
				}
			}

			if (countFreeCells) {
				int indexOfNextCell = indicesOfFreeCells[rand() % countFreeCells];
				int nextX = neighbrs[indexOfNextCell]->x;
				int nextY = neighbrs[indexOfNextCell]->y;

				//забронировали место
				preparedToNext[nextX][nextY] = true;
				(*automatas)[nextX][nextY].isAlive = true;
				ageOfOrg[nextX][nextY] = ageOfOrg[x][y];
				energyOfOrg[nextX][nextY] = energyOfOrg[x][y];
				delete[] indicesOfFreeCells;

				if (ageOfOrg[nextX][nextY] >= MATURATY_AGE) {
					//birth child
					energyOfOrg[nextX][nextY] -= PARTURATION_ENERGY;
					automata->isAlive = true;
					ageOfOrg[x][y] = 0;
					energyOfOrg[x][y] = 0;
				}

			} else {
				automata->isAlive = true;
			}

		}
 	}
};

template<int N>
class SmartOrgSystem : public Rule {

	const int CELL_ENERGY_MAX = 10;
	const int INC_ENERGY = 1;
	const int INC_FOOD = 5;
	const int DEC_FOOD = 2;
	const int MAX_FOOD = 35;
	const int MAX_AGE = 35;
	const int MATURATY_AGE = 3;
	const int PARTURATION_ENERGY = 3;

	float energyOfOrg[N][N];
	int ageOfOrg[N][N];
	bool preparedToNext[N][N];

	Cell** *automatas; //указатель на автоматы в настоящий момент времени(не в предыдущий)
public:

	SmartOrgSystem<N>(Cell** *automatas) : automatas(automatas) {
		Cell::MAX_ENERGY = CELL_ENERGY_MAX;
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				energyOfOrg[i][j] = 0;
				ageOfOrg[i][j] = 0;
				//питательность среды в начале максимальна
				(*automatas)[i][j].energy = CELL_ENERGY_MAX;
			}
		}
	}
	void prerule(Cell** automatas) {
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++)
				preparedToNext[i][j] = false;
		}
	}


	void rule(Cell* automata, Cell* prevAutomata) {
		int x = prevAutomata->x;
		int y = prevAutomata->y;

		automata->energy = std::min((float)CELL_ENERGY_MAX, prevAutomata->energy + (float)INC_ENERGY);

		//если это не та ячейка, которая "забита" некоторым автоматом для прыжка в следующий такт времени
		if (!preparedToNext[x][y]) {
			//изначально предполагаем что клетка по любому будет мертвый(не будет живого организма)
			automata->isAlive = false;
		} else
			return;


		if (prevAutomata->isAlive) {
			if  (ageOfOrg[x][y]++ > MAX_AGE) {
				return;
			}

			float food = std::min(prevAutomata->energy, (float) INC_FOOD);
			energyOfOrg[x][y] = std::min(energyOfOrg[x][y] + food, (float) MAX_FOOD);
			automata->energy = prevAutomata->energy - food;

			energyOfOrg[x][y] = energyOfOrg[x][y] - DEC_FOOD;

			if (energyOfOrg[x][y] <= 0) {
				return;
			}

			Cell** neighbrs;
			int countNeighbrs;
			prevAutomata->getNeigbrs(neighbrs, countNeighbrs);

			int countFreeCells = 0;
			int* indicesOfFreeCells = new int[countNeighbrs];
			for (int i = 0; i < countNeighbrs; i++) {
				//if neigbor is free and not busy cell on the current iteration
				if (!neighbrs[i]->isAlive && !(*automatas)[neighbrs[i]->x][neighbrs[i]->y].isAlive) {
					indicesOfFreeCells[countFreeCells++] = i;
				}
			}

			if (countFreeCells) {
				int indexOfNextCell = indicesOfFreeCells[0];
				float maxFreeEnergy = neighbrs[indicesOfFreeCells[0]]->energy;
				for (int i = 1; i < countFreeCells; i++) {
					if (neighbrs[indicesOfFreeCells[i]]->energy >= maxFreeEnergy) {
						//добавляем элемент случайности, чтобы не прыгал постоянно в клетку с максимальной питательностью в определенном направлении
						if (neighbrs[indicesOfFreeCells[i]]->energy == maxFreeEnergy && rand() % 2 == 0)
							continue;
						indexOfNextCell = indicesOfFreeCells[i];
						maxFreeEnergy = neighbrs[indicesOfFreeCells[i]]->energy;
					}
				}
				int nextX = neighbrs[indexOfNextCell]->x;
				int nextY = neighbrs[indexOfNextCell]->y;

				//забронировали место
				preparedToNext[nextX][nextY] = true;
				(*automatas)[nextX][nextY].isAlive = true;

				ageOfOrg[nextX][nextY] = ageOfOrg[x][y];
				energyOfOrg[nextX][nextY] = energyOfOrg[x][y];
				delete[] indicesOfFreeCells;

				if (ageOfOrg[nextX][nextY] >= MATURATY_AGE) {
					//birth child
					energyOfOrg[nextX][nextY] -= PARTURATION_ENERGY;
					automata->isAlive = true;
					ageOfOrg[x][y] = 0;
					energyOfOrg[x][y] = 0;
				}

			} else {
				//нет свободного места куда прыгать - остаемся на месте
				automata->isAlive = true;
			}
		}
 	}
};
