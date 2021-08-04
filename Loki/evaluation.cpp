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
#include "evaluation.h"


/*
Initialization of constants (PSQT are handled in psqt.cpp)
*/
#define S(m, e) Score(m, e) // Inspired by Stockfish/Ethereal. Just a nice and clean way of initializing the constants.

// Material values.
const Score pawn_value = S(98, 108);
const Score knight_value = S(405, 393);
const Score bishop_value = S(415, 381);
const Score rook_value = S(526, 625);
const Score queen_value = S(1120, 1306);


// Material imbalances.
const Score bishop_pair = S(18, 55);
const Score knight_pawn_penaly = S(1, 1);
const Score rook_pawn_bonus = S(3, 1);


// Pawn evaluation values.
const Score doubled_penalty = S(5, 22);
const Score doubled_isolated_penalty = S(16, 15);
const Score isolated_penalty = S(11, 6);
const Score backwards_penalty = S(7, 1);

const Score passedPawnTable[64] = {
		S(34, 26)	, S(52, 11)	,	S(20, 5)	,	S(13, 50)	,	S(3, 27)	,	S(0, 66)	,	S(4, 66)	,	S(16, 4)	,
		S(0, 0)		, S(2, 0)	,	S(4, 1)		,	S(12, 19)	,	S(0, 23)	,	S(17, 1)	,	S(1, 1)		,	S(1, 2)		,
		S(0, 1)		, S(0, 6)	,	S(1, 0)		,	S(2, 3)		,	S(1, 3)		,	S(9, 0)		,	S(16, 2)	,	S(9, 0)		,
		S(3, 27)	, S(2, 19)	,	S(1, 21)	,	S(0, 18)	,	S(0, 7)		,	S(14, 13)	,	S(18, 15)	,	S(5, 23)	,
		S(6, 53)	, S(36, 35)	,	S(19, 35)	,	S(14, 18)	,	S(28, 35)	,	S(18, 31)	,	S(1, 49)	,	S(1, 51)	,
		S(40, 107)	, S(62, 73)	,	S(20, 59)	,	S(1, 51)	,	S(40, 45)	,	S(42, 49)	,	S(12, 71)	,	S(5, 61)	,
		S(84, 137)	, S(48, 163),	S(26, 145)	,	S(66, 91)	,	S(116, 107)	,	S(54, 133)	,	S(16, 129)	,	S(38, 153)	,
		S(63, 74)	, S(16, 13)	,	S(47, 13)	,	S(4, 19)	,	S(11, 11)	,	S(44, 32)	,	S(26, 30)	,	S(11, 17)
};


// Space evaluation values
const Score space_bonus[32] = {
		S(-40, 2)	,	S(-21, 19)	,	S(66, 9)	,	S(41, 14)	,	S(36, 12)	,	S(35, 10)	,	S(23, 15)	, S(18, 17),
		S(5, 26)	,	S(2, 24)	,	S(-1, 27)	,	S(0, 23)	,	S(1, 30)	,	S(12, 26)	,	S(13, 23)	, S(20, 13),
		S(31, 24)	,	S(40, 10)	,	S(39, 41)	,	S(74, -43)	,	S(69, 36)	,	S(68, 24)	,	S(99, 58)	, S(144, 61),
		S(90, 7)	,	S(201, 36)	,	S(90, 4)	,	S(101, 61)	,	S(148, 73)	,	S(219, 38)	,	S(50, 4)	, S(187, 69),
};


// Mobility evaluation values
const Score knightMobility[9] = { S(-69, -76), S(-36, -68), S(-17, -30), S(-2, -14), S(4, -6), S(11, 8), // Knight
		S(20, 9), S(33, 12), S(42, 5)
};
const Score bishopMobility[14] = { S(-46, -54), S(-21, -30), S(11, -13), S(28, 13), S(36, 28), S(50, 37),
	S(54, 51), S(59, 51), S(59, 62), S(66, 59), S(83, 57), S(102, 56),
	S(82, 69), S(91, 73)
};
const Score rookMobility[15] = { S(-60,-82), S(-24,-15), S(0, 17) ,S(3, 43), S(4, 72), S(14,100), // Rook. TODO: Re-tune this
  S(20,102), S(30,122), S(41,133), S(41 ,139), S(41,153), S(45,160),
  S(57,165), S(58,170), S(67,175)
};
const Score queenMobility[28] = {
	S(-38, -66), S(-25, -14), S(-1, -21), S(21, 6), S(31, 34), S(30, 39), S(36, 46), S(40, 46), S(46, 61), // Queen
	S(45, 70), S(52, 80), S(53, 92), S(54, 107), S(61, 107), S(69, 109),
	S(71, 118), S(66, 131), S(69, 137), S(67, 144), S(69, 156), S(97, 138),
	S(105, 144), S(96, 156), S(85, 172), S(111, 191),
	S(129, 170), S(115, 170), S(96, 202)
};
const std::array<const Score*, 4> mobility_bonus = {
	knightMobility,
	bishopMobility,
	rookMobility,
	queenMobility
};


