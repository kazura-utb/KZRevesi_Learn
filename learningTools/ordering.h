/***************************************************************************
* Name  : ordering.h
* Brief : ��̕��ёւ��֘A�̏������s��
* Date  : 2016/02/02
****************************************************************************/

#include "stdafx.h"

#pragma once

/* ���胊�X�g */

typedef struct {
	char move;
	UINT64 rev;
}MOVELIST;

UINT32 GetOrderPosition(UINT64 blank);
UINT32 MoveOrderingMiddle(INT8 *pos_list, UINT64 b_board, UINT64 w_board,
	HashTable *hash, UINT64 moves, UINT64 rev_list[], INT32 depth, UINT32 empty,
	INT32 alpha, INT32 beta, UINT32 color);
UINT32 MoveOrderingEnd(INT8 *pos_list, UINT64 b_board, UINT64 w_board,
	HashTable *hash, UINT64 moves, UINT64 rev_list[], UINT32 depth);