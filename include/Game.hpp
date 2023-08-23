#ifndef GAME_HPP
#define GAME_HPP

#include "Texture.hpp"

#include <SDL2/SDL.h>

#include <iostream>
#include <stack>

enum class GameMode
{
	NONE, SINGLE_PLAYER, MULTI_PLAYER
};

class GameState;

class Game
{
private:
	const char* title_;
	int screen_width_;
	int screen_height_;
	bool is_running_;
	GameMode game_mode_;

	SDL_Window* window_;
	SDL_Renderer* renderer_;

	std::stack<GameState*> states_;

public:
	Game();

	~Game();

	bool IsRunning();

	SDL_Window* GetWindow();
	
	SDL_Renderer* GetRenderer();
	
	[[nodiscard]] bool Initialize();

	void ChangeState(GameState* state);

	void PushState(GameState* state);

	void PopState();
	
	void Finalize();
	
	void Run();

	void Stop();

	void SetGameMode(enum GameMode mode);
	
	GameMode GameMode();

	void HandleEvents();

	void Tick();

	void Render();
};

#endif