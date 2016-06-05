/*#
  # 局面のパターンを管理、操作を行います
  #
  ##############################################
  */

#include "stdafx.h"
#include "GetPattern.h"
#include "eval.h"
#include "AI.h"

/* 評価パターンテーブル(現在のステージにより内容が変わるポインタ) */
double *hori_ver1;
double *hori_ver2;
double *hori_ver3;
double *dia_ver1;
double *dia_ver2;
double *dia_ver3;
double *dia_ver4;
double *edge;
double *corner5_2;
double *corner3_3;
double *triangle;
double *mobility;
double *parity;

/* 評価パターンテーブル(おおもと) */
double hori_ver1_data[60][INDEX_NUM];
double hori_ver2_data[60][INDEX_NUM];
double hori_ver3_data[60][INDEX_NUM];
double dia_ver1_data[60][INDEX_NUM];
double dia_ver2_data[60][INDEX_NUM / 3];
double dia_ver3_data[60][INDEX_NUM / 9];
double dia_ver4_data[60][INDEX_NUM / 27];
double edge_data[60][INDEX_NUM * 9];
double edge_cor_data[60][INDEX_NUM * 9];
double corner5_2_data[60][INDEX_NUM * 9];
double corner3_3_data[60][INDEX_NUM * 3];
double triangle_data[60][INDEX_NUM * 9];
double mobility_data[60][MOBILITY_NUM];
double parity_data[60][PARITY_NUM];

double key_hori_ver1[4];
double key_hori_ver2[4];
double key_hori_ver3[4];
double key_dia_ver1[2];
double key_dia_ver2[4];
double key_dia_ver3[4];
double key_dia_ver4[4];
double key_edge[4];
double key_corner5_2[8];
double key_corner3_3[4];
double key_triangle[4];
double key_mobility;
double key_parity;
double key_constant;

double eval_sum;

UINT64 a1 = 1ULL;					/* a1 */
UINT64 a2 = (1ULL << 1);			/* a2 */
UINT64 a3 = (1ULL << 2);			/* a3 */
UINT64 a4 = (1ULL << 3);			/* a4 */
UINT64 a5 = (1ULL << 4);			/* a5 */
UINT64 a6 = (1ULL << 5);			/* a6 */
UINT64 a7 = (1ULL << 6);			/* a7 */
UINT64 a8 = (1ULL << 7);			/* a8 */

UINT64 b1 = (1ULL << 8);			/* b1 */
UINT64 b2 = (1ULL << 9);			/* b2 */
UINT64 b3 = (1ULL << 10);			/* b3 */
UINT64 b4 = (1ULL << 11);			/* b4 */
UINT64 b5 = (1ULL << 12);			/* b5 */
UINT64 b6 = (1ULL << 13);			/* b6 */
UINT64 b7 = (1ULL << 14);			/* b7 */
UINT64 b8 = (1ULL << 15);			/* b8 */

UINT64 c1 = (1ULL << 16);			/* c1 */
UINT64 c2 = (1ULL << 17);			/* c2 */
UINT64 c3 = (1ULL << 18);			/* c3 */
UINT64 c4 = (1ULL << 19);			/* c4 */
UINT64 c5 = (1ULL << 20);			/* c5 */
UINT64 c6 = (1ULL << 21);			/* c6 */
UINT64 c7 = (1ULL << 22);			/* c7 */
UINT64 c8 = (1ULL << 23);			/* c8 */

UINT64 d1 = (1ULL << 24);			/* d1 */
UINT64 d2 = (1ULL << 25);			/* d2 */
UINT64 d3 = (1ULL << 26);			/* d3 */
UINT64 d4 = (1ULL << 27);			/* d4 */
UINT64 d5 = (1ULL << 28);			/* d5 */
UINT64 d6 = (1ULL << 29);			/* d6 */
UINT64 d7 = (1ULL << 30);			/* d7 */
UINT64 d8 = (1ULL << 31);			/* d8 */

UINT64 e1 = (1ULL << 32);			/* e1 */
UINT64 e2 = (1ULL << 33);			/* e2 */
UINT64 e3 = (1ULL << 34);			/* e3 */
UINT64 e4 = (1ULL << 35);			/* e4 */
UINT64 e5 = (1ULL << 36);			/* e5 */
UINT64 e6 = (1ULL << 37);			/* e6 */
UINT64 e7 = (1ULL << 38);			/* e7 */
UINT64 e8 = (1ULL << 39);			/* e8 */

UINT64 f1 = (1ULL << 40);			/* f1 */
UINT64 f2 = (1ULL << 41);			/* f2 */
UINT64 f3 = (1ULL << 42);			/* f3 */
UINT64 f4 = (1ULL << 43);			/* f4 */
UINT64 f5 = (1ULL << 44);			/* f5 */
UINT64 f6 = (1ULL << 45);			/* f6 */
UINT64 f7 = (1ULL << 46);			/* f7 */
UINT64 f8 = (1ULL << 47);			/* f8 */

UINT64 g1 = (1ULL << 48);			/* g1 */
UINT64 g2 = (1ULL << 49);			/* g2 */
UINT64 g3 = (1ULL << 50);			/* g3 */
UINT64 g4 = (1ULL << 51);			/* g4 */
UINT64 g5 = (1ULL << 52);			/* g5 */
UINT64 g6 = (1ULL << 53);			/* g6 */
UINT64 g7 = (1ULL << 54);			/* g7 */
UINT64 g8 = (1ULL << 55);			/* g8 */

UINT64 h1 = (1ULL << 56);			/* h1 */
UINT64 h2 = (1ULL << 57);			/* h2 */
UINT64 h3 = (1ULL << 58);			/* h3 */
UINT64 h4 = (1ULL << 59);			/* h4 */
UINT64 h5 = (1ULL << 60);			/* h5 */
UINT64 h6 = (1ULL << 61);			/* h6 */
UINT64 h7 = (1ULL << 62);			/* h7 */
UINT64 h8 = (1ULL << 63);			/* h8 */

int board[64];

char eval_table_list[14][32] =
{
	"prologue", "opening1", "opening2",
	"opening3", "middle1", "middle2",
	"middle3", "middle4", "middle5",
	"end1", "end2", "end3", "end4",
	"end5"
};

int pos_eval[64] =
{
	8, 1, 5, 6, 6, 5, 1, 8,
	1, 0, 2, 3, 3, 2, 0, 1,
	5, 2, 7, 4, 4, 7, 2, 5,
	6, 3, 4, 0, 0, 4, 3, 6,
	6, 3, 4, 0, 0, 4, 3, 6,
	5, 2, 7, 4, 4, 7, 2, 5,
	1, 0, 2, 3, 3, 2, 0, 1,
	8, 1, 5, 6, 6, 5, 1, 8
};

/* A conversion table from the 2^8 edge values for one player to
   the corresponding base-3 value. */
static short base_conversion[256];

/* For each of the 3^8 edges, edge_stable[] holds an 8-bit mask
   where a bit is set if the corresponding disc can't be changed EVER. */
static short edge_stable[6561];


