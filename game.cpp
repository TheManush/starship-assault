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
#include <algorithm>
using namespace std;

Game::Game(const char* title, int x, int y, int w, int h, Uint32 flags)
	:window(NULL), renderer(NULL), up(0), down(0), left(0), right(0), fire(0)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) cout << "SDL_INIT HAS FAILED! SDL_ERROR: " << SDL_GetError() << endl;

	TTF_Init();

	window = SDL_CreateWindow(title, x, y, w, h, flags);

	SDL_Surface* icon = IMG_Load("player_lvl2.png");
	SDL_SetWindowIcon(window, icon);

	if (window == NULL) cout << "Window failed to open. Error: " << SDL_GetError() << endl;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (window == NULL) cout << "Render failed to create. Error: " << SDL_GetError() << endl;
};

Game::~Game() {};

void Game::run() {
	gameLoop();
}

void Game::gameLoop() {
	Entity player;

	Entity bulletHead;
	Entity* bulletTail = new Entity();
	bulletTail = &bulletHead;

	Entity fighterHead;
	Entity* fighterTail = new Entity();
	fighterTail = &fighterHead;

	Entity playerLife;
	Entity titleImage;

	int enemySpawnTimer = 0;
	int lifeSpawnTimer = 0;
	int backgroundY = 0;
	int score = 0;

	const int LIFE_PACK_SPEED = 3;

	// Cache textures
	SDL_Texture* backgroundTextureTop = loadTexture((char*)"stars0.png");
	SDL_Texture* backgroundTextureMid = loadTexture((char*)"stars1.png");
	SDL_Texture* backgroundTextureBot = loadTexture((char*)"stars2.png");
	SDL_Texture* bulletTexture = loadTexture((char*)"bullet1.png");
	SDL_Texture* bulletTexture2 = loadTexture((char*)"bullet2.png");
	SDL_Texture* enemyTexture1 = loadTexture((char*)"spaceship12.png");
	SDL_Texture* enemyTexture2 = loadTexture((char*)"enemyShip_lvl2.png");
	SDL_Texture* enemyTexture3 = loadTexture((char*)"enemyShip_lvl3.png");
	SDL_Texture* enemyTexture4 = loadTexture((char*)"enemyShip_lvl4.png");
	SDL_Texture* enemyTexture5 = loadTexture((char*)"enemyShip_lvl5.png");
	SDL_Texture* enemyTexture6 = loadTexture((char*)"enemyShip_lvl6.png");
	SDL_Texture* playerLifeTexture = loadTexture((char*)"life1.png");
	SDL_Texture* playerTexture = loadTexture((char*)"player.png");
	SDL_Texture* playerTexture1 = loadTexture((char*)"player_lvl2.png");
	SDL_Texture* playerTexture2 = loadTexture((char*)"player_lvl3.png");
	SDL_Texture* titleScreenTexture = loadTexture((char*)"title6.png");


	HighScoreFile hf;

	// Retrieve high scores
	std::vector<int> highScores = hf.getHighScores();
	const int NUM_HIGH_SCORES = 10;
	// init player
	player.x = 500;
	player.y = 640;
	player.dx = 0;
	player.dy = 0;
	player.side = SIDE_PLAYER;
	player.texture = playerTexture;
	SDL_QueryTexture(player.texture, NULL, NULL, &player.w, &player.h);

	// init sound
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
	{
		cout << "couldn't initialize SDL Mixer" << endl;
		exit(1);
	}

	Mix_AllocateChannels(MAX_SND_CHANNELS);

	Sound sound;
	sound.initSounds();
	sound.playMusic(-1);

	Mix_Music* bgMusic = Mix_LoadMUS("titleMusic1.mp3");
	if (!bgMusic) {
		std::cerr << "Failed to load background music: " << Mix_GetError() << std::endl;
	}

	Mix_PlayMusic(bgMusic, -1);
	TTF_Font* mainFont = TTF_OpenFont("mainFont.ttf", 32);
	SDL_Color textColor = { 255, 255, 255 };

	// Init title screen
	if (gameState == GameState::START) {
		Mix_ResumeMusic();
		titleImage.x = (1270 / 2 - 954 / 2);
		titleImage.y = (720 / 2 - 87 / 2) - 200;
		titleImage.texture = titleScreenTexture;
		SDL_QueryTexture(titleImage.texture, NULL, NULL, &titleImage.w, &titleImage.h);
	}

	// Main loop
	while (gameState != GameState::EXIT) {
		handleEvents(); // Collect and process user input

		// Handle pause state
		if (gameState == GameState::PAUSE) {
			Mix_ResumeMusic();
			renderPauseScreen();
		}
		else {
			// Only update the game if not paused
			prepareScene(); // Sets up rendering

			// Handles game start, game over and play again states
			if (gameState == GameState::GAMEOVER || gameState == GameState::START) {
				Mix_ResumeMusic();
				player.dx = 0;
				player.dy = 0;
				player.texture = NULL;
				if (score > 0) {
					std::vector<int> highScores = hf.getHighScores(); // Get current high scores
					highScores.push_back(score); // Add the current score to the list
					std::sort(highScores.rbegin(), highScores.rend()); // Sort in descending order

					// Keep only the top NUM_HIGH_SCORES scores
					if (highScores.size() > NUM_HIGH_SCORES) {
						highScores.resize(NUM_HIGH_SCORES);
					}
					finalscore = score;
					// Save updated high scores
					hf.setHighScores(highScores);
				}
				score = 0;
			}
			else if (gameState == GameState::PLAYAGAIN) {
				player.x = 500;
				player.y = 640;
				player.texture = loadTexture((char*)"player.png");
				SDL_QueryTexture(player.texture, NULL, NULL, &player.w, &player.h);
				gameState = GameState::PLAY;
			}

			// Update player texture to sprite
			if (gameState != GameState::GAMEOVER && gameState != GameState::START) {
				if (player.texture != playerTexture && playerLifeScore == 1) {
					player.texture = playerTexture;
					SDL_QueryTexture(player.texture, NULL, NULL, &player.w, &player.h);
				}
				else if (playerLifeScore == 2) {
					player.texture = playerTexture1;
					SDL_QueryTexture(player.texture, NULL, NULL, &player.w, &player.h);
				}
				else if (playerLifeScore == 3) {
					player.texture = playerTexture2;
					SDL_QueryTexture(player.texture, NULL, NULL, &player.w, &player.h);
				}
			}

			player.x += player.dx;
			player.y += player.dy;

			// Clamp player position to screen bounds
			int screenWidth = 1270; // Assuming screen width
			int screenHeight = 720; // Assuming screen height

			if (player.x < 0) player.x = 0;
			if (player.y < 0) player.y = 0;
			if (player.x + player.w > screenWidth) player.x = screenWidth - player.w;
			if (player.y + player.h > screenHeight) player.y = screenHeight - player.h;

			// Player key input
			if (player.reload > 0) player.reload--;

			if (up) {
				player.y -= PLAYER_SPEED;
			}

			if (down) {
				player.y += PLAYER_SPEED;
			}

			if (left) {
				player.x -= PLAYER_SPEED;
			}

			if (right) {
				player.x += PLAYER_SPEED;
			}

			if (gameState != GameState::GAMEOVER && gameState != GameState::START) {
				playerHitEnemy(player, fighterHead, sound); // check for player collision
				playerHitLife(player, playerLife, sound); // check for player collision
			}

			// Allow fire bullet every 8 frames
			if (fire && player.reload == 0) {
				player.reload = 8;
				sound.playSound(sound.SND_PLAYER_FIRE, CH_PLAYER);

				// Create bullet
				Entity* bullet = new Entity();
				memset(bullet, 0, sizeof(Entity));

				bulletTail->next = bullet;
				bulletTail = bullet;

				bullet->x = player.x + 8;
				bullet->y = player.y;
				bullet->dx = 0;
				bullet->dy = -PLAYER_BULLET_SPEED;
				bullet->health = 1;
				bullet->side = SIDE_PLAYER;
				if (playerLifeScore < 2) {
					bullet->texture = bulletTexture;
				}
				else {
					bullet->texture = bulletTexture2;
				}

				SDL_QueryTexture(bullet->texture, NULL, NULL, &bullet->w, &bullet->h);
			}

			// Reset background
			if (++backgroundY > 2160) {
				backgroundY = 0;
			}

			// Draw background
			SDL_Rect dest, src;

			int y = backgroundY;

			if (y > 1440) dest.y = y - 2160;
			dest.x = 0;
			if (y < 1440) dest.y = y;
			SDL_QueryTexture(backgroundTextureTop, NULL, NULL, &dest.w, &dest.h);
			SDL_RenderCopy(renderer, backgroundTextureTop, NULL, &dest);

			if (y > 1440) dest.y = y - 2880;
			dest.x = 0;
			if (y < 1440) dest.y = y - 720;
			SDL_QueryTexture(backgroundTextureBot, NULL, NULL, &dest.w, &dest.h);
			SDL_RenderCopy(renderer, backgroundTextureBot, NULL, &dest);

			dest.y = y - 1440;
			dest.x = 0;
			SDL_QueryTexture(backgroundTextureMid, NULL, NULL, &dest.w, &dest.h);
			SDL_RenderCopy(renderer, backgroundTextureMid, NULL, &dest);

			if (--lifeSpawnTimer <= 0 && !isLifeGenerated && gameState != GameState::GAMEOVER && gameState != GameState::START) {
				playerLife.x = rand() % 1270;
				playerLife.y = rand() % 710;
				playerLife.side = SIDE_ALIEN;
				isLifeGenerated = true;
				lifeSpawnTimer = 600 + (rand() % 1200);
			}


			// Generate enemy ships every 0.5 - 1.5 seconds
			if (--enemySpawnTimer <= 0) {
				Entity* enemy = new Entity();
				memset(enemy, 0, sizeof(Entity));
				fighterTail->next = enemy;
				fighterTail = enemy;

				enemy->x = rand() % 1270;
				enemy->y = 0;
				enemy->side = SIDE_ALIEN;

				// Load different enemy
				int enemyType = rand() % 5;
				switch (enemyType) {
				case 0:
					enemy->texture = enemyTexture1;
					SDL_QueryTexture(enemy->texture, NULL, NULL, &enemy->w, &enemy->h);
					enemy->health = 1;
					break;
				case 1:
					enemy->texture = enemyTexture2;
					SDL_QueryTexture(enemy->texture, NULL, NULL, &enemy->w, &enemy->h);
					enemy->health = 1;
					break;
				case 2:
					enemy->texture = enemyTexture3;
					SDL_QueryTexture(enemy->texture, NULL, NULL, &enemy->w, &enemy->h);
					enemy->health = 2;
					break;
				case 3:
					enemy->texture = enemyTexture4;
					SDL_QueryTexture(enemy->texture, NULL, NULL, &enemy->w, &enemy->h);
					enemy->health = 2;
					break;
				case 4:
					enemy->texture = enemyTexture5;
					SDL_QueryTexture(enemy->texture, NULL, NULL, &enemy->w, &enemy->h);
					enemy->health = 3;
					break;
				case 5:
					enemy->texture = enemyTexture6;
					SDL_QueryTexture(enemy->texture, NULL, NULL, &enemy->w, &enemy->h);
					enemy->health = 4;
					break;
				default:
					break;
				}

				enemy->dy = (2 + (rand() % 4));

				if (score > 200) {
					enemySpawnTimer = 40 + (rand() % 80); // Increased values
				}
				else if (score > 300) {
					enemySpawnTimer = 30 + (rand() % 60); // Increased values
				}
				else if (score > 700) {
					enemySpawnTimer = 20 + (rand() % 40); // Increased values
				}
				else if (score > 1000) {
					enemySpawnTimer = 15 + (rand() % 30); // Increased values
				}
				else {
					enemySpawnTimer = 60 + (rand() % 120); // Increased values
				}
			}

			// Handle physics and render for each bullet
			Entity* b = new Entity();
			Entity* prev = new Entity();
			prev = &bulletHead;

			for (b = bulletHead.next; b != NULL; b = b->next) {
				b->x += b->dx;
				b->y += b->dy;

				blit(b->texture, b->x, b->y);

				if (b->y < 0 || bulletHitFighter(b, fighterHead, sound)) {
					if (b == bulletTail) {
						bulletTail = prev;
					}
					prev->next = b->next;
					delete b;
					b = prev;
				}

				prev = b;
			}

			// Handle physics and render for each enemy
			Entity* e = new Entity();
			Entity* prevv = new Entity();
			prevv = &fighterHead;

			for (e = fighterHead.next; e != NULL; e = e->next) {
				e->x += e->dx;
				e->y += e->dy;

				blit(e->texture, e->x, e->y);

				if (e->health == 0) {
					score += 10;
					sound.playSound(sound.SND_ALIEN_DIE, CH_ALIEN_DIE);
				}

				if (e->y > 720) {
					if (score != 0) score -= 10;
				}

				if (e->y > 720 || e->health == 0) {
					if (e == fighterTail) {
						fighterTail = prevv;
					}
					prevv->next = e->next;
					delete e;
					e = prevv;
				}

				prevv = e;
			}

			// Render player life
			playerLife.texture = playerLifeTexture;
			SDL_QueryTexture(playerLife.texture, NULL, NULL, &playerLife.w, &playerLife.h);
			if (isLifeGenerated) blit(playerLife.texture, playerLife.x, playerLife.y);

			// Render title screen
			if (gameState == GameState::START) blit(titleImage.texture, titleImage.x, titleImage.y);

			// Display player over everything    
			blit(player.texture, player.x, player.y);

			// Draws HUD
			std::string s = std::to_string(score);
			drawHud(s, textColor, mainFont);

			presentScene(); // Displays scene
		}

		SDL_Delay(16); // Limits fps 
	}

	// Close up
	Mix_FreeMusic(bgMusic);
	Mix_CloseAudio();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(mainFont);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}



