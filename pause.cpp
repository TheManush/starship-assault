#include "game.h"
#include "sound.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include "highScoreFile.h"
using namespace std;


void Game::renderPauseScreen() {
	mainFont1 = TTF_OpenFont("mainFont.ttf", 40);
	textColor1 = { 255, 255, 255 };
	// Fill the screen with a black color
	SDL_Texture* pauseBackgroundTexture = loadTexture((char*)"space1.png");
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Opaque black
	SDL_RenderClear(renderer); // Clear with the current drawing color

	// Define the destination rectangle to cover the entire screen
	SDL_Rect bgRect;
	bgRect.x = 0;
	bgRect.y = 0;

	// Get the renderer's output size for proper scaling
	SDL_GetRendererOutputSize(renderer, &bgRect.w, &bgRect.h);

	// Render the pause background
	SDL_RenderCopy(renderer, pauseBackgroundTexture, nullptr, &bgRect);

	// Optional: Add "Game Paused" text in the center
	SDL_Surface* textSurface = TTF_RenderText_Solid(mainFont1, "Game Paused", textColor1);
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

	SDL_Rect textRect;
	textRect.x = bgRect.w / 2 - textSurface->w / 2;
	textRect.y = bgRect.h / 3 - textSurface->h / 2;
	textRect.w = textSurface->w;
	textRect.h = textSurface->h;

	SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(textTexture);

	// Render "Return to Title" button
	SDL_Rect buttonRect;
	buttonRect.w = 200;
	buttonRect.h = 50;
	buttonRect.x = bgRect.w / 2 - buttonRect.w / 2;
	buttonRect.y = bgRect.h / 2 - buttonRect.h / 2;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Red button
	SDL_RenderFillRect(renderer, &buttonRect);

	textSurface = TTF_RenderText_Solid(mainFont1, "Return to Title", { 255, 255, 255 });
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

	SDL_Rect buttonTextRect;
	buttonTextRect.x = buttonRect.x + (buttonRect.w / 2) - (textSurface->w / 2);
	buttonTextRect.y = buttonRect.y + (buttonRect.h / 2) - (textSurface->h / 2);
	buttonTextRect.w = textSurface->w;
	buttonTextRect.h = textSurface->h;

	SDL_RenderCopy(renderer, textTexture, nullptr, &buttonTextRect);

	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(textTexture);

	// Update the screen
	SDL_RenderPresent(renderer);
}

void Game::openInstructionsWindow() {
	SDL_Window* instructionsWindow = SDL_CreateWindow(
		"Instructions",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1280, 720, // Window size
		SDL_WINDOW_SHOWN
	);

	if (!instructionsWindow) {
		std::cerr << "Failed to create instructions window: " << SDL_GetError() << std::endl;
		return;
	}

	SDL_Renderer* instructionsRenderer = SDL_CreateRenderer(instructionsWindow, -1, SDL_RENDERER_ACCELERATED);
	if (!instructionsRenderer) {
		std::cerr << "Failed to create instructions renderer: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(instructionsWindow);
		return;
	}

	// Load background image
	SDL_Texture* backgroundTexture = IMG_LoadTexture(instructionsRenderer, "back20.png");
	if (!backgroundTexture) {
		std::cerr << "Failed to load background texture: " << SDL_GetError() << std::endl;
		SDL_DestroyRenderer(instructionsRenderer);
		SDL_DestroyWindow(instructionsWindow);
		return;
	}

	SDL_SetRenderDrawColor(instructionsRenderer, 255, 255, 255, 255);
	SDL_RenderClear(instructionsRenderer);

	// Render background texture
	SDL_RenderCopy(instructionsRenderer, backgroundTexture, NULL, NULL);

	// Load and render instructions text
	TTF_Font* instructionsFont = TTF_OpenFont("mainFont.ttf", 36);
	SDL_Color white = { 255, 255, 255, 255 }; // White text color

	std::string instructionsText =
		"Instructions:\n"
		"1. Press arrows to move.\n"
		"2. Press space to shoot.\n"
		"3. Press 'P' to pause the game.\n"
		"4. Press 'P' again to unpause the game.\n"
		"5. Gain 10 points for hitting an enemy.\n"
		"6. Lose 10 points for missing to hit an enemy.\n"
		"7. Gaining health pack gives you an extra life";

	SDL_Surface* instrSurface = TTF_RenderText_Blended_Wrapped(instructionsFont, instructionsText.c_str(), white, 1200);
	if (!instrSurface) {
		std::cerr << "Failed to render text surface for instructions: " << TTF_GetError() << std::endl;
		TTF_CloseFont(instructionsFont);
		SDL_DestroyTexture(backgroundTexture);
		SDL_DestroyRenderer(instructionsRenderer);
		SDL_DestroyWindow(instructionsWindow);
		return;
	}

	SDL_Texture* instrTexture = SDL_CreateTextureFromSurface(instructionsRenderer, instrSurface);

	// Calculate text position for centering
	SDL_Rect textRect;
	textRect.w = instrSurface->w;
	textRect.h = instrSurface->h;
	textRect.x = (1280 - textRect.w) / 2 +100; // Center horizontally
	textRect.y = (720 - textRect.h) / 2; // Center vertically

	// Render instructions texture
	SDL_RenderCopy(instructionsRenderer, instrTexture, NULL, &textRect);
	SDL_RenderPresent(instructionsRenderer);

	// Wait for user to close the instructions window
	bool running = true;
	SDL_Event e;
	while (running) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				running = false;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				running = false;
			}
		}
	}

	// Clean up
	SDL_FreeSurface(instrSurface);
	SDL_DestroyTexture(instrTexture);
	TTF_CloseFont(instructionsFont);
	SDL_DestroyTexture(backgroundTexture);
	SDL_DestroyRenderer(instructionsRenderer);
	SDL_DestroyWindow(instructionsWindow);
}