void init_index_board(UINT64 bk, UINT64 wh)
{
	board[0] = (int)(bk & a1) + (int)((wh & a1) * 2);
	board[1] = (int)((bk & a2) >> 1) + (int)((wh & a2));
	board[2] = (int)((bk & a3) >> 2) + (int)((wh & a3) >> 1);
	board[3] = (int)((bk & a4) >> 3) + (int)((wh & a4) >> 2);
	board[4] = (int)((bk & a5) >> 4) + (int)((wh & a5) >> 3);
	board[5] = (int)((bk & a6) >> 5) + (int)((wh & a6) >> 4);
	board[6] = (int)((bk & a7) >> 6) + (int)((wh & a7) >> 5);
	board[7] = (int)((bk & a8) >> 7) + (int)((wh & a8) >> 6);
	board[8] = (int)((bk & b1) >> 8) + (int)((wh & b1) >> 7);
	board[9] = (int)((bk & b2) >> 9) + (int)((wh & b2) >> 8);
	board[10] = (int)((bk & b3) >> 10) + (int)((wh & b3) >> 9);
	board[11] = (int)((bk & b4) >> 11) + (int)((wh & b4) >> 10);
	board[12] = (int)((bk & b5) >> 12) + (int)((wh & b5) >> 11);
	board[13] = (int)((bk & b6) >> 13) + (int)((wh & b6) >> 12);
	board[14] = (int)((bk & b7) >> 14) + (int)((wh & b7) >> 13);
	board[15] = (int)((bk & b8) >> 15) + (int)((wh & b8) >> 14);
	board[16] = (int)((bk & c1) >> 16) + (int)((wh & c1) >> 15);
	board[17] = (int)((bk & c2) >> 17) + (int)((wh & c2) >> 16);
	board[18] = (int)((bk & c3) >> 18) + (int)((wh & c3) >> 17);
	board[19] = (int)((bk & c4) >> 19) + (int)((wh & c4) >> 18);
	board[20] = (int)((bk & c5) >> 20) + (int)((wh & c5) >> 19);
	board[21] = (int)((bk & c6) >> 21) + (int)((wh & c6) >> 20);
	board[22] = (int)((bk & c7) >> 22) + (int)((wh & c7) >> 21);
	board[23] = (int)((bk & c8) >> 23) + (int)((wh & c8) >> 22);
	board[24] = (int)((bk & d1) >> 24) + (int)((wh & d1) >> 23);
	board[25] = (int)((bk & d2) >> 25) + (int)((wh & d2) >> 24);
	board[26] = (int)((bk & d3) >> 26) + (int)((wh & d3) >> 25);
	board[27] = (int)((bk & d4) >> 27) + (int)((wh & d4) >> 26);
	board[28] = (int)((bk & d5) >> 28) + (int)((wh & d5) >> 27);
	board[29] = (int)((bk & d6) >> 29) + (int)((wh & d6) >> 28);
	board[30] = (int)((bk & d7) >> 30) + (int)((wh & d7) >> 29);
	board[31] = (int)((bk & d8) >> 31) + (int)((wh & d8) >> 30);
	board[32] = (int)((bk & e1) >> 32) + (int)((wh & e1) >> 31);
	board[33] = (int)((bk & e2) >> 33) + (int)((wh & e2) >> 32);
	board[34] = (int)((bk & e3) >> 34) + (int)((wh & e3) >> 33);
	board[35] = (int)((bk & e4) >> 35) + (int)((wh & e4) >> 34);
	board[36] = (int)((bk & e5) >> 36) + (int)((wh & e5) >> 35);
	board[37] = (int)((bk & e6) >> 37) + (int)((wh & e6) >> 36);
	board[38] = (int)((bk & e7) >> 38) + (int)((wh & e7) >> 37);
	board[39] = (int)((bk & e8) >> 39) + (int)((wh & e8) >> 38);
	board[40] = (int)((bk & f1) >> 40) + (int)((wh & f1) >> 39);
	board[41] = (int)((bk & f2) >> 41) + (int)((wh & f2) >> 40);
	board[42] = (int)((bk & f3) >> 42) + (int)((wh & f3) >> 41);
	board[43] = (int)((bk & f4) >> 43) + (int)((wh & f4) >> 42);
	board[44] = (int)((bk & f5) >> 44) + (int)((wh & f5) >> 43);
	board[45] = (int)((bk & f6) >> 45) + (int)((wh & f6) >> 44);
	board[46] = (int)((bk & f7) >> 46) + (int)((wh & f7) >> 45);
	board[47] = (int)((bk & f8) >> 47) + (int)((wh & f8) >> 46);
	board[48] = (int)((bk & g1) >> 48) + (int)((wh & g1) >> 47);
	board[49] = (int)((bk & g2) >> 49) + (int)((wh & g2) >> 48);
	board[50] = (int)((bk & g3) >> 50) + (int)((wh & g3) >> 49);
	board[51] = (int)((bk & g4) >> 51) + (int)((wh & g4) >> 50);
	board[52] = (int)((bk & g5) >> 52) + (int)((wh & g5) >> 51);
	board[53] = (int)((bk & g6) >> 53) + (int)((wh & g6) >> 52);
	board[54] = (int)((bk & g7) >> 54) + (int)((wh & g7) >> 53);
	board[55] = (int)((bk & g8) >> 55) + (int)((wh & g8) >> 54);
	board[56] = (int)((bk & h1) >> 56) + (int)((wh & h1) >> 55);
	board[57] = (int)((bk & h2) >> 57) + (int)((wh & h2) >> 56);
	board[58] = (int)((bk & h3) >> 58) + (int)((wh & h3) >> 57);
	board[59] = (int)((bk & h4) >> 59) + (int)((wh & h4) >> 58);
	board[60] = (int)((bk & h5) >> 60) + (int)((wh & h5) >> 59);
	board[61] = (int)((bk & h6) >> 61) + (int)((wh & h6) >> 60);
	board[62] = (int)((bk & h7) >> 62) + (int)((wh & h7) >> 61);
	board[63] = (int)((bk & h8) >> 63) + (int)((wh & h8) >> 62);

}

/**
 * @brief Get some potential moves.
 *
 * @param P UINT64 with player's discs.
 * @param dir flipping direction.
 * @return some potential moves in a 64-bit unsigned integer.
 */
unsigned long long get_some_potential_moves(const unsigned long long P, const int dir)
{
	return (P << dir | P >> dir);
}

/**
 * @brief Get potential moves.
 *
 * Get the list of empty squares in contact of a player square.
 *
 * @param P UINT64 with player's discs.
 * @param O UINT64 with opponent's discs.
 * @return all potential moves in a 64-bit unsigned integer.
 */
unsigned long long get_potential_moves(UINT64 P, UINT64 O, UINT64 blank)
{
	return (get_some_potential_moves(O & 0x7E7E7E7E7E7E7E7Eull, 1) // horizontal
		| get_some_potential_moves(O & 0x00FFFFFFFFFFFF00ull, 8)   // vertical
		| get_some_potential_moves(O & 0x007E7E7E7E7E7E00ull, 7)   // diagonals
		| get_some_potential_moves(O & 0x007E7E7E7E7E7E00ull, 9))
		& blank; // mask with empties
}

