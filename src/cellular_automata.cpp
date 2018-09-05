/*
 * main.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: myvaheed
 */

#include "CAutomateSystem.h"
#include "Renderer.h"

const int N = 100;

bool bPause = true;

int main()
{
	//удаляем файл с предыдущими результатами
	const char* filename = "SmartNature_NumberAlive.txt";
	std::string removeCmd = std::string("rm ") + filename;
	system(removeCmd.c_str());

	DeploymentCameraSystem<N> caSystem(DeploymentCameraSystem<N>::SMART_NATURE);
	Renderer<N> renderer;
	renderer.init();

	//флаг для возможности обработать событие удерживания кнопки мыши(для ручного рисования клеток в сетке)
	bool isMouseSelection = false;

	//для вывода в файл результатов вычислений через определенный такт времени
	unsigned long int iteration = 0;
	while(true) {

		SDL_Event e;

		// обработка нажатий
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				SDL_Log("Program quit");
				return 0;
			}
			int x, y;


			switch (e.type) {
			case SDL_MOUSEBUTTONUP:

				if (e.button.button == SDL_BUTTON_LEFT) {
					isMouseSelection = false;
				}
				break;

			case SDL_MOUSEBUTTONDOWN:

				if (e.button.button == SDL_BUTTON_LEFT) {
					isMouseSelection = true;
				}

				x = e.button.x;
				y = e.button.y;


				if ((x + 1) <= renderer.getWidthAutomata() * N
						&& (y + 1) <= renderer.getWidthAutomata() * N) {
					//меняем состояние автомата
					caSystem.reverseStateAutomata(
							x / renderer.getWidthAutomata(),
							y / renderer.getWidthAutomata());
				}

				//обработка события нажатия на кнопку Run/Pause
				renderer.handleEvent(x, y, bPause);

				break;

			case SDL_MOUSEMOTION:

				if (isMouseSelection) {
					x = e.button.x;
					y = e.button.y;
					if ((x + 1) <= renderer.getWidthAutomata() * N
							&& (y + 1) <= renderer.getWidthAutomata() * N) {
						caSystem.reverseStateAutomata(
								x / renderer.getWidthAutomata(),
								y / renderer.getWidthAutomata());
					}

					renderer.handleEvent(x, y, bPause);
				}
				break;
			}

		}

	    if (!bPause) {
	    	caSystem.step();

	    	//вывод в файл результатов вычисления
//			iteration++;
//			if (iteration % 50 == 0) {
//				caSystem.writeDataToFile(filename, iteration,
//						CAutomataSystem<N>::DATA_TYPE::NUMBER_ALIVE);
//			}
//
//			if (iteration == 5000) {
//				return 0;
//			}
	    }

		renderer.render(caSystem.getAutomatas());


	}

	return 0;
}



