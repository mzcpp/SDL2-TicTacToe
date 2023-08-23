#include "States/BoardState.hpp"
#include "Utils/Constants.hpp"
#include "Game.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cassert>
#include <memory>
#include <vector>
#include <string>

std::unique_ptr<BoardState> BoardState::board_state_ = std::make_unique<BoardState>();

BoardState* BoardState::Instance()
{
	return board_state_.get();
}

bool BoardState::Enter(Game* game)
{
	InitBoard();

	std::srand(std::time(nullptr));
	std::rand();
	player_turn_ = std::rand() % 2;

	x_score_ = 0;
	o_score_ = 0;
	
	single_player_ = game->GameMode() == GameMode::SINGLE_PLAYER;

	game_ = game;
	font_ = TTF_OpenFont("res/font/font.ttf", 48);

	if (font_ == nullptr)
	{
		printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	return InitSymbolsTexture() && InitMessageTextures();
}

void BoardState::Exit()
{
	TTF_CloseFont(font_);
	font_ = nullptr;

	symbols_texture_->FreeTexture();

	for (const std::unique_ptr<Texture>& message_texture : message_textures_)
	{
		message_texture->FreeTexture();
	}
}

void BoardState::InitBoard()
{
	board_viewport_.x = 0;
	board_viewport_.y = 0;
	board_viewport_.w = constants::screen_width;
	board_viewport_.h = constants::screen_height * 9.0 / 12.0;

	score_viewport_.x = 0;
	score_viewport_.y = board_viewport_.h;
	score_viewport_.w = constants::screen_width;
	score_viewport_.h = constants::screen_height * 3.0 / 12.0;

	constexpr int rect_separator_width = 15;
	const int rect_side = (board_viewport_.w - (2 * rect_separator_width)) / 3;

	int rect_col = 0;
	int rect_row = 0;

	for (std::size_t i = 0; i < 9; ++i)
	{
		if (i == 3 || i == 6)
		{
			rect_col = 0;
			rect_row += rect_side + rect_separator_width;
		}

		Cell& board_cell = board_.grid_[i];

		board_cell.symbol_ = CellSymbol::EMPTY;
		board_cell.rect_.x = rect_col;
		board_cell.rect_.y = rect_row;
		board_cell.rect_.w = rect_side;
		board_cell.rect_.h = rect_side;
		board_cell.render_win_ = false;

		rect_col += rect_side + rect_separator_width;
	}

	board_.dimension_ = 3;
	board_.free_cells_ = board_.grid_.size();
	board_.n_symbols_to_win_ = board_.dimension_;
	board_.clicked_cell_index_ = -1;
	board_.win_ = false;
	board_.win_symbol_ = CellSymbol::EMPTY;
	board_.reset_ = false;
}

bool BoardState::InitSymbolsTexture()
{
	symbols_texture_ = std::make_unique<Texture>();

	if (!symbols_texture_->LoadFromPath(game_->GetRenderer(), "res/gfx/symbols.png"))
	{
		printf("Failed to load symbols texture!\n");
		return false;
	}

	constexpr int symbol_dimension = 230;

	for (std::size_t i = 0; i < 2; ++i)
	{
		symbols_sprites_clips_[i].x = i * symbol_dimension;
		symbols_sprites_clips_[i].y = 0;
		symbols_sprites_clips_[i].w = symbol_dimension;
		symbols_sprites_clips_[i].h = symbol_dimension;
	}

	return true;
}

bool BoardState::InitMessageTextures()
{
	message_textures_.resize(4);

	const SDL_Color text_color = { 0x00, 0x00, 0x00, 0xFF };
	const std::vector<std::string> messages = { "TIE", "Press M for menu", std::to_string(x_score_), std::to_string(o_score_) };

	for (std::size_t i = 0; i < message_textures_.size(); ++i)
	{
		message_textures_[i] = std::make_unique<Texture>();

		if (!message_textures_[i]->LoadFromText(game_->GetRenderer(), font_, messages[i].c_str(), text_color))
		{
			printf("Failed to render text!\n");
			return false;
		}
	}

	return true;
}

void BoardState::ResetBoard()
{
	for (Cell& cell : board_.grid_)
	{
		cell.symbol_ = CellSymbol::EMPTY;
		cell.render_win_ = false;
	}

	player_turn_ = !player_turn_;

	board_.free_cells_ = board_.grid_.size();
	board_.win_ = false;
	board_.reset_ = false;
}

void BoardState::Pause()
{
}
	
void BoardState::Resume()
{
}

void BoardState::HandleEvents()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			game_->Stop();
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			if (!board_.win_ && board_.free_cells_ != 0 && (!single_player_ || player_turn_))
			{
				SetClickedCellIndex();
			}
			else
			{
				board_.reset_ = true;
			}
		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_m)
		{
			game_->SetGameMode(GameMode::NONE);
			game_->PopState();
		}
	}
}