double check_h_ver1()
{
	int key;
	double eval;

	/* a2 b2 c2 d2 e2 f2 g2 h2 */
	/* a7 b7 c7 d7 e7 f7 g7 h7 */
	/* b1 b2 b3 b4 b5 b6 b7 b8 */
	/* g1 g2 g3 g4 g5 g6 g7 g8 */

	key = board[A2];
	key += 3 * board[B2];
	key += 9 * board[C2];
	key += 27 * board[D2];
	key += 81 * board[E2];
	key += 243 * board[F2];
	key += 729 * board[G2];
	key += 2187 * board[H2];

	eval = hori_ver1[key];

	key_hori_ver1[0] = eval;

	//if(hori_ver1[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[A7];
	key += 3 * board[B7];
	key += 9 * board[C7];
	key += 27 * board[D7];
	key += 81 * board[E7];
	key += 243 * board[F7];
	key += 729 * board[G7];
	key += 2187 * board[H7];

	eval += hori_ver1[key];
	key_hori_ver1[1] = hori_ver1[key];

	//if(hori_ver1[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[B1];
	key += 3 * board[B2];
	key += 9 * board[B3];
	key += 27 * board[B4];
	key += 81 * board[B5];
	key += 243 * board[B6];
	key += 729 * board[B7];
	key += 2187 * board[B8];

	eval += hori_ver1[key];
	key_hori_ver1[2] = hori_ver1[key];

	//if(hori_ver1[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[G1];
	key += 3 * board[G2];
	key += 9 * board[G3];
	key += 27 * board[G4];
	key += 81 * board[G5];
	key += 243 * board[G6];
	key += 729 * board[G7];
	key += 2187 * board[G8];

	eval += hori_ver1[key];
	key_hori_ver1[3] = hori_ver1[key];
	//if(hori_ver1[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	return eval;
}

double check_h_ver2()
{
	int key;
	double eval;

	/* a3 b3 c3 d3 e3 f3 g3 h3 */
	/* a6 b6 c6 d6 e6 f6 g6 h6 */
	/* c1 c2 c3 c4 c5 c6 c7 c8 */
	/* f1 f2 f3 f4 f5 f6 f7 f8 */

	key = board[A3];
	key += 3 * board[B3];
	key += 9 * board[C3];
	key += 27 * board[D3];
	key += 81 * board[E3];
	key += 243 * board[F3];
	key += 729 * board[G3];
	key += 2187 * board[H3];

	eval = hori_ver2[key];
	key_hori_ver2[0] = hori_ver2[key];
	//if(hori_ver2[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[A6];
	key += 3 * board[B6];
	key += 9 * board[C6];
	key += 27 * board[D6];
	key += 81 * board[E6];
	key += 243 * board[F6];
	key += 729 * board[G6];
	key += 2187 * board[H6];

	eval += hori_ver2[key];
	key_hori_ver2[1] = hori_ver2[key];
	//if(hori_ver2[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[C1];
	key += 3 * board[C2];
	key += 9 * board[C3];
	key += 27 * board[C4];
	key += 81 * board[C5];
	key += 243 * board[C6];
	key += 729 * board[C7];
	key += 2187 * board[C8];

	eval += hori_ver2[key];
	key_hori_ver2[2] = hori_ver2[key];
	//if(hori_ver2[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[F1];
	key += 3 * board[F2];
	key += 9 * board[F3];
	key += 27 * board[F4];
	key += 81 * board[F5];
	key += 243 * board[F6];
	key += 729 * board[F7];
	key += 2187 * board[F8];

	eval += hori_ver2[key];
	key_hori_ver2[3] = hori_ver2[key];
	//if(hori_ver2[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	return eval;

}

double check_h_ver3()
{
	int key;
	double eval;

	/* a4 b4 c4 d4 e4 f4 g4 h4 */
	/* a5 b5 c5 d5 e5 f5 g5 h5 */
	/* d1 d2 d3 d4 d5 d6 d7 d8 */
	/* e1 e2 e3 e4 e5 e6 e7 e8 */

	key = board[A4];
	key += 3 * board[B4];
	key += 9 * board[C4];
	key += 27 * board[D4];
	key += 81 * board[E4];
	key += 243 * board[F4];
	key += 729 * board[G4];
	key += 2187 * board[H4];

	eval = hori_ver3[key];
	key_hori_ver3[0] = hori_ver3[key];
	//if(hori_ver3[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[A5];
	key += 3 * board[B5];
	key += 9 * board[C5];
	key += 27 * board[D5];
	key += 81 * board[E5];
	key += 243 * board[F5];
	key += 729 * board[G5];
	key += 2187 * board[H5];

	eval += hori_ver3[key];
	key_hori_ver3[1] = hori_ver3[key];
	//if(hori_ver3[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[D1];
	key += 3 * board[D2];
	key += 9 * board[D3];
	key += 27 * board[D4];
	key += 81 * board[D5];
	key += 243 * board[D6];
	key += 729 * board[D7];
	key += 2187 * board[D8];

	eval += hori_ver3[key];
	key_hori_ver3[2] = hori_ver3[key];
	//if(hori_ver3[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[E1];
	key += 3 * board[E2];
	key += 9 * board[E3];
	key += 27 * board[E4];
	key += 81 * board[E5];
	key += 243 * board[E6];
	key += 729 * board[E7];
	key += 2187 * board[E8];

	eval += hori_ver3[key];
	key_hori_ver3[3] = hori_ver3[key];
	//if(hori_ver3[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	return eval;

}

double check_dia_ver1()
{
	int key;
	double eval;

	/* a1 b2 c3 d4 e5 f6 g7 h8 */
	/* h1 g2 f3 e4 d5 c6 b7 a8  */

	key = board[A1];
	key += 3 * board[B2];
	key += 9 * board[C3];
	key += 27 * board[D4];
	key += 81 * board[E5];
	key += 243 * board[F6];
	key += 729 * board[G7];
	key += 2187 * board[H8];

	eval = dia_ver1[key];
	key_dia_ver1[0] = dia_ver1[key];

	//if(dia_ver1[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[H1];
	key += 3 * board[G2];
	key += 9 * board[F3];
	key += 27 * board[E4];
	key += 81 * board[D5];
	key += 243 * board[C6];
	key += 729 * board[B7];
	key += 2187 * board[A8];

	eval += dia_ver1[key];
	key_dia_ver1[1] = dia_ver1[key];
	//if(dia_ver1[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	return eval;
}

double check_dia_ver2()
{
	int key;
	double eval;

	/* a2 b3 c4 d5 e6 f7 g8 */
	/* b1 c2 d3 e4 f5 g6 h7 */
	/* h2 g3 f4 e5 d6 c7 b8 */
	/* g1 f2 e3 d4 c5 b6 a7 */

	key = board[A2];
	key += 3 * board[B3];
	key += 9 * board[C4];
	key += 27 * board[D5];
	key += 81 * board[E6];
	key += 243 * board[F7];
	key += 729 * board[G8];

	eval = dia_ver2[key];
	key_dia_ver2[0] = dia_ver2[key];
	//if(dia_ver2[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[B1];
	key += 3 * board[C2];
	key += 9 * board[D3];
	key += 27 * board[E4];
	key += 81 * board[F5];
	key += 243 * board[G6];
	key += 729 * board[H7];

	eval += dia_ver2[key];
	key_dia_ver2[1] = dia_ver2[key];
	//if(dia_ver2[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[H2];
	key += 3 * board[G3];
	key += 9 * board[F4];
	key += 27 * board[E5];
	key += 81 * board[D6];
	key += 243 * board[C7];
	key += 729 * board[B8];

	eval += dia_ver2[key];
	key_dia_ver2[2] = dia_ver2[key];
	//if(dia_ver2[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[G1];
	key += 3 * board[F2];
	key += 9 * board[E3];
	key += 27 * board[D4];
	key += 81 * board[C5];
	key += 243 * board[B6];
	key += 729 * board[A7];

	eval += dia_ver2[key];
	key_dia_ver2[3] = dia_ver2[key];
	//if(dia_ver2[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	return eval;
}

