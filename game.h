#pragma once

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include "structs.h"
#include "sound.h"
#include <SDL_ttf.h>
#include <string>

enum class GameState { PLAY, EXIT, GAMEOVER, PLAYAGAIN, START, MENU, PAUSE };


class Game {
public:
    Game(const char* title, int x, int y, int w, int h, Uint32 flags);
    ~Game();
    void run(); // runs the game
   

private:
    void renderPauseScreen();
    void gameLoop(); // main game loop
    void handleEvents(); // handles game events
    void handleKeyDownEvent(SDL_KeyboardEvent* event);
    void handleMouseDownEvent(SDL_MouseButtonEvent* event);
    void prepareScene(); // prepares render
    void presentScene(); // displays render
    SDL_Texture* loadTexture(char* filename); // loads an image
    void blit(SDL_Texture* texture, int x, int y); // displays an image
    void doKeyUp(SDL_KeyboardEvent* event);
    void doKeyDown(SDL_KeyboardEvent* event);
    int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2); // checks if two rectangles overlap
    int bulletHitFighter(Entity* b, Entity fighterHead, Sound sound); // checks if a bullet has hit an enemy figher using collision function
    void drawHud(std::string textureText, SDL_Color textColor, TTF_Font* font); // draws the score and high score
    void playerHitEnemy(Entity player, Entity fighterHead, Sound sound);
    void playerHitLife(Entity player, Entity life, Sound sound);
    void openInstructionsWindow();
    void showLeaderboard();
    int finalscore = 0;
    void openCreditsWindow();
    Sound sound;
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Movement
    int up, down, left, right;
    TTF_Font* mainFont1;
    SDL_Color textColor1;
    int fire;
    int playerLifeScore = 1;
    int gameHighscore = 0;
    bool isLifeGenerated = false;

    GameState gameState = GameState::START;
};
