#pragma once

#include "Rule.h"
#include "memory.h"
#include <fstream>
#include <iomanip>

template<int N>
class DeploymentCameraSystem {
	Cell** map;
	Cell** prevAutomatas;

	int countRules = 0;
	const int MAX_RULES = 12;

	//почему двойной указатель? изначально была идея сделать в программе кнопку для переключения правил,
	//но потом просто решил собрать для каждой правилы отдельную прогу, можешь сделать кнопку на примере кнопки Run/Pause,
	//при нажатии на которой последовательно будут меняться правила, ну и придется там еще с дополнительной инициализацией повозиться при изменении правил
	Rule** rules;
	int currentRule = 0;

public:

	enum RulesSystem {
		GAME_LIFE, NEURAL_NETWORK, NATURE, SMART_NATURE
	};

	Cell** getMapWithCameras() {
		return map;
	}

	void swap(Cell** &automatas, Cell** &prevAutomatas) {
		Cell** forSwapping = 0;
		forSwapping = automatas;
		automatas = prevAutomatas;
		prevAutomatas = forSwapping;
	}

	void reverseStateCell(int i, int j) {
		if (!map)
			return;
		map[i][j].isAlive = !map[i][j].isAlive;
	}

	enum DATA_TYPE {
		NUMBER_ALIVE,
		COMMON_ENERGY
	};

	void writeDataToFile(const char* filename, int indexIter, DATA_TYPE dataType) {
		std::fstream file(filename, std::ios_base::out | std::ios_base::app);
		file << std::setiosflags(std::ios::fixed) << std::setprecision(6);

		double value = 0;
		switch(dataType) {
		case NUMBER_ALIVE:
#pragma omp parallel for
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					if (map[i][j].isAlive) {
						value++;
					}
				}
			}
			break;
		case COMMON_ENERGY:
#pragma omp parallel for
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					value += map[i][j].energy;
				}
			}
			break;
		}

		file << indexIter << "\t" << value << "\n";

		file.close();
	}

	DeploymentCameraSystem(RulesSystem rulesSystem) {

		map = new Cell*[N];
		prevAutomatas = new Cell*[N];

		for (int i = 0; i < N; i++) {
			map[i] = new Cell[N];
			prevAutomatas[i] = new Cell[N];
		}
		
		//init automatas
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				map[i][j].init(i, j);
				prevAutomatas[i][j].init(i, j);
			}
		}

		omp_set_num_threads(2);

		//neigbrs init
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				if (i > 0) {
					map[i][j].addNeighb(&map[i - 1][j]);
					map[i - 1][j].addNeighb(&map[i][j]);

					prevAutomatas[i][j].addNeighb(&prevAutomatas[i - 1][j]);
					prevAutomatas[i - 1][j].addNeighb(&prevAutomatas[i][j]);

					if (j > 0) {
						map[i][j].addNeighb(&map[i - 1][j - 1]);
						map[i - 1][j - 1].addNeighb(&map[i][j]);

						prevAutomatas[i][j].addNeighb(&prevAutomatas[i - 1][j - 1]);
						prevAutomatas[i - 1][j - 1].addNeighb(&prevAutomatas[i][j]);
					}
				}
				if (j > 0) {
					map[i][j].addNeighb(&map[i][j - 1]);
					map[i][j - 1].addNeighb(&map[i][j]);

					prevAutomatas[i][j].addNeighb(&prevAutomatas[i][j - 1]);
					prevAutomatas[i][j - 1].addNeighb(&prevAutomatas[i][j]);

					if (i != N - 1) {
						map[i][j].addNeighb(&map[i + 1][j - 1]);
						map[i + 1][j - 1].addNeighb(&map[i][j]);

						prevAutomatas[i][j].addNeighb(
								&prevAutomatas[i + 1][j - 1]);
						prevAutomatas[i + 1][j - 1].addNeighb(
								&prevAutomatas[i][j]);
					}
				}
			}
		}

		rules = new Rule*[MAX_RULES];

		switch (rulesSystem) {
		case GAME_LIFE:
			rules[countRules++] = new GameLifeRule<N>(map);
			break;
		case NEURAL_NETWORK:
			rules[countRules++] = new NeuralNetworkRule<N>(map);
			break;
		case NATURE:
			rules[countRules++] = new OrgSystem<N>(&map);
			break;
		case SMART_NATURE:
			rules[countRules++] = new SmartOrgSystem<N>(&map);
			break;
		}
	}

	void step() {
		swap(map, prevAutomatas);
		rules[currentRule]->prerule(map);
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				rules[currentRule]->rule(&map[i][j], &prevAutomatas[i][j]);
			}
		}
	}

	~DeploymentCameraSystem() {
		for (int i = 0; i < N; i++) {
			delete map[i];
		}
		delete[] map;

		for (int i = 0; i < N; i++) {
			delete prevAutomatas[i];
		}
		delete[] prevAutomatas;

		for (int i = 0; i < countRules; i++) {
			delete[] rules[i];
		}
		delete[] rules;
	}
};
