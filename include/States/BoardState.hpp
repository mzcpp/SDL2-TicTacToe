#ifndef BOARD_STATE_HPP
#define BOARD_STATE_HPP

#include "States/GameState.hpp"
#include "Texture.hpp"

#include <SDL2/SDL.h>

#include <memory>
#include <array>
#include <vector>

class Game;

enum class CellSymbol
{
	EMPTY, X, O
};

struct Cell
{
	CellSymbol symbol_;
	SDL_Rect rect_;
	bool render_win_;
};

enum class MessageType
{
	TIE, MENU, X_SCORE, O_SCORE
};

struct Board
{
	std::array<Cell, 9> grid_;
	std::size_t dimension_;
	std::size_t free_cells_;
	std::size_t n_symbols_to_win_;
	
	int clicked_cell_index_;
	bool win_;
	CellSymbol win_symbol_;
	bool reset_;
};

class BoardState : public GameState
{
private:
	static std::unique_ptr<BoardState> board_state_;
	
	Board board_;

	bool player_turn_;
	int x_score_;
	int o_score_;
	bool single_player_;

	TTF_Font* font_;
	Game* game_;

	std::unique_ptr<Texture> symbols_texture_;
	std::vector<std::unique_ptr<Texture>> message_textures_;
	SDL_Rect symbols_sprites_clips_[2];
	SDL_Rect board_viewport_;
	SDL_Rect score_viewport_;

	void InitBoard();
	
	bool InitSymbolsTexture();
	
	bool InitMessageTextures();

	void ResetBoard();

	void RenderBoard();
	
	void RenderInfo();

	void SetClickedCellIndex();

	bool CheckWin(bool set_render_win_flag = true, CellSymbol* winning_symbol = nullptr);

	bool CheckRowsColsWin(bool set_render_win_flag = true, CellSymbol* winning_symbol = nullptr);
	
	bool CheckDiagonalsWin(bool set_render_win_flag = true, CellSymbol* winning_symbol = nullptr);

	bool CheckSymbolsWin(std::size_t index, std::vector<std::size_t>* winning_indices, bool set_render_win_flag = true, CellSymbol* winning_symbol = nullptr);

	int BestMove();

	int Minimax(int depth, bool is_maximizing);

public:
	BoardState() = default;

	static BoardState* Instance();

	bool Enter(Game* game);
	
	void Exit();

	void Pause();
	
	void Resume();

	void HandleEvents();
	
	void Tick();
	
	void Render();
};

#endif