// Piece evaluation values.
const Score outpost = S(31, 13);
const Score reachable_outpost = S(18, -2);
const Score knight_on_kingring = S(8, -13);
const Score defended_knight = S(0, 10);
const Score bishop_on_kingring = S(11, 4);
const Score bishop_on_queen = S(32, 24);
const Score bad_bishop_coeff = S(0, 5);
const Score doubled_rooks = S(31, 9);
const Score rook_on_queen = S(6, 49);
const Score rook_on_kingring = S(34, -20);
const Score rook_open_file = S(43, -11);
const Score rook_semi_open_file = S(11, 19);
const Score rook_behind_passer = S(0, 10);
const Score queen_on_kingring = S(3, 19);
const Score threatened_queen = S(52, 70);

// Penalties for early queen development in the middlegame.
const Score queen_development_penalty[5] = { S(0, 0), S(0, 0), S(0, 0), S(3, 0), S(12, 0) };


// King safety evaluation
const Score king_open_file_penalty = S(100, 0);
const Score king_semi_open_file_penalty = S(50, 0);
const Score pawnless_flank = S(248, -78);

// This has been taken from the CPW and tuned a little. In the future, it should be tuned even more.
const Score safety_table[100] = {
		S(0, 0),		S(2, 2),		S(12, 2),		S(-11, 2),		S(7, 5),		S(16, 4),		S(17, 11),		S(27, 15),		S(34, 22),		S(63, 39),
		S(34, 22),		S(63, 39),		S(60, 26),		S(35, 19),		S(18, 28),		S(13, 21),		S(26, 26),		S(39, 35),		S(12, 28),		S(31, 19),
		S(12, 28),		S(31, 19),		S(18, 22),		S(43, 27),		S(37, 47),		S(57, 39),		S(67, 63),		S(70, 72),		S(95, 83),		S(87, 113),
		S(95, 83),		S(87, 113),		S(109, 105),	S(109, 119),	S(131, 131),	S(141, 145),	S(139, 159),	S(161, 157),	S(179, 181),	S(181, 179),
		S(179, 181),	S(181, 179),	S(197, 173),	S(203, 201),	S(215, 225),	S(229, 219),	S(233, 229),	S(249, 229),	S(253, 247),	S(263, 255),
		S(253, 247),	S(263, 255),	S(259, 267),	S(269, 295),	S(281, 283),	S(309, 305),	S(301, 297),	S(317, 327),	S(329, 333),	S(331, 331),
		S(329, 333),	S(331, 331),	S(359, 333),	S(347, 359),	S(381, 379),	S(389, 385),	S(387, 379),	S(407, 399),	S(415, 403),	S(419, 423),
		S(415, 403),	S(419, 423),	S(429, 417),	S(429, 437),	S(433, 447),	S(457, 477),	S(475, 459),	S(467, 479),	S(479, 481),	S(501, 491),
		S(479, 481),	S(501, 491),	S(499, 521),	S(515, 515),	S(541, 539),	S(547, 551),	S(557, 539),	S(559, 559),	S(563, 573),	S(583, 571),
		S(563, 573),	S(583, 571),	S(583, 579),	S(593, 589),	S(593, 611),	S(617, 625),	S(627, 647),	S(631, 637),	S(639, 641),	S(655, 651),
};

// Penalties for storming pawns on the side the king is castled on.
const Score pawnStorm[64] = {
		S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)			,	S(0, 0)		,	S(0, 0)		,	S(0, 0),
		S(57, -194)	,	S(223, -8)	,	S(139, 76)	,	S(33, 128)	,	S(-159, -232)	,	S(-3, 66)	,	S(133, 146)	,	S(27, 78),
		S(13, -136)	,	S(-275, 0)	,	S(171, -4)	,	S(123, 60)	,	S(-79, -14)		,	S(147, 86)	,	S(73, 100)	,	S(-1, -10),
		S(10, 178)	,	S(0, -8)	,	S(188, 56)	,	S(140, 64)	,	S(80, -54)		,	S(30, -10)	,	S(60, -20)	,	S(-8, 14),
		S(-11, 16)	,	S(75, -80)	,	S(45, 78)	,	S(-77, 60)	,	S(-87, -30)		,	S(-1, 14)	,	S(67, -26)	,	S(-3, 16),
		S(-101, 108),	S(11, -136)	,	S(-45, 96)	,	S(-291, 58)	,	S(-3, 20)		,	S(1, 18)	,	S(37, -18)	,	S(-17, 18),
		S(-60, 64)	,	S(18, -24)	,	S(74, -30)	,	S(80, -24)	,	S(6, 38)		,	S(2, 48)	,	S(44, 4)	,	S(-14, -34),
		S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)			,	S(0, 0)		,	S(0, 0)		,	S(0, 0)
};

// Penalties for the distance between the king and a pawn on the flank.
const Score king_pawn_distance_penalty[8] = {
	S(0, 0),
	S(-18, -38),
	S(-8, -32),
	S(-5, -19),
	S(-8, -8),
	S(-12, 36),
	S(81, -40),
	S(-132, 27),
};

