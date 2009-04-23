#include "TicTacToeGame.hpp"

namespace tictactoe {

	/* 
	 *	Random AI, useful for testing
	 */

	class PlayerAIRandom : public Player {
	public:

		virtual void playTurn(TicTacToeGame& game) {

			int targetIndex = game.getRandom(0, game.gridWidth() * game.gridHeight() * 2);
			int currIndex = 0;

			while (true) {

				for (CellIndex i = 0; i < game.gridWidth(); ++i) {

					for (CellIndex j = 0; j < game.gridHeight(); ++j) {

						if (game.getCell(i, j).isEmpty()) {

							if (currIndex == targetIndex) {

								game.tickCell(i, j);
								return;
							}

							++currIndex;
						}
					}
				}
			}
		}
	};

}
