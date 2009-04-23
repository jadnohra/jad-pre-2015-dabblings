#include "TicTacToeAIPlayerRLValueIter.h"

#include "float.h"
#include "math.h"
#include <vector>

namespace tictactoe {

	namespace PlayerRLValueIterDetail
	{
		typedef std::vector<Key> Keys;

		enum CellOccupant
		{
			Cell_Empty = 0, Cell_Self, Cell_Opponent
		};

		enum ConfigTransform
		{
			Xfm_None = -1, Xfm_MirrorHoriz, Xfm_MirrorVert, Xfm_MirrorDouble
		};

		inline CellOccupant other(CellOccupant occupant)
		{
			switch (occupant)
			{
				case Cell_Self: return Cell_Opponent;
				case Cell_Opponent: return Cell_Self;
			}

			return Cell_Empty;
		}

		struct State;

		struct DecodedState
		{
			CellOccupant cells[9];
			CellOccupant turn;

			DecodedState()
			{
			}

			DecodedState(CellOccupant occupant, CellOccupant turn_)
			{
				for (int i = 0; i < 9; ++i)
				{
					cells[i] = occupant;
				}
				turn = turn_;
			}

			DecodedState(const State& state);
			
			DecodedState(TicTacToeGame& game, CellType self, CellOccupant turn_ = Cell_Self)
			{
				for (CellIndex i = 0; i < 9; ++i)
				{
					CellAddress address;
					game.fromGridIndex(i, address.horiz, address.vert);

					const Cell& cell = game.getCell(address);

					if (cell.isEmpty())
					{
						cells[i] = Cell_Empty;
					}
					else if (cell.is(self))
					{
						cells[i] = Cell_Self;
					}
					else
					{
						cells[i] = Cell_Opponent;
					}
				}

				turn = turn_;
			}

			void transform(ConfigTransform xfm) 
			{
				if (xfm == Xfm_MirrorHoriz)
				{
					std::swap(cells[0], cells[2]);
					std::swap(cells[3], cells[5]);
					std::swap(cells[6], cells[8]);

				}
				else if (xfm == Xfm_MirrorVert)
				{
					std::swap(cells[0], cells[6]);
					std::swap(cells[1], cells[7]);
					std::swap(cells[2], cells[8]);
				}
				else if (xfm == Xfm_MirrorDouble)
				{
					std::swap(cells[0], cells[2]);
					std::swap(cells[3], cells[5]);
					std::swap(cells[6], cells[8]);

					std::swap(cells[0], cells[6]);
					std::swap(cells[1], cells[7]);
					std::swap(cells[2], cells[8]);
				}
			}

			int diffIndex(const DecodedState& comp) const
			{
				for (int i = 0; i < 9; ++i)
				{
					if (cells[i] != comp.cells[i])
						return i;
				}

				return -1;
			}

			int freeIndex() const
			{
				for (int i = 0; i < 9; ++i)
				{
					if (cells[i] == Cell_Empty)
						return i;
				}

				return -1;
			}

		};

		struct State
		{
			union 
			{
				struct 
				{
					Key c0 : 2;
					Key c1 : 2;
					Key c2 : 2;
					Key c3 : 2;
					Key c4 : 2;
					Key c5 : 2;
					Key c6 : 2;
					Key c7 : 2;
					Key c8 : 2;
					Key turn : 2;
				};

				struct 
				{
					Key cells;
				};

				struct 
				{
					Key key;
				};
			};

			State()
			:	key(0)
			{
			}

			State(const DecodedState& state)
			:	key(0)
			{
				init(state);
			}

			void init(const DecodedState& state)
			{
				c0 = state.cells[0];
				c1 = state.cells[1];
				c2 = state.cells[2];
				c3 = state.cells[3];
				c4 = state.cells[4];
				c5 = state.cells[5];
				c6 = state.cells[6];
				c7 = state.cells[7];
				c8 = state.cells[8];
				turn = state.turn;
			}

			State(const State& state)
			:	key(0)
			{
				*this = state;
			}

			State(const Key& stateKey)
			:	key(0)
			{
				this->key = stateKey;
			}

			State(const State& state, int action)
			:	key(0)
			{
				*this = state;

				switch (action)
				{
					case 0: c0 = state.turn; break;
					case 1: c1 = state.turn; break;
					case 2: c2 = state.turn; break;
					case 3: c3 = state.turn; break;
					case 4: c4 = state.turn; break;
					case 5: c5 = state.turn; break;
					case 6: c6 = state.turn; break;
					case 7: c7 = state.turn; break;
					case 8: c8 = state.turn; break;
				}

				turn = other((CellOccupant) state.turn);
			}

			State transform(ConfigTransform xfm) const
			{
				DecodedState decState(*this);

				decState.transform(xfm);

				return State(decState);
			}
		};

		struct StateInfo
		{
			State state;
			float value;
			bool isTerminal;
			CellOccupant winner;

