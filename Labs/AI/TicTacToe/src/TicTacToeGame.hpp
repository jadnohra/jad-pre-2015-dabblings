#ifndef _TicTacToeGame_hpp
#define _TicTacToeGame_hpp

#include "SuperHeader.hpp"
#include "Array.hpp"
#include "Random.hpp"

namespace tictactoe {

	typedef int CellIndex;

	class TicTacToeGame;

	enum CellType {

		Cell_None = -1, Cell_Empty, Cell_X, Cell_O
	};

	struct Cell {

		CellType type;

		Cell() : type(Cell_None) {}

		inline bool isEmpty() const { return type == Cell_Empty; }
		inline bool is(const CellType& comp) const { return type == comp; }
	};

	class Player {
	public:

		Player() : mCellType(Cell_None) {}
		~Player() {}

		virtual void attachToGame(TicTacToeGame& game) {}

		inline CellType getOppositeCellType() { return mCellType == Cell_X ? Cell_O : Cell_X; }
		inline const CellType& getCellType() { return mCellType; }

		virtual void create(CellType cellType) {

			mCellType = cellType;
		}

		virtual void playTurn(TicTacToeGame& game) = 0;

	protected:

		CellType mCellType;
	};

	struct CellAddress {

		int horiz;
		int vert;

		CellAddress() {}
		CellAddress(const int& horiz, const int& vert) {
		
			this->horiz = horiz;
			this->vert = vert;
		}

		inline const bool operator==(const CellAddress& comp) const {

			return (this->horiz == comp.horiz && this->vert == comp.vert);
		}
	};

	class TicTacToeGame {
	public:

		TicTacToeGame();

		void create(int width = 3, int height = 3);

		void setPlayer(int index, Player* pPlayer) {

			if (index < 2)
				mPlayer[index] = pPlayer;
			else
				assert(false);
		}

		inline bool isValidAddress(const CellIndex& horiz, const CellIndex& vert) { 

			return (horiz >= 0 && horiz < gridWidth()) && (vert >= 0 && vert < gridHeight());
		}

		Cell* safeCell(const CellIndex& horiz, const CellIndex& vert) {

			if (isValidAddress(horiz, vert))
				return &cell(horiz, vert);

			return NULL;
		}

		const Cell* safeGetCell(const CellIndex& horiz, const CellIndex& vert) {

			return safeCell(horiz, vert);
		}

		inline CellIndex& gridWidth() { return mGridWidth; }
		inline CellIndex& gridHeight() { return mGridHeight; }

		const Cell& getCell(const CellIndex& horiz, const CellIndex& vert) { return mGrid[toGridIndex(horiz, vert)]; }
		Cell& cell(const CellIndex& horiz, const CellIndex& vert) { return mGrid[toGridIndex(horiz, vert)]; }

		inline CellIndex toGridIndex(const CellIndex& horiz, const CellIndex& vert) {

			return (vert * mGridWidth) + horiz;
		}

		inline void fromGridIndex(const CellIndex& gridIndex, CellIndex& outHoriz, CellIndex& outVert) {

			outVert = gridIndex / mGridWidth;
			outHoriz = gridIndex % mGridWidth;
		}

		bool isDetectedGameEnd(Player*& pOutWinner, bool detectWinningRows = true); 
		bool advanceTurn();

		void tickCell(const CellIndex& horiz, const CellIndex& vert);

		inline void resetRandomSeed(float seed) { mRandom.reset(seed); }
		inline int getRandom(int min, int max) { return mRandom.next(min, max); }
		inline unsigned int getRandom() { return mRandom.next(); }

		inline int getTurnIndex() { return mTurnIndex; }

		void debugPrintGrid();
		void debugPrintGridIndices();

	public:

		inline bool isValidAddress(const CellAddress& address) { return isValidAddress(address.horiz, address.vert); };
		inline const Cell& getCell(const CellAddress& address) { return getCell(address.horiz, address.vert); }
		inline void tickCell(const CellAddress& address) { return tickCell(address.horiz, address.vert); }

	protected:

		typedef StaticArray<Cell, int> Cells;

		CellIndex mGridWidth;
		CellIndex mGridHeight;

		Cells mGrid;

		PtrWeak<Player> mPlayer[2];

		PtrWeak<Player> mTurnPlayer;

		Random mRandom;

		int mTurnIndex;

	protected:

		enum { RowCount = 8 };
		enum { RowSize = 3 };

		static const CellAddress mRows[RowCount][RowSize];

		int countInRow(const int& rowIndex, const CellType& cellType);
	};

}

#endif