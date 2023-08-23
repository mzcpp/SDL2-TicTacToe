#include "Button.hpp"
#include "States/BoardState.hpp"

Button::Button(Game* game, TTF_Font* font) : 
	game_(game),
	font_(font),
	top_left_({ 0, 0 }), 
	button_texture_(std::make_unique<Texture>()), 
	text_(""), 
	mouse_over_(false), 
	reload_(false),
	enabled_(true)
{
}

void Button::SetPosition(int x, int y)
{
	top_left_.x = x;
	top_left_.y = y;
}

void Button::SetText(const std::string& text)
{
	text_ = text;
}

std::string Button::Text()
{
	return text_;
}

bool Button::MouseOver()
{
	return mouse_over_;
}

void Button::SetReload(bool reload)
{
	reload_ = reload;
}
	
bool Button::Reload()
{
	return reload_;
}

void Button::SetEnabled(bool enabled)
{
	enabled_ = enabled;
}

bool Button::Enabled()
{
	return enabled_;
}

Texture* Button::GetTexture()
{
	return button_texture_.get();
}

void Button::SetButtonFlags()
{
	const bool mouse_overlaps_button = MouseOverlapsButton();

	if ((mouse_over_ && !mouse_overlaps_button) || (!mouse_over_ && mouse_overlaps_button))
	{
		mouse_over_ = mouse_overlaps_button;
		reload_ = true;
	}
}

void Button::HandleEvent(SDL_Event* e)
{
	if (e->type == SDL_MOUSEMOTION)
	{
		SetButtonFlags();
	}
	else if (e->type == SDL_MOUSEBUTTONUP && MouseOverlapsButton())
	{
		game_->SetGameMode(text_ == "Singleplayer" ? GameMode::SINGLE_PLAYER : GameMode::MULTI_PLAYER);
		game_->PushState(BoardState::Instance());
	}
}

void Button::Tick(bool force_update)
{
	if (reload_ || force_update)
	{
		SDL_Color text_color = { 0x00, 0x00, 0x00, 0xFF };

		if (mouse_over_)
		{
			text_color = { 0xFF, 0x00, 0x00, 0xFF };
		}

		if (!enabled_)
		{
			text_color = { 0x00, 0x00, 0x00, 0x19 };
		}

		FreeTexture();
		
		button_texture_->LoadFromText(game_->GetRenderer(), font_, text_.c_str(), text_color);
		reload_ = false;
	}
}

void Button::Render(SDL_Renderer* renderer)
{
	button_texture_->Render(renderer, top_left_.x, top_left_.y);
}

void Button::FreeTexture()
{
	button_texture_->FreeTexture();
}

bool Button::MouseOverlapsButton() const
{
	SDL_Point mouse_position;
	SDL_GetMouseState(&mouse_position.x, &mouse_position.y);
	
	SDL_Rect button_bounding_box = { top_left_.x, top_left_.y, button_texture_->Width(), button_texture_->Height() };

	return enabled_ && SDL_PointInRect(&mouse_position, &button_bounding_box);
}