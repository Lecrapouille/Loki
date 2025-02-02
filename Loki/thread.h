/*
	Loki, a UCI-compliant chess playing software
	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)

	Loki is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Loki is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef THREAD_H
#define THREAD_H
#include "movegen.h"
#include "search_const.h"
#include "evaluation.h"



class SearchInfo_t {
public:
	long long starttime = 0;
	long long stoptime = 0;
	
	int depth = MAXDEPTH;
	int seldepth = 0;
	int depthset = MAXDEPTH;
	
	bool timeset = false;
	int movestogo = 0;
	bool infinite = false;

	long nodes = 0;

	bool quit = false;
	bool stopped = false;

	int fh = 0;
	int fhf = 0;

	SearchInfo_t() {

	}
	SearchInfo_t(const SearchInfo_t& s);

	// Just set the values in the struct to the default.
	void clear();
};


/*

The MoveStats_t struct holds all move ordering and pruning statistics.

*/

struct MoveStats_t {
	// Holds the moves played to get to a position in search. Used for countermoves.
	unsigned int moves_path[MAXDEPTH + 1] = { 0 };

	// Countermoves
	unsigned int counterMoves[64][64] = { {0} };

	// History heuristic
	int history[2][64][64] = { {{0}} };

	// Killer moves
	unsigned int killers[MAXDEPTH + 1][2] = { {0} };

	// Static evaluations
	int static_eval[MAXDEPTH + 1] = { 0 };

};


// SearchThread_t is a structure that holds all information local to a thread. This includes static evaluations, move ordering etc..
class SearchThread_t {
public:
	GameState_t* pos = new GameState_t;
	SearchInfo_t* info = new SearchInfo_t;
	Eval::Evaluate<NORMAL>* eval = new Eval::Evaluate<NORMAL>();

	int thread_id = 0;


	void setKillers(int ply, int move);
	
	// All move ordering and pruning statistics is held in stats
	MoveStats_t stats;

	void update_move_heuristics(int best_move, int depth, MoveList* ml);
	void clear_move_heuristics();
	
	~SearchThread_t() {
		delete pos;
		delete info;
		delete eval;
	}
};



class ThreadPool_t {
public:
	ThreadPool_t(int num_threads) {
		threads = new SearchThread_t[num_threads];
		threadNum = num_threads;
	}

	~ThreadPool_t() {
		delete[] threads;
	}

	void init_threads(GameState_t* pos, SearchInfo_t* info);

	SearchThread_t* at(int index) {
		if (index < threadNum) {
			return &threads[index];
		}
		else {
			return nullptr;
		}
	}

	int count(){
		return threadNum;
	}

private:
	SearchThread_t* threads = nullptr;

	int threadNum = 0;
};










#endif