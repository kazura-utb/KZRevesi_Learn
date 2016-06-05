#pragma once

#ifndef REVERSI_H
#define REVERSI_H 1

/*
##########################
#
# #include
#
###########################
*/

#include "stdio.h"
#include "stdlib.h"
#include <Windows.h>
#include "time.h"
#include "math.h"

/*
##########################
#
# #define
#
###########################
*/

#define MAX 4200000
#define PATTERN_NUM 47

#define BLACK 0
#define WHITE 1

#define FIRST_BK 34628173824
#define FIRST_WH 68853694464

#define INDEX_NUM 6561
#define MOBILITY_NUM 1
#define PARITY_NUM 16

#define MASS_NUM 8

#define WIN 1
#define LOSS -1
#define DRAW 0
#define CONTINUE -2

#define BLACK 0
#define WHITE 1

#define LEN 64
#define FILE_LEN 256

/* 1石あたりの評価値 */
#define EVAL_ONE_STONE 10000

#define MPC_DEPTH_MIN 3
#define END_MPC_DEPTH_MIN 13

/*
##########################
#
# extern
#
###########################
*/

/* ビットボード */
extern UINT64 black;
extern UINT64 white;


/* 候補手リスト */

struct MOVELIST {
	char move;
	UINT64 rev;
};

/* インデックスボード */
extern int board[64];

/* 各座標の取得 */
extern UINT64 a1;			/* a1 */
extern UINT64 a2;			/* a2 */
extern UINT64 a3;			/* a3 */
extern UINT64 a4;			/* a4 */
extern UINT64 a5;			/* a5 */
extern UINT64 a6;			/* a6 */
extern UINT64 a7;			/* a7 */
extern UINT64 a8;			/* a8 */

extern UINT64 b1;			/* b1 */
extern UINT64 b2;			/* b2 */
extern UINT64 b3;			/* b3 */
extern UINT64 b4;			/* b4 */
extern UINT64 b5;			/* b5 */
extern UINT64 b6;			/* b6 */
extern UINT64 b7;			/* b7 */
extern UINT64 b8;			/* b8 */

extern UINT64 c1;			/* c1 */
extern UINT64 c2;			/* c2 */
extern UINT64 c3;			/* c3 */
extern UINT64 c4;			/* c4 */
extern UINT64 c5;			/* c5 */
extern UINT64 c6;			/* c6 */
extern UINT64 c7;			/* c7 */
extern UINT64 c8;			/* c8 */

extern UINT64 d1;			/* d1 */
extern UINT64 d2;			/* d2 */
extern UINT64 d3;			/* d3 */
extern UINT64 d4;			/* d4 */
extern UINT64 d5;			/* d5 */
extern UINT64 d6;			/* d6 */
extern UINT64 d7;			/* d7 */
extern UINT64 d8;			/* d8 */

extern UINT64 e1;			/* e1 */
extern UINT64 e2;			/* e2 */
extern UINT64 e3;			/* e3 */
extern UINT64 e4;			/* e4 */
extern UINT64 e5;			/* e5 */
extern UINT64 e6;			/* e6 */
extern UINT64 e7;			/* e7 */
extern UINT64 e8;			/* e8 */

extern UINT64 f1;			/* f1 */
extern UINT64 f2;			/* f2 */
extern UINT64 f3;			/* f3 */
extern UINT64 f4;			/* f4 */
extern UINT64 f5;			/* f5 */
extern UINT64 f6;			/* f6 */
extern UINT64 f7;			/* f7 */
extern UINT64 f8;			/* f8 */

extern UINT64 g1;			/* g1 */
extern UINT64 g2;			/* g2 */
extern UINT64 g3;			/* g3 */
extern UINT64 g4;			/* g4 */
extern UINT64 g5;			/* g5 */
extern UINT64 g6;			/* g6 */
extern UINT64 g7;			/* g7 */
extern UINT64 g8;			/* g8 */

extern UINT64 h1;			/* h1 */
extern UINT64 h2;			/* h2 */
extern UINT64 h3;			/* h3 */
extern UINT64 h4;			/* h4 */
extern UINT64 h5;			/* h5 */
extern UINT64 h6;			/* h6 */
extern UINT64 h7;			/* h7 */
extern UINT64 h8;			/* h8 */

/* 各パターンのテーブルを明示 */
extern UINT64 p_pattern[72];
extern UINT64 CStone;

//extern int m_list[64];
extern int eval_list[34];

/* 探索時間 */
extern double start_t, end_t;
/* 探索ノード数 */
extern UINT64 COUNT_NODE, COUNT_NODE2, BASE_NODE_NUM;
extern char CountNodeTime_str[64];

/* 前回の手 */
extern int move_x, move_y;

/*CPUの手*/
extern int CPU_MOVE;
extern int print_eval_flag;

/* 手を戻す時の参照用 */
extern UINT64 UndoBoard_B[120];
extern UINT64 UndoBoard_W[120];
extern int Undo_color[120];
extern int Undo_x[120], Undo_y[120];

/* 現在のターンの色 */
extern int NowTurn;

/* x * MASS_NUM + y から、座標に変換 */
extern char cordinates_table[64][3];

/* 評価値表示用 */
extern double key_hori_ver1[4];
extern double key_hori_ver2[4];
extern double key_hori_ver3[4];
extern double key_dia_ver1[2];
extern double key_dia_ver2[4];
extern double key_dia_ver3[4];
extern double key_dia_ver4[4];
extern double key_edge[4];
extern double key_edge_cor[4];
extern double key_corner4_2[4];
extern double key_corner3_3[4];
//extern double key_triangle[4];
extern double key_mobility;
extern double key_constant;
//extern double key_pot_mobility;
//extern double key_pality;
extern double eval_sum;
//extern double mobility;

extern int NowStage;

extern int MAX_MOVE;

/*
##########################
#
# func
#
###########################
*/

/* 初期化関連 */
void InitSystem(void);
void InitBoard(void);
void ClearListBoxItem(int);

/* 置換表関連 */
void init_substitution_table(struct Hash *hash);
void init_substitution_table_eval(struct Hash *hash);
void init_substitution_table_winloss(struct Hash *hash);
void init_substitution_table_exact(struct Hash *hash);

void auto_game(void *);
void cpy_stack(int[], int[], int);
void print_line(int[]);

void read_eval_table(int);

UINT64 CreateMoves(UINT64, UINT64, int *);

int CountBit2(int[], int);
int CountBit(UINT64);
int CountPotMob(UINT64, UINT64);

void configCPU(int);

#endif