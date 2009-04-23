#include "SuperHeader.hpp"

#include "TicTacToeGame.hpp"
#include "TicTacToeAIPlayerRandom.hpp"
#include "TicTacToeAIPlayerHeuristic.hpp"
#include "TicTacToeAIPlayerConsoleInput.hpp"
#include "TicTacToeAIPlayerAlphaBetaMinimax.hpp"
#include "TicTacToeAIPlayerRLValueIter.h"

#include <ctime>
#include <iostream>

template <typename P1, typename P2>
std::pair<int, int> testPlayers(int gameCount, bool doRandomSeed, bool switchStarter,
								 bool printDebug, int absoluteWinner = -1)
{
	using namespace tictactoe;
	
	int testCount = gameCount;
	bool testSucceeded = true;

	int p1WinCount = 0;
	int p2WinCount = 0;
	
	if (doRandomSeed)
		srand((unsigned int) time(0));

	using namespace tictactoe;

	int gameIndex = 0;
	while (gameIndex < testCount) {

		TicTacToeGame game;
		game.create();

		{
			float seed = (float) rand() / (float) RAND_MAX;
			seed = ((float) ((int) (seed * 1000.0f))) / 1000.0f;
			
			game.resetRandomSeed(seed);
		}

		if (printDebug)
		{
			game.debugPrintGrid();
		}

		P1 p1;
		P2 p2;

		if (switchStarter)
		{
			if (gameIndex % 2 == 0)
			{
				p1.create(Cell_X);
				p2.create(Cell_O);
			}
			else
			{
				p1.create(Cell_O);
				p2.create(Cell_X);
			}

		} else
		{
			p1.create(Cell_X);
			p2.create(Cell_O);
		}

		game.setPlayer(0, &p1);
		game.setPlayer(1, &p2);

		PtrWeak<Player> winner;

		if (printDebug)
		{
			printf("New game\n");
		}

		while (!game.isDetectedGameEnd(winner.ptrRef())) {

			game.advanceTurn();
			
			if (printDebug)
			{
				game.debugPrintGrid();
			}
		}

		if (winner.isValid()) {
			
			if (absoluteWinner != -1
				&& (absoluteWinner == 0 ? &p1 != winner.ptr() : &p2 != winner.ptr()))
			{
				testSucceeded = false;
				printf("problem: %c Wins\n", p1.getCellType() == Cell_X ? 'X' : 'O');
			}


			if (winner.ptr() == &p1) {

				if (printDebug)
				{
					printf("%c Wins\n", p1.getCellType() == Cell_X ? 'X' : 'O');
				}

				++p1WinCount;

			} else {

				if (printDebug)
				{
					printf("%c Wins\n", p2.getCellType() == Cell_X ? 'X' : 'O');
				}

				++p2WinCount;
			}

		} else {

			if (printDebug)
			{
				printf("Draw\n");
			}
		}

		++gameIndex;
	}

	if (absoluteWinner != -1)
	{
		if (testSucceeded)
		{
			printf("Test Succeeded\n");
		}
		else
		{
			printf("Test Failed\n");
		}
	}

	return std::pair<int, int>(p1WinCount, p2WinCount);
}

int _tmain(int argc, _TCHAR* argv[])
{

	bool doWaitForInput = false;

	printf("Tic-Tac-Toe AI testbed\n");
	printf("----------------------\n\n");

	printf("Testing AIs...\n");

	//test random game
	if (true) {

		using namespace tictactoe;

		printf("Test: Random vs. Random\n");
		testPlayers<PlayerAIRandom, PlayerAIRandom>(1, false, true, false);
	}

	//test random vs. heuristic game
	if (true) {

		using namespace tictactoe;
	
		printf("Test: Random vs. Rule-based\n");
		testPlayers<PlayerAIRandom, PlayerAIHeuristic>(100, false, true, false, 1);

		doWaitForInput = true;
	}

	//test random vs. alpha-beta game
	if (true) {

		using namespace tictactoe;
	
		printf("Test: Random vs. Alpha-beta minimax\n");
		testPlayers<PlayerAIRandom, PlayerAIAlphaBetaMinimax>(100, false, true, false, 1);

		doWaitForInput = true;
	}

	//test random vs. RL game
	if (true) {

		using namespace tictactoe;
	
		printf("Test: Random vs. Reinforcement Learning value iteration\n");
		testPlayers<PlayerAIRandom, PlayerRLValueIter>(100, false, true, false, 1);

		doWaitForInput = true;
	}

	//test heuristic vs. alpha-beta game
	if (true) {

		using namespace tictactoe;
	
		printf("Test: Rule-based vs. Alpha-beta minimax\n");
		testPlayers<PlayerAIHeuristic, PlayerAIAlphaBetaMinimax>(100, false, true, false, 1);

		doWaitForInput = true;
	}

	//test RL vs. alpha-beta game
	if (true) {

		using namespace tictactoe;
	
		printf("Test: Reinforcement Learning value iteration vs. Alpha-beta minimax\n");
		testPlayers<PlayerRLValueIter, PlayerAIAlphaBetaMinimax>(100, false, true, false, 0);

		doWaitForInput = true;
	}


	if (false)
	{
		using namespace tictactoe;

		while (true) {

			testPlayers<PlayerConsoleInput, PlayerAIHeuristic>(1, false, true, true);
		}
		
	}

	if (true)
	{
		using namespace tictactoe;

		while (true) {

			printf("\n\n\nTest: Human vs. Reinforcement Learning value iteration\n");

			{
				printf("\nsquare indices:");

				TicTacToeGame game;
				game.create();
				game.debugPrintGridIndices();
				printf("\n");
			}

			testPlayers<PlayerConsoleInput, PlayerRLValueIter>(1, false, true, true);
		}
		
	}

	if (doWaitForInput)
	{
		printf("Press 'Enter' to exit...\n");
		std::cin.get();
	}

	return 0;
}