void BoardState::Tick()
{
	if (board_.clicked_cell_index_ != -1 && board_.free_cells_ != 0)
	{
		board_.grid_[board_.clicked_cell_index_].symbol_ = player_turn_ ? CellSymbol::X : CellSymbol::O;
		--board_.free_cells_;

		board_.clicked_cell_index_ = -1;
		
		board_.win_ = CheckWin(true, &board_.win_symbol_);

		if (board_.win_)
		{
			player_turn_ ? ++x_score_ : ++o_score_;
		}
		else
		{
			player_turn_ = !player_turn_;
		}
	}
	else if (single_player_ && !player_turn_ && !board_.win_ && board_.free_cells_ != 0)
	{
		board_.grid_[BestMove()].symbol_ = CellSymbol::O;
		--board_.free_cells_;
		
		board_.win_ = CheckWin(true, &board_.win_symbol_);

		if (board_.win_)
		{			
			++o_score_;
		}
		else
		{
			player_turn_ = !player_turn_;
		}
	}
	else if (board_.reset_)
	{
		ResetBoard();
	}
}

void BoardState::Render()
{
	SDL_Renderer* renderer = game_->GetRenderer();

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);

	RenderBoard();
	RenderInfo();

	SDL_RenderPresent(renderer);
}

void BoardState::RenderBoard()
{
	SDL_Renderer* renderer = game_->GetRenderer();

	SDL_RenderSetViewport(renderer, &board_viewport_);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	for (const Cell& board_cell : board_.grid_)
	{
		SDL_RenderFillRect(renderer, &board_cell.rect_);

		if (board_cell.render_win_)
		{
			SDL_SetRenderDrawColor(renderer, 0x00, 0xB4, 0x00, 0xFF);
			SDL_RenderFillRect(renderer, &board_cell.rect_);
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		}

		if (board_cell.symbol_ != CellSymbol::EMPTY)
		{
			const int symbol_sprite_index = board_cell.symbol_ == CellSymbol::X;
			symbols_texture_->Render(renderer, board_cell.rect_.x, board_cell.rect_.y, &symbols_sprites_clips_[symbol_sprite_index], 1.0);	
		}
	}
	
	SDL_RenderSetViewport(renderer, NULL);
}

void BoardState::RenderInfo()
{
	SDL_Renderer* renderer = game_->GetRenderer();

	SDL_RenderSetViewport(renderer, &score_viewport_);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	const SDL_Rect info_background_rect = { 0, 0, score_viewport_.w, score_viewport_.h };
	SDL_RenderFillRect(renderer, &info_background_rect);

	const double symbols_scale = 0.25;
	const SDL_Rect& symbol_rect = symbols_sprites_clips_[0];

	const int o_x = score_viewport_.w * 2 / 7;
	const int o_y = score_viewport_.h * 1 / 6;

	symbols_texture_->Render(renderer, o_x, o_y, &symbols_sprites_clips_[0], symbols_scale);

	const int x_x = score_viewport_.w * 5 / 7 - (symbol_rect.w * symbols_scale);
	const int x_y = score_viewport_.h * 1 / 6;

	symbols_texture_->Render(renderer, x_x, x_y, &symbols_sprites_clips_[1], symbols_scale);

	const int x_score_msg_index = static_cast<int>(MessageType::X_SCORE);
	const int o_score_msg_index = static_cast<int>(MessageType::O_SCORE);

	message_textures_[o_score_msg_index]->Render(renderer, o_x + ((symbol_rect.w * symbols_scale / 2) - message_textures_[o_score_msg_index]->Width() / 2), score_viewport_.h / 2);
	message_textures_[x_score_msg_index]->Render(renderer, x_x + ((symbol_rect.w * symbols_scale / 2) - message_textures_[x_score_msg_index]->Width() / 2), score_viewport_.h / 2);

	if (!board_.win_)
	{
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

		if (board_.free_cells_ == 0)
		{
			const int tie_msg_index = static_cast<int>(MessageType::TIE);

			message_textures_[tie_msg_index]->Render(renderer, 
													(score_viewport_.w / 2) - (message_textures_[tie_msg_index]->Width() / 2), 
													o_y * 2);
		}
		else
		{
			const int line_x = (player_turn_ || single_player_) ? x_x : o_x;
			const int line_y = (player_turn_ || single_player_) ? x_y : o_y;

			for (std::size_t i = 5; i < 10; ++i)
			{
				SDL_RenderDrawLine(renderer, line_x, line_y + i + symbol_rect.h * symbols_scale, line_x + symbol_rect.w * symbols_scale, line_y + i + symbol_rect.h * symbols_scale);
			}
		}
		
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);	
	}
	else
	{
		const int score_msg_index = static_cast<int>(player_turn_ ? MessageType::X_SCORE : MessageType::O_SCORE);
		const std::string updated_score = std::to_string(player_turn_ ? x_score_ : o_score_);

		if (!message_textures_[score_msg_index]->LoadFromText(game_->GetRenderer(), font_, updated_score.c_str(), { 0x00, 0x00, 0x00, 0xFF }))
		{
			printf("Failed to render score text!\n");
		}
	}

	const int menu_msg_index = static_cast<int>(MessageType::MENU);

	message_textures_[menu_msg_index]->Render(renderer, (score_viewport_.w / 2) - (message_textures_[menu_msg_index]->Width() / 2), score_viewport_.h - message_textures_[menu_msg_index]->Height());

	SDL_RenderSetViewport(renderer, NULL);
}

void BoardState::SetClickedCellIndex()
{
	SDL_Point mouse_position = { 0, 0 };
	SDL_GetMouseState(&mouse_position.x, &mouse_position.y);
	
	//printf("%d %d\n", mouse_position.x, mouse_position.y);

	for (std::size_t i = 0; i < board_.grid_.size(); ++i)
	{
		if (board_.grid_[i].symbol_ == CellSymbol::EMPTY && SDL_PointInRect(&mouse_position, &board_.grid_[i].rect_))
	 	{
	 		board_.clicked_cell_index_ = i;
	 		return;
		}
	}
}

bool BoardState::CheckWin(bool set_render_win_flag, CellSymbol* winning_symbol)
{
	return CheckRowsColsWin(set_render_win_flag, winning_symbol) || CheckDiagonalsWin(set_render_win_flag, winning_symbol);
}

bool BoardState::CheckRowsColsWin(bool set_render_win_flag, CellSymbol* winning_symbol)
{
	std::vector<std::size_t> rows_winning_indices;
	std::vector<std::size_t> cols_winning_indices;
	
	rows_winning_indices.reserve(board_.n_symbols_to_win_);
	cols_winning_indices.reserve(board_.n_symbols_to_win_);

	for (std::size_t i = 0; i < board_.dimension_; i++)
	{
	    for (std::size_t j = 0; j < board_.dimension_; j++)
	    {
			const std::size_t row_index = i * board_.dimension_ + j;

	    	if (CheckSymbolsWin(row_index, &rows_winning_indices, set_render_win_flag, winning_symbol))
	    	{
	    		return true;
	    	}

			const std::size_t col_index = j * board_.dimension_ + i;

	    	if (CheckSymbolsWin(col_index, &cols_winning_indices, set_render_win_flag, winning_symbol))
	    	{
	    		return true;
	    	}
	    }

		rows_winning_indices.clear();
		cols_winning_indices.clear();
	}

	return false;
}
	
bool BoardState::CheckDiagonalsWin(bool set_render_win_flag, CellSymbol* winning_symbol)
{
	const int board_dim = static_cast<int>(board_.dimension_);

	std::vector<std::size_t> winning_indices;
	winning_indices.reserve(board_.n_symbols_to_win_);

	for (int i = 0; i < board_dim * 2; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			const int k = i - j;
			
			if (k < board_dim && j < board_dim)
			{
				const int index = k * board_dim + j;
				assert(index >= 0);

				if (CheckSymbolsWin(static_cast<std::size_t>(index), &winning_indices, set_render_win_flag, winning_symbol))
		    	{
		    		return true;
		    	}
			}
		}

		winning_indices.clear();
	}

	winning_indices.clear();

	int l = 0;

	for (int i = 0; i < board_dim * 2; i++)
	{
		if (i >= board_dim)
		{
			++l;
		}

		for (int j = 0; j <= i; j++)
		{
			const int k = board_dim - j - 1;
			
			if (k >= 0 && k - l >= 0)
			{
				const int index = (i - j - l) * board_dim + (k - l);
				assert(index >= 0);

				if (CheckSymbolsWin(static_cast<std::size_t>(index), &winning_indices, set_render_win_flag, winning_symbol))
		    	{
		    		return true;
		    	}
			}
		}

		winning_indices.clear();
	}

	return false;
}

bool BoardState::CheckSymbolsWin(std::size_t index, std::vector<std::size_t>* winning_indices, bool set_render_win_flag, CellSymbol* winning_symbol)
{
	assert(winning_indices != nullptr);

	if (board_.grid_[index].symbol_ == CellSymbol::EMPTY)
	{
		winning_indices->clear();
		return false;
	}

	if (!winning_indices->empty() && board_.grid_[index].symbol_ != board_.grid_[winning_indices->back()].symbol_)
	{
		winning_indices->clear();
	}
	
	winning_indices->push_back(index);

	if (winning_indices->size() == board_.n_symbols_to_win_)
	{
		if (set_render_win_flag)
		{
			for (std::size_t winning_index : *winning_indices)
			{
				board_.grid_[winning_index].render_win_ = true;
			}
		}

		if (winning_symbol != nullptr)
		{
			*winning_symbol = board_.grid_[(*winning_indices)[0]].symbol_;
		}

		return true;
	}

	return false;
}

int BoardState::BestMove()
{
	int best_score = std::numeric_limits<int>::max();
	int best_index = 0;

	for (std::size_t i = 0; i < board_.grid_.size(); ++i)
	{
		if (board_.grid_[i].symbol_ == CellSymbol::EMPTY)
		{
			board_.grid_[i].symbol_ = CellSymbol::O;
			--board_.free_cells_;

			int score = Minimax(0, true);

			board_.grid_[i].symbol_ = CellSymbol::EMPTY;
			++board_.free_cells_;

			if (score < best_score)
			{
				best_score = score;
				best_index = i;
			}
		}
	}

	return best_index;
}

int BoardState::Minimax(int depth, bool is_maximizing)
{
	CellSymbol win_symbol = CellSymbol::EMPTY;
	const bool win = CheckWin(false, &win_symbol);

	if (win || depth == 2)
	{
		if (win_symbol == CellSymbol::X)
		{
			return 1;
		}
		else if (win_symbol == CellSymbol::O)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}

	if (board_.free_cells_ == 0)
	{
		return 0;
	}

	int best_index = is_maximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

	for (std::size_t i = 0; i < board_.grid_.size(); ++i)
	{
		if (board_.grid_[i].symbol_ == CellSymbol::EMPTY)
		{
			board_.grid_[i].symbol_ = is_maximizing ? CellSymbol::X : CellSymbol::O;
			--board_.free_cells_;

			int index = Minimax(depth + 1, !is_maximizing);
			
			board_.grid_[i].symbol_ = CellSymbol::EMPTY;
			++board_.free_cells_;
			
			best_index = is_maximizing ? std::max(index, best_index) : std::min(index, best_index);
		}
	}

	return best_index;
}