			StateInfo(const DecodedState& decodedState)
			:	state(decodedState)
			,	value(0.0f)
			{
				analyze();
			}

			StateInfo(const Key& stateKey)
			:	state(stateKey)
			,	value(0.0f)
			{
				analyze();
			}

			bool isFull() const
			{
				DecodedState decoded(state);

				for (int i = 0; i < 9; ++i)
				{
					if (decoded.cells[i] == Cell_Empty)
						return false;
				}

				return true;
			}

			bool isWinner(CellOccupant occupant) const
			{
				enum { WinningRowCount = 8 };
				enum { WinningCellCount = 3 };

				const int WinningRows[WinningRowCount][WinningCellCount] = {
					{ 0, 1, 2 },
					{ 3, 4, 5 },
					{ 6, 7, 8 },
					{ 0, 3, 6 },
					{ 1, 4, 7 },
					{ 2, 5, 8 },
					{ 0, 4, 8 },
					{ 2, 4, 6 }
				};

				DecodedState decoded(state);

				for (int i = 0; i < WinningRowCount; ++i)
				{
					bool isDetectedRow = true;

					for (int j = 0; j < WinningCellCount; ++j)
					{
						if (decoded.cells[WinningRows[i][j]] != occupant)
						{
							isDetectedRow = false;
							break;
						}
					}

					if (isDetectedRow)
						return true;
				}

				return false;
			}

			void analyze()
			{
				if (isWinner(Cell_Opponent))
				{
					winner = Cell_Opponent;
					isTerminal = true;
					value = -1.0f;
				}
				else if (isWinner(Cell_Self))
				{
					winner = Cell_Self;
					isTerminal = true;
					value = 1.0f;

				} else if (isFull())
				{
					winner = Cell_Empty;
					isTerminal = true;

				} else
				{
					winner = Cell_Empty;
					isTerminal = false;
				}
			}

			void getNextStateKeys(Keys& nextStateKeys) const
			{
				DecodedState decoded(state);

				for (int i = 0; i < 9; ++i)
				{
					if (decoded.cells[i] == Cell_Empty)
					{
						nextStateKeys.push_back(State(state, i).key);
					}
				}
			}
		};

		DecodedState::DecodedState(const State& state)
		{
			cells[0] = (CellOccupant) state.c0;
			cells[1] = (CellOccupant) state.c1;
			cells[2] = (CellOccupant) state.c2;
			cells[3] = (CellOccupant) state.c3;
			cells[4] = (CellOccupant) state.c4;
			cells[5] = (CellOccupant) state.c5;
			cells[6] = (CellOccupant) state.c6;
			cells[7] = (CellOccupant) state.c7;
			cells[8] = (CellOccupant) state.c8;
			turn = (CellOccupant) state.turn;
		}
	
		typedef std::map<PlayerRLValueIterDetail::Key, PlayerRLValueIterDetail::StateInfo*> StateInfoMap;

		bool containsMirrors(StateInfoMap& map, const State& state)
		{
			if (map.find(state.transform(Xfm_MirrorHoriz).key) != map.end())
				return true;

			if (map.find(state.transform(Xfm_MirrorVert).key) != map.end())
				return true;

			if (map.find(state.transform(Xfm_MirrorDouble).key) != map.end())
				return true;

			return false;
		}

		void destroy(StateInfoMap& map)
		{
			for (StateInfoMap::iterator it = map.begin();
			 it != map.end(); ++it)
			{
				delete it->second;
			}
			map.clear();
		}

		void recurseCreate(StateInfoMap& map, StateInfo* pCurrStateInfo)
		{
			Keys nextKeys;
			
			pCurrStateInfo->getNextStateKeys(nextKeys);

			for (Keys::iterator nextIt = nextKeys.begin();
				 nextIt != nextKeys.end(); ++nextIt)
			{
				StateInfo testNextStateInfo(*nextIt);

				if (testNextStateInfo.isTerminal)
				{
					if (!containsMirrors(map, *nextIt))
					{
						StateInfo* pInfo = new StateInfo(*nextIt);
						map[pInfo->state.key] = pInfo;
					}

					return;
				}
			}

			for (Keys::iterator nextIt = nextKeys.begin();
				 nextIt != nextKeys.end(); ++nextIt)
			{
				if (!containsMirrors(map, *nextIt))
				{
					StateInfo* pInfo = new StateInfo(*nextIt);
					map[pInfo->state.key] = pInfo;
					
					recurseCreate(map, pInfo);
				}
			}
		}

		void create(StateInfoMap& map)
		{
			destroy(map);

			StateInfo* pInfo1 = new StateInfo(DecodedState(Cell_Empty, Cell_Self));
			map[pInfo1->state.key] = pInfo1;

			StateInfo* pInfo2 = new StateInfo(DecodedState(Cell_Empty, Cell_Opponent));
			map[pInfo2->state.key] = pInfo2;

			recurseCreate(map, pInfo1);
			recurseCreate(map, pInfo2);
		}

