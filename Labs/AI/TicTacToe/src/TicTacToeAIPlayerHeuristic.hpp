#include "TicTacToeGame.hpp"

#include <vector>

namespace tictactoe {

	/* 
	 *	Perfect playing AI based on Tic-Tac-Toe's perfect play rules
	 *  This is a direct implementation of the rules found in: http://en.wikipedia.org/wiki/Tic-Tac-Toe
	 */
	class PlayerAIHeuristic : public Player {
	public:

		virtual void attachToGame(TicTacToeGame& game);
		virtual void playTurn(TicTacToeGame& game);

	protected:

		bool playTurn_Rule1(TicTacToeGame& game);
		bool playTurn_Rule2(TicTacToeGame& game);
		bool playTurn_Rule3(TicTacToeGame& game);
		bool playTurn_Rule4(TicTacToeGame& game);
		bool playTurn_Rule5(TicTacToeGame& game);
		bool playTurn_Rule6(TicTacToeGame& game);
		bool playTurn_Rule7(TicTacToeGame& game);
		bool playTurn_Rule8(TicTacToeGame& game);

	protected:

		struct CellAddressArray : std::vector<CellAddress> {
		};

	protected:

		bool findForkOpportunity(TicTacToeGame& game, const CellType& cellType, CellAddress& opportunityAddress);
		bool findNextRowWith(TicTacToeGame& game, const CellType& cellType, const int& count, int& inOutRowIndex, const CellAddress* pOverrideCellAddress = NULL, const CellType* pOverrideCellType = NULL);
		int countInRow(TicTacToeGame& game, const int& rowIndex, const CellType& cellType, const CellAddress* pOverrideCellAddress = NULL, const CellType* pOverrideCellType = NULL);

		void fillWithNeighbors(TicTacToeGame& game, const CellAddress& address, CellAddressArray& arr, const CellType* pAllowedCellType = NULL);

	protected:

		enum CornerEnum {

			Corner_None = -1, Corner_TopLeft, Corner_TopRight, Corner_BottomLeft, Corner_BottomRight, CornerCount
		};

		static CornerEnum oppositeCorner(const CornerEnum& corner);

		enum { RowCount = 8 };
		enum { RowSize = 3 };

		static const CellAddress mRows[RowCount][RowSize];

		static const CellType kEmtpyCellType = Cell_Empty;

	protected:

		CellAddress mCorners[CornerCount];
		CellAddress mCenter;
	};



}
