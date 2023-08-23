#ifndef MENU_STATE_HPP
#define MENU_STATE_HPP

#include "Button.hpp"
#include "States/GameState.hpp"
#include "Texture.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <vector>

class Game;

class MenuState : public GameState
{
private:
	static std::unique_ptr<MenuState> menu_state_;

	TTF_Font* font_;
	Game* game_;

	std::unique_ptr<Texture> title_texture_;
	std::vector<std::unique_ptr<Button>> menu_buttons_;

	bool InitTextures();

public:
	MenuState() = default;

	static MenuState* Instance();

	bool Enter(Game* game);
	
	void Exit();

	void Pause();
	
	void Resume();

	void HandleEvents();
	
	void Tick();
	
	void Render();
};

#endif