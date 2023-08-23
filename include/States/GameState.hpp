#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "Game.hpp"

class GameState
{
public:
	virtual bool Enter(Game* game) = 0;
	virtual void Exit() = 0;

	virtual void Pause() = 0;
	virtual void Resume() = 0;

	virtual void HandleEvents() = 0;
	virtual void Tick() = 0;
	virtual void Render() = 0;

	void ChangeState(Game* game, GameState* state)
	{
		game->ChangeState(state);
	}

	virtual ~GameState()
	{
	}
};

#endif