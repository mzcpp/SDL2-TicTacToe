#include "Button.hpp"
#include "States/BoardState.hpp"
#include "States/MenuState.hpp"
#include "Utils/Constants.hpp"
#include "Game.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cassert>
#include <memory>
#include <vector>
#include <string>

std::unique_ptr<MenuState> MenuState::menu_state_ = std::make_unique<MenuState>();

MenuState* MenuState::Instance()
{
	return menu_state_.get();
}

bool MenuState::Enter(Game* game)
{
	game_ = game;
	font_ = TTF_OpenFont("res/font/font.ttf", 58);

	if (font_ == nullptr)
	{
		printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	return InitTextures();
}

void MenuState::Exit()
{
	TTF_CloseFont(font_);
	font_ = nullptr;

	title_texture_->FreeTexture();

	for (const std::unique_ptr<Button>& button : menu_buttons_)
	{
		button->FreeTexture();
	}
}

bool MenuState::InitTextures()
{
	title_texture_ = std::make_unique<Texture>();

	if (!title_texture_->LoadFromPath(game_->GetRenderer(), "res/gfx/title.png"))
	{
		printf("Failed to load title texture!\n");
		return false;
	}

	const std::vector<std::string> menu_texts = { "Singleplayer", "Multiplayer" };
	const int button_height = 400;

	menu_buttons_.resize(2);

	for (std::size_t i = 0; i < menu_texts.size(); ++i)
	{
		menu_buttons_[i] = std::make_unique<Button>(game_, font_);
		menu_buttons_[i]->SetText(menu_texts[i].c_str());
		menu_buttons_[i]->Tick(true);
		menu_buttons_[i]->SetPosition((constants::screen_width / 2) - (menu_buttons_[i]->GetTexture()->Width() / 2), button_height + i * 100);
	}

	return true;
}

void MenuState::Pause()
{
}
	
void MenuState::Resume()
{
}

void MenuState::HandleEvents()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			game_->Stop();
		}
		else if (e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEMOTION)
		{
			for (const std::unique_ptr<Button>& button : menu_buttons_)
			{
				button->HandleEvent(&e);
			}
		}
	}
}

void MenuState::Tick()
{
	for (const std::unique_ptr<Button>& button : menu_buttons_)
	{
		button->Tick();
	}
}

void MenuState::Render()
{
	SDL_Renderer* renderer = game_->GetRenderer();

	SDL_RenderSetViewport(renderer, NULL);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(renderer);

	SDL_Rect src = { 0, 0, title_texture_->Width(), title_texture_->Height() };

	const int title_height = 200;
	constexpr double scale = 6.0;

	title_texture_->Render(renderer, (constants::screen_width / 2) - ((title_texture_->Width() * scale) / 2), title_height, &src, scale);

	for (const std::unique_ptr<Button>& button : menu_buttons_)
	{
		button->Render(renderer);
	}

	SDL_RenderPresent(renderer);
}