double check_dia_ver3()
{
	int key;
	double eval;

	key = board[A3];
	key += 3 * board[B4];
	key += 9 * board[C5];
	key += 27 * board[D6];
	key += 81 * board[E7];
	key += 243 * board[F8];

	eval = dia_ver3[key];
	key_dia_ver3[0] = dia_ver3[key];
	//if(dia_ver3[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[C1];
	key += 3 * board[D2];
	key += 9 * board[E3];
	key += 27 * board[F4];
	key += 81 * board[G5];
	key += 243 * board[H6];

	eval += dia_ver3[key];
	key_dia_ver3[1] = dia_ver3[key];
	//if(dia_ver3[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[H3];
	key += 3 * board[G4];
	key += 9 * board[F5];
	key += 27 * board[E6];
	key += 81 * board[D7];
	key += 243 * board[C8];

	eval += dia_ver3[key];
	key_dia_ver3[2] = dia_ver3[key];
	//if(dia_ver3[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	key = board[F1];
	key += 3 * board[E2];
	key += 9 * board[D3];
	key += 27 * board[C4];
	key += 81 * board[B5];
	key += 243 * board[A6];

	eval += dia_ver3[key];
	key_dia_ver3[3] = dia_ver3[key];
	//if(dia_ver3[key] == 0)
	//{
	//	COUNT_NO_PATTERN++;
	//}
	//else
	//{
	//	COUNT_PATTERN++;
	//}

	return eval;
}

double check_dia_ver4()
{
	int key;
	double eval;

	key = board[A4];
	key += 3 * board[B5];
	key += 9 * board[C6];
	key += 27 * board[D7];
	key += 81 * board[E8];

	eval = dia_ver4[key];
	key_dia_ver4[0] = dia_ver4[key];

	key = board[D1];
	key += 3 * board[E2];
	key += 9 * board[F3];
	key += 27 * board[G4];
	key += 81 * board[H5];

	eval += dia_ver4[key];
	key_dia_ver4[1] = dia_ver4[key];

	key = board[H4];
	key += 3 * board[G5];
	key += 9 * board[F6];
	key += 27 * board[E7];
	key += 81 * board[D8];

	eval += dia_ver4[key];
	key_dia_ver4[2] = dia_ver4[key];

	key = board[E1];
	key += 3 * board[D2];
	key += 9 * board[C3];
	key += 27 * board[B4];
	key += 81 * board[A5];

	eval += dia_ver4[key];
	key_dia_ver4[3] = dia_ver4[key];

	return eval;
}

double check_edge()
{
	int key;
	double eval;

	key = board[A1];
	key += 3 * board[A2];
	key += 9 * board[A3];
	key += 27 * board[A4];
	key += 81 * board[A5];
	key += 243 * board[A6];
	key += 729 * board[A7];
	key += 2187 * board[A8];
	key += 6561 * board[B2];
	key += 19683 * board[B7];

	eval = edge[key];
	key_edge[0] = edge[key];

	key = board[H1];
	key += 3 * board[H2];
	key += 9 * board[H3];
	key += 27 * board[H4];
	key += 81 * board[H5];
	key += 243 * board[H6];
	key += 729 * board[H7];
	key += 2187 * board[H8];
	key += 6561 * board[G2];
	key += 19683 * board[G7];

	eval += edge[key];
	key_edge[1] = edge[key];

	key = board[A1];
	key += 3 * board[B1];
	key += 9 * board[C1];
	key += 27 * board[D1];
	key += 81 * board[E1];
	key += 243 * board[F1];
	key += 729 * board[G1];
	key += 2187 * board[H1];
	key += 6561 * board[B2];
	key += 19683 * board[G2];

	eval += edge[key];
	key_edge[2] = edge[key];

	key = board[A8];
	key += 3 * board[B8];
	key += 9 * board[C8];
	key += 27 * board[D8];
	key += 81 * board[E8];
	key += 243 * board[F8];
	key += 729 * board[G8];
	key += 2187 * board[H8];
	key += 6561 * board[B7];
	key += 19683 * board[G7];

	eval += edge[key];
	key_edge[3] = edge[key];

	return eval;
}

double check_corner3_3()
{
	int key;
	double eval;

	key = board[A1];
	key += 3 * board[A2];
	key += 9 * board[A3];
	key += 27 * board[B1];
	key += 81 * board[B2];
	key += 243 * board[B3];
	key += 729 * board[C1];
	key += 2187 * board[C2];
	key += 6561 * board[C3];

	eval = corner3_3[key];
	key_corner3_3[0] = corner3_3[key];

	key = board[H1];
	key += 3 * board[H2];
	key += 9 * board[H3];
	key += 27 * board[G1];
	key += 81 * board[G2];
	key += 243 * board[G3];
	key += 729 * board[F1];
	key += 2187 * board[F2];
	key += 6561 * board[F3];

	eval += corner3_3[key];
	key_corner3_3[1] = corner3_3[key];

	key = board[A8];
	key += 3 * board[A7];
	key += 9 * board[A6];
	key += 27 * board[B8];
	key += 81 * board[B7];
	key += 243 * board[B6];
	key += 729 * board[C8];
	key += 2187 * board[C7];
	key += 6561 * board[C6];

	eval += corner3_3[key];
	key_corner3_3[2] = corner3_3[key];

	key = board[H8];
	key += 3 * board[H7];
	key += 9 * board[H6];
	key += 27 * board[G8];
	key += 81 * board[G7];
	key += 243 * board[G6];
	key += 729 * board[F8];
	key += 2187 * board[F7];
	key += 6561 * board[F6];

	eval += corner3_3[key];
	key_corner3_3[3] = corner3_3[key];

	return eval;
}

double check_corner5_2()
{
	int key;
	double eval;

	key = board[A1];
	key += 3 * board[B1];
	key += 9 * board[C1];
	key += 27 * board[D1];
	key += 81 * board[E1];
	key += 243 * board[A2];
	key += 729 * board[B2];
	key += 2187 * board[C2];
	key += 6561 * board[D2];
	key += 19683 * board[E2];

	eval = corner5_2[key];
	key_corner5_2[0] = corner5_2[key];

	key = board[A8];
	key += 3 * board[B8];
	key += 9 * board[C8];
	key += 27 * board[D8];
	key += 81 * board[E8];
	key += 243 * board[A7];
	key += 729 * board[B7];
	key += 2187 * board[C7];
	key += 6561 * board[D7];
	key += 19683 * board[E7];

	eval += corner5_2[key];
	key_corner5_2[1] = corner5_2[key];

	key = board[H1];
	key += 3 * board[G1];
	key += 9 * board[F1];
	key += 27 * board[E1];
	key += 81 * board[D1];
	key += 243 * board[H2];
	key += 729 * board[G2];
	key += 2187 * board[F2];
	key += 6561 * board[E2];
	key += 19683 * board[D2];

	eval += corner5_2[key];
	key_corner5_2[2] = corner5_2[key];

	key = board[H8];
	key += 3 * board[G8];
	key += 9 * board[F8];
	key += 27 * board[E8];
	key += 81 * board[D8];
	key += 243 * board[H7];
	key += 729 * board[G7];
	key += 2187 * board[F7];
	key += 6561 * board[E7];
	key += 19683 * board[D7];

	eval += corner5_2[key];
	key_corner5_2[3] = corner5_2[key];

	key = board[A1];
	key += 3 * board[A2];
	key += 9 * board[A3];
	key += 27 * board[A4];
	key += 81 * board[A5];
	key += 243 * board[B1];
	key += 729 * board[B2];
	key += 2187 * board[B3];
	key += 6561 * board[B4];
	key += 19683 * board[B5];

	eval += corner5_2[key];
	key_corner5_2[4] = corner5_2[key];

	key = board[H1];
	key += 3 * board[H2];
	key += 9 * board[H3];
	key += 27 * board[H4];
	key += 81 * board[H5];
	key += 243 * board[G1];
	key += 729 * board[G2];
	key += 2187 * board[G3];
	key += 6561 * board[G4];
	key += 19683 * board[G5];

	eval += corner5_2[key];
	key_corner5_2[5] = corner5_2[key];

	key = board[A8];
	key += 3 * board[A7];
	key += 9 * board[A6];
	key += 27 * board[A5];
	key += 81 * board[A4];
	key += 243 * board[B8];
	key += 729 * board[B7];
	key += 2187 * board[B6];
	key += 6561 * board[B5];
	key += 19683 * board[B4];

	eval += corner5_2[key];
	key_corner5_2[6] = corner5_2[key];

	key = board[H8];
	key += 3 * board[H7];
	key += 9 * board[H6];
	key += 27 * board[H5];
	key += 81 * board[H4];
	key += 243 * board[G8];
	key += 729 * board[G7];
	key += 2187 * board[G6];
	key += 6561 * board[G5];
	key += 19683 * board[G4];

	eval += corner5_2[key];
	key_corner5_2[7] = corner5_2[key];

	return eval;
}

double check_triangle()
{
	int key;
	double eval;

	key = board[A1];
	key += 3 * board[A2];
	key += 9 * board[A3];
	key += 27 * board[A4];
	key += 81 * board[B1];
	key += 243 * board[B2];
	key += 729 * board[B3];
	key += 2187 * board[C1];
	key += 6561 * board[C2];
	key += 19683 * board[D1];

	eval = triangle[key];
	key_triangle[0] = triangle[key];

	key = board[H1];
	key += 3 * board[H2];
	key += 9 * board[H3];
	key += 27 * board[H4];
	key += 81 * board[G1];
	key += 243 * board[G2];
	key += 729 * board[G3];
	key += 2187 * board[F1];
	key += 6561 * board[F2];
	key += 19683 * board[E1];

	eval += triangle[key];
	key_triangle[1] = triangle[key];

	key = board[A8];
	key += 3 * board[A7];
	key += 9 * board[A6];
	key += 27 * board[A5];
	key += 81 * board[B8];
	key += 243 * board[B7];
	key += 729 * board[B6];
	key += 2187 * board[C8];
	key += 6561 * board[C7];
	key += 19683 * board[D8];

	eval += triangle[key];
	key_triangle[2] = triangle[key];

	key = board[H8];
	key += 3 * board[H7];
	key += 9 * board[H6];
	key += 27 * board[H5];
	key += 81 * board[G8];
	key += 243 * board[G7];
	key += 729 * board[G6];
	key += 2187 * board[F8];
	key += 6561 * board[F7];
	key += 19683 * board[E8];

	eval += triangle[key];
	key_triangle[3] = triangle[key];

	return eval;
}

double check_parity(UINT64 blank)
{
	int pal;

	pal = CountBit(blank & 0x0f0f0f0f) % 2;
	pal |= (CountBit(blank & 0xf0f0f0f0) % 2) << 1;
	pal |= (CountBit(blank & 0x0f0f0f0f00000000) % 2) << 2;
	pal |= (CountBit(blank & 0xf0f0f0f000000000) % 2) << 3;

	key_parity = parity[pal];

	return parity[pal];
}

/***************************************************************************
* Name  : CreateMoves
* Args  : bk - 黒の配置ビット列
* wh - 白の配置ビット列
* count - 着手可能数の格納変数
* Brief : 着手可能数と着手可能位置のビット列を計算する
* Return: 着手可能位置のビット列
****************************************************************************/
UINT64 CreateMoves(UINT64 bk_p, UINT64 wh_p, UINT32 *p_count_p)
{
	UINT64 moves;
	st_bit stbit_bk, stbit_wh;

	stbit_bk.high = (bk_p >> 32);
	stbit_bk.low = (bk_p & 0xffffffff);

	stbit_wh.high = (wh_p >> 32);
	stbit_wh.low = (wh_p & 0xffffffff);

	*p_count_p = bit_mob(stbit_bk, stbit_wh, &moves);

	return moves;

}

double check_mobility(UINT64 b_board, UINT64 w_board)
{
	UINT32 mob1, mob2;

	CreateMoves(b_board, w_board, &mob1);
	CreateMoves(w_board, b_board, &mob2);

	return mobility[(mob1 - mob2) + (MOBILITY_NUM / 2)];
}

int pow_table[10] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683 };


int symmetry(int index_num)
{
	if (index_num == 0)
	{
		return 0;
	}
	return index_num;
}

/* 線対称 */
int convert_index_sym(int index_num, int num_table[])
{
	int i;
	int s_index_num = 0;
	int *p_num = &num_table[0];
	for (i = 0; index_num != 0; i++)
	{
		s_index_num += symmetry(index_num % 3) * pow_table[p_num[i]];
		index_num /= 3;
	}

	return s_index_num;
}

INT32 Evaluation(UINT8 *board, UINT64 b_board, UINT64 w_board, UINT32 color, UINT32 stage)
{
	double eval = 0;

	color = 0;
	/* 現在の色とステージでポインタを指定 */
	hori_ver1 = hori_ver1_data[stage];
	hori_ver2 = hori_ver2_data[stage];
	hori_ver3 = hori_ver3_data[stage];
	dia_ver1 = dia_ver1_data[stage];
	dia_ver2 = dia_ver2_data[stage];
	dia_ver3 = dia_ver3_data[stage];
	dia_ver4 = dia_ver4_data[stage];
	edge = edge_data[stage];
	corner5_2 = corner5_2_data[stage];
	corner3_3 = corner3_3_data[stage];
	triangle = triangle_data[stage];
	mobility = mobility_data[stage];
	parity = parity_data[stage];
#if 0
	eval = check_h_ver1(board);
	eval += check_h_ver2(board);
	eval += check_h_ver3(board);

	eval += check_dia_ver1(board);
	eval += check_dia_ver2(board);
	eval += check_dia_ver3(board);
	eval += check_dia_ver4(board);

	eval += check_edge(board);
	eval += check_corner5_2(board);
	eval += check_corner3_3(board);
	eval += check_triangle(board);

	eval += check_mobility(b_board, w_board);
	eval += constant_data[color][stage];

	eval_sum = eval;
	eval *= EVAL_ONE_STONE;
#endif
	return (INT32)eval;

}

/* 対称変換テーブル */
int hori_convert_table[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };
int dia2_convert_table[7] = { 6, 5, 4, 3, 2, 1, 0 };
int dia3_convert_table[6] = { 5, 4, 3, 2, 1, 0 };
int dia4_convert_table[8] = { 4, 3, 2, 1, 0 };
int edge_convert_table[10] = { 7, 6, 5, 4, 3, 2, 1, 0, 9, 8 };
int corner3_3_convert_table[9] = { 0, 3, 6, 1, 4, 7, 2, 5, 8 };
int triangle_convert_table[10] = { 0, 4, 7, 9, 1, 5, 8, 2, 6, 3 };

void write_h_ver1(FILE *fp)
{
	int key;
	int sym_key;  //対称形を正規化する

	/* a2 b2 c2 d2 e2 f2 g2 h2 */
	/* a7 b7 c7 d7 e7 f7 g7 h7 */
	/* b1 b2 b3 b4 b5 b6 b7 b8 */
	/* g1 g2 g3 g4 g5 g6 g7 g8 */

	key = board[A2];
	key += 3 * board[B2];
	key += 9 * board[C2];
	key += 27 * board[D2];
	key += 81 * board[E2];
	key += 243 * board[F2];
	key += 729 * board[G2];
	key += 2187 * board[H2];

	/* 対称形が存在するため、小さいインデックス番号に正規化(パターンの出現頻度を多くする) */
	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	fprintf(fp, "%d\n", key);

	key = board[A7];
	key += 3 * board[B7];
	key += 9 * board[C7];
	key += 27 * board[D7];
	key += 81 * board[E7];
	key += 243 * board[F7];
	key += 729 * board[G7];
	key += 2187 * board[H7];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	fprintf(fp, "%d\n", key);

	key = board[B1];
	key += 3 * board[B2];
	key += 9 * board[B3];
	key += 27 * board[B4];
	key += 81 * board[B5];
	key += 243 * board[B6];
	key += 729 * board[B7];
	key += 2187 * board[B8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	fprintf(fp, "%d\n", key);

	key = board[G1];
	key += 3 * board[G2];
	key += 9 * board[G3];
	key += 27 * board[G4];
	key += 81 * board[G5];
	key += 243 * board[G6];
	key += 729 * board[G7];
	key += 2187 * board[G8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	fprintf(fp, "%d\n", key);

}

void write_h_ver2(FILE *fp)
{
	int key;
	int sym_key;

	key = pow_table[0] * board[A3];
	key += pow_table[1] * board[B3];
	key += pow_table[2] * board[C3];
	key += pow_table[3] * board[D3];
	key += pow_table[4] * board[E3];
	key += pow_table[5] * board[F3];
	key += pow_table[6] * board[G3];
	key += pow_table[7] * board[H3];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[A6];
	key += pow_table[1] * board[B6];
	key += pow_table[2] * board[C6];
	key += pow_table[3] * board[D6];
	key += pow_table[4] * board[E6];
	key += pow_table[5] * board[F6];
	key += pow_table[6] * board[G6];
	key += pow_table[7] * board[H6];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[C1];
	key += pow_table[1] * board[C2];
	key += pow_table[2] * board[C3];
	key += pow_table[3] * board[C4];
	key += pow_table[4] * board[C5];
	key += pow_table[5] * board[C6];
	key += pow_table[6] * board[C7];
	key += pow_table[7] * board[C8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[F1];
	key += pow_table[1] * board[F2];
	key += pow_table[2] * board[F3];
	key += pow_table[3] * board[F4];
	key += pow_table[4] * board[F5];
	key += pow_table[5] * board[F6];
	key += pow_table[6] * board[F7];
	key += pow_table[7] * board[F8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);
}

void write_h_ver3(FILE *fp)
{
	int key;
	int sym_key;

	key = pow_table[0] * board[A4];
	key += pow_table[1] * board[B4];
	key += pow_table[2] * board[C4];
	key += pow_table[3] * board[D4];
	key += pow_table[4] * board[E4];
	key += pow_table[5] * board[F4];
	key += pow_table[6] * board[G4];
	key += pow_table[7] * board[H4];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[A5];
	key += pow_table[1] * board[B5];
	key += pow_table[2] * board[C5];
	key += pow_table[3] * board[D5];
	key += pow_table[4] * board[E5];
	key += pow_table[5] * board[F5];
	key += pow_table[6] * board[G5];
	key += pow_table[7] * board[H5];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[D1];
	key += pow_table[1] * board[D2];
	key += pow_table[2] * board[D3];
	key += pow_table[3] * board[D4];
	key += pow_table[4] * board[D5];
	key += pow_table[5] * board[D6];
	key += pow_table[6] * board[D7];
	key += pow_table[7] * board[D8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[E1];
	key += pow_table[1] * board[E2];
	key += pow_table[2] * board[E3];
	key += pow_table[3] * board[E4];
	key += pow_table[4] * board[E5];
	key += pow_table[5] * board[E6];
	key += pow_table[6] * board[E7];
	key += pow_table[7] * board[E8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);
}

void write_diag_ver1(FILE *fp)
{
	int key;
	int sym_key;

	key = pow_table[0] * board[A1];
	key += pow_table[1] * board[B2];
	key += pow_table[2] * board[C3];
	key += pow_table[3] * board[D4];
	key += pow_table[4] * board[E5];
	key += pow_table[5] * board[F6];
	key += pow_table[6] * board[G7];
	key += pow_table[7] * board[H8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H1];
	key += pow_table[1] * board[G2];
	key += pow_table[2] * board[F3];
	key += pow_table[3] * board[E4];
	key += pow_table[4] * board[D5];
	key += pow_table[5] * board[C6];
	key += pow_table[6] * board[B7];
	key += pow_table[7] * board[A8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);
}

void write_diag_ver2(FILE *fp)
{
	int key;
	int sym_key;

	key = pow_table[0] * board[A2];
	key += pow_table[1] * board[B3];
	key += pow_table[2] * board[C4];
	key += pow_table[3] * board[D5];
	key += pow_table[4] * board[E6];
	key += pow_table[5] * board[F7];
	key += pow_table[6] * board[G8];

	sym_key = convert_index_sym(key, dia2_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[B1];
	key += pow_table[1] * board[C2];
	key += pow_table[2] * board[D3];
	key += pow_table[3] * board[E4];
	key += pow_table[4] * board[F5];
	key += pow_table[5] * board[G6];
	key += pow_table[6] * board[H7];

	sym_key = convert_index_sym(key, dia2_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[G1];
	key += pow_table[1] * board[F2];
	key += pow_table[2] * board[E3];
	key += pow_table[3] * board[D4];
	key += pow_table[4] * board[C5];
	key += pow_table[5] * board[B6];
	key += pow_table[6] * board[A7];

	sym_key = convert_index_sym(key, dia2_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H2];
	key += pow_table[1] * board[G3];
	key += pow_table[2] * board[F4];
	key += pow_table[3] * board[E5];
	key += pow_table[4] * board[D6];
	key += pow_table[5] * board[C7];
	key += pow_table[6] * board[B8];

	sym_key = convert_index_sym(key, dia2_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);
}

void write_diag_ver3(FILE *fp)
{
	int key;
	int sym_key;

	key = pow_table[0] * board[A3];
	key += pow_table[1] * board[B4];
	key += pow_table[2] * board[C5];
	key += pow_table[3] * board[D6];
	key += pow_table[4] * board[E7];
	key += pow_table[5] * board[F8];

	sym_key = convert_index_sym(key, dia3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[C1];
	key += pow_table[1] * board[D2];
	key += pow_table[2] * board[E3];
	key += pow_table[3] * board[F4];
	key += pow_table[4] * board[G5];
	key += pow_table[5] * board[H6];

	sym_key = convert_index_sym(key, dia3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[F1];
	key += pow_table[1] * board[E2];
	key += pow_table[2] * board[D3];
	key += pow_table[3] * board[C4];
	key += pow_table[4] * board[B5];
	key += pow_table[5] * board[A6];

	sym_key = convert_index_sym(key, dia3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H3];
	key += pow_table[1] * board[G4];
	key += pow_table[2] * board[F5];
	key += pow_table[3] * board[E6];
	key += pow_table[4] * board[D7];
	key += pow_table[5] * board[C8];

	sym_key = convert_index_sym(key, dia3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);
}

void write_diag_ver4(FILE *fp)
{
	int key;
	int sym_key;

	key = pow_table[0] * board[A4];
	key += pow_table[1] * board[B5];
	key += pow_table[2] * board[C6];
	key += pow_table[3] * board[D7];
	key += pow_table[4] * board[E8];

	sym_key = convert_index_sym(key, dia4_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[D1];
	key += pow_table[1] * board[E2];
	key += pow_table[2] * board[F3];
	key += pow_table[3] * board[G4];
	key += pow_table[4] * board[H5];

	sym_key = convert_index_sym(key, dia4_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[E1];
	key += pow_table[1] * board[D2];
	key += pow_table[2] * board[C3];
	key += pow_table[3] * board[B4];
	key += pow_table[4] * board[A5];

	sym_key = convert_index_sym(key, dia4_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H4];
	key += pow_table[1] * board[G5];
	key += pow_table[2] * board[F6];
	key += pow_table[3] * board[E7];
	key += pow_table[4] * board[D8];

	sym_key = convert_index_sym(key, dia4_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);
}

void write_edge_ver2(FILE *fp)
{
	int key;
	int sym_key;

	key = pow_table[0] * board[A1];
	key += pow_table[1] * board[A2];
	key += pow_table[2] * board[A3];
	key += pow_table[3] * board[A4];
	key += pow_table[4] * board[A5];
	key += pow_table[5] * board[A6];
	key += pow_table[6] * board[A7];
	key += pow_table[7] * board[A8];
	key += pow_table[8] * board[B2];
	key += pow_table[9] * board[B7];

	sym_key = convert_index_sym(key, edge_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[A1];
	key += pow_table[1] * board[B1];
	key += pow_table[2] * board[C1];
	key += pow_table[3] * board[D1];
	key += pow_table[4] * board[E1];
	key += pow_table[5] * board[F1];
	key += pow_table[6] * board[G1];
	key += pow_table[7] * board[H1];
	key += pow_table[8] * board[B2];
	key += pow_table[9] * board[G2];

	sym_key = convert_index_sym(key, edge_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H1];
	key += pow_table[1] * board[H2];
	key += pow_table[2] * board[H3];
	key += pow_table[3] * board[H4];
	key += pow_table[4] * board[H5];
	key += pow_table[5] * board[H6];
	key += pow_table[6] * board[H7];
	key += pow_table[7] * board[H8];
	key += pow_table[8] * board[G2];
	key += pow_table[9] * board[G7];

	sym_key = convert_index_sym(key, edge_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[A8];
	key += pow_table[1] * board[B8];
	key += pow_table[2] * board[C8];
	key += pow_table[3] * board[D8];
	key += pow_table[4] * board[E8];
	key += pow_table[5] * board[F8];
	key += pow_table[6] * board[G8];
	key += pow_table[7] * board[H8];
	key += pow_table[8] * board[B7];
	key += pow_table[9] * board[G7];

	sym_key = convert_index_sym(key, edge_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);
}

void write_corner5_2(FILE *fp)
{
	int key;

	key = pow_table[0] * board[A1];
	key += pow_table[1] * board[B1];
	key += pow_table[2] * board[C1];
	key += pow_table[3] * board[D1];
	key += pow_table[4] * board[E1];
	key += pow_table[5] * board[A2];
	key += pow_table[6] * board[B2];
	key += pow_table[7] * board[C2];
	key += pow_table[8] * board[D2];
	key += pow_table[9] * board[E2];

	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[A8];
	key += pow_table[1] * board[B8];
	key += pow_table[2] * board[C8];
	key += pow_table[3] * board[D8];
	key += pow_table[4] * board[E8];
	key += pow_table[5] * board[A7];
	key += pow_table[6] * board[B7];
	key += pow_table[7] * board[C7];
	key += pow_table[8] * board[D7];
	key += pow_table[9] * board[E7];

	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H1];
	key += pow_table[1] * board[G1];
	key += pow_table[2] * board[F1];
	key += pow_table[3] * board[E1];
	key += pow_table[4] * board[D1];
	key += pow_table[5] * board[H2];
	key += pow_table[6] * board[G2];
	key += pow_table[7] * board[F2];
	key += pow_table[8] * board[E2];
	key += pow_table[9] * board[D2];

	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H8];
	key += pow_table[1] * board[G8];
	key += pow_table[2] * board[F8];
	key += pow_table[3] * board[E8];
	key += pow_table[4] * board[D8];
	key += pow_table[5] * board[H7];
	key += pow_table[6] * board[G7];
	key += pow_table[7] * board[F7];
	key += pow_table[8] * board[E7];
	key += pow_table[9] * board[D7];

	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[A1];
	key += pow_table[1] * board[A2];
	key += pow_table[2] * board[A3];
	key += pow_table[3] * board[A4];
	key += pow_table[4] * board[A5];
	key += pow_table[5] * board[B1];
	key += pow_table[6] * board[B2];
	key += pow_table[7] * board[B3];
	key += pow_table[8] * board[B4];
	key += pow_table[9] * board[B5];

	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H1];
	key += pow_table[1] * board[H2];
	key += pow_table[2] * board[H3];
	key += pow_table[3] * board[H4];
	key += pow_table[4] * board[H5];
	key += pow_table[5] * board[G1];
	key += pow_table[6] * board[G2];
	key += pow_table[7] * board[G3];
	key += pow_table[8] * board[G4];
	key += pow_table[9] * board[G5];

	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[A8];
	key += pow_table[1] * board[A7];
	key += pow_table[2] * board[A6];
	key += pow_table[3] * board[A5];
	key += pow_table[4] * board[A4];
	key += pow_table[5] * board[B8];
	key += pow_table[6] * board[B7];
	key += pow_table[7] * board[B6];
	key += pow_table[8] * board[B5];
	key += pow_table[9] * board[B4];

	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H8];
	key += pow_table[1] * board[H7];
	key += pow_table[2] * board[H6];
	key += pow_table[3] * board[H5];
	key += pow_table[4] * board[H4];
	key += pow_table[5] * board[G8];
	key += pow_table[6] * board[G7];
	key += pow_table[7] * board[G6];
	key += pow_table[8] * board[G5];
	key += pow_table[9] * board[G4];

	fprintf(fp, "%d\n", key);
}

void write_corner3_3(FILE *fp)
{
	int key;
	int sym_key;

	key = pow_table[0] * board[A1];
	key += pow_table[1] * board[A2];
	key += pow_table[2] * board[A3];
	key += pow_table[3] * board[B1];
	key += pow_table[4] * board[B2];
	key += pow_table[5] * board[B3];
	key += pow_table[6] * board[C1];
	key += pow_table[7] * board[C2];
	key += pow_table[8] * board[C3];

	sym_key = convert_index_sym(key, corner3_3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H1];
	key += pow_table[1] * board[H2];
	key += pow_table[2] * board[H3];
	key += pow_table[3] * board[G1];
	key += pow_table[4] * board[G2];
	key += pow_table[5] * board[G3];
	key += pow_table[6] * board[F1];
	key += pow_table[7] * board[F2];
	key += pow_table[8] * board[F3];

	sym_key = convert_index_sym(key, corner3_3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[A8];
	key += pow_table[1] * board[A7];
	key += pow_table[2] * board[A6];
	key += pow_table[3] * board[B8];
	key += pow_table[4] * board[B7];
	key += pow_table[5] * board[B6];
	key += pow_table[6] * board[C8];
	key += pow_table[7] * board[C7];
	key += pow_table[8] * board[C6];

	sym_key = convert_index_sym(key, corner3_3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);

	key = pow_table[0] * board[H8];
	key += pow_table[1] * board[H7];
	key += pow_table[2] * board[H6];
	key += pow_table[3] * board[G8];
	key += pow_table[4] * board[G7];
	key += pow_table[5] * board[G6];
	key += pow_table[6] * board[F8];
	key += pow_table[7] * board[F7];
	key += pow_table[8] * board[F6];

	sym_key = convert_index_sym(key, corner3_3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}
	fprintf(fp, "%d\n", key);
}

void write_triangle(FILE *fp)
{
	int key;
	int sym_key;  //対称形を正規化する

	key = board[A1];
	key += 3 * board[A2];
	key += 9 * board[A3];
	key += 27 * board[A4];
	key += 81 * board[B1];
	key += 243 * board[B2];
	key += 729 * board[B3];
	key += 2187 * board[C1];
	key += 6561 * board[C2];
	key += 19683 * board[D1];

	sym_key = convert_index_sym(key, triangle_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	fprintf(fp, "%d\n", key);

	key = board[H1];
	key += 3 * board[H2];
	key += 9 * board[H3];
	key += 27 * board[H4];
	key += 81 * board[G1];
	key += 243 * board[G2];
	key += 729 * board[G3];
	key += 2187 * board[F1];
	key += 6561 * board[F2];
	key += 19683 * board[E1];

	sym_key = convert_index_sym(key, triangle_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	fprintf(fp, "%d\n", key);

	key = board[A8];
	key += 3 * board[A7];
	key += 9 * board[A6];
	key += 27 * board[A5];
	key += 81 * board[B8];
	key += 243 * board[B7];
	key += 729 * board[B6];
	key += 2187 * board[C8];
	key += 6561 * board[C7];
	key += 19683 * board[D8];

	sym_key = convert_index_sym(key, triangle_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	fprintf(fp, "%d\n", key);

	key = board[H8];
	key += 3 * board[H7];
	key += 9 * board[H6];
	key += 27 * board[H5];
	key += 81 * board[G8];
	key += 243 * board[G7];
	key += 729 * board[G6];
	key += 2187 * board[F8];
	key += 6561 * board[F7];
	key += 19683 * board[E8];

	sym_key = convert_index_sym(key, triangle_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	fprintf(fp, "%d\n", key);


}

int convert_index_parity_sym(int key)
{
	int cnt = CountBit(key);

	if (cnt == 0 || cnt == 4)
	{
		return key;
	}

	if (cnt == 1)
	{
		return 1;
	}

	if (cnt == 2)
	{
		return 3;
	}

	if (cnt == 3)
	{
		return 7;
	}

	return key;

}

void write_pality(FILE *fp, UINT64 blank, int color)
{
	int key = 0;
	int sym_key;

	key |= CountBit(blank & 0x0f0f0f0f) % 2;
	key |= (CountBit(blank & 0xf0f0f0f0) % 2) << 1;
	key |= (CountBit(blank & 0x0f0f0f0f00000000) % 2) << 2;
	key |= (CountBit(blank & 0xf0f0f0f000000000) % 2) << 3;

	sym_key = convert_index_parity_sym(key);
	if (key > sym_key)
	{
		key = sym_key;
	}

	fprintf(fp, "%d\n", key);
}

#if 0
void ReadEvalDat()
{
	FILE *fp;
	int stage = 0;
	int i;
	int bufSize = 1.5 * 1024 * 1024;
	char *buf = (char *)malloc(bufSize);
	char *line, *ctr;
	char file_path[32];

	float *p_table, *p_table_op;

	while(stage < 60)
	{
		//sprintf_s(file_path, "src\\eval_table_%s.dat", eval_table_list[stage]);
		sprintf_s(file_path, "src\\%d.dat", stage);
		if(fopen_s(&fp, file_path, "r") != 0){
			exit(0);
			//stage++;
			//continue;
		}

		fread(buf, sizeof(char), bufSize, fp);
		line = strtok_s(buf, "\n", &ctr);

		/* horizon_ver1 */
		p_table = hori_ver1_data[0][stage];
		p_table_op = hori_ver1_data[1][stage];
		for(i = 0; i < 6561; i++)
		{
			p_table[i] = atof(line);
			/* opponent */
			p_table_op[opponent_feature(i, 8)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* horizon_ver2 */
		p_table = hori_ver2_data[0][stage];
		p_table_op = hori_ver2_data[1][stage];
		for(i = 0; i < 6561; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 8)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* horizon_ver3 */
		p_table = hori_ver3_data[0][stage];
		p_table_op = hori_ver3_data[1][stage];
		for(i = 0; i < 6561; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 8)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* diagram_ver1 */
		p_table = dia_ver1_data[0][stage];
		p_table_op = dia_ver1_data[1][stage];
		for(i = 0; i < 6561; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 8)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* diagram_ver2 */
		p_table = dia_ver2_data[0][stage];
		p_table_op = dia_ver2_data[1][stage];
		for(i = 0; i < 2187; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 7)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* diagram_ver3 */
		p_table = dia_ver3_data[0][stage];
		p_table_op = dia_ver3_data[1][stage];
		for(i = 0; i < 729; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 6)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* diagram_ver4 */
		p_table = dia_ver4_data[0][stage];
		p_table_op = dia_ver4_data[1][stage];
		for(i = 0; i < 243; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 5)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* diagram_ver5 */
		p_table = dia_ver5_data[0][stage];
		p_table_op = dia_ver5_data[1][stage];
		for (i = 0; i < 81; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 4)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* edge */
		p_table = edge_data[0][stage];
		p_table_op = edge_data[1][stage];
		for(i = 0; i < 59049; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 10)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* corner5 + 2X */
		p_table = edge_cor_data[0][stage];
		p_table_op = edge_cor_data[1][stage];
		for(i = 0; i < 59049; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 10)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* corner4_2 */
		p_table = corner4_2_data[0][stage];
		p_table_op = corner4_2_data[1][stage];
		for(i = 0; i < 59049; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 10)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* corner3_3 */
		p_table = corner3_3_data[0][stage];
		p_table_op = corner3_3_data[1][stage];
		for(i = 0; i < 19683; i++)
		{
			p_table[i] = atof(line);
			p_table_op[opponent_feature(i, 9)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* mobility */
		/*p_table = mobility_data[stage];
		for (i = 0; i < MOBILITY_NUM; i++)
		{
		p_table[i] = atof(line);
		line = strtok_s(NULL, "\n", &ctr);
		}
		*/
		/*pot_mobility_data[stage] = atof(line);
		line = strtok_s(NULL, "\n", &ctr);*/
		//constant_data[stage] = atof(line);
		//line = strtok_s(NULL, "\n", &ctr);

		stage++;
		fclose(fp);
	}

	free(buf);
}



void readMPCInfo(){

	FILE *fp;
	char filename[32];

	//sprintf_s(filename, 32, "src\\mpc%02d.dat", i);
	sprintf_s(filename, 32, "src\\mpc.dat");
	if (fopen_s(&fp, filename, "r")){
		exit(0);
	}
	memset(MPCInfo, 0, sizeof(MPCINFO) * 24);

	for (int j = 0; j < 22; j++){
		fscanf_s(fp, "%d", &(MPCInfo[j].depth));
		fscanf_s(fp, "%d", &(MPCInfo[j].offset));
		fscanf_s(fp, "%d", &(MPCInfo[j].deviation));
	}
	fclose(fp);
}
#endif
