#include "TicTacToeGame.hpp"

namespace tictactoe {

	/* 
	 *	Alpha-Beta minimax version using code adapted from Steve Chapel's Tic-Tac-Toe program
	 *
	 *	Steve Chapel's description:
	 *	Tic-Tac-Toe program by Steve Chapel schapel@cs.ucsb.edu
	 *	Uses alpha-beta pruning minimax search to play a "perfect" game.
	 *	The alpha-beta pruning can be removed, but will increase search time.
	 *	The heuristic and move ordering in Best_Move() can also be removed with
	 *	an increase in search time. 
	*/

	class PlayerAIAlphaBetaMinimax : public Player {
	public:

		virtual void playTurn(TicTacToeGame& game);
	};

}
