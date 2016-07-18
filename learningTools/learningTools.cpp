// learningTools.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <Windows.h>
#include <nmmintrin.h>

#include "learningTools.h"
#include "bit64.h"
#include "type.h"
#include "rev.h"
#include "move.h"
#include "eval.h"
#include "cpu.h"
#include "hash.h"
#include "mpc_learn.h"

/* 各座標 */
#define A1 0			/* A1 */
#define A2 1			/* A2 */
#define A3 2			/* A3 */
#define A4 3			/* A4 */
#define A5 4			/* A5 */
#define A6 5			/* A6 */
#define A7 6			/* A7 */
#define A8 7			/* A8 */

#define B1 8			/* B1 */
#define B2 9			/* B2 */
#define B3 10			/* B3 */
#define B4 11			/* B4 */
#define B5 12			/* B5 */
#define B6 13			/* B6 */
#define B7 14			/* B7 */
#define B8 15			/* B8 */

#define C1 16			/* C1 */
#define C2 17			/* C2 */
#define C3 18			/* C3 */
#define C4 19			/* C4 */
#define C5 20			/* C5 */
#define C6 21			/* C6 */
#define C7 22			/* C7 */
#define C8 23			/* C8 */

#define D1 24			/* D1 */
#define D2 25			/* D2 */
#define D3 26			/* D3 */
#define D4 27			/* D4 */
#define D5 28			/* D5 */
#define D6 29			/* D6 */
#define D7 30			/* D7 */
#define D8 31			/* D8 */

#define E1 32			/* E1 */
#define E2 33			/* E2 */
#define E3 34			/* E3 */
#define E4 35			/* E4 */
#define E5 36			/* E5 */
#define E6 37			/* E6 */
#define E7 38			/* E7 */
#define E8 39			/* E8 */

#define F1 40			/* F1 */
#define F2 41			/* F2 */
#define F3 42			/* F3 */
#define F4 43			/* F4 */
#define F5 44			/* F5 */
#define F6 45			/* F6 */
#define F7 46			/* F7 */
#define F8 47			/* F8 */

#define G1 48			/* G1 */
#define G2 49			/* G2 */
#define G3 50			/* G3 */
#define G4 51			/* G4 */
#define G5 52			/* G5 */
#define G6 53			/* G6 */
#define G7 54			/* G7 */
#define G8 55			/* G8 */

#define H1 56			/* H1 */
#define H2 57			/* H2 */
#define H3 58			/* H3 */
#define H4 59			/* H4 */
#define H5 60			/* H5 */
#define H6 61			/* H6 */
#define H7 62			/* H7 */
#define H8 63			/* H8 */

int hori1cnt[INDEX_NUM] = { 0 };
int hori2cnt[INDEX_NUM] = { 0 };
int hori3cnt[INDEX_NUM] = { 0 };
int diag1cnt[INDEX_NUM] = { 0 };
int diag2cnt[INDEX_NUM / 3] = { 0 };
int diag3cnt[INDEX_NUM / 9] = { 0 };
int diag4cnt[INDEX_NUM / 27] = { 0 };
int edgecnt[INDEX_NUM * 9] = { 0 };
int cor52cnt[INDEX_NUM * 9] = { 0 };
int cor33cnt[INDEX_NUM * 3] = { 0 };
int trianglecnt[INDEX_NUM * 9] = { 0 };
int mobcnt[MOBILITY_NUM] = { 0 };
int paritycnt[PARITY_NUM] = { 0 };

/* 探索ベクトル */
double hori_ver1_data_d[INDEX_NUM];
double hori_ver2_data_d[INDEX_NUM];
double hori_ver3_data_d[INDEX_NUM];
double dia_ver1_data_d[INDEX_NUM];
double dia_ver2_data_d[INDEX_NUM / 3];
double dia_ver3_data_d[INDEX_NUM / 9];
double dia_ver4_data_d[INDEX_NUM / 27];
double edge_data_d[INDEX_NUM * 9];
double corner5_2_data_d[INDEX_NUM * 9];
double corner3_3_data_d[INDEX_NUM * 3];
double triangle_data_d[INDEX_NUM * 9];
double mobility_data_d[MOBILITY_NUM];
double parity_data_d[PARITY_NUM];

const int pow_table[10] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683 };

/* 対称変換テーブル */
const int hori_convert_table[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };
const int dia2_convert_table[7] = { 6, 5, 4, 3, 2, 1, 0 };
const int dia3_convert_table[6] = { 5, 4, 3, 2, 1, 0 };
const int dia4_convert_table[5] = { 4, 3, 2, 1, 0 };
const int dia5_convert_table[4] = { 3, 2, 1, 0 };
const int edge_convert_table[10] = { 7, 6, 5, 4, 3, 2, 1, 0, 9, 8 };
//const int edge_cor_convert_table[10] = { 0, 1, 6, 7, 8, 9, 2, 3, 4, 5 };
//const int corner4_2_convert_table[10] = { 1, 0, 5, 4, 3, 2, 9, 8, 7, 6 };
const int corner3_3_convert_table[9] = { 0, 3, 6, 1, 4, 7, 2, 5, 8 };
const int triangle_convert_table[10] = { 0, 4, 7, 9, 1, 5, 8, 2, 6, 3 };


int bk_win = 0, wh_win = 0;
int game_num1, game_num2;
int rand_index_array[926590 - 839127];

int score[129] = { 0 };
int kifuNum[129] = { 0 };

extern int INF_DEPTH;

void init_index_board(int *board, UINT64 bk, UINT64 wh)
{
	board[0] = (int)(bk & a1) + (int)(wh & a1) * 2;
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

void init_substitution_table_exact(HashTable *hash)
{
	int i;
	for (i = 0; i < g_casheSize; i++)
	{

		hash->entry[i].deepest.lower = -INF_SCORE;
		hash->entry[i].deepest.upper = INF_SCORE;
		hash->entry[i].newest.lower = -INF_SCORE;
		hash->entry[i].newest.upper = INF_SCORE;
	}
}

void init_substitution_table_winloss(HashTable *hash)
{
	int i;
	for (i = 0; i < g_casheSize; i++)
	{
		hash->entry[i].deepest.lower = -2;
		hash->entry[i].deepest.upper = 2;
		hash->entry[i].newest.lower = -2;
		hash->entry[i].newest.upper = 2;
	}
}

void initCountTable()
{
	memset(hori1cnt, 0, sizeof(hori1cnt));
	memset(hori2cnt, 0, sizeof(hori2cnt));
	memset(hori3cnt, 0, sizeof(hori3cnt));
	memset(diag1cnt, 0, sizeof(diag1cnt));
	memset(diag2cnt, 0, sizeof(diag2cnt));
	memset(diag3cnt, 0, sizeof(diag3cnt));
	memset(diag4cnt, 0, sizeof(diag4cnt));
	memset(edgecnt, 0, sizeof(edgecnt));
	memset(cor52cnt, 0, sizeof(cor52cnt));
	memset(cor33cnt, 0, sizeof(cor33cnt));
	memset(mobcnt, 0, sizeof(mobcnt));
	memset(paritycnt, 0, sizeof(paritycnt));

}


/* 線対称 */
int convert_index_sym(int index_num, const int *num_table)
{
	int i;
	int s_index_num = 0;
	for (i = 0; index_num != 0; i++)
	{
		s_index_num += (index_num % 3) * pow_table[num_table[i]];
		index_num /= 3;
	}

	return s_index_num;
}

void write_h_ver1(USHORT *keyIndex, int *board)
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

	keyIndex[0] = key;
	hori1cnt[key]++;

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

	keyIndex[1] = key;
	hori1cnt[key]++;

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

	keyIndex[2] = key;
	hori1cnt[key]++;

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

	keyIndex[3] = key;
	hori1cnt[key]++;
}
void write_h_ver2(USHORT *keyIndex, int *board)
{
	int key;
	int sym_key;

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

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[0] = key;
	hori2cnt[key]++;

	key = board[A6];
	key += 3 * board[B6];
	key += 9 * board[C6];
	key += 27 * board[D6];
	key += 81 * board[E6];
	key += 243 * board[F6];
	key += 729 * board[G6];
	key += 2187 * board[H6];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[1] = key;
	hori2cnt[key]++;

	key = board[C1];
	key += 3 * board[C2];
	key += 9 * board[C3];
	key += 27 * board[C4];
	key += 81 * board[C5];
	key += 243 * board[C6];
	key += 729 * board[C7];
	key += 2187 * board[C8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[2] = key;
	hori2cnt[key]++;

	key = board[F1];
	key += 3 * board[F2];
	key += 9 * board[F3];
	key += 27 * board[F4];
	key += 81 * board[F5];
	key += 243 * board[F6];
	key += 729 * board[F7];
	key += 2187 * board[F8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[3] = key;
	hori2cnt[key]++;
}
void write_h_ver3(USHORT *keyIndex, int *board)
{
	int key;
	int sym_key;

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

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[0] = key;
	hori3cnt[key]++;

	key = board[A5];
	key += 3 * board[B5];
	key += 9 * board[C5];
	key += 27 * board[D5];
	key += 81 * board[E5];
	key += 243 * board[F5];
	key += 729 * board[G5];
	key += 2187 * board[H5];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[1] = key;
	hori3cnt[key]++;

	key = board[D1];
	key += 3 * board[D2];
	key += 9 * board[D3];
	key += 27 * board[D4];
	key += 81 * board[D5];
	key += 243 * board[D6];
	key += 729 * board[D7];
	key += 2187 * board[D8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[2] = key;
	hori3cnt[key]++;

	key = board[E1];
	key += 3 * board[E2];
	key += 9 * board[E3];
	key += 27 * board[E4];
	key += 81 * board[E5];
	key += 243 * board[E6];
	key += 729 * board[E7];
	key += 2187 * board[E8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[3] = key;
	hori3cnt[key]++;

}
void write_dia_ver1(USHORT *keyIndex, int *board)
{
	int key;
	int sym_key;


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

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[0] = key;
	diag1cnt[key]++;

	key = board[H1];
	key += 3 * board[G2];
	key += 9 * board[F3];
	key += 27 * board[E4];
	key += 81 * board[D5];
	key += 243 * board[C6];
	key += 729 * board[B7];
	key += 2187 * board[A8];

	sym_key = convert_index_sym(key, hori_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[1] = key;
	diag1cnt[key]++;

}
void write_dia_ver2(USHORT *keyIndex, int *board)
{
	int key;
	int sym_key;

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

	sym_key = convert_index_sym(key, dia2_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[0] = key;
	diag2cnt[key]++;

	key = board[B1];
	key += 3 * board[C2];
	key += 9 * board[D3];
	key += 27 * board[E4];
	key += 81 * board[F5];
	key += 243 * board[G6];
	key += 729 * board[H7];

	sym_key = convert_index_sym(key, dia2_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[1] = key;
	diag2cnt[key]++;

	key = board[H2];
	key += 3 * board[G3];
	key += 9 * board[F4];
	key += 27 * board[E5];
	key += 81 * board[D6];
	key += 243 * board[C7];
	key += 729 * board[B8];

	sym_key = convert_index_sym(key, dia2_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[2] = key;
	diag2cnt[key]++;

	key = board[G1];
	key += 3 * board[F2];
	key += 9 * board[E3];
	key += 27 * board[D4];
	key += 81 * board[C5];
	key += 243 * board[B6];
	key += 729 * board[A7];

	sym_key = convert_index_sym(key, dia2_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[3] = key;
	diag2cnt[key]++;

}
void write_dia_ver3(USHORT *keyIndex, int *board)
{
	int key;
	int sym_key;

	key = board[A3];
	key += 3 * board[B4];
	key += 9 * board[C5];
	key += 27 * board[D6];
	key += 81 * board[E7];
	key += 243 * board[F8];

	sym_key = convert_index_sym(key, dia3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[0] = key;
	diag3cnt[key]++;

	key = board[C1];
	key += 3 * board[D2];
	key += 9 * board[E3];
	key += 27 * board[F4];
	key += 81 * board[G5];
	key += 243 * board[H6];

	sym_key = convert_index_sym(key, dia3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[1] = key;
	diag3cnt[key]++;

	key = board[H3];
	key += 3 * board[G4];
	key += 9 * board[F5];
	key += 27 * board[E6];
	key += 81 * board[D7];
	key += 243 * board[C8];

	sym_key = convert_index_sym(key, dia3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[2] = key;
	diag3cnt[key]++;

	key = board[F1];
	key += 3 * board[E2];
	key += 9 * board[D3];
	key += 27 * board[C4];
	key += 81 * board[B5];
	key += 243 * board[A6];

	sym_key = convert_index_sym(key, dia3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[3] = key;
	diag3cnt[key]++;

}
void write_dia_ver4(USHORT *keyIndex, int *board)
{
	int key;
	int sym_key;

	key = board[A4];
	key += 3 * board[B5];
	key += 9 * board[C6];
	key += 27 * board[D7];
	key += 81 * board[E8];

	sym_key = convert_index_sym(key, dia4_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[0] = key;
	diag4cnt[key]++;

	key = board[D1];
	key += 3 * board[E2];
	key += 9 * board[F3];
	key += 27 * board[G4];
	key += 81 * board[H5];

	sym_key = convert_index_sym(key, dia4_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[1] = key;
	diag4cnt[key]++;

	key = board[H4];
	key += 3 * board[G5];
	key += 9 * board[F6];
	key += 27 * board[E7];
	key += 81 * board[D8];

	sym_key = convert_index_sym(key, dia4_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[2] = key;
	diag4cnt[key]++;

	key = board[E1];
	key += 3 * board[D2];
	key += 9 * board[C3];
	key += 27 * board[B4];
	key += 81 * board[A5];

	sym_key = convert_index_sym(key, dia4_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[3] = key;
	diag4cnt[key]++;

}

/*
void write_dia_ver5(USHORT *keyIndex, int *board)
{
	int key;
	int sym_key;

	key = board[D1];
	key += 3 * board[C2];
	key += 9 * board[B3];
	key += 27 * board[A4];

	sym_key = convert_index_sym(key, dia5_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[0] = key;
	diag5cnt[key]++;

	key = board[D8];
	key += 3 * board[C7];
	key += 9 * board[B6];
	key += 27 * board[A5];

	sym_key = convert_index_sym(key, dia5_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[1] = key;
	diag5cnt[key]++;

	key = board[E1];
	key += 3 * board[F2];
	key += 9 * board[G3];
	key += 27 * board[H4];

	sym_key = convert_index_sym(key, dia5_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[2] = key;
	diag5cnt[key]++;

	key = board[E8];
	key += 3 * board[F7];
	key += 9 * board[G6];
	key += 27 * board[H5];

	sym_key = convert_index_sym(key, dia5_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[3] = key;
	diag5cnt[key]++;

}
*/

void write_edge(USHORT *keyIndex, int *board)
{
	int key;
	int sym_key;

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

	sym_key = convert_index_sym(key, edge_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[0] = key;
	edgecnt[key]++;

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

	sym_key = convert_index_sym(key, edge_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[1] = key;
	edgecnt[key]++;

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

	sym_key = convert_index_sym(key, edge_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[2] = key;
	edgecnt[key]++;

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

	sym_key = convert_index_sym(key, edge_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[3] = key;
	edgecnt[key]++;

}
/*
void write_edge_cor(USHORT *keyIndex, int *board)
{
int key;
int sym_key;

key = board[B2];
key += 3 * board[A1];
key += 9 * board[A2];
key += 27 * board[A3];
key += 81 * board[A4];
key += 243 * board[A5];
key += 729 * board[B1];
key += 2187 * board[C1];
key += 6561 * board[D1];
key += 19683 * board[E1];

sym_key = convert_index_sym(key, edge_cor_convert_table);
if(key > sym_key)
{
key = sym_key;
}

keyIndex[0] = key;
edgecorcnt[key]++;

key = board[G2];
key += 3 * board[H1];
key += 9 * board[G1];
key += 27 * board[F1];
key += 81 * board[E1];
key += 243 * board[D1];
key += 729 * board[H2];
key += 2187 * board[H3];
key += 6561 * board[H4];
key += 19683 * board[H5];

sym_key = convert_index_sym(key, edge_cor_convert_table);
if(key > sym_key)
{
key = sym_key;
}

keyIndex[1] = key;
edgecorcnt[key]++;

key = board[G7];
key += 3 * board[H8];
key += 9 * board[H7];
key += 27 * board[H6];
key += 81 * board[H5];
key += 243 * board[H4];
key += 729 * board[G8];
key += 2187 * board[F8];
key += 6561 * board[E8];
key += 19683 * board[D8];

sym_key = convert_index_sym(key, edge_cor_convert_table);
if(key > sym_key)
{
key = sym_key;
}

keyIndex[2] = key;
edgecorcnt[key]++;

key = board[B7];
key += 3 * board[A8];
key += 9 * board[B8];
key += 27 * board[C8];
key += 81 * board[D8];
key += 243 * board[E8];
key += 729 * board[A7];
key += 2187 * board[A6];
key += 6561 * board[A5];
key += 19683 * board[A4];

sym_key = convert_index_sym(key, edge_cor_convert_table);
if(key > sym_key)
{
key = sym_key;
}

keyIndex[3] = key;
edgecorcnt[key]++;

}
void write_corner4_2(USHORT *keyIndex, int *board)
{
int key;
int sym_key;

key = board[A1];
key += 3 * board[A8];
key += 9 * board[A3];
key += 27 * board[A4];
key += 81 * board[A5];
key += 243 * board[A6];
key += 729 * board[B3];
key += 2187 * board[B4];
key += 6561 * board[B5];
key += 19683 * board[B6];

sym_key = convert_index_sym(key, corner4_2_convert_table);
if(key > sym_key)
{
key = sym_key;
}

keyIndex[0] = key;
cor42cnt[key]++;

key = board[H1];
key += 3 * board[A1];
key += 9 * board[F1];
key += 27 * board[E1];
key += 81 * board[D1];
key += 243 * board[C1];
key += 729 * board[F2];
key += 2187 * board[E2];
key += 6561 * board[D2];
key += 19683 * board[C2];

sym_key = convert_index_sym(key, corner4_2_convert_table);
if(key > sym_key)
{
key = sym_key;
}

keyIndex[1] = key;
cor42cnt[key]++;

key = board[H1];
key += 3 * board[H8];
key += 9 * board[H3];
key += 27 * board[H4];
key += 81 * board[H5];
key += 243 * board[H6];
key += 729 * board[G3];
key += 2187 * board[G4];
key += 6561 * board[G5];
key += 19683 * board[G6];

sym_key = convert_index_sym(key, corner4_2_convert_table);
if(key > sym_key)
{
key = sym_key;
}

keyIndex[2] = key;
cor42cnt[key]++;

key = board[A8];
key += 3 * board[H8];
key += 9 * board[C8];
key += 27 * board[D8];
key += 81 * board[E8];
key += 243 * board[F8];
key += 729 * board[C7];
key += 2187 * board[D7];
key += 6561 * board[E7];
key += 19683 * board[F7];

sym_key = convert_index_sym(key, corner4_2_convert_table);
if(key > sym_key)
{
key = sym_key;
}

keyIndex[3] = key;
cor42cnt[key]++;

}
*/

void write_corner3_3(USHORT *keyIndex, int *board)
{
	int key;
	int sym_key;

	key = board[A1];
	key += 3 * board[A2];
	key += 9 * board[A3];
	key += 27 * board[B1];
	key += 81 * board[B2];
	key += 243 * board[B3];
	key += 729 * board[C1];
	key += 2187 * board[C2];
	key += 6561 * board[C3];

	sym_key = convert_index_sym(key, corner3_3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[0] = key;
	cor33cnt[key]++;

	key = board[H1];
	key += 3 * board[H2];
	key += 9 * board[H3];
	key += 27 * board[G1];
	key += 81 * board[G2];
	key += 243 * board[G3];
	key += 729 * board[F1];
	key += 2187 * board[F2];
	key += 6561 * board[F3];

	sym_key = convert_index_sym(key, corner3_3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[1] = key;
	cor33cnt[key]++;

	key = board[A8];
	key += 3 * board[A7];
	key += 9 * board[A6];
	key += 27 * board[B8];
	key += 81 * board[B7];
	key += 243 * board[B6];
	key += 729 * board[C8];
	key += 2187 * board[C7];
	key += 6561 * board[C6];

	sym_key = convert_index_sym(key, corner3_3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[2] = key;
	cor33cnt[key]++;

	key = board[H8];
	key += 3 * board[H7];
	key += 9 * board[H6];
	key += 27 * board[G8];
	key += 81 * board[G7];
	key += 243 * board[G6];
	key += 729 * board[F8];
	key += 2187 * board[F7];
	key += 6561 * board[F6];

	sym_key = convert_index_sym(key, corner3_3_convert_table);
	if (key > sym_key)
	{
		key = sym_key;
	}

	keyIndex[3] = key;
	cor33cnt[key]++;

}

void write_corner5_2(USHORT *keyIndex, int *board)
{
	int key;

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

	keyIndex[0] = key;
	cor52cnt[key]++;

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

	keyIndex[1] = key;
	cor52cnt[key]++;

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

	keyIndex[2] = key;
	cor52cnt[key]++;

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

	keyIndex[3] = key;
	cor52cnt[key]++;

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

	keyIndex[4] = key;
	cor52cnt[key]++;

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

	keyIndex[5] = key;
	cor52cnt[key]++;

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

	keyIndex[6] = key;
	cor52cnt[key]++;

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

	keyIndex[7] = key;
	cor52cnt[key]++;
}

void write_triangle(USHORT *keyIndex, int *board)
{
	int key;
	int sym_key;

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
	keyIndex[0] = key;
	trianglecnt[key]++;

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
	keyIndex[1] = key;
	trianglecnt[key]++;

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
	keyIndex[2] = key;
	trianglecnt[key]++;

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
	keyIndex[3] = key;
	trianglecnt[key]++;
}

int convert_index_parity_sym(int key)
{
	switch (key)
	{
	case 2:
	case 4:
	case 8:
		key = 1;
		break;
	case 5:
	case 6:
	case 9:
	case 10:
	case 12:
		key = 3;
		break;
	case 11:
	case 13:
	case 14:
		key = 7;
		break;
	}

	return key;
}

void write_parity(USHORT *keyIndex, UINT64 blank)
{
	int key;
	int sym_key;

	key = CountBit(blank & 0x0f0f0f0f) % 2;
	key |= (CountBit(blank & 0xf0f0f0f0) % 2) << 1;
	key |= (CountBit(blank & 0x0f0f0f0f00000000) % 2) << 2;
	key |= (CountBit(blank & 0xf0f0f0f000000000) % 2) << 3;
#if 1
	sym_key = convert_index_parity_sym(key);
	if (key > sym_key)
	{
		key = sym_key;
	}
#endif
	keyIndex[0] = key;
	paritycnt[key]++;
}

int ConvertWhorMoveToNum(char whorData)
{
	if (whorData == 0x00)
	{
		return -1;
	}
	return (((whorData % 10) - 1) * 8) + ((whorData / 10) - 1);
}

int getFeatureIndex(USHORT* keyIndex, char* whorData, int stage, char *t)
{
	int color = BLACK;
	int move;
	int offset = 0;
	UINT64 bk = FIRST_BK, wh = FIRST_WH;
	UINT64 rev;

	// ゲームヘッダから石差を取得
	int i;
	for (i = 1; i < stage + 2; i++)
	{
		move = ConvertWhorMoveToNum(whorData[i]);
		if (move == -1)
		{
			continue;
		}
		if (color == BLACK)
		{
			rev = GetRev[move](bk, wh);
			/* 黒パス？ */
			if (rev == 0)
			{
				rev = GetRev[move](wh, bk);
				/* 一応合法手になっているかのチェック */
				if (rev == 0)
				{
					break;
				}
				bk ^= rev;
				wh ^= ((1ULL << move) | rev);
			}
			else
			{
				bk ^= ((1ULL << move) | rev);
				wh ^= rev;
				color ^= 1;
			}
		}
		else
		{
			rev = GetRev[move](wh, bk);
			/* 白パス？ */
			if (rev == 0)
			{
				rev = GetRev[move](bk, wh);
				/* 一応合法手になっているかのチェック */
				if (rev == 0)
				{
					break;
				}
				bk ^= ((1ULL << move) | rev);
				wh ^= rev;
			}
			else
			{
				bk ^= rev;
				wh ^= ((1ULL << move) | rev);
				color ^= 1;
			}
		}


		if (i == stage + 1){

			// 石差
			int board[64] = { 0 };
			*t = whorData[0] - 32;
			init_index_board(board, bk, wh);

			write_h_ver1(&keyIndex[offset + 0], board);
			write_h_ver2(&keyIndex[offset + 4], board);
			write_h_ver3(&keyIndex[offset + 8], board);
			write_dia_ver1(&keyIndex[offset + 12], board);
			write_dia_ver2(&keyIndex[offset + 14], board);
			write_dia_ver3(&keyIndex[offset + 18], board);
			write_dia_ver4(&keyIndex[offset + 22], board);
			write_edge(&keyIndex[offset + 26], board);
			write_corner5_2(&keyIndex[offset + 34], board);
			write_corner3_3(&keyIndex[offset + 38], board);
			write_triangle(&keyIndex[offset + 42], board);

			UINT32 bk_mob, wh_mob;
			CreateMoves(bk, wh, &bk_mob);
			CreateMoves(wh, bk, &wh_mob);
			bk_mob = bk_mob - wh_mob + (MOBILITY_NUM / 2);
			keyIndex[offset + 46] = bk_mob;
			mobcnt[bk_mob]++;

			write_parity(&keyIndex[offset + 47], ~(bk | wh));

			offset += PATTERN_NUM;
		}

	}

	if (i < stage + 1){
		return -1;
	}

	return offset;
}

int getFeatureIndex2(char* t, USHORT* keyIndex, char* kifuData, int stage)
{
	int color = BLACK;
	int move;
	int byteCounter = 0;
	UINT64 bk = FIRST_BK, wh = FIRST_WH;
	UINT64 rev;
	bool badFlag = false;
	int turn = 0;
	int offset = 0;
	UINT32 temp;

	while (1)
	{
		while (kifuData[byteCounter] == ' '){
			byteCounter++;
		}

		/* 英字を読み込む */
		/* オシマイ */
		if (!isalpha(kifuData[byteCounter]))
		{
			break;
		}
		/* 数字を読み込む */
		/* なぜかおかしな棋譜データが結構ある */
		if (!isdigit(kifuData[byteCounter + 1]))
		{
			badFlag = true;
			break;
		}

		move = ((kifuData[byteCounter] - 'A') * 8) + (kifuData[byteCounter + 1] - '1');
		/* a9 とか明らかに間違った手を含んでいる棋譜がある */
		if (move < 0 || move >= 64)
		{
			badFlag = true;
			break;
		}
		/* なぜかすでに置いたマスに置いている棋譜がある */
		if (bk & (1ULL << move) || wh & (1ULL << move))
		{
			badFlag = true;
			break;
		}

		if (color == BLACK)
		{
			rev = GetRev[move](bk, wh);
			/* 黒パス？ */
			if (rev == 0)
			{
				rev = GetRev[move](wh, bk);
				/* 一応合法手になっているかのチェック */
				if (rev == 0)
				{
					badFlag = true;
					break;
				}
				bk ^= rev;
				wh ^= ((1ULL << move) | rev);
			}
			else
			{
				bk ^= ((1ULL << move) | rev);
				wh ^= rev;
				color ^= 1;
			}
		}
		else
		{
			rev = GetRev[move](wh, bk);
			/* 白パス？ */
			if (rev == 0)
			{
				rev = GetRev[move](bk, wh);
				/* 一応合法手になっているかのチェック */
				if (rev == 0)
				{
					badFlag = true;
					break;
				}
				bk ^= ((1ULL << move) | rev);
				wh ^= rev;
			}
			else
			{
				bk ^= rev;
				wh ^= ((1ULL << move) | rev);
				color ^= 1;
			}
		}

		/* 対局データ生成 */
		if (turn == stage)
		{
			int board[64];
			init_index_board(board, bk, wh);

			write_h_ver1(&keyIndex[offset + 0], board);
			write_h_ver2(&keyIndex[offset + 4], board);
			write_h_ver3(&keyIndex[offset + 8], board);
			write_dia_ver1(&keyIndex[offset + 12], board);
			write_dia_ver2(&keyIndex[offset + 14], board);
			write_dia_ver3(&keyIndex[offset + 18], board);
			write_dia_ver4(&keyIndex[offset + 22], board);
			write_edge(&keyIndex[offset + 26], board);
			write_corner5_2(&keyIndex[offset + 30], board);
			write_corner3_3(&keyIndex[offset + 38], board);
			write_triangle(&keyIndex[offset + 42], board);
#if 0
			UINT32 bk_mob, wh_mob;
			CreateMoves(bk, wh, &bk_mob);
			CreateMoves(wh, bk, &wh_mob);
			//bk_mob = bk_mob - wh_mob + (MOBILITY_NUM / 2);
			SHORT mob = bk_mob;
			keyIndex[offset + 46] = (USHORT)mob;
			mobcnt[bk_mob]++;
#endif
			write_parity(&keyIndex[offset + 46], ~(bk | wh));
			offset += PATTERN_NUM;

		}

		byteCounter += 2;
		turn++;
	}

	if (turn <= stage || badFlag == true)
	{
		return -1;
	}
	else
	{
		*t = CountBit(bk) - CountBit(wh);
	}

	return offset;

}

int convertWtbToAscii(char* t, USHORT* keyIndex, int stage)
{
	char buf[6800], file_name[64];
	FILE *rfp;
	int read_len, error, count = 0, index_count = 0;
	int ret;

	for (int i = 1976;; i++){

		sprintf_s(file_name, "kifu\\WTH_%d.wtb", i);
		if ((error = fopen_s(&rfp, file_name, "rb")) != 0){
			break;
		}

		fread(buf, sizeof(char), 16, rfp); //ヘッダ読み捨て
		while ((read_len = fread(buf, sizeof(char), 6800, rfp)) != 0)
		{
			for (int i = 0; i < read_len / 68; i++)
			{
				// 白除去インデックスの棋譜はスルー
				//for (int cnt = 0; cnt < sizeof(rand_index_array) / sizeof(int); cnt++){
				//	if (index_count == rand_index_array[cnt]){
				//		//printf("skip kifu...\n");
				//		wh_win--;
				//		index_count++;
				//		continue;
				//	}
				//}
				// ステージごとのパターンを抽出
				ret = getFeatureIndex(&keyIndex[count* PATTERN_NUM], &buf[i * 68 + 7], stage, &t[count]);
				count++;

				index_count++;
			}

		}

		fclose(rfp);

	}
	return count;
}

int convertKifuToAscii(char* t, USHORT* keyIndex, int stage)
{
	char buf[512], file_name[64];
	FILE *rfp;
	int error, count = 0, index_count = game_num1;
	int offset, ret;

	if ((error = fopen_s(&rfp, "kifu\\kifu.dat", "rb")) != 0){
		return -1;
	}

	// example
	// F5D6C3D3C4F4C5B3C2B4 E3E6C6F6～(略)～H5H8H7 0
	while (fgets(buf, 512, rfp) != NULL)
	{
		ret = getFeatureIndex2(&t[count], &keyIndex[count * PATTERN_NUM], buf, stage);

		// 石差を取得
		if (t[count] < -64 || t[count] > 64){
			printf("assert!!! st1ULL diff over at %d\n", count);
			exit(1);
		}
		if (ret != -1)
		{
			count++;
			index_count++;
		}
	}

	fclose(rfp);

	return count;

#if 0
	for (int i = 1;; i++){

		sprintf_s(file_name, "kifu\\%02dE4.gam.%d.new", (i + 1) / 2, ((i - 1) % 2) + 1);
		if ((error = fopen_s(&rfp, file_name, "rb")) != 0){
			break;
		}

		// example
		// F5D6C3D3C4F4C5B3C2B4 E3E6C6F6～(略)～H5H8H7 0
		while (fgets(buf, 512, rfp) != NULL)
		{
			ret = getFeatureIndex2(&t[count], &keyIndex[count * PATTERN_NUM], buf, stage);

			// 石差を取得
			if (t[count] < -64 || t[count] > 64){
				printf("assert!!! st1ULL diff over at %d\n", count);
				exit(1);
			}
			if (ret != -1)
			{
				count++;
				index_count++;
			}
		}
		fclose(rfp);

	}
#endif

}

void writeTable(char *file_name, int stage)
{
	int error;
	FILE *fp;

	if ((error = fopen_s(&fp, file_name, "w")) != 0){
		return;
	}

	int i;
	for (i = 0; i < 6561; i++)
	{
		fprintf(fp, "%lf\n", hori_ver1_data[0][stage][i]);
	}
	for (i = 0; i < 6561; i++)
	{
		fprintf(fp, "%lf\n", hori_ver2_data[0][stage][i]);
	}
	for (i = 0; i < 6561; i++)
	{
		fprintf(fp, "%lf\n", hori_ver3_data[0][stage][i]);
	}
	for (i = 0; i < 6561; i++)
	{
		fprintf(fp, "%lf\n", dia_ver1_data[0][stage][i]);
	}
	for (i = 0; i < 2187; i++)
	{
		fprintf(fp, "%lf\n", dia_ver2_data[0][stage][i]);
	}
	for (i = 0; i < 729; i++)
	{
		fprintf(fp, "%lf\n", dia_ver3_data[0][stage][i]);
	}
	for (i = 0; i < 243; i++)
	{
		fprintf(fp, "%lf\n", dia_ver4_data[0][stage][i]);
	}
	for (i = 0; i < 59049; i++)
	{
		fprintf(fp, "%lf\n", edge_data[0][stage][i]);
	}
	for (i = 0; i < 59049; i++)
	{
		fprintf(fp, "%lf\n", corner5_2_data[0][stage][i]);
	}
	for (i = 0; i < 19683; i++)
	{
		fprintf(fp, "%lf\n", corner3_3_data[0][stage][i]);
	}
	for (i = 0; i < 59049; i++)
	{
		fprintf(fp, "%lf\n", triangle_data[0][stage][i]);
	}
#if 0
	for (i = 0; i < MOBILITY_NUM; i++)
	{
		fprintf(fp, "%lf\n", mobility_data[stage][i]);
	}
#endif
	for (i = 0; i < PARITY_NUM; i++)
	{
		fprintf(fp, "%lf\n", parity_data[0][stage][i]);
	}

	fclose(fp);
}

double normalize(double a, UINT64 fcnt, double d){

	if (fcnt == 0){
		return 0;
	}

	return a * min((double)0.01, 1 / (double)fcnt) * d;
}

double culcrk(float ** f, USHORT* index)
{
	int i;
	double et = 0.0;
	for (i = 0; i < PATTERN_NUM; i++){
		et += f[i][index[i]];
	}

	et += f[i][0] * (short)index[i];

	return et;
}

int table_addr[] = {
	0, 4, 8, 12, 14, 18, 22, 26, 30, 34, 38, 42
};
int table_num[] = {
	INDEX_NUM, INDEX_NUM, INDEX_NUM, INDEX_NUM,
	INDEX_NUM / 3, INDEX_NUM / 9, INDEX_NUM / 27, INDEX_NUM / 81,
	INDEX_NUM * 9, INDEX_NUM * 9, INDEX_NUM * 9, INDEX_NUM * 3
};

double culcSum(USHORT* keyIndex, int stage)
{
	double eval = 0;
	int counter = 0;

	eval += hori_ver1_data[0][stage][keyIndex[counter++]];
	eval += hori_ver1_data[0][stage][keyIndex[counter++]];
	eval += hori_ver1_data[0][stage][keyIndex[counter++]];
	eval += hori_ver1_data[0][stage][keyIndex[counter++]];

	eval += hori_ver2_data[0][stage][keyIndex[counter++]];
	eval += hori_ver2_data[0][stage][keyIndex[counter++]];
	eval += hori_ver2_data[0][stage][keyIndex[counter++]];
	eval += hori_ver2_data[0][stage][keyIndex[counter++]];

	eval += hori_ver3_data[0][stage][keyIndex[counter++]];
	eval += hori_ver3_data[0][stage][keyIndex[counter++]];
	eval += hori_ver3_data[0][stage][keyIndex[counter++]];
	eval += hori_ver3_data[0][stage][keyIndex[counter++]];

	eval += dia_ver1_data[0][stage][keyIndex[counter++]];
	eval += dia_ver1_data[0][stage][keyIndex[counter++]];

	eval += dia_ver2_data[0][stage][keyIndex[counter++]];
	eval += dia_ver2_data[0][stage][keyIndex[counter++]];
	eval += dia_ver2_data[0][stage][keyIndex[counter++]];
	eval += dia_ver2_data[0][stage][keyIndex[counter++]];

	eval += dia_ver3_data[0][stage][keyIndex[counter++]];
	eval += dia_ver3_data[0][stage][keyIndex[counter++]];
	eval += dia_ver3_data[0][stage][keyIndex[counter++]];
	eval += dia_ver3_data[0][stage][keyIndex[counter++]];

	eval += dia_ver4_data[0][stage][keyIndex[counter++]];
	eval += dia_ver4_data[0][stage][keyIndex[counter++]];
	eval += dia_ver4_data[0][stage][keyIndex[counter++]];
	eval += dia_ver4_data[0][stage][keyIndex[counter++]];

	eval += edge_data[0][stage][keyIndex[counter++]];
	eval += edge_data[0][stage][keyIndex[counter++]];
	eval += edge_data[0][stage][keyIndex[counter++]];
	eval += edge_data[0][stage][keyIndex[counter++]];

	eval += corner5_2_data[0][stage][keyIndex[counter++]];
	eval += corner5_2_data[0][stage][keyIndex[counter++]];
	eval += corner5_2_data[0][stage][keyIndex[counter++]];
	eval += corner5_2_data[0][stage][keyIndex[counter++]];
	eval += corner5_2_data[0][stage][keyIndex[counter++]];
	eval += corner5_2_data[0][stage][keyIndex[counter++]];
	eval += corner5_2_data[0][stage][keyIndex[counter++]];
	eval += corner5_2_data[0][stage][keyIndex[counter++]];

	eval += corner3_3_data[0][stage][keyIndex[counter++]];
	eval += corner3_3_data[0][stage][keyIndex[counter++]];
	eval += corner3_3_data[0][stage][keyIndex[counter++]];
	eval += corner3_3_data[0][stage][keyIndex[counter++]];

	eval += triangle_data[0][stage][keyIndex[counter++]];
	eval += triangle_data[0][stage][keyIndex[counter++]];
	eval += triangle_data[0][stage][keyIndex[counter++]];
	eval += triangle_data[0][stage][keyIndex[counter++]];

	eval += parity_data[0][stage][keyIndex[counter]];

	return eval;
}

void culc_d_data(USHORT* keyIndex, double error)
{
	double eval = 0;
	int counter = 0;

	hori_ver1_data_d[keyIndex[counter++]] += error;
	hori_ver1_data_d[keyIndex[counter++]] += error;
	hori_ver1_data_d[keyIndex[counter++]] += error;
	hori_ver1_data_d[keyIndex[counter++]] += error;

	hori_ver2_data_d[keyIndex[counter++]] += error;
	hori_ver2_data_d[keyIndex[counter++]] += error;
	hori_ver2_data_d[keyIndex[counter++]] += error;
	hori_ver2_data_d[keyIndex[counter++]] += error;

	hori_ver3_data_d[keyIndex[counter++]] += error;
	hori_ver3_data_d[keyIndex[counter++]] += error;
	hori_ver3_data_d[keyIndex[counter++]] += error;
	hori_ver3_data_d[keyIndex[counter++]] += error;

	dia_ver1_data_d[keyIndex[counter++]] += error;
	dia_ver1_data_d[keyIndex[counter++]] += error;

	dia_ver2_data_d[keyIndex[counter++]] += error;
	dia_ver2_data_d[keyIndex[counter++]] += error;
	dia_ver2_data_d[keyIndex[counter++]] += error;
	dia_ver2_data_d[keyIndex[counter++]] += error;

	dia_ver3_data_d[keyIndex[counter++]] += error;
	dia_ver3_data_d[keyIndex[counter++]] += error;
	dia_ver3_data_d[keyIndex[counter++]] += error;
	dia_ver3_data_d[keyIndex[counter++]] += error;

	dia_ver4_data_d[keyIndex[counter++]] += error;
	dia_ver4_data_d[keyIndex[counter++]] += error;
	dia_ver4_data_d[keyIndex[counter++]] += error;
	dia_ver4_data_d[keyIndex[counter++]] += error;

	edge_data_d[keyIndex[counter++]] += error;
	edge_data_d[keyIndex[counter++]] += error;
	edge_data_d[keyIndex[counter++]] += error;
	edge_data_d[keyIndex[counter++]] += error;

	corner5_2_data_d[keyIndex[counter++]] += error;
	corner5_2_data_d[keyIndex[counter++]] += error;
	corner5_2_data_d[keyIndex[counter++]] += error;
	corner5_2_data_d[keyIndex[counter++]] += error;
	corner5_2_data_d[keyIndex[counter++]] += error;
	corner5_2_data_d[keyIndex[counter++]] += error;
	corner5_2_data_d[keyIndex[counter++]] += error;
	corner5_2_data_d[keyIndex[counter++]] += error;

	corner3_3_data_d[keyIndex[counter++]] += error;
	corner3_3_data_d[keyIndex[counter++]] += error;
	corner3_3_data_d[keyIndex[counter++]] += error;
	corner3_3_data_d[keyIndex[counter++]] += error;

	triangle_data_d[keyIndex[counter++]] += error;
	triangle_data_d[keyIndex[counter++]] += error;
	triangle_data_d[keyIndex[counter++]] += error;
	triangle_data_d[keyIndex[counter++]] += error;

	//mobility_data_d[0] += error * (SHORT)keyIndex[counter++];
	parity_data_d[keyIndex[counter++]] += error;

}

void culc_weight(double scale, int stage)
{
	int i;
	for (i = 0; i < INDEX_NUM; i++)
	{
		hori_ver1_data[0][stage][i] += normalize(scale, hori1cnt[i], hori_ver1_data_d[i]);
	}
	for (i = 0; i < INDEX_NUM; i++)
	{
		hori_ver2_data[0][stage][i] += normalize(scale, hori2cnt[i], hori_ver2_data_d[i]);
	}
	for (i = 0; i < INDEX_NUM; i++)
	{
		hori_ver3_data[0][stage][i] += normalize(scale, hori3cnt[i], hori_ver3_data_d[i]);
	}

	for (i = 0; i < INDEX_NUM; i++)
	{
		dia_ver1_data[0][stage][i] += normalize(scale, diag1cnt[i], dia_ver1_data_d[i]);
	}
	for (i = 0; i < INDEX_NUM / 3; i++)
	{
		dia_ver2_data[0][stage][i] += normalize(scale, diag2cnt[i], dia_ver2_data_d[i]);
	}
	for (i = 0; i < INDEX_NUM / 9; i++)
	{
		dia_ver3_data[0][stage][i] += normalize(scale, diag3cnt[i], dia_ver3_data_d[i]);
	}
	for (i = 0; i < INDEX_NUM / 27; i++)
	{
		dia_ver4_data[0][stage][i] += normalize(scale, diag4cnt[i], dia_ver4_data_d[i]);
	}

	for (i = 0; i < INDEX_NUM * 9; i++)
	{
		edge_data[0][stage][i] += normalize(scale, edgecnt[i], edge_data_d[i]);
	}

	for (i = 0; i < INDEX_NUM * 9; i++)
	{
		corner5_2_data[0][stage][i] += normalize(scale, cor52cnt[i], corner5_2_data_d[i]);
	}

	for (i = 0; i < INDEX_NUM * 3; i++)
	{
		corner3_3_data[0][stage][i] += normalize(scale, cor33cnt[i], corner3_3_data_d[i]);
	}

	for (i = 0; i < INDEX_NUM * 9; i++)
	{
		triangle_data[0][stage][i] += normalize(scale, trianglecnt[i], triangle_data_d[i]);
	}
#if 0
	for (i = 0; i < MOBILITY_NUM; i++)
	{
		mobility_data[stage][i] += 0;
	}
#endif
	for (i = 0; i < PARITY_NUM; i++)
	{
		parity_data[0][stage][i] += normalize(scale, paritycnt[i], parity_data_d[i]);
	}

}

void Learning(char* t, USHORT* keyIndex, int sample_num, int stage)
{
	int i, k, l, maxloop = 1000;
	int counter;
	double ave_error[2] = { 1000, 65536 };
	double alpha = 4000, scalealpha = 0.006;
	double func, deltafunc, error, et;
	double errorsum;


	/* 評価パターンテーブル(おおもと) */
#if 0
	double hori_ver1_data[INDEX_NUM] = { 0 };
	double hori_ver2_data[INDEX_NUM] = { 0 };
	double hori_ver3_data[INDEX_NUM] = { 0 };
	double dia_ver1_data[INDEX_NUM] = { 0 };
	double dia_ver2_data[INDEX_NUM / 3] = { 0 };
	double dia_ver3_data[INDEX_NUM / 9] = { 0 };
	double dia_ver4_data[INDEX_NUM / 27] = { 0 };
	double edge_data[INDEX_NUM * 9] = { 0 };
	double corner5_2_data[INDEX_NUM * 9] = { 0 };
	double corner3_3_data[INDEX_NUM * 3] = { 0 };
	double triangle_data[INDEX_NUM * 9] = { 0 };
	double mobility_data[MOBILITY_NUM] = { 0 };
	double parity_data[PARITY_NUM] = { 0 };
#endif

	char table_file_name[64];
	sprintf_s(table_file_name, "%d.dat", stage);

	/* 最急降下法 */
	for (k = 0; k < maxloop && ave_error[1] - ave_error[0] >(ave_error[1] / (double)500); k++){

		// ベクトル初期化
		memset(hori_ver1_data_d, 0, sizeof(hori_ver1_data_d));
		memset(hori_ver2_data_d, 0, sizeof(hori_ver2_data_d));
		memset(hori_ver3_data_d, 0, sizeof(hori_ver3_data_d));
		memset(dia_ver1_data_d, 0, sizeof(dia_ver1_data_d));
		memset(dia_ver2_data_d, 0, sizeof(dia_ver2_data_d));
		memset(dia_ver3_data_d, 0, sizeof(dia_ver3_data_d));
		memset(dia_ver4_data_d, 0, sizeof(dia_ver4_data_d));
		memset(edge_data_d, 0, sizeof(edge_data_d));
		memset(corner5_2_data_d, 0, sizeof(corner5_2_data_d));
		memset(corner3_3_data_d, 0, sizeof(corner3_3_data_d));
		memset(triangle_data_d, 0, sizeof(triangle_data_d));
		//memset(mobility_data_d, 0, sizeof(mobility_data_d));
		memset(parity_data_d, 0, sizeof(parity_data_d));

		errorsum = 0;
		deltafunc = 0;
		counter = 0;

		/* 局面ごとに誤差を計算 */
		for (l = 0; l < sample_num; l++)
		{
			error = t[l] - culcSum(&keyIndex[l * PATTERN_NUM], stage);
			culc_d_data(&keyIndex[l * PATTERN_NUM], error);
			errorsum += (error * error);
		}

		// 傾きベクトルからそれぞれの特徴の係数を計算
		culc_weight(scalealpha, stage);

		/* 平均誤差の出力 */
		if (k % 10 == 0)
		{
			ave_error[1] = ave_error[0];
			ave_error[0] = errorsum / (double)(sample_num);
			printf("averrage error = %f\n", ave_error[0]);
			writeTable(table_file_name, stage);
		}
	}

}

int culcEval()
{
	/* 教師信号(石差) */
	char *t = (char *)malloc(MAX * sizeof(char));
	if (t == NULL){
		printf("tの領域を確保できませんでした\n");
		return 0;
	}
	/* 1局面ごとの特徴の番号 */
	USHORT *keyIndex = (USHORT *)malloc(sizeof(USHORT) * MAX * PATTERN_NUM);
	if (keyIndex == NULL){
		printf("keyIndexの領域を確保できませんでした\n");
		return 0;
	}

	int stage = 0;
	int sum_score = 0;
	int rand_flag = FALSE;
	char buf[16];

	while (stage < 60){

		printf("starting %d stage learning...\n", stage);
		// 評価パターンテーブル初期化
		memset(keyIndex, 0, sizeof(USHORT) * MAX * PATTERN_NUM);

		// 評価パターンカウントテーブル初期化
		initCountTable();

		// 棋譜数初期化
		memset(kifuNum, 0, sizeof(kifuNum));

		// wtbファイルの読み込みとAsciiへの変換
		//game_num1 = convertWtbToAscii(t, keyIndex, stage);
		//printf("finish game 1\n");
		game_num2 = convertKifuToAscii(t, keyIndex, stage);

		printf("game:%d\n", game_num2);

		// 評価テーブルの作成
		Learning(t, keyIndex, game_num2, stage);

		stage++;
		sum_score = 0;
	}

	free(t);
	free(keyIndex);

	return 0;

}

int getKifuLine2(char buf[][512])
{
	char whorData[6800 * 5], file_name[64];
	FILE *rfp;
	int read_len, error, count = 0;

	sprintf_s(file_name, "kifu\\WTH_%d.wtb", 1976);
	if ((error = fopen_s(&rfp, file_name, "rb")) != 0){
		return count;
	}

	fread(whorData, sizeof(char), 16, rfp); //ヘッダ読み捨て

	while (count < 500){

		read_len = fread(whorData, sizeof(char), 6800 * 5, rfp);

		for (int i = 0; i < read_len / 68; i++)
		{
			if (rand() % 256 == 0){
				// ステージごとのパターンを抽出
				memcpy_s(&buf[count], 512, &whorData[i * 68 + 7], 68);
				count++;
			}
		}
	}

	fclose(rfp);

	return count;
}
#if 0
void culcExp(int* scoreList, int* resultList)
{

	double ave_error[2] = { 65535, 65534 };
	int k;
	double errorsum, deltafunc, error, et;
	double d[500];
	double w[2];

	/* 最急降下法 */
	for (k = 0; k < 5000 && ave_error[1] - ave_error[0] >(ave_error[1] / (double)500); k++){

		memset(d, 0, sizeof(float) * 500);
		errorsum = 0;
		deltafunc = 0;
		error = 0;

		/* 局面ごとに誤差を計算 */
		for (int l = 0; l < 500; l++){

			et = 0;

			/* 線形誤差を算出 error = Σ(e - e'(w)) */
			et = culcFunc(w, scoreList);

			error = (resultList[l] - exp(-et)); // t[l] は l番目の教師信号(石差)
			errorsum += error * error;

			/* 勾配ベクトルの更新 */
			for (int i = 0; i < 2; i++){
				d[l * 2 + i] += error;
			}
		}

		/* 各得点を更新 */
		for (int l = 0; l < 500; l++){
			for (int i = 0; i < 2; i++) {

				/* 勾配ベクトルの微分係数 */
				/* パラメータ更新 */
				w[i] += normalize(0.05, d[l * PATTERN_NUM + i]);
			}
			//feature[i][0] += scalealpha * 1 / (float)featureCount[i][0] * d[l * PATTERN_NUM + i];
		}

		/* 平均誤差の出力 */
		if (k % 10 == 0)
		{
			//printf("%d/%d\n", k, maxloop);
			//fopen_s(&fp, trace_file_name, "a+");
			ave_error[1] = ave_error[0];
			ave_error[0] = errorsum / (double)(sample_num);
			//fprintf_s(fp, "平均２乗誤差= %f\n", ave_error[0]);
			printf("averrage error = %f\n", ave_error[0]);
			//fclose(fp);
			writeTable(table_file_name, feature);
		}
	}

	free(d);


}
#endif


void GenerateKifu()
{

}

int _tmain(int argc, _TCHAR* argv[])
{
	BOOL result;

	result = AlocMobilityFunc();
	culcEval();

	//CulclationMpcValue();

	//GenerateKifu( );

	return 0;
}

