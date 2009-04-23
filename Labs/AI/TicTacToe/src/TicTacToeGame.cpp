#include "TicTacToeGame.hpp"

namespace tictactoe {

const CellAddress TicTacToeGame::mRows[RowCount][RowSize] = 
{
	{CellAddress(0,0), CellAddress(1,0), CellAddress(2,0)},
	{CellAddress(0,1), CellAddress(1,1), CellAddress(2,1)},
	{CellAddress(0,2), CellAddress(1,2), CellAddress(2,2)},

	{CellAddress(0,0), CellAddress(0,1), CellAddress(0,2)},
	{CellAddress(1,0), CellAddress(1,1), CellAddress(1,2)},
	{CellAddress(2,0), CellAddress(2,1), CellAddress(2,2)},

	{CellAddress(0,0), CellAddress(1,1), CellAddress(2,2)},
	{CellAddress(2,0), CellAddress(1,1), CellAddress(0,2)},
};

TicTacToeGame::TicTacToeGame() {

	resetRandomSeed(0);
}

void TicTacToeGame::create(int width, int height) {

	assert(width > 0);
	assert(height > 0);

	mTurnIndex = 0;

	mGridWidth = width;
	mGridHeight = height;

	mGrid.create(toGridIndex(mGridWidth, mGridHeight));

	for (CellIndex i = 0; i < gridWidth(); ++i) {

		for (CellIndex j = 0; j < gridHeight(); ++j) {

			cell(i, j).type = Cell_Empty;
		}
	}
}

void TicTacToeGame::debugPrintGrid() {

	printf("\n"); 
	
	for (CellIndex y = 0; y < gridHeight(); ++y) {

		for (CellIndex x = 0; x < gridWidth(); ++x) {

			switch (getCell(x, y).type) {

				case Cell_None: printf("N "); break;
				case Cell_Empty: printf("- "); break;
				case Cell_X: printf("X "); break;
				case Cell_O: printf("O "); break;
				default: printf("? "); break;
			}
		}

		printf("\n"); 
	}
}

void TicTacToeGame::debugPrintGridIndices() {

	printf("\n"); 
	
	for (CellIndex y = 0; y < gridHeight(); ++y) {

		for (CellIndex x = 0; x < gridWidth(); ++x) {

			printf("%d ", (int) toGridIndex(x, y));
		}

		printf("\n"); 
	}
}


bool TicTacToeGame::isDetectedGameEnd(Player*& pOutWinner, bool detectWinningRows) {

	pOutWinner = NULL;

	if (detectWinningRows)
	{
		//TODO

		for (int pi = 0; pi < 2; ++pi) {

			for (int i = 0; i < RowCount; ++i) {

				if (countInRow(i, mPlayer[pi]->getCellType()) == RowSize) {

					pOutWinner = mPlayer[pi];
					return true;
				}
			}
		}
	}

	{
		for (CellIndex i = 0; i < gridWidth(); ++i) {

			for (CellIndex j = 0; j < gridHeight(); ++j) {

				if (getCell(i, j).type == Cell_Empty)
					return false;
			}
		}

		return true;
	}

	return false;
}

bool TicTacToeGame::advanceTurn() {

	PtrWeak<Player> winner;

	if (isDetectedGameEnd(winner.ptrRef()))
		return false;

	if (mTurnPlayer.isNull()) {

		assert(mPlayer[0].isValid());
		assert(mPlayer[1].isValid());

		mPlayer[0]->attachToGame(dref(this));
		mPlayer[1]->attachToGame(dref(this));

		mTurnPlayer = mPlayer[0];
	}

	mTurnPlayer->playTurn(dref(this));

	mTurnPlayer = (mTurnPlayer.ptr() == mPlayer[0].ptr() ? mPlayer[1].ptr() : mPlayer[0].ptr());

	++mTurnIndex;

	return true;
}

void TicTacToeGame::tickCell(const CellIndex& horiz, const CellIndex& vert) {

	assert(mTurnPlayer.isValid());
	Cell* pCell = safeCell(horiz, vert);

	assert(pCell);
	assert(pCell->type == Cell_Empty);

	pCell->type = mTurnPlayer->getCellType();
}

int TicTacToeGame::countInRow(const int& rowIndex, const CellType& cellType) {

	int count = 0;

	for (int i = 0; i < RowSize; ++i) {

		if (getCell(mRows[rowIndex][i]).is(cellType)) {

			++count;
		}
	}

	return count;
}

}