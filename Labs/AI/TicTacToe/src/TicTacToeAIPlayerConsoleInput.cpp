#include "TicTacToeAIPlayerConsoleInput.hpp"

#include <iostream>

namespace tictactoe {

void PlayerConsoleInput::playTurn(TicTacToeGame& game) {

	CellAddress address;
	CellIndex index;
	bool isSet = false;

	while (!isSet) {

		isSet = true;

		printf("\n");
		std::cout << "choose square: ";
		std::cin >> index;

		game.fromGridIndex(index, address.horiz, address.vert);

		if (!game.isValidAddress(address)) {

			printf("invalid\n");
			isSet = false;

		} else if (!game.getCell(address).isEmpty()) {

			printf("occupied\n");
			isSet = false;
		}
	} 

	if (isSet) {

		game.tickCell(address);
	}
}

}