void Game::showLeaderboard() {
	// Create a new SDL window for the leaderboard
	SDL_Window* lbWindow = SDL_CreateWindow("Leaderboard", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
	SDL_Renderer* lbRenderer = SDL_CreateRenderer(lbWindow, -1, SDL_RENDERER_ACCELERATED);

	// Load background image
	SDL_Texture* backgroundTexture = IMG_LoadTexture(lbRenderer, "moon5.jpg");
	if (!backgroundTexture) {
		std::cerr << "Failed to load background texture! SDL_image Error: " << IMG_GetError() << std::endl;
		return;
	}

	// Display the leaderboard content
	SDL_Color textColor = { 255, 255, 255, 255 };
	TTF_Font* font = TTF_OpenFont("mainFont.ttf", 40);
	if (!font) {
		std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
		return;
	}

	TTF_Font* titleFont = TTF_OpenFont("mainFont.ttf", 80);
	if (!titleFont) {
		std::cerr << "Failed to load title font! SDL_ttf Error: " << TTF_GetError() << std::endl;
		return;
	}

	SDL_SetRenderDrawColor(lbRenderer, 0, 0, 0, 255);
	SDL_RenderClear(lbRenderer);

	// Render the background
	SDL_RenderCopy(lbRenderer, backgroundTexture, NULL, NULL);

	// Render the title
	std::string titleText = "LEADERBOARD";
	SDL_Surface* titleSurface = TTF_RenderText_Solid(titleFont, titleText.c_str(), textColor);
	SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(lbRenderer, titleSurface);
	SDL_Rect titleRect = { (1280 - titleSurface->w) / 2, 20, titleSurface->w, titleSurface->h };
	SDL_RenderCopy(lbRenderer, titleTexture, NULL, &titleRect);
	SDL_FreeSurface(titleSurface);
	SDL_DestroyTexture(titleTexture);

	// Load high scores
	HighScoreFile hf;
	std::vector<int> highScores = hf.getHighScores();

	int y = 150; // Adjusting y to start below the title
	for (size_t i = 0; i < highScores.size(); ++i) {
		std::string entry = std::to_string(i + 1) + ". " + std::to_string(highScores[i]);
		SDL_Surface* entrySurface = TTF_RenderText_Solid(font, entry.c_str(), textColor);
		SDL_Texture* entryTexture = SDL_CreateTextureFromSurface(lbRenderer, entrySurface);

		// Center the entry text
		SDL_Rect entryRect = { (1280 - entrySurface->w) / 2, y, entrySurface->w, entrySurface->h };
		SDL_RenderCopy(lbRenderer, entryTexture, NULL, &entryRect);

		SDL_FreeSurface(entrySurface);
		SDL_DestroyTexture(entryTexture);
		y += 50; // Vertical spacing between entries
	}

	SDL_RenderPresent(lbRenderer);

	// Event loop to wait for user input to close the leaderboard window
	bool lbOpen = true;
	while (lbOpen) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				lbOpen = false;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					lbOpen = false;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				// Handle mouse click events if needed
				break;
			default:
				break;
			}
		}
	}

	// Clean up resources
	SDL_DestroyTexture(backgroundTexture);
	TTF_CloseFont(font);
	TTF_CloseFont(titleFont);
	SDL_DestroyRenderer(lbRenderer);
	SDL_DestroyWindow(lbWindow);
}


