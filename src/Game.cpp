#include "States/MenuState.hpp"
#include "Utils/Constants.hpp"
#include "Game.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>

Game::Game() : 
	title_(constants::game_title), 
	screen_width_(constants::screen_width), 
	screen_height_(constants::screen_height), 
	is_running_(false),
	game_mode_(GameMode::NONE),
	window_(nullptr),
	renderer_(nullptr)
{
}

Game::~Game()
{
	while (!states_.empty())
	{
		states_.top()->Exit();
		states_.pop();
	}

	Finalize();
}

bool Game::IsRunning()
{
	return is_running_;
}

SDL_Window* Game::GetWindow()
{
	return window_;
}

SDL_Renderer* Game::GetRenderer()
{
	return renderer_;
}

bool Game::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not be initialized! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
	{
		printf("%s\n", "Warning: Texture filtering is not enabled!");
	}

	window_ = SDL_CreateWindow(title_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width_, screen_height_, SDL_WINDOW_SHOWN);

	if (window_ == nullptr)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);

	if (renderer_ == nullptr)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_SetRenderDrawColor(renderer_, 0xFF, 0xFF, 0xFF, 0xFF);

	constexpr int img_flags = IMG_INIT_PNG;

	if (!(IMG_Init(img_flags) & img_flags))
	{
		printf("SDL_image could not be initialized! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not be initialized! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	return true;
}

void Game::ChangeState(GameState* state)
{
	if (!states_.empty())
	{
		states_.top()->Exit();
		states_.pop();
	}

	states_.emplace(state);
	states_.top()->Enter(this);
}
	
void Game::PushState(GameState* state)
{
	if (!states_.empty())
	{
		states_.top()->Pause();
	}

	states_.emplace(state);
	states_.top()->Enter(this);
}

void Game::PopState()
{
	if (!states_.empty())
	{
		states_.top()->Exit();
		states_.pop();
	}

	if (!states_.empty())
	{
		states_.top()->Resume();
	}
}

void Game::Finalize()
{
	SDL_DestroyWindow(window_);
	window_ = nullptr;
	
	SDL_DestroyRenderer(renderer_);
	renderer_ = nullptr;

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void Game::Run()
{
	if (!Initialize())
	{
		Finalize();
		return;
	}

	is_running_ = true;
	ChangeState(MenuState::Instance());

	constexpr long double ms = 1.0 / 60.0;

	std::uint64_t last_time = SDL_GetPerformanceCounter();
	long double delta = 0.0;

	double timer = SDL_GetTicks();

	int frames = 0;
	int ticks = 0;

	while (is_running_)
	{
		const std::uint64_t now = SDL_GetPerformanceCounter();
		const long double elapsed = static_cast<long double>(now - last_time) / static_cast<long double>(SDL_GetPerformanceFrequency());
		
		last_time = now;
		delta += elapsed;

		HandleEvents();

		while (delta >= ms)
		{
			Tick();
			delta -= ms;
			++ticks;
		}

		//printf("%Lf\n", delta / ms);
		Render();
		++frames;

		if (SDL_GetTicks() - timer > 1000)
		{
			timer += 1000;
			printf("Frames: %d, Ticks: %d\n", frames, ticks);
			frames = 0;
			ticks = 0;
		}
	}
}

void Game::Stop()
{
	is_running_ = false;
}

void Game::SetGameMode(enum GameMode mode)
{
	game_mode_ = mode;
}
	
GameMode Game::GameMode()
{
	return game_mode_;
}

void Game::HandleEvents()
{
	states_.top()->HandleEvents();		
}

void Game::Tick()
{
	states_.top()->Tick();
}

void Game::Render()
{	
	states_.top()->Render();
}