		StateInfo* findStateInfo(StateInfoMap& map, Key stateKey, ConfigTransform& xfm)
		{
			StateInfoMap::iterator it = map.find(stateKey);

			xfm = Xfm_None;
			if (it != map.end())
				return it->second;

			State state(stateKey);

			xfm = Xfm_MirrorHoriz;
			it = map.find(state.transform(xfm).key);
			if (it != map.end())
				return it->second;

			xfm = Xfm_MirrorVert;
			it = map.find(state.transform(xfm).key);
			if (it != map.end())
				return it->second;

			xfm = Xfm_MirrorDouble;
			it = map.find(state.transform(xfm).key);
			if (it != map.end())
				return it->second;

			return NULL;
		}
	
		void learn(StateInfoMap& map)
		{
			Keys keys;
			float maxVDiff = 0.0f;
			int iterCount = 0;

			do
			{
				maxVDiff = 0.0f;
				++iterCount;

				for (StateInfoMap::iterator it = map.begin(); it != map.end(); ++it)
				{
					StateInfo& stateInfo = *it->second;

					if (!stateInfo.isTerminal)
					{
						keys.clear();
						stateInfo.getNextStateKeys(keys);

						float maxQ = stateInfo.state.turn == Cell_Opponent ? FLT_MAX : -FLT_MAX;

						for (Keys::iterator nextIt = keys.begin(); nextIt != keys.end(); ++nextIt)
						{
							ConfigTransform xfm;

							StateInfo* pNextState = findStateInfo(map, *nextIt, xfm);
							float reward = 0.0f;

							if (pNextState)
							{
								// Assume a perfect playing opponent
								if (stateInfo.state.turn == Cell_Opponent)
								{
									if (reward + pNextState->value < maxQ)
									{
										maxQ = reward + pNextState->value;
									}
								}
								else
								{
									if (reward + pNextState->value > maxQ)
									{
										maxQ = reward + pNextState->value;
									}
								}
							}
						}

						float vDiff = fabs(stateInfo.value - maxQ);
						if (vDiff > maxVDiff)
							maxVDiff = vDiff;

						stateInfo.value = maxQ;
					}
				}

			} while (maxVDiff != 0.0f);
		}

		struct MapHolder
		{
			StateInfoMap stateInfoMap;
			int refCount;

			MapHolder()
			:	refCount(0)
			{
				create(stateInfoMap);
				learn(stateInfoMap);
			}

			~MapHolder()
			{
				destroy(stateInfoMap);
			}
		};
		
		static StateInfoMap* getMap()
		{
			static MapHolder* pHolder = NULL; 

			if (pHolder == NULL)
			{
				// Note: memory leak map is never deleted.
				pHolder = new MapHolder();
			}

			++pHolder->refCount;
			return &pHolder->stateInfoMap;
		}
	}
}

namespace tictactoe {

PlayerRLValueIter::PlayerRLValueIter()
{
	m_StateInfoMap = PlayerRLValueIterDetail::getMap();
}

PlayerRLValueIter::~PlayerRLValueIter()
{
}

void PlayerRLValueIter::playTurn(TicTacToeGame& game) {

	//this AI can only play 3x3 
	assert((game.gridWidth() == 3) && (game.gridHeight() == 3));

	using namespace PlayerRLValueIterDetail;

	DecodedState gameState(game, this->getCellType());
	State encodedGameState(gameState);

	ConfigTransform xfm;

	StateInfo* pStateInfo = findStateInfo(*m_StateInfoMap, encodedGameState.key, xfm);

	int tickIndex = -1;

	if (pStateInfo)
	{
		const StateInfo& info = *pStateInfo;
		Keys nextStateKeys;
		
		info.getNextStateKeys(nextStateKeys);
		StateInfo* pBestState = NULL;
		ConfigTransform bestStateXfm;

		for (Keys::const_iterator it = nextStateKeys.begin();
			 it != nextStateKeys.end(); ++it)
		{
			ConfigTransform nextXfm;
			StateInfo* pNextStateInfo = findStateInfo(*m_StateInfoMap, *(it), nextXfm);

			if (pNextStateInfo)
			{
				if (pBestState == NULL || (pNextStateInfo->value > pBestState->value))
				{
					pBestState = pNextStateInfo;
					bestStateXfm = nextXfm;
				}
			}
		}

		if (pBestState)
		{
			DecodedState bestState(pBestState->state);
			bestState.transform(bestStateXfm);
			bestState.transform(xfm);

			tickIndex = gameState.diffIndex(bestState);
		}
	}

	if (tickIndex == -1)
	{
		tickIndex = gameState.freeIndex();
	}

	if (tickIndex != -1)
	{
		CellAddress address;

		game.fromGridIndex(tickIndex, address.horiz, address.vert);
		game.tickCell(address);
	}
}

}