// Penalties for open files near the king. Indexed by file number.
const Score open_kingfile_penalty[8] = { S(-26, 50), S(143, -118), S(108, -38), S(31, 30), S(77, -58), S(-6, 50), S(131, -78), S(124, -98) };

// Penalties for semi-open files near the king. Again, indexed by file number
const Score semiopen_kingfile_penalty[8] = { S(-23, 58), S(107, -154), S(80, -98), S(-9, 60), S(-7, 10), S(0, 32), S(47, -12), S(-37, 62) };


#undef S



namespace Eval {

	/// <summary>
	/// Evaluate a position with a side-relative score.
	/// </summary>
	/// <param name="_pos">The position object that is to be evaluated.</param>
	/// <returns>A numerical score for the position, relative to the side to move.</returns>
	int Evaluate<T>::score(const GameState_t* _pos) {
		// Step 1. Clear the object and store the position object.
		clear();
		pos = _pos;

		// Step 2. Evaluate material
		material<WHITE>(); material<BLACK>();

		// Step 3. Evaluate piece placements
		psqt<WHITE>(); psqt<BLACK>();

		// Step 4. Material imbalances
		imbalance<WHITE>(); imbalance<BLACK>();

		// Step 5. Pawn structure evaluation
		pawns<WHITE>(); pawns<BLACK>();

		// Step 6. Space evaluation
		space<WHITE>(); space<BLACK>();

		// Step 7. Mobility
		mobility<WHITE, KNIGHT>(); mobility<BLACK, KNIGHT>();
		mobility<WHITE, BISHOP>(); mobility<BLACK, BISHOP>();
		mobility<WHITE, ROOK>(); mobility<BLACK, ROOK>();
		mobility<WHITE, QUEEN>(); mobility<BLACK, QUEEN>();

		// Step 8. King safety evaluation.
		king_safety<WHITE>(); king_safety<BLACK>();

		// Step 9. Compute the phase and interpolate the middle- and endgame scores.
		int phase = game_phase();
		
		int v = (phase * mg_score + (24 - phase) * eg_score) / 24;

		// Step 10. Add tempo for the side to move, make the score side-relative and return
		v += (pos->side_to_move == WHITE) ? tempo : -tempo;
		v *= (pos->side_to_move == WHITE) ? 1 : -1;

		return v;
	}


	/// <summary>
	/// Calculate the game phase based on the amount of material left on the board.
	/// </summary>
	/// <returns>A number between 0 and 24 representing the game phase.</returns>
	int Evaluate<T>::game_phase() {
		// We calculate the game phase by giving 1 point for each bishop and knight, 2 for each rook and 4 for each queen.
		// This gives the starting position a phase of 24.
		int p = 0;

		p += 1 * (countBits(pos->pieceBBS[KNIGHT][WHITE] | pos->pieceBBS[KNIGHT][BLACK]));
		p += 1 * (countBits(pos->pieceBBS[BISHOP][WHITE] | pos->pieceBBS[BISHOP][BLACK]));
		p += 2 * (countBits(pos->pieceBBS[ROOK][WHITE] | pos->pieceBBS[ROOK][BLACK]));
		p += 4 * (countBits(pos->pieceBBS[QUEEN][WHITE] | pos->pieceBBS[QUEEN][BLACK]));

		return std::min(24, p);
	}


	/// <summary>
	/// Clear the Evaluate object.
	/// </summary>
	void Evaluate<T>::clear() {
		// Clear the EvalData.
		Data = ZeroData;

		// Clear the scores and the position pointer.
		mg_score = eg_score = 0;
		pos = nullptr;
	}
}













/// <summary>
/// Loop through a set of positions, evaluate each one, mirror it and evaluate it again. These two values should be the same, otherwise the evaluation function is broken.
/// </summary>
/*void Eval::Debug::eval_balance() {

	GameState_t* pos = new GameState_t;

	int total = 0;
	int passed = 0;
	int failed = 0;

	int w_ev = 0;
	int b_ev = 0;

	for (int p = 0; p < test_positions.size(); p++) {
		total++;
		pos->parseFen(test_positions[p]);

		w_ev = evaluate(pos);

		pos->mirror_board();

		b_ev = evaluate(pos);


		if (w_ev == b_ev) {
			std::cout << "Position " << (p + 1) << "	--->	" << "PASSED:" << " " << w_ev << " == " << b_ev << std::endl;
			passed++;
		}
		else {
			std::cout << "Position " << (p + 1) << "	--->	" << "FAILED: " << w_ev << " != " << b_ev << "		(FEN: " << test_positions[p] << ")" << std::endl;
			failed++;
		}
	}

	std::cout << total << " positions analyzed." << std::endl;
	std::cout << passed << " positions passed." << std::endl;
	std::cout << failed << " positions failed. (" << (double(failed) / double(total)) * 100.0 << "%)" << std::endl;

	delete pos;
}*/