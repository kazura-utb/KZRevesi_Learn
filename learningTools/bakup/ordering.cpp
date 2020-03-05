/***************************************************************************
* Name  : ordering.cpp
* Brief : ��̕��ёւ��֘A�̏������s��
* Date  : 2016/02/02
****************************************************************************/

#include "stdafx.h"
#include "rev.h"
#include "bit64.h"
#include "board.h"
#include "cpu.h"
#include "eval.h"
#include "hash.h"
#include "move.h"
#include "ordering.h"

void sort_move_list(char *move_list, int eval_list[], int cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int temp, swaps;
	char ctemp;

	if (cnt == 1){ return; }
	while (1)
	{
		swaps = 0;
		for (i = 0; i + h < cnt; i++)
		{
			if (eval_list[i] < eval_list[i + h])
			{
				ctemp = move_list[i];
				move_list[i] = move_list[i + h];
				move_list[i + h] = ctemp;
				temp = eval_list[i];
				eval_list[i] = eval_list[i + h];
				eval_list[i + h] = temp;
				swaps++;
			}
		}
		if (h == 1)
		{
			if (swaps == 0)
			{
				break;
			}
		}
		else
		{
			h = h * 10 / 13;
		}
	}
}

void sort_move_list(MOVELIST *move_list, int eval_list[], int cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int temp, swaps;
	MOVELIST ctemp;

	if (cnt == 1){ return; }
	while (1)
	{
		swaps = 0;
		for (i = 0; i + h < cnt; i++)
		{
			if (eval_list[i] < eval_list[i + h])
			{
				ctemp = move_list[i];
				move_list[i] = move_list[i + h];
				move_list[i + h] = ctemp;
				temp = eval_list[i];
				eval_list[i] = eval_list[i + h];
				eval_list[i + h] = temp;
				swaps++;
			}
		}
		if (h == 1)
		{
			if (swaps == 0)
			{
				break;
			}
		}
		else
		{
			h = h * 10 / 13;
		}
	}
}

/* �R���\�[�g */
void SortListUseTable(MOVELIST *pos_list, INT32 move_list[], INT32 cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int temp, swaps;
	MOVELIST move_temp;

	if (cnt <= 1){ return; }
	while (1)
	{
		swaps = 0;
		for (i = 0; i + h < cnt; i++)
		{
			if (move_list[i] > move_list[i + h])
			{
				temp = move_list[i];
				move_list[i] = move_list[i + h];
				move_list[i + h] = temp;
				move_temp = pos_list[i];
				pos_list[i] = pos_list[i + h];
				pos_list[i + h] = move_temp;
				swaps++;
			}
		}
		if (h == 1)
		{
			if (swaps == 0)
			{
				break;
			}
		}
		else
		{
			h = h * 10 / 13;
		}
	}
}

/* �R���\�[�g */
void SortListUseTable(INT8 *pos_list, INT32 move_list[], UINT64 rev_list[], INT32 cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int temp, swaps;
	char move_temp;
	UINT64 rev_temp;

	if (cnt <= 1){ return; }
	while (1)
	{
		swaps = 0;
		for (i = 0; i + h < cnt; i++)
		{
			if (move_list[i] > move_list[i + h])
			{
				temp = move_list[i];
				move_list[i] = move_list[i + h];
				move_list[i + h] = temp;

				move_temp = pos_list[i];
				pos_list[i] = pos_list[i + h];
				pos_list[i + h] = move_temp;

				rev_temp = rev_list[i];
				rev_list[i] = rev_list[i + h];
				rev_list[i + h] = rev_temp;

				swaps++;
			}
		}
		if (h == 1)
		{
			if (swaps == 0)
			{
				break;
			}
		}
		else
		{
			h = h * 10 / 13;
		}
	}
}

/* ���̈��萫���Z�o */
int get_corner_stability(UINT64 color){

	int n_stable = 0;

	if (color & a1){ n_stable += CountBit(color & (0x0000000000000103)); }	// a1, a2, b1
	if (color & a8){ n_stable += CountBit(color & (0x00000000000080c0)); }	// a7, a8, b8
	if (color & h1){ n_stable += CountBit(color & (0x0301000000000000)); }	// g1, h1, h2
	if (color & h8){ n_stable += CountBit(color & (0xc080000000000000)); }	// g8, h7, h8

	return n_stable;
}

/* �ÓI�����Â�(�r�b�g�񂩂�̎擾) */
UINT32 GetOrderPosition(UINT64 blank)
{
	/* 8 point*/
	UINT64 moves;

	if ((moves = blank & 0x8100000000000081) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 7 point */
	if ((moves = blank & 0x240000240000) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 6 point*/
	if ((moves = blank & 0x1800008181000018) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 5 point*/
	if ((moves = blank & 0x2400810000810024) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 4 point*/
	if ((moves = blank & 0x182424180000) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 3 point */
	if ((moves = blank & 0x18004242001800) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 2 point*/
	if ((moves = blank & 0x24420000422400) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 1 point*/
	if ((moves = blank & 0x4281000000008142) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}

	/* 0 point*/
	return CountBit((~blank) & (blank - 1));

}

/* ���Ւ��՗p move ordering */
UINT32 MoveOrderingMiddle(INT8 *pos_list, UINT64 b_board, UINT64 w_board,
	HashTable *hash, UINT64 moves, UINT64 rev_list[], INT32 depth, UINT32 empty,
	INT32 alpha, INT32 beta, UINT32 color)
{
	UINT32 cnt = 0;
	UINT32 pos = 0;
	UINT64 rev;
	UINT64 move_b, move_w, ligal_move_w;
	UINT32 move_cnt;
	INT32 score;
	INT32 key, lower, upper;
	INT32 score_list[35];

	if (depth < 4 && g_limitDepth >= 6){
		do{
			pos = CountBit((moves & (-(INT64)moves)) - 1);

			/* ���]�f�[�^�擾 */
			rev = GetRev[pos](b_board, w_board);
			move_b = b_board ^ ((1ULL << pos) | rev);
			move_w = w_board^rev;
			InitIndexBoard(move_w, move_b);

			score = OrderingAlphaBeta(move_w, move_b, 5 - depth, -NEGAMAX, -NEGAMIN,
				color ^ 1, 60 - empty, 0);
			score_list[cnt] = score;
			pos_list[cnt] = (char)pos;
			rev_list[cnt] = rev;
			cnt++;
			moves = moves & (moves - 1);
		} while (moves);
	}
	else if (depth == 4){
		do{
			pos = CountBit((moves & (-(INT64)moves)) - 1);

			/* ���]�f�[�^�擾 */
			rev = GetRev[pos](b_board, w_board);

			move_b = b_board ^ ((1ULL << pos) | rev);
			move_w = w_board^rev;
			InitIndexBoard(move_w, move_b);

			score = Evaluation(g_board, move_w, move_b, color ^ 1, 60 - empty);
			score_list[cnt] = score;
			pos_list[cnt] = (char)pos;
			rev_list[cnt] = rev;
			cnt++;
			moves = moves & (moves - 1);
		} while (moves);
	}
	else
	{
		do{
			score = 0;
			pos = CountBit((moves & (-(INT64)moves)) - 1);

			/* ���]�f�[�^�擾 */
			rev = GetRev[pos](b_board, w_board);

			move_b = b_board ^ ((1ULL << pos) | rev);
			move_w = w_board^rev;

			ligal_move_w = CreateMoves(move_w, move_b, &move_cnt);

			key = KEY_HASH_MACRO(move_w, move_b);
			if (hash->data[key].b_board == move_w && hash->data[key].w_board == move_b){
				if (hash->data[key].depth + 1 >= g_limitDepth - depth){
					lower = hash->data[key].lower;
					upper = hash->data[key].upper;
					if (upper == lower){
						score -= 65536;
					}
					else if (lower >= -alpha)
					{
						score -= 65536;
					}
					else if (upper <= -beta)
					{
						score -= 65536;
					}
					else{
						score -= 8192;
					}
				}
				else{
					score -= 4096;
				}
			}

			/* �ʒu���_ */
			score -= posEval[pos] << 2;
			score += CountBit(rev) << 4;
			/* �G�̒���\�����擾 */
			score += ((CountBit(ligal_move_w) + CountBit(ligal_move_w & 0x8100000000000081)) << 4) 
				- get_corner_stability(move_b);
			score += CountBit(GetPotentialMoves(move_w, move_b, ~(move_b | move_w))) << 3;
			score_list[cnt] = score;
			pos_list[cnt] = (char)pos;
			rev_list[cnt] = rev;
			cnt++;
			moves = moves & (moves - 1);

		} while (moves);
	}
	/* �G�̓��_�̏��Ȃ����Ƀ\�[�g */
	SortListUseTable(pos_list, score_list, rev_list, cnt);

	return cnt;
}

/*  �I�՗p move ordering */
UINT32 MoveOrderingEnd(INT8 *pos_list, UINT64 b_board, UINT64 w_board, 
	HashTable *hash, UINT64 moves, UINT64 rev_list[], UINT32 depth)
{
	UINT32 cnt = 0;
	int pos = 0;
	UINT64 rev, ligal_move_w;
	UINT64 move_b, move_w;
	UINT32 move_cnt;
	int score;
	int score_list[35];
	//int key;

	do{
		score = 0;
		pos = CountBit((moves & (-(INT64)moves)) - 1);
		/* ���]�f�[�^�擾 */
		rev = GetRev[pos](b_board, w_board);

		move_b = b_board ^ ((1ULL << pos) | rev);
		move_w = w_board^rev;

		ligal_move_w = CreateMoves(move_w, move_b, &move_cnt);

		/* �G�̒���\�����擾 */
		score += ((CountBit(ligal_move_w) + CountBit(ligal_move_w & 0x8100000000000081)) << 4);
		score -= get_corner_stability(move_b);
		//score += CountBit(get_potential_moves(move_w, move_b, ~(move_b | move_w)));

		score_list[cnt] = score;
		pos_list[cnt] = (char)pos;
		rev_list[cnt] = rev;
		cnt++;
		moves = moves & (moves - 1);
	} while (moves);

	/* �G�̓��_�̏��Ȃ����Ƀ\�[�g */
	SortListUseTable(pos_list, score_list, rev_list, cnt);

	return cnt;
}


/*  �I�՗p move ordering */
char MoveOrderingEnd(MOVELIST *pos_list, UINT64 b_board, UINT64 w_board, UINT64 moves)
{
	char cnt = 0;
	int pos = 0;
	UINT64 rev, ligal_move_w;
	UINT64 move_b, move_w;
	UINT32 move_cnt;
	int score = 0;
	int score_list[36];

	do{
		score;
		pos = CountBit((moves & (-(INT64)moves)) - 1);
		/* ���]�f�[�^�擾 */
		rev = GetRev[pos](b_board, w_board);

		move_b = b_board ^ ((1ULL << pos) | rev);
		move_w = w_board^rev;

		ligal_move_w = CreateMoves(move_w, move_b, &move_cnt);

		/* �G�ɋ��𖾂��n���Ȃ� */
		score = CountBit(ligal_move_w & 0x8100000000000081) << 1;
		/* �K�؂Ȑ��b�ł��̉\����̏ꍇ */
		if ((1ULL << pos) & 0x42C300000000C342){
			if (score == 0)
			{
				score--;
			}
		}
		/*if(0x8100000000000081 & (one << pos)){
		score -= 4;
		}*/

		/* �G�̒���\�����擾 */
		score += (move_cnt << 5);
		score += get_corner_stability(move_w) - get_corner_stability(move_b);

		score_list[cnt] = score;
		pos_list[cnt].move = (char)pos;
		pos_list[cnt].rev = rev;
		cnt++;
		moves = moves & (moves - 1);
	} while (moves);

	/* �G�̓��_�̏��Ȃ����Ƀ\�[�g */
	SortListUseTable(pos_list, score_list, cnt);

	return cnt;
}