void Game::openCreditsWindow() {
	// Create SDL window for credits
	SDL_Window* creditsWindow = SDL_CreateWindow(
		"Credits",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1280, 720, // Window size
		SDL_WINDOW_SHOWN
	);
	// Create renderer
	SDL_Renderer* creditsRenderer = SDL_CreateRenderer(creditsWindow, -1, SDL_RENDERER_ACCELERATED);
	if (!creditsRenderer) {
		std::cerr << "Failed to create renderer  " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(creditsWindow);
		return;
	}

	// Set render draw color and clear the renderer
	SDL_SetRenderDrawColor(creditsRenderer, 0, 0, 0, 255); // Black background
	SDL_RenderClear(creditsRenderer);

	// Load and render credits text
	TTF_Font* creditsFont = TTF_OpenFont("mainFont.ttf", 36); // Replace with your font and size
	

	SDL_Color white = { 255, 255, 255, 255 }; // White text color

	// Credits text
	std::string creditsText = "Credits:\n"
		"Ahnaf Mahbub Khan. Roll-38\n"
		"Email: ahnafkhan414@gmail.com\n"
		"Abdullah-Ash-Sakafy. Roll-34\n"
		"Email: abdullahsakafy@gmail.com\n"
		"Anika Sanzida Upoma. Roll-02\n"
		"Email: anikasanzida31593@gmail.com\n";

	// Render credits text
	SDL_Surface* creditsSurface = TTF_RenderText_Blended_Wrapped(creditsFont, creditsText.c_str(), white, 1200);
	if (!creditsSurface) {
		std::cerr << "Failed to render text s " << TTF_GetError() << std::endl;
		TTF_CloseFont(creditsFont);
		SDL_DestroyRenderer(creditsRenderer);
		SDL_DestroyWindow(creditsWindow);
		return;
	}

	SDL_Texture* creditsTexture = SDL_CreateTextureFromSurface(creditsRenderer, creditsSurface);
	if (!creditsTexture) {
		std::cerr << "Failed to create texture  " << SDL_GetError() << std::endl;
		SDL_FreeSurface(creditsSurface);
		TTF_CloseFont(creditsFont);
		SDL_DestroyRenderer(creditsRenderer);
		SDL_DestroyWindow(creditsWindow);
		return;
	}

	// Calculate text position for centering
	SDL_Rect textRect;
	textRect.w = creditsSurface->w;
	textRect.h = creditsSurface->h;
	textRect.x = (1280 - textRect.w) / 2 +100; // Center horizontally
	textRect.y = (720 - textRect.h) / 2; // Center vertically

	// Render credits texture
	SDL_RenderCopy(creditsRenderer, creditsTexture, NULL, &textRect);

	// Render "Press Esc to return" text in small size at the top left corner
	TTF_Font* smallFont = TTF_OpenFont("mainFont.ttf", 24); // Small font size for the message
	

	std::string returnMessage = "Press Esc to return";
	SDL_Surface* returnSurface = TTF_RenderText_Blended(smallFont, returnMessage.c_str(), white);
	

	SDL_Texture* returnTexture = SDL_CreateTextureFromSurface(creditsRenderer, returnSurface);

	SDL_Rect returnRect = { 20, 20, returnSurface->w, returnSurface->h };
	SDL_RenderCopy(creditsRenderer, returnTexture, NULL, &returnRect);

	// Present renderer
	SDL_RenderPresent(creditsRenderer);

	// Event loop to wait for user to close the window (press ESC key)
	bool running = true;
	SDL_Event e;
	while (running) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				running = false;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				running = false;
			}
		}
	}

	// Clean up resources
	SDL_FreeSurface(returnSurface);
	SDL_DestroyTexture(returnTexture);
	SDL_FreeSurface(creditsSurface);
	SDL_DestroyTexture(creditsTexture);
	TTF_CloseFont(creditsFont);
	TTF_CloseFont(smallFont);
	SDL_DestroyRenderer(creditsRenderer);
	SDL_DestroyWindow(creditsWindow);
}
