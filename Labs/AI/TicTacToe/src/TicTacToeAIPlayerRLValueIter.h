#include "TicTacToeGame.hpp"

#include <map>

namespace tictactoe {

	/* 
	 *	This version is a simple Machine-Learning, Reinforcement learning value iteration implementation
	 *  it plays perfect Tic-Tac-Toe
	 */

	namespace PlayerRLValueIterDetail
	{
		typedef unsigned __int32 Key;
		struct StateInfo;
	}

	class PlayerRLValueIter : public Player {
	public:

		PlayerRLValueIter();
		virtual ~PlayerRLValueIter();

		virtual void playTurn(TicTacToeGame& game);

	protected:

		typedef std::map<PlayerRLValueIterDetail::Key, PlayerRLValueIterDetail::StateInfo*> StateInfoMap;

	protected:

		StateInfoMap* m_StateInfoMap;
	};

}
