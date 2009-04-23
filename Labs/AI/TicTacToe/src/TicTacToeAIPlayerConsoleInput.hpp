#include "TicTacToeGame.hpp"

namespace tictactoe {

	/* 
	 *	AI that takes it's input from the console enabling Human players
	 */
	class PlayerConsoleInput : public Player {
	public:

		virtual void playTurn(TicTacToeGame& game);
	};

}