void Game::prepareScene() {
	// SDL_SetRenderDrawColor(renderer, 96, 128, 255, 255);
	SDL_RenderClear(renderer);
}

void Game::presentScene() {
	SDL_RenderPresent(renderer);
}

SDL_Texture* Game::loadTexture(char* filename)
{
	SDL_Texture* texture = NULL;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	texture = IMG_LoadTexture(renderer, filename);

	return texture;
}

void Game::blit(SDL_Texture* texture, int x, int y)
{
	SDL_Rect dest;

	dest.x = x;
	dest.y = y;
	SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);

	SDL_RenderCopy(renderer, texture, NULL, &dest);
}

void Game::doKeyDown(SDL_KeyboardEvent* event)
{
	if (event->repeat == 0)
	{
		if (event->keysym.scancode == SDL_SCANCODE_RETURN) gameState = GameState::PLAYAGAIN;

		if (event->keysym.scancode == SDL_SCANCODE_SPACE && gameState == GameState::PLAY)
		{
			fire = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_UP)
		{
			up = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_DOWN)
		{
			down = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_LEFT)
		{
			left = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_RIGHT)
		{
			right = 1;
		}
	}
}

void Game::doKeyUp(SDL_KeyboardEvent* event)
{
	if (event->repeat == 0)
	{
		if (event->keysym.scancode == SDL_SCANCODE_SPACE)
		{
			fire = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_UP)
		{
			up = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_DOWN)
		{
			down = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_LEFT)
		{
			left = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_RIGHT)
		{
			right = 0;
		}
	}
}

int Game::collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
	return (max(x1, x2) < min(x1 + w1, x2 + w2)) && (max(y1, y2) < min(y1 + h1, y2 + h2));
}

int Game::bulletHitFighter(Entity* b, Entity fighterHead, Sound sound) // checks if a bullet has hit an enemy figher using collision function
{
	Entity* e;

	for (e = fighterHead.next; e != NULL; e = e->next)
	{
		if (e->side != b->side && collision(b->x, b->y, b->w, b->h, e->x, e->y, e->w, e->h))
		{
			b->health = 0;
			e->health--;
			sound.playSound(sound.SND_ALIEN_HIT, CH_ALIEN_HIT);

			return 1;
		}
	}

	return 0;
}

void Game::playerHitEnemy(Entity player, Entity fighterHead, Sound sound) // checks if the player has collided with an enemy
{
	Entity* e;

	for (e = fighterHead.next; e != NULL; e = e->next)
	{
		if (e->side != player.side && collision(player.x, player.y, player.w, player.h, e->x, e->y, e->w, e->h))
		{
			playerLifeScore--;
			e->health = 0;
			if (playerLifeScore <= 0) {
				sound.playSound(sound.SND_PLAYER_DIE, CH_ANY);
				gameState = GameState::GAMEOVER;
			}
		}
	}

}

void Game::playerHitLife(Entity player, Entity life, Sound sound) // checks if the player has collided with an enemy
{
	if (isLifeGenerated && life.side != player.side && collision(player.x, player.y, player.w, player.h, life.x, life.y, life.w, life.h))
	{
		if (playerLifeScore < 3)playerLifeScore++;
		isLifeGenerated = false;
		sound.playSound(sound.SND_POWERUP, CH_ANY);
	}

}

void Game::drawHud(std::string textureText, SDL_Color textColor, TTF_Font* font) {
	if (gameState == GameState::PLAY)
	{
		Mix_PauseMusic();
		std::string desc = "Score: ";
		std::string displayText = desc + textureText;

		SDL_Surface* textSurface = TTF_RenderText_Solid(font, displayText.data(), textColor);
		if (textSurface == NULL) cout << "Unable to load text surface" << endl;
		SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (messageTexture == NULL) cout << "Unable load texture" << endl;

		SDL_Rect scoreRect; //create a rect
		scoreRect.x = 100;  //controls the rect's x coordinate 
		scoreRect.y = 30; // controls the rect's y coordinte
		scoreRect.w = 100; // controls the width of the rect
		scoreRect.h = 100; // controls the height of the rect

		SDL_QueryTexture(messageTexture, NULL, NULL, &scoreRect.w, &scoreRect.h);
		SDL_RenderCopy(renderer, messageTexture, NULL, &scoreRect);

		SDL_FreeSurface(textSurface);
		SDL_DestroyTexture(messageTexture);
	}

	// Game Start
	if (gameState == GameState::START) {
		SDL_Rect playRect; // Create a rect for the Play button
		playRect = { 1280 - 350, 50, 300, 80 };
		SDL_Texture* playButtonTexture = nullptr;
		SDL_Surface* tempSurface = IMG_Load("but2.png");
		playButtonTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
		SDL_FreeSurface(tempSurface);
		// Render the button background texture
		SDL_RenderCopy(renderer, playButtonTexture, NULL, &playRect);
		
		// Render the Play text with larger font size
		TTF_Font* largeFont = TTF_OpenFont("mainFont.ttf", 36); // Increase font size here
		SDL_Surface* textSurface = TTF_RenderText_Solid(largeFont, "Play", textColor);
		SDL_Texture* playTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

		SDL_Rect textRect = playRect;
		SDL_QueryTexture(playTexture, NULL, NULL, &textRect.w, &textRect.h);

		// Center the text inside the button
		textRect.x = playRect.x + (playRect.w - textRect.w) / 2;
		textRect.y = playRect.y + (playRect.h - textRect.h) / 2;

		SDL_RenderCopy(renderer, playTexture, NULL, &textRect);

		// Draw "Instructions" button
		SDL_Rect instrRect={ 1280 - 350, 50+80+20, 300, 80 };
		
		SDL_RenderCopy(renderer, playButtonTexture, NULL, &instrRect);

		// Render "Instructions" text
		SDL_Surface* instrSurface = TTF_RenderText_Solid(largeFont, "Instructions", textColor);
		SDL_Texture* instrTexture = SDL_CreateTextureFromSurface(renderer, instrSurface);
		SDL_Rect instrTextRect = instrRect;
		SDL_QueryTexture(instrTexture, NULL, NULL, &instrTextRect.w, &instrTextRect.h);
		instrTextRect.x = instrRect.x + (instrRect.w - instrTextRect.w) / 2;
		instrTextRect.y = instrRect.y + (instrRect.h - instrTextRect.h) / 2;
		SDL_RenderCopy(renderer, instrTexture, NULL, &instrTextRect);

		// Draw "Leaderboard" button
		SDL_Rect lbRect={ 1280 - 350,50+2*( 80 + 20), 300, 80 };
		SDL_RenderCopy(renderer, playButtonTexture, NULL, &lbRect);

		// Render "Leaderboard" text
		SDL_Surface* lbSurface = TTF_RenderText_Solid(largeFont, "Leaderboard", textColor);
		SDL_Texture* lbTexture = SDL_CreateTextureFromSurface(renderer, lbSurface);
		SDL_Rect lbTextRect = lbRect;
		SDL_QueryTexture(lbTexture, NULL, NULL, &lbTextRect.w, &lbTextRect.h);
		lbTextRect.x = lbRect.x + (lbRect.w - lbTextRect.w) / 2;
		lbTextRect.y = lbRect.y + (lbRect.h - lbTextRect.h) / 2;
		SDL_RenderCopy(renderer, lbTexture, NULL, &lbTextRect);
		
		SDL_Rect credRect = { 1280 - 350, 50 + 3 * (80 + 20), 300, 80 };
		SDL_RenderCopy(renderer, playButtonTexture, NULL, &credRect);

		// Render "Credits" text
		SDL_Surface* credSurface = TTF_RenderText_Solid(largeFont, "Credits", textColor);
		SDL_Texture* credTexture = SDL_CreateTextureFromSurface(renderer, credSurface);
		SDL_Rect credTextRect = credRect;
		SDL_QueryTexture(credTexture, NULL, NULL, &credTextRect.w, &credTextRect.h);
		credTextRect.x = credRect.x + (credRect.w - credTextRect.w) / 2;
		credTextRect.y = credRect.y + (credRect.h - credTextRect.h) / 2;
		SDL_RenderCopy(renderer, credTexture, NULL, &credTextRect);



		SDL_Rect quitRect = { 1280 - 350, 50 + 4 * (80 + 20), 300, 80 };
		SDL_RenderCopy(renderer, playButtonTexture, NULL, &quitRect);

		// Render "Quit" text
		SDL_Surface* quitSurface = TTF_RenderText_Solid(largeFont, "Quit", textColor);
		SDL_Texture* quitTexture = SDL_CreateTextureFromSurface(renderer, quitSurface);
		SDL_Rect quitTextRect = quitRect;
		SDL_QueryTexture(quitTexture, NULL, NULL, &quitTextRect.w, &quitTextRect.h);
		quitTextRect.x = quitRect.x + (quitRect.w - quitTextRect.w) / 2;
		quitTextRect.y = quitRect.y + (quitRect.h - quitTextRect.h) / 2;
		SDL_RenderCopy(renderer, quitTexture, NULL, &quitTextRect);

		// Clean up
		SDL_FreeSurface(quitSurface);
		SDL_DestroyTexture(quitTexture);
		SDL_FreeSurface(credSurface);
		SDL_DestroyTexture(credTexture);
		SDL_DestroyTexture(playTexture);
		SDL_FreeSurface(instrSurface);
		SDL_DestroyTexture(instrTexture);
		SDL_DestroyTexture(lbTexture);
		SDL_FreeSurface(lbSurface);
		SDL_DestroyTexture(playButtonTexture);
		TTF_CloseFont(largeFont);
	
    }

	//SDL_Texture* bgTexture = loadTexture((char*)"space1.png"); 

// Game Over
	if (gameState == GameState::GAMEOVER) {
		Mix_ResumeMusic();

		// Load your background texture
		SDL_Texture* bgTexture = loadTexture((char*)"space1.png"); // Replace with your actual texture path
		SDL_Rect fullScreenRect = { 0, 0, 1280, 720 };
		SDL_RenderCopy(renderer, bgTexture, NULL, &fullScreenRect);

		// Load the word art texture for "Game Over"
		SDL_Texture* wordArtTexture = loadTexture((char*)"test3.png"); // Replace with your word art texture path
		SDL_Rect wordArtRect = { 1280 / 2 - 240, 50, 480, 150 }; // Adjust these values based on the size of your texture
		SDL_RenderCopy(renderer, wordArtTexture, NULL, &wordArtRect);

		// Render Final Score
		std::string finalScoreText = "Your Score: " + std::to_string(finalscore);
		SDL_Surface* finalScoreSurface = TTF_RenderText_Solid(font, finalScoreText.c_str(), textColor);
		SDL_Texture* finalScoreTexture = SDL_CreateTextureFromSurface(renderer, finalScoreSurface);
		SDL_Rect finalScoreRect = { 1280 / 2 - finalScoreSurface->w / 2, 220, finalScoreSurface->w, finalScoreSurface->h }; // Adjust position as needed
		SDL_RenderCopy(renderer, finalScoreTexture, NULL, &finalScoreRect);

		// Render "Restart" text
		SDL_Surface* restartSurface = TTF_RenderText_Solid(font, "Restart", textColor);
		SDL_Texture* restartTexture = SDL_CreateTextureFromSurface(renderer, restartSurface);
		SDL_Rect restartRect = { 1280 / 2 - restartSurface->w / 2, 280 + 50 , restartSurface->w, restartSurface->h };
		SDL_RenderCopy(renderer, restartTexture, NULL, &restartRect);

		// Load button textures for "Yes" and "No"
		SDL_Texture* yesButtonTexture = loadTexture((char*)"greenbut.png"); // Replace with your "Yes" button texture path
		SDL_Texture* noButtonTexture = loadTexture((char*)"redbut.png"); // Replace with your "No" button texture path

		// Render "Yes" button texture
		SDL_Rect yesBgRect = { 1280 / 2 - 110, 340 + 50, 100, 50 }; // Adjust these values as needed
		SDL_RenderCopy(renderer, yesButtonTexture, NULL, &yesBgRect);

		// Render "No" button texture
		SDL_Rect noBgRect = { 1280 / 2 + 10, 340 + 50, 100, 50 }; // Adjust these values as needed
		SDL_RenderCopy(renderer, noButtonTexture, NULL, &noBgRect);

		// Render "Yes" text
		SDL_Surface* yesSurface = TTF_RenderText_Solid(font, "YES", textColor);
		SDL_Texture* yesTexture = SDL_CreateTextureFromSurface(renderer, yesSurface);
		SDL_Rect yesRect = { yesBgRect.x + (yesBgRect.w - yesSurface->w) / 2, yesBgRect.y + (yesBgRect.h - yesSurface->h) / 2, yesSurface->w, yesSurface->h };
		SDL_RenderCopy(renderer, yesTexture, NULL, &yesRect);

		// Render "No" text
		SDL_Surface* noSurface = TTF_RenderText_Solid(font, "NO", textColor);
		SDL_Texture* noTexture = SDL_CreateTextureFromSurface(renderer, noSurface);
		SDL_Rect noRect = { noBgRect.x + (noBgRect.w - noSurface->w) / 2, noBgRect.y + (noBgRect.h - noSurface->h) / 2, noSurface->w, noSurface->h };
		SDL_RenderCopy(renderer, noTexture, NULL, &noRect);

		// Cleanup
		SDL_FreeSurface(finalScoreSurface);
		SDL_DestroyTexture(finalScoreTexture);
		SDL_FreeSurface(restartSurface);
		SDL_DestroyTexture(restartTexture);
		SDL_FreeSurface(yesSurface);
		SDL_DestroyTexture(yesTexture);
		SDL_FreeSurface(noSurface);
		SDL_DestroyTexture(noTexture);
	}
}
