#include "game.h"
#include "structs.h"
#include "defs.h"
#include "sound.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include "highScoreFile.h"
#include <SDL_mixer.h>

void Game::handleEvents() {
	SDL_Event evnt;

	while (SDL_PollEvent(&evnt)) {
		switch (evnt.type) {
		case SDL_QUIT:
			gameState = GameState::EXIT;
			break;

		case SDL_KEYDOWN:
			handleKeyDownEvent(&evnt.key);
			break;

		case SDL_KEYUP:
			doKeyUp(&evnt.key);
			break;

		case SDL_MOUSEBUTTONDOWN:
			handleMouseDownEvent(&evnt.button);
			break;

		default:
			break;
		}
	}
}
void Game::handleKeyDownEvent(SDL_KeyboardEvent* event) {
	if (event->repeat == 0) {
		if (event->keysym.sym == SDLK_p) {
			if (gameState == GameState::PLAY) {
				gameState = GameState::PAUSE;
			}
			else if (gameState == GameState::PAUSE) {
				gameState = GameState::PLAY;
			}
		}
		else if (event->keysym.sym == SDLK_RETURN && gameState == GameState::GAMEOVER) {
			gameState = GameState::PLAYAGAIN;
		}
		else if (event->keysym.sym == SDLK_SPACE && gameState == GameState::PLAY) {
			fire = 1;
		}
		else if (event->keysym.sym == SDLK_UP) {
			up = 1;
		}
		else if (event->keysym.sym == SDLK_DOWN) {
			down = 1;
		}
		else if (event->keysym.sym == SDLK_LEFT) {
			left = 1;
		}
		else if (event->keysym.sym == SDLK_RIGHT) {
			right = 1;
		}
		else {
			doKeyDown(event); // Handle other keydown events
		}
	}
}
void Game::handleMouseDownEvent(SDL_MouseButtonEvent* event) {
	if (gameState == GameState::START) {
		int mouseX = event->x;
		int mouseY = event->y;

		SDL_Rect playRect = { 1280 - 350, 50, 300, 80 };
		SDL_Rect instrRect = { 1280 - 350, 50 + 80 + 20, 300, 80 };
		SDL_Rect lbRect = { 1280 - 350,50 + 2 * (80 + 20), 300, 80 };
		SDL_Rect creditsRect = { 1280 - 350,50 +3*( 80 + 20), 300, 80 };
		SDL_Rect quitRect = { 1280 - 350, 50 + 4 * (80 + 20), 300, 80 };

		if (mouseX >= playRect.x && mouseX <= (playRect.x + playRect.w) &&
			mouseY >= playRect.y && mouseY <= (playRect.y + playRect.h)) {
			gameState = GameState::PLAYAGAIN; // Restart the game
		}
		else if (mouseX >= instrRect.x && mouseX <= (instrRect.x + instrRect.w) &&
			mouseY >= instrRect.y && mouseY <= (instrRect.y + instrRect.h)) {
			openInstructionsWindow(); // Open instructions window
		}
		else if (mouseX >= lbRect.x && mouseX <= (lbRect.x + lbRect.w) &&
			mouseY >= lbRect.y && mouseY <= (lbRect.y + lbRect.h)) {
			showLeaderboard(); // Show leaderboard
		}
		else if (mouseX >= creditsRect.x && mouseX <= (creditsRect.x + creditsRect.w) &&
			mouseY >= creditsRect.y && mouseY <= (creditsRect.y + creditsRect.h)) {
			openCreditsWindow(); // Open credits window
		}
		else if (mouseX >= quitRect.x && mouseX <= (quitRect.x + quitRect.w) &&
			mouseY >= quitRect.y && mouseY <= (quitRect.y + quitRect.h)) {
			// Set flag to quit or handle SDL quit event
			gameState = GameState::EXIT; // Example function to handle quitting gracefully
		}
	
	}
	// Existing logic for other states
	else if (gameState == GameState::PAUSE) {
		int mouseX = event->x;
		int mouseY = event->y;

		SDL_Rect buttonRect = { 1270 / 2 - 100, 720 / 2 - 25, 200, 50 };

		if (mouseX >= buttonRect.x && mouseX <= (buttonRect.x + buttonRect.w) &&
			mouseY >= buttonRect.y && mouseY <= (buttonRect.y + buttonRect.h)) {
			gameState = GameState::START; // Return to title screen
		}
	}
	else if (gameState == GameState::GAMEOVER) {
		int mouseX = event->x;
		int mouseY = event->y;

		// Yes button rect
		SDL_Rect yesBgRect = { 1280 / 2 - 110, 340+50, 100, 50 };
		SDL_Rect noBgRect = { 1280 / 2 + 10, 340+50, 100, 50 };

		if (mouseX >= yesBgRect.x && mouseX <= (yesBgRect.x + yesBgRect.w) &&
			mouseY >= yesBgRect.y && mouseY <= (yesBgRect.y + yesBgRect.h)) {
			gameState = GameState::PLAYAGAIN; // Restart the game
		}

		if (mouseX >= noBgRect.x && mouseX <= (noBgRect.x + noBgRect.w) &&
			mouseY >= noBgRect.y && mouseY <= (noBgRect.y + noBgRect.h)) {
			gameState = GameState::START; // Return to title screen
		}
	}
}

