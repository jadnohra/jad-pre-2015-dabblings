#include "TicTacToeAIPlayerHeuristic.hpp"

namespace tictactoe {

const CellAddress PlayerAIHeuristic::mRows[RowCount][RowSize] = 
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

PlayerAIHeuristic::CornerEnum PlayerAIHeuristic::oppositeCorner(const CornerEnum& corner) {

	switch (corner) {

		case Corner_TopLeft: return Corner_BottomRight;
		case Corner_TopRight: return Corner_BottomLeft;
		case Corner_BottomLeft: return Corner_TopRight;
		case Corner_BottomRight: return Corner_TopLeft;
	}

	return Corner_None;
}


void PlayerAIHeuristic::attachToGame(TicTacToeGame& game) {

	//this AI can only play 3x3 
	assert((game.gridWidth() == 3) && (game.gridHeight() == 3));

	{
		mCorners[Corner_TopLeft].horiz = 0;
		mCorners[Corner_TopLeft].vert = 0;

		mCorners[Corner_TopRight].horiz = game.gridWidth() - 1;
		mCorners[Corner_TopRight].vert = 0;

		mCorners[Corner_BottomLeft].horiz = 0;
		mCorners[Corner_BottomLeft].vert = game.gridHeight() - 1;

		mCorners[Corner_BottomRight].horiz = game.gridWidth() - 1;
		mCorners[Corner_BottomRight].vert = game.gridHeight() - 1;
	}

	{
		mCenter.horiz = (game.gridWidth() / 2);
		mCenter.vert = (game.gridHeight() / 2);
	}
}

void PlayerAIHeuristic::playTurn(TicTacToeGame& game) {

	bool didNotPlay = false;

	if (!playTurn_Rule1(game))
		if (!playTurn_Rule2(game))
			if (!playTurn_Rule3(game))
				if (!playTurn_Rule4(game))
					if (!playTurn_Rule5(game))
						if (!playTurn_Rule6(game))
							if (!playTurn_Rule7(game))
								if (!playTurn_Rule8(game)) {

									didNotPlay = true;
								}

	assert(didNotPlay == false);
}

bool PlayerAIHeuristic::playTurn_Rule1(TicTacToeGame& game) {

	int rowIndex = 0;

	while (findNextRowWith(game, getCellType(), 2, rowIndex)) {

		for (int i = 0; i < RowSize; ++i) {

			if (game.getCell(mRows[rowIndex][i]).isEmpty()) {

				game.tickCell(mRows[rowIndex][i]);
				return true;
			}
		}

		++rowIndex;
	}

	return false;
}

bool PlayerAIHeuristic::playTurn_Rule2(TicTacToeGame& game) {

	int rowIndex = 0;

	while (findNextRowWith(game, getOppositeCellType(), 2, rowIndex)) {

		for (int i = 0; i < RowSize; ++i) {

			if (game.getCell(mRows[rowIndex][i]).isEmpty()) {

				game.tickCell(mRows[rowIndex][i]);
				return true;
			}
		}

		++rowIndex;
	}

	return false;
}

bool PlayerAIHeuristic::playTurn_Rule3(TicTacToeGame& game) {

	CellAddress forkAddress;

	if (findForkOpportunity(game, getCellType(), forkAddress)) {

		game.tickCell(forkAddress);
		return true;
	}

	return false;
}

bool PlayerAIHeuristic::playTurn_Rule4(TicTacToeGame& game) { 

	//option1
	{
		for (CellIndex i = 0; i < game.gridWidth(); ++i) {

			for (CellIndex j = 0; j < game.gridHeight(); ++j) {

				if (game.getCell(i, j).isEmpty()) {

					CellAddress testAddress(i, j);
					int checkRowIndex = 0;

					if (findNextRowWith(game, getCellType(), 2, checkRowIndex, &testAddress, &getCellType())) {

						game.tickCell(testAddress);
						return true;
					}
				}
			}
		}
	}

	//option2
	{
		CellAddress forkAddress;

		if (findForkOpportunity(game, getOppositeCellType(), forkAddress)) {

			game.tickCell(forkAddress);
			return true;
		}
	}

	return false; 
}

bool PlayerAIHeuristic::playTurn_Rule5(TicTacToeGame& game) { 
	
	if (game.getCell(mCenter).isEmpty()) {

		game.tickCell(mCenter);
		return true;
	}

	return false; 
}

bool PlayerAIHeuristic::playTurn_Rule6(TicTacToeGame& game) { 

	const CellType oppCellType = getOppositeCellType();

	for (int i = 0; i < CornerCount; ++i) {

		if (game.getCell(mCorners[i]).is(oppCellType)) {

			CornerEnum oppCorner = oppositeCorner((CornerEnum) i);

			if (game.getCell(mCorners[oppCorner]).isEmpty()) {

				game.tickCell(mCorners[oppCorner]);
				return true;
			}
		}
	}

	return false; 
}

bool PlayerAIHeuristic::playTurn_Rule7(TicTacToeGame& game) { 
	
	for (int i = 0; i < CornerCount; ++i) {

		if (game.getCell(mCorners[i]).isEmpty()) {

			game.tickCell(mCorners[i]);
			return true;
		}
	}

	return false; 
}

bool PlayerAIHeuristic::playTurn_Rule8(TicTacToeGame& game) { 
	
	for (CellIndex i = 0; i < game.gridWidth(); ++i) {

		for (CellIndex j = 0; j < game.gridHeight(); ++j) {

			if (game.getCell(i, j).isEmpty()) {

				game.tickCell(i, j);
				return true;
			}
		}
	}

	//no more cells
	return true; 
}

bool PlayerAIHeuristic::findForkOpportunity(TicTacToeGame& game, const CellType& cellType, CellAddress& forkAddress) {

	int checkRowIndex = 0;

	while (findNextRowWith(game, cellType, 2, checkRowIndex)) {

		//can we create a new different row with 2 occupied cells?

		for (CellIndex i = 0; i < game.gridWidth(); ++i) {

			for (CellIndex j = 0; j < game.gridHeight(); ++j) {

				if (game.getCell(i, j).isEmpty()) {

					//does setting this cell produce a new row with 2 cells? 
					CellAddress testAddress(i, j);

					int createdCheckRowIndex = 0;

					while (findNextRowWith(game, cellType, 2, createdCheckRowIndex, &testAddress, &cellType)) {

						if (createdCheckRowIndex != checkRowIndex) {

							//we detected a fork opportunity
							forkAddress = testAddress;

							return true;
						}

						++createdCheckRowIndex;
					}

				}
			}
		}

		++checkRowIndex;
	}

	return false;
}

bool PlayerAIHeuristic::findNextRowWith(TicTacToeGame& game, const CellType& cellType, const int& count, int& inOutRowIndex, const CellAddress* pOverrideCellAddress, const CellType* pOverrideCellType) {

	for (/*outRowIndex = 0*/; inOutRowIndex < RowCount; ++inOutRowIndex) {

		if (countInRow(game, inOutRowIndex, cellType, pOverrideCellAddress, pOverrideCellType) == count) {

			return true;
		}
	}

	return false;
}

int PlayerAIHeuristic::countInRow(TicTacToeGame& game, const int& rowIndex, const CellType& cellType, const CellAddress* pOverrideCellAddress, const CellType* pOverrideCellType) {

	if (pOverrideCellAddress)
		assert(pOverrideCellType);

	int count = 0;

	for (int i = 0; i < RowSize; ++i) {

		if (pOverrideCellAddress && *pOverrideCellAddress == mRows[rowIndex][i]) {

			if (cellType == *pOverrideCellType)
				++count;

		} else {

			if (game.getCell(mRows[rowIndex][i]).is(cellType)) {

				++count;
			}
		}
	}

	return count;
}

void PlayerAIHeuristic::fillWithNeighbors(TicTacToeGame& game, const CellAddress& address, CellAddressArray& arr, const CellType* pAllowedCellType) {

	arr.resize(0);

	CellAddress testAddress;

	//upper row
	if (game.isValidAddress(address.horiz, address.vert - 1)) {

		{
			testAddress.horiz = address.horiz - 1;
			testAddress.vert = address.vert - 1;

			if (game.isValidAddress(testAddress) && (!pAllowedCellType || game.getCell(testAddress).is(*pAllowedCellType)))
				arr.push_back(testAddress);
		}

		{
			testAddress.horiz = address.horiz;
			testAddress.vert = address.vert - 1;

			if (game.isValidAddress(testAddress) && (!pAllowedCellType || game.getCell(testAddress).is(*pAllowedCellType)))
				arr.push_back(testAddress);
		}

		{
			testAddress.horiz = address.horiz + 1;
			testAddress.vert = address.vert - 1;

			if (game.isValidAddress(testAddress) && (!pAllowedCellType || game.getCell(testAddress).is(*pAllowedCellType)))
				arr.push_back(testAddress);
		}
	}

	//same row
	{
		{
			testAddress.horiz = address.horiz - 1;
			testAddress.vert = address.vert;

			if (game.isValidAddress(testAddress) && (!pAllowedCellType || game.getCell(testAddress).is(*pAllowedCellType)))
				arr.push_back(testAddress);
		}
		
		{
			testAddress.horiz = address.horiz + 1;
			testAddress.vert = address.vert;

			if (game.isValidAddress(testAddress) && (!pAllowedCellType || game.getCell(testAddress).is(*pAllowedCellType)))
				arr.push_back(testAddress);
		}
	}

	//lower row
	if (game.isValidAddress(address.horiz, address.vert + 1)) {

		{
			testAddress.horiz = address.horiz - 1;
			testAddress.vert = address.vert + 1;

			if (game.isValidAddress(testAddress) && (!pAllowedCellType || game.getCell(testAddress).is(*pAllowedCellType)))
				arr.push_back(testAddress);
		}

		{
			testAddress.horiz = address.horiz;
			testAddress.vert = address.vert + 1;

			if (game.isValidAddress(testAddress) && (!pAllowedCellType || game.getCell(testAddress).is(*pAllowedCellType)))
				arr.push_back(testAddress);
		}

		{
			testAddress.horiz = address.horiz + 1;
			testAddress.vert = address.vert + 1;

			if (game.isValidAddress(testAddress) && (!pAllowedCellType || game.getCell(testAddress).is(*pAllowedCellType)))
				arr.push_back(testAddress);
		}
	}
}

}
