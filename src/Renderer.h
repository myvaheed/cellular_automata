/*
 * Renderer.h
 *
 *  Created on: Dec 7, 2017
 *      Author: myvaheed
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#define LINUX

#include "Automata.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"



template<int N>
class Renderer {
	int numMaxCamera = 6;

	//основные объекты при отрисовке
	SDL_Window* win;
	SDL_Renderer* ren;

	SDL_Texture* buildingCellTxt;
	SDL_Texture* emptyCellTxt;
	SDL_Texture** cameraCellTxt;

	//автоматы рисуем в cells
	SDL_Rect cells[N][N];

	SDL_Texture* pause;
	SDL_Texture* run;

	//переменная чтоб указать либо на pause, либо на run.
	SDL_Texture* player;

	//Кнопки Run/Pause рисуем в rectPlayer
	SDL_Rect rectPlayer;

	//опции экрана
	SDL_DisplayMode displayMode;

	int widthCell = 0;

public:

	~Renderer() {
		finalize();
	}
//обработка нажатия кнопки Run/Pause
	void handleEvent(int x, int y, bool& bPause);
	void finalize();
	void init();
	void render(Cell** auts);

	int getWidthAutomata() {
		return widthCell;
	}
};


template<int N>
void Renderer<N>::init() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw std::runtime_error(
				std::string("SDL_Init Error: ") + SDL_GetError());
	}

	SDL_GetDesktopDisplayMode(0,&displayMode);

	win = SDL_CreateWindow("CellularAutomata", 0, 0, displayMode.w,
			displayMode.h, SDL_WINDOW_SHOWN);
	if (win == nullptr) {
		throw std::runtime_error(
				std::string("SDL_CreateWindow Error: ") + SDL_GetError());
	}

	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (ren == nullptr) {
		throw std::runtime_error(
				std::string("SDL_CreateRenderer Error: ") + SDL_GetError());
	}

	buildingCellTxt = IMG_LoadTexture(ren, "blue.png");
	emptyCellTxt = IMG_LoadTexture(ren, "white.png");

	cameraCellTxt = new SDL_Texture*[N];
	std::string filename;
	for (int i = 0; i < numMaxCamera; i++) {
		filename = std::to_string(i) + ".png";
		cameraCellTxt[i] = IMG_LoadTexture(ren, filename.c_str());
	}

	run = IMG_LoadTexture(ren, "run.png");
	pause = IMG_LoadTexture(ren, "pause.png");
	player = run;

	widthCell = std::min(displayMode.w / N, displayMode.h / N);
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			cells[i][j].x = i * widthCell;
			cells[i][j].y = j * widthCell;
			cells[i][j].w = widthCell;
			cells[i][j].h = widthCell;
		}
	}

	//расположение rectPlayer
	rectPlayer.x = widthCell * N + 200;
	rectPlayer.y = 200;
	rectPlayer.w = 192;
	rectPlayer.h = 192;

	//цвет фона
	SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
}

template<int N>
void Renderer<N>::render(Cell** auts) {
	SDL_RenderClear(ren);

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (auts[i][j].isAlive) {
				SDL_RenderCopy(ren, buildingCellTxt, NULL, &cells[i][j]);
			}
			else {
				int energyIndex = std::floor(auts[i][j].energy / Cell::MAX_ENERGY * (numMaxCamera - 1));
				SDL_RenderCopy(ren, cameraCellTxt[energyIndex], NULL, &cells[i][j]);
			}
		}
	}
	SDL_RenderCopy(ren, player, NULL, &rectPlayer);
	SDL_RenderPresent(ren);
	//задержка в миллисекундах
	SDL_Delay(100);
}

template<int N>
void Renderer<N>::handleEvent(int x, int y, bool& bPause) {

	//player button
	if ((x >= rectPlayer.x && x < rectPlayer.x + rectPlayer.w) &&
			(y >= rectPlayer.y && y < rectPlayer.y + rectPlayer.h)) {
		bPause = !bPause;
		if (bPause)
			player = run;
		else
			player = pause;
	}
}

template<int N>
void Renderer<N>::finalize() {
	SDL_DestroyTexture(buildingCellTxt);
	SDL_DestroyTexture(emptyCellTxt);

	for (int i = 0; i < numMaxCamera; i++) {
		SDL_DestroyTexture(cameraCellTxt[i]);
	}
	delete[] cameraCellTxt;

	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

#endif /* RENDERER_H_ */
