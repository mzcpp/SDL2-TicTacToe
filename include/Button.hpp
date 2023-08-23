#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "Game.hpp"
#include "Texture.hpp"

#include <SDL2/SDL.h>

#include <memory>
#include <string>

class Button
{
private:
	Game* game_;
	TTF_Font* font_;
	SDL_Point top_left_;
	std::unique_ptr<Texture> button_texture_;
	std::string text_;
	bool mouse_over_;
	bool reload_;
	bool enabled_;

	void SetButtonFlags();
	
	bool MouseOverlapsButton() const;

public:
	Button(Game* game, TTF_Font* font);

	void SetPosition(int x, int y);

	void SetText(const std::string& text);
	
	std::string Text();

	bool MouseOver();

	void SetReload(bool reload);

	bool Reload();

	void SetEnabled(bool enabled);

	bool Enabled();

	Texture* GetTexture();

	void HandleEvent(SDL_Event* e);

	void Tick(bool force_update = false);

	void Render(SDL_Renderer* renderer);

	void FreeTexture();
};

#endif