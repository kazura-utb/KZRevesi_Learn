#include "stdafx.h"
#include "bit64.h"
#include "board.h"
#include "move.h"
#include "rev.h"
#include "cpu.h"
#include "endgame.h"
#include "empty.h"
#include "hash.h"
#include "eval.h"
#include "ordering.h"

#include "count_last_flip_carry_64.h"

#include <stdio.h>
#include <omp.h>

/**
* @brief Stability Cutoff (TC).
*
* @param search Current position.
* @param alpha Alpha bound.
* @param score Score to return in case of a cutoff is found.
* @return 'true' if a cutoff is found, false otherwise.
*/
BOOL search_SC_NWS(UINT64 bk, UINT64 wh, INT32 empty, INT32 alpha, INT32 *score)
{
	if (alpha >= NWS_STABILITY_THRESHOLD[empty]) {
		*score = 64 - 2 * get_stability(wh, bk);
		if (*score <= alpha) {
			return TRUE;
		}
	}
	return FALSE;
}



BOOL TableCutOff(HashInfo *hashInfo, UINT64 bk, UINT64 wh, UINT32 color, INT32 empty,
	INT32 *alpha, INT32 *beta, INT32 *score, INT32 *bestmove, INT32 *selectivity
)
{
	BOOL      ret;

	ret = FALSE;
	if (hashInfo != NULL)
	{
		if (hashInfo->depth >= empty && hashInfo->selectivity >= g_mpc_level)
		{
			if (hashInfo->upper <= *alpha)
			{
				// transposition table cutoff
				*selectivity = hashInfo->selectivity;
				*score = hashInfo->upper;
				ret = TRUE;
			}
			else if (hashInfo->lower >= *beta)
			{
				// transposition table cutoff
				*selectivity = hashInfo->selectivity;
				*score = hashInfo->lower;
				ret = TRUE;
			}
			else if (hashInfo->lower == hashInfo->upper)
			{
				// transposition table cutoff
				*selectivity = hashInfo->selectivity;
				*score = hashInfo->lower;
				ret = TRUE;
			}
			else
			{	// change window width
				*alpha = max(*alpha, hashInfo->lower);
				*beta = min(*beta, hashInfo->upper);
			}
		}

		*bestmove = hashInfo->bestmove;
	}

	return ret;
}



BOOL CheckTableCutOff(
	HashTable *hash, UINT32 *key, UINT64 bk, UINT64 wh, UINT32 color, INT32 empty,
	INT32 alpha, INT32 beta, INT32 *score
)
{
	BOOL      ret;
	HashInfo *hashInfo;

	ret = FALSE;
	*key = KEY_HASH_MACRO(bk, wh, color);
	hashInfo = HashGet(hash, *key, bk, wh);
	if (hashInfo != NULL)
	{
		if (hashInfo->depth >= empty && hashInfo->selectivity >= g_mpc_level)
		{
			if (hashInfo->upper <= alpha)
			{
				// transposition table cutoff
				*score = hashInfo->upper;
				ret = TRUE;
			}
			else if (hashInfo->lower >= beta)
			{
				// transposition table cutoff
				*score = hashInfo->lower;
				ret = TRUE;
			}
			else if (hashInfo->lower == hashInfo->upper)
			{
				// transposition table cutoff
				*score = hashInfo->lower;
				ret = TRUE;
			}
		}
	}

	return ret;
}



BOOL CheckTableCutOff_PV(
	HashTable *hash, UINT32 *key, UINT64 bk, UINT64 wh, UINT32 color, INT32 empty,
	INT32 alpha, INT32 beta, INT32 *score
)
{
	BOOL      ret;
	HashInfo *hashInfo;

	ret = FALSE;
	*key = KEY_HASH_MACRO_PV(bk, wh, color);
	hashInfo = HashGet(hash, *key, bk, wh);
	if (hashInfo != NULL)
	{
		if (hashInfo->depth >= empty && hashInfo->selectivity >= g_mpc_level)
		{
			if (hashInfo->upper <= alpha)
			{
				// transposition table cutoff
				*score = hashInfo->upper;
				ret = TRUE;
			}
			else if (hashInfo->lower >= beta)
			{
				// transposition table cutoff
				*score = hashInfo->lower;
				ret = TRUE;
			}
			else if (hashInfo->lower == hashInfo->upper)
			{
				// transposition table cutoff
				*score = hashInfo->lower;
				ret = TRUE;
			}
		}
	}

	return ret;
}



/***************************************************************************
* Name  : PVS_SearchDeepExact
* Brief : PV Search ���s���A�]���l����ɍőP����擾
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �ǂސ[��
*         empty     : �󂫃}�X��
*         alpha     : ���̃m�[�h�ɂ����鉺���l
*         beta      : ���̃m�[�h�ɂ��������l
*         color     : CPU�̐F
*         hash      : �u���\�̐擪�|�C���^
*         pass_cnt  : ���܂ł̃p�X�̐�(�Q�J�E���g�ŏI���Ƃ݂Ȃ�)
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 NWS_SearchDeepExact(UINT64 bk, UINT64 wh, INT32 empty, UINT32 parity, UINT32 color, HashTable *hash,
	INT32 alpha, UINT32 passed, INT32 *p_selectivity)
{
	int score, bestscore, lower, upper, bestmove;
	UINT32 key;
	MoveList movelist[36], *iter;
	Move *move;
	const INT32 beta = alpha + 1;
	HashInfo *hashInfo;

	/* �A�{�[�g���� */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	bestmove = NOMOVE;
	lower = alpha;
	upper = beta;

	if (g_empty > 12 && empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
	{
		return AB_SearchExact(bk, wh, ~(bk | wh), empty,
			color, alpha, beta, passed, parity, p_selectivity);
	}

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &score))
	{
		// up to max threshold
		*p_selectivity = g_mpc_level;
		
		return score;
	}

	/************************************************************
	*
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/

	key = KEY_HASH_MACRO(bk, wh, color);
	/* transposition cutoff ? */
	if (g_tableFlag)
	{
		hashInfo = HashGet(hash, key, bk, wh);
		if (TableCutOff(hashInfo, bk, wh, color, empty, &lower, &upper, &score, &bestmove, p_selectivity))
		{
			
			return score;
		}
	}

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) �t�F�[�Y
	*
	*************************************************************/
#if 1
	if (g_mpcFlag && g_mpc_level < g_max_cut_table_size &&
		empty >= MPC_END_MIN_DEPTH && empty <= MPC_END_MAX_DEPTH && empty <= g_empty - 1)
	{
		MPCINFO *mpcInfo_p = &mpcInfo_end[empty - MPC_END_MIN_DEPTH];
		INT32 value = (INT32)((alpha * EVAL_ONE_STONE) - (mpcInfo_p->deviation * MPC_END_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		INT32 eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed);

		if (eval < value)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, alpha, empty, NOMOVE, g_mpc_level, g_infscore);
			// store cutoff level
			*p_selectivity = g_mpc_level;
			
			return alpha;
		}

		value = (INT32)((beta * EVAL_ONE_STONE) + (mpcInfo_p->deviation * MPC_END_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed);

		if (eval > value)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, beta, empty, NOMOVE, g_mpc_level, g_infscore);
			// store cutoff level
			*p_selectivity = g_mpc_level;
			
			return beta;
		}
	}
#endif


	/************************************************************
	*
	* Principal Variation Search(PVS) �t�F�[�Y
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;
	INT32 selectivity = g_mpc_level; // init now thresould

	if (moveCount == 0)
	{
		if (passed) {
			// game end...
			bestscore = GetEndScore[g_solveMethod](bk, wh, empty);
			bestmove = NOMOVE;
			
			*p_selectivity = g_mpc_level;
		}
		else
		{
			bestscore = -NWS_SearchDeepExact(wh, bk, empty, parity, color ^ 1, hash, -upper, 1, p_selectivity);
			bestmove = NOMOVE;
		}
	}
	else {

		// �����flip-bit�����߂�move�\���̂ɕۑ�
		StoreMovelist(movelist, bk, wh, moves);

		BOOL pv_flag = TRUE;
		if (moveCount > 1)
		{
			if (g_empty - empty <= 14)
			{
				SortMoveListMiddle(movelist, bk, wh, hash, NULL, empty, alpha, beta, color);
			}
			else
			{
				// ��̕��בւ�
				SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
		}

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		bestscore = -g_infscore;
		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			selectivity = g_mpc_level;
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;
			// PV�\���L���p
			g_pvline[g_empty - empty] = move->pos;

			score = -NWS_SearchDeepExact(move_w, move_b, empty - 1, parity ^ board_parity_bit[move->pos],
				color ^ 1, hash, -lower - 1, 0, &selectivity);

			if (score >= upper)
			{
				bestscore = score;
				bestmove = move->pos;
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (score > lower)
				{
					lower = score;
				}
			}

		}
	}

	if (g_empty - empty == 0)
	{
		g_move = bestmove;
	}
	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, g_mpc_level, g_infscore);
	*p_selectivity = selectivity;

	return bestscore;
}



/***************************************************************************
* Name  : PVS_SearchDeepExact
* Brief : PV Search ���s���A�]���l����ɍőP����擾
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �ǂސ[��
*         empty     : �󂫃}�X��
*         alpha     : ���̃m�[�h�ɂ����鉺���l
*         beta      : ���̃m�[�h�ɂ��������l
*         color     : CPU�̐F
*         hash      : �u���\�̐擪�|�C���^
*         pass_cnt  : ���܂ł̃p�X�̐�(�Q�J�E���g�ŏI���Ƃ݂Ȃ�)
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 PVS_SearchDeepExact(UINT64 bk, UINT64 wh, INT32 empty, INT32 parity, UINT32 color, HashTable *hash, HashTable *pvHash,
	INT32 alpha, INT32 beta, UINT32 passed, INT32 *p_selectivity)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	INT32 score;
	INT32 bestscore;
	INT32 lower;
	INT32 upper;
	INT32 bestmove;
	UINT32 key;
	UINT32 pv_key;
	MoveList movelist[36], *iter;
	Move *move;
	HashInfo *hashInfo;

	bestmove = NOMOVE;
	lower = alpha;
	upper = beta;

	if (g_empty > 12 && empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
	{
		return AB_SearchExact(bk, wh, ~(bk | wh), empty,
			color, alpha, beta, passed, parity, p_selectivity);
	}


	/************************************************************
	*
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/

	key = KEY_HASH_MACRO(bk, wh, color);
	pv_key = KEY_HASH_MACRO_PV(bk, wh, color);
	/* transposition cutoff ? */
	if (g_tableFlag)
	{
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo && TableCutOff(hashInfo, bk, wh, color, empty, &lower, &upper, &score, &bestmove, p_selectivity))
		{
			
			return score;
		}

		if (hashInfo == NULL)
		{
			hashInfo = HashGet(pvHash, pv_key, bk, wh);
			if (hashInfo && TableCutOff(hashInfo, bk, wh, color, empty, &lower, &upper, &score, &bestmove, p_selectivity))
			{
				
				return score;
			}
		}
	}

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) �t�F�[�Y
	*
	*************************************************************/
#if 1
	if (g_mpcFlag && g_mpc_level < g_max_cut_table_size &&
		empty >= MPC_END_MIN_DEPTH && empty <= MPC_END_MAX_DEPTH)
	{
		MPCINFO *mpcInfo_p = &mpcInfo_end[empty - MPC_END_MIN_DEPTH];
		INT32 value = (INT32)((alpha * EVAL_ONE_STONE) - (mpcInfo_p->deviation * MPC_END_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		INT32 eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed);

		if (eval < value)
		{
			//HashUpdate(hash, key, bk, wh, alpha, beta, alpha, empty, NOMOVE, g_mpc_level, g_infscore);
			// store cutoff level
			*p_selectivity = g_mpc_level;
			
			return alpha;
		}

		value = (INT32)((beta * EVAL_ONE_STONE) + (mpcInfo_p->deviation * MPC_END_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed);

		if (eval > value)
		{
			//HashUpdate(hash, key, bk, wh, alpha, beta, beta, empty, NOMOVE, g_mpc_level, g_infscore);
			// store cutoff level
			*p_selectivity = g_mpc_level;
			
			return beta;
		}
	}
#endif


	/************************************************************
	*
	* Principal Variation Search(PVS) �t�F�[�Y
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;
	INT32 selectivity = g_mpc_level; // init now thresould
	BOOL pv_flag = TRUE;

	// �����flip-bit�����߂�move�\���̂ɕۑ�
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL)
	{
		if (passed) {
			// game end...
			bestscore = GetEndScore[g_solveMethod](bk, wh, empty);
			bestmove = NOMOVE;
			
			*p_selectivity = g_mpc_level;
		}
		else
		{
			bestscore = -PVS_SearchDeepExact(wh, bk, empty, parity, color ^ 1, hash, pvHash, -upper, -lower, 1, p_selectivity);
			bestmove = NOMOVE;
		}
	}
	else {

		if (moveCount > 1)
		{
			if (g_empty - empty <= 14)
			{
				SortMoveListMiddle(movelist, bk, wh, hash, NULL, empty, alpha, beta, color);
			}
			else
			{
				// ��̕��בւ�
				SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
		}

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		bestscore = -g_infscore;
		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			selectivity = g_mpc_level;
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;
			// PV�\���L���p
			g_pvline[g_empty - empty] = move->pos;

			if (pv_flag)
			{
				score = -PVS_SearchDeepExact(move_w, move_b, empty - 1, parity ^ board_parity_bit[move->pos],
					color ^ 1, hash, pvHash, -upper, -lower, 0, &selectivity);
				pv_flag = FALSE;
			}
			else
			{
				score = -NWS_SearchDeepExact(move_w, move_b, empty - 1, parity ^ board_parity_bit[move->pos],
					color ^ 1, hash, -lower - 1, 0, &selectivity);

				if (lower < score && score < upper)
				{
					selectivity = g_mpc_level;
					lower = score;
					score = -PVS_SearchDeepExact(move_w, move_b, empty - 1, parity ^ board_parity_bit[move->pos],
						color ^ 1, hash, pvHash, -upper, -lower, 0, &selectivity);
				}
			}

			if (score >= upper)
			{
				bestscore = score;
				bestmove = move->pos;
				break;
			}

			if (score > bestscore)
			{
				bestscore = score;
				bestmove = move->pos;
				if (score > lower)
				{
					lower = score;
				}
			}

		}
	}

	if (g_empty - empty == 0)
	{
		g_move = bestmove;
	}
	/* �u���\�ɓo�^ */
	if (bestscore > alpha && bestscore < beta) HashUpdate(pvHash, pv_key, bk, wh, alpha, beta, bestscore, empty, bestmove, g_mpc_level, g_infscore);
	else HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, g_mpc_level, g_infscore);
	*p_selectivity = selectivity;

	return bestscore;
}


#if 0
/***************************************************************************
* Name  : PVS_SearchDeepExact_YBWC
* Brief : PV Search ���s���A�]���l����ɍőP����擾
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �ǂސ[��
*         empty     : �󂫃}�X��
*         alpha     : ���̃m�[�h�ɂ����鉺���l
*         beta      : ���̃m�[�h�ɂ��������l
*         color     : CPU�̐F
*         hash      : �u���\�̐擪�|�C���^
*         pass_cnt  : ���܂ł̃p�X�̐�(�Q�J�E���g�ŏI���Ƃ݂Ȃ�)
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 PVS_SearchDeepExact_YBWC(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	int score, upper, bestmove;
	int max_thread_num = omp_get_max_threads();
	int *lower = (int *)malloc(sizeof(int) * max_thread_num);

	for (int i = 0; i < max_thread_num; i++) lower[i] = alpha;

	MoveList movelist[34], movelist2[34], *iter;
	Move *move[4];

	bestmove = NOMOVE;
	upper = beta;

	/************************************************************
	*
	* Principal Variation Search(PVS) �t�F�[�Y
	*
	*************************************************************/

	UINT32 move_cnt1, move_cnt2;
	UINT64 moves = CreateMoves(bk, wh, &move_cnt1);
	UINT64 move_b[4], move_w[4];

	// �����flip-bit�����߂�move�\���̂ɕۑ�
	StoreMovelist(movelist, bk, wh, moves);

	BOOL pv_flag = TRUE;
	if (move_cnt1 > 1)
	{
		if (empty > 15)
		{
			SortMoveListMiddle(movelist, bk, wh, hash, empty, alpha, beta, color);
		}
		else if (empty > 6)
		{
			// ��̕��בւ�
			SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
		}
	}

	PVLINE line[4];
	int max_lower = -INF_SCORE;

	// ���ڂ��őP�Œ���
	iter = movelist->next;
	move[0] = &(iter->move);
	move_b[0] = bk ^ ((1ULL << move[0]->pos) | move[0]->rev);
	move_w[0] = wh ^ move[0]->rev;

	score = -PVS_SearchDeepExact(move_w[0], move_b[0],
		empty - 1, color ^ 1, hash, -upper, -lower[0], 0, &line[0]);

	// PV�̌��ʂőS�X���b�h�̃����X�V
	for (int n = 0; n < max_thread_num; n++) {
		lower[n] = score;
	}

	// 1��ڂ�YB
#pragma omp parallel for private(score, iter) schedule(dynamic, 1)
	for (int i0 = 1; i0 < (int)move_cnt1; i0++)
	{
		int tid = -1;
		iter = movelist->next;
		// iter��i0�̒l�ɂ���Ċ��߂�(���[�v���񎞂̂�)
		for (int j = 1; tid == -1 && j < i0; j++) iter = iter->next;

		tid = omp_get_thread_num();

		move[tid] = &(iter->move);
		move_b[tid] = bk ^ ((1ULL << move[tid]->pos) | move[tid]->rev);
		move_w[tid] = wh ^ move[tid]->rev;

		score = -PVS_SearchDeepExact(move_w[tid], move_b[tid],
			empty - 1, color ^ 1, hash, -lower[tid] - 1, -lower[tid], 0, &line[tid]);

		if (lower[tid] < score && score < upper)
		{
			score = -PVS_SearchDeepExact(move_w[tid], move_b[tid],
				empty - 1, color ^ 1, hash, -lower[tid], upper, 0, &line[tid]);
		}

		if (score > lower[tid]) lower[tid] = score;
		iter = iter->next;
	}

	max_lower = -INF_SCORE;
	for (int n = 0; n < max_thread_num; n++)
	{
		if (max_lower < -lower[n]) max_lower = -lower[n];
	}

	free(lower);

	return -max_lower;
}
#endif


INT32 AB_SearchExact(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty, UINT32 color, INT32 alpha, INT32 beta,
	UINT32 passed, INT32 quad_parity, INT32 *p_selectivity)
{
	INT32 max;                    //���݂̍ō��]���l
	INT32 eval;                   //�]���l�̕ۑ�
	INT32 pos;
	UINT64 pos_bit, rev;
	UINT64 moves;

	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	// parity moving
	if (empty == 4)
	{
		*p_selectivity = g_mpc_level;
		return SearchEmpty_4(bk, wh, blank, empty, quad_parity, alpha, beta, 0);
	}

	g_countNode++;

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &max))
	{
		*p_selectivity = g_mpc_level;
		
		return max;
	}

	max = -g_infscore;

	//MoveList movelist[EMPTIES_DEEP_TO_SHALLOW_SEARCH + 1], *iter = movelist;
	//UINT64 moves = CreateMoves(bk, wh, &move_cnt);
	//INT32 selectivity; // init now thresould


	// �����flip-bit�����߂�move�\���̂ɕۑ�
	//StoreMovelist(movelist, bk, wh, moves);
	//if (move_cnt > 1)
	//{
	//	if (empty >= 8) SortFastfirst(movelist, bk, wh);
	//	else SortPotentionalFastfirst(movelist, bk, wh, blank);
	//}

#if 0
	UINT64 moves = blank;
	while (moves)
	{
		pos = CountBit((~moves) & (moves - 1));
		pos_bit = 1ULL << pos;
		if ((rev = GetRev[pos](bk, wh)) != 0)
		{
			eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
				empty - 1, color ^ 1, -beta, -alpha, 0, quad_parity, &selectivity);

			/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
			if (eval > max)
			{
				max = eval;
				if (beta <= max)
				{
					break;
				}
				if (max > alpha)
				{
					alpha = max;
					pline->argmove[0] = pos;
					memcpy(pline->argmove + 1, line.argmove, line.cmove);
					pline->cmove = line.cmove + 1;
				}
			}
		}
		moves ^= pos_bit;
	}
#else
	// first move odd parity
	moves = blank;
	while (moves)
	{
		pos = CountBit((~moves) & (moves - 1));
		pos_bit = 1ULL << pos;
		if (quad_parity & board_parity_bit[pos])
		{
			if ((rev = GetRev[pos](bk, wh)) != 0)
			{
				eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
					empty - 1, color ^ 1, -beta, -alpha, 0, quad_parity ^ board_parity_bit[pos], p_selectivity);

				if (beta <= eval)
				{
					return eval;
				}
				/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
				if (eval > max)
				{
					max = eval;
					if (max > alpha)
					{
						alpha = max;
					}
				}
			}
		}
		moves ^= pos_bit;
	}

	// after, even parity
	moves = blank;
	while (moves)
	{
		pos = CountBit((~moves) & (moves - 1));
		pos_bit = 1ULL << pos;
		if ((quad_parity & board_parity_bit[pos]) == 0)
		{
			if ((rev = GetRev[pos](bk, wh)) != 0)
			{
				eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
					empty - 1, color ^ 1, -beta, -alpha, 0, quad_parity ^ board_parity_bit[pos], p_selectivity);

				if (beta <= eval)
				{
					return eval;
				}
				/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
				if (eval > max)
				{
					max = eval;
					if (max > alpha)
					{
						alpha = max;
					}
				}
			}
		}
		moves ^= pos_bit;
	}

#endif

	if (max == -g_infscore)
	{
		// �łĂȂ�����
		if (passed)
		{
			max = GetEndScore[g_solveMethod](bk, wh, empty);
			
			*p_selectivity = g_mpc_level;
		}
		else
		{
			max = -AB_SearchExact(wh, bk, blank, empty,
				color ^ 1, -beta, -alpha, 1, quad_parity, p_selectivity);

		}
	}

	//*p_selectivity = selectivity; // store level

	return max;

}



/***************************************************************************
* Name  : PVS_SearchDeepWinLoss
* Brief : PV Search ���s���A�]���l����ɍőP����擾
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �ǂސ[��
*         empty     : �󂫃}�X��
*         alpha     : ���̃m�[�h�ɂ����鉺���l
*         beta      : ���̃m�[�h�ɂ��������l
*         color     : CPU�̐F
*         hash      : �u���\�̐擪�|�C���^
*         pass_cnt  : ���܂ł̃p�X�̐�(�Q�J�E���g�ŏI���Ƃ݂Ȃ�)
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 PVS_SearchDeepWinLoss(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, INT32* p_selectivity)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	int score, bestscore, lower, upper, bestmove;
	UINT32 key;
	MoveList movelist[48], *iter;
	Move *move;
	HashInfo *hashInfo;

	bestmove = NOMOVE;
	lower = alpha;
	upper = beta;

	if (g_empty > 12 && empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
	{
		UINT64 blank = ~(bk | wh);
		//UINT32 quad_parity[4];
		//create_quad_parity(quad_parity, blank);
		return AB_SearchWinLoss(bk, wh, blank, empty,
			color, alpha, beta, passed, p_selectivity);
	}

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &score))
	{
		// up to max threshold
		*p_selectivity = g_mpc_level;

		return score;
	}

	/************************************************************
	*
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/
	/* �L�[�𐶐� */
	key = KEY_HASH_MACRO(bk, wh, color);
	/* transposition cutoff ? */
	if (g_tableFlag)
	{
		score = -g_infscore;
		// �r�������J�n
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->depth >= empty && hashInfo->selectivity >= g_mpc_level)
			{
				int hash_upper;
				hash_upper = hashInfo->upper;
				if (hash_upper <= lower)
				{
					// transposition table cutoff
					*p_selectivity = hashInfo->selectivity;
					
					return hash_upper;
				}
				int hash_lower;
				hash_lower = hashInfo->lower;
				if (hash_lower >= upper)
				{
					// transposition table cutoff
					*p_selectivity = hashInfo->selectivity;
					
					return hash_lower;
				}
				if (hash_lower == hash_upper)
				{
					// transposition table cutoff
					*p_selectivity = hashInfo->selectivity;
					
					return hash_lower;
				}

				// change window width
				lower = max(lower, hash_lower);
				upper = min(upper, hash_upper);
			}

			bestmove = hashInfo->bestmove;
		}
	}

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) �t�F�[�Y
	*
	*************************************************************/
#if 1
	if (g_mpcFlag && g_mpc_level < g_max_cut_table_size &&
		empty >= MPC_END_MIN_DEPTH && empty <= MPC_END_MAX_DEPTH && empty <= g_empty - 1)
	{
		MPCINFO *mpcInfo_p = &mpcInfo_end[empty - MPC_END_MIN_DEPTH];
		INT32 value = (INT32)((alpha * EVAL_ONE_STONE) - (mpcInfo_p->deviation * MPC_END_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		INT32 eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed);

		if (eval < value)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, alpha, empty, NOMOVE, g_mpc_level, WIN + 1);
			// store cutoff level
			*p_selectivity = g_mpc_level;
			
			return alpha;
		}

		value = (INT32)((beta * EVAL_ONE_STONE) + (mpcInfo_p->deviation * MPC_END_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed);

		if (eval > value)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, beta, empty, NOMOVE, g_mpc_level, WIN + 1);
			// store cutoff level
			*p_selectivity = g_mpc_level;
			
			return beta;
		}
	}
#endif


	/************************************************************
	*
	* Principal Variation Search(PVS) �t�F�[�Y
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;
	INT32 selectivity = g_mpc_level; // init now thresould

	// �����flip-bit�����߂�move�\���̂ɕۑ�
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL)
	{
		if (passed) {
			// game end...
			bestscore = GetEndScore[g_solveMethod](bk, wh, empty);
			bestmove = NOMOVE;
			
			*p_selectivity = g_mpc_level;
		}
		else
		{
			bestscore = -PVS_SearchDeepWinLoss(wh, bk, empty, color ^ 1, hash, -upper, -lower, 1, p_selectivity);
			bestmove = NOMOVE;
		}
	}
	else {

		BOOL pv_flag = TRUE;
		if (moveCount > 1)
		{
			if (g_empty - empty <= 14)
			{
				SortMoveListMiddle(movelist, bk, wh, hash, NULL, empty, alpha, beta, color);
			}
			else
			{
				// ��̕��בւ�
				SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
		}

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		bestscore = -g_infscore;
		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			selectivity = g_mpc_level;
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;
			// PV�\���L���p
			g_pvline[g_empty - empty] = move->pos;

			if (pv_flag)
			{
				score = -PVS_SearchDeepWinLoss(move_w, move_b,
					empty - 1, color ^ 1, hash, -upper, -lower, 0, &selectivity);
			}
			else
			{
				score = -PVS_SearchDeepWinLoss(move_w, move_b,
					empty - 1, color ^ 1, hash, -lower - 1, -lower, 0, &selectivity);

				if (lower < score && score < upper)
				{
					selectivity = g_mpc_level;
					lower = score;
					score = -PVS_SearchDeepWinLoss(move_w, move_b,
						empty - 1, color ^ 1, hash, -upper, -lower, 0, &selectivity);
				}
			}

			if (score >= upper)
			{
				bestscore = score;
				bestmove = move->pos;
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				pv_flag = FALSE;
				if (score > lower)
				{
					lower = score;
				}
			}

		}
	}

	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, g_mpc_level, WIN + 1);
	*p_selectivity = selectivity;

	if (g_empty == empty)
	{
		g_move = bestmove;
	}

	return bestscore;
}

/***************************************************************************
* Name  : AB_SearchWinLoss
* Brief : PV Search ���s���A�]���l����ɍőP����擾
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �ǂސ[��
*         empty     : �󂫃}�X��
*         alpha     : ���̃m�[�h�ɂ����鉺���l
*         beta      : ���̃m�[�h�ɂ��������l
*         color     : CPU�̐F
*         hash      : �u���\�̐擪�|�C���^
*         pass_cnt  : ���܂ł̃p�X�̐�(�Q�J�E���g�ŏI���Ƃ݂Ȃ�)
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 AB_SearchWinLoss(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 color, INT32 alpha, INT32 beta, UINT32 passed, INT32* p_selectivity)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	INT32 max;                    //���݂̍ō��]���l
	INT32 eval;                   //�]���l�̕ۑ�
	INT32 pos;
	UINT64 pos_bit, rev;

	// parity moving
	if (empty == 5)
	{
		UINT32 parity =
			((CountBit(blank & quad_parity_bitmask[3]) % 2) << 3) |
			((CountBit(blank & quad_parity_bitmask[2]) % 2) << 2) |
			((CountBit(blank & quad_parity_bitmask[1]) % 2) << 1) |
			(CountBit(blank & quad_parity_bitmask[0]) % 2);

		*p_selectivity = g_mpc_level;
		return SearchEmpty_5(bk, wh, blank, empty, parity, alpha, beta, 0);
	}

	g_countNode++;

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &max))
	{
		*p_selectivity = g_mpc_level;

		return max;
	}

	max = LOSS - 1;

	UINT32 move_cnt;
	MoveList movelist[48], *iter = movelist;
	UINT64 moves = CreateMoves(bk, wh, &move_cnt);
	INT32 selectivity; // init now thresould

	if (move_cnt == 0)
	{
		// �łĂȂ�����
		if (passed)
		{
			max = GetEndScore[g_solveMethod](bk, wh, empty);
			
		}
		else
		{
			max = -AB_SearchWinLoss(wh, bk, blank, empty,
				color ^ 1, -beta, -alpha, 1, p_selectivity);
		}
	}
	else
	{
		// �����flip-bit�����߂�move�\���̂ɕۑ�
		StoreMovelist(movelist, bk, wh, moves);
		if (move_cnt > 1)
		{
			if (empty >= 8) SortFastfirst(movelist, bk, wh);
			else SortPotentionalFastfirst(movelist, bk, wh, blank);
		}

#if 1
		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			pos_bit = 1ULL << pos;
			rev = iter->move.rev;

			eval = -AB_SearchWinLoss(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
				empty - 1, color ^ 1, -beta, -alpha, 0, &selectivity);

			if (beta <= eval)
			{
				max = eval;
				break;
			}

			/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
			if (eval > max)
			{
				max = eval;
				if (max > alpha) alpha = max;
			}
		}
#else
		// first move odd parity
		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			if (quad_parity[board_parity[pos]])
			{
				pos_bit = 1ULL << pos;
				rev = iter->move.rev;
				// reverse parity
				quad_parity[board_parity[pos]] ^= 1;
				eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
					empty - 1, color ^ 1, -beta, -alpha, 0, quad_parity, &selectivity);
				// restore parity
				create_quad_parity(quad_parity, blank);
				if (beta <= eval)
				{
					max = eval;
					*p_selectivity = selectivity; // store level
					return max;
				}

				/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
				if (eval > max)
				{
					max = eval;
					if (max > alpha)
					{
						alpha = max;
						pline->argmove[0] = pos;
						memcpy(pline->argmove + 1, line.argmove, line.cmove);
						pline->cmove = line.cmove + 1;
					}
				}
			}
		}

		// after, even parity
		iter = movelist;
		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			if (!quad_parity[board_parity[pos]])
			{
				pos_bit = 1ULL << pos;
				rev = iter->move.rev;
				// reverse parity
				quad_parity[board_parity[pos]] ^= 1;
				eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
					empty - 1, color ^ 1, -beta, -alpha, 0, quad_parity, &selectivity);
				// restore parity
				create_quad_parity(quad_parity, blank);
				if (beta <= eval)
				{
					max = eval;
					*p_selectivity = selectivity; // store level
					return max;
				}

				/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
				if (eval > max)
				{
					max = eval;
					if (max > alpha)
					{
						alpha = max;
						pline->argmove[0] = pos;
						memcpy(pline->argmove + 1, line.argmove, line.cmove);
						pline->cmove = line.cmove + 1;
					}
				}
			}
		}

#endif
	}

	//*p_selectivity = selectivity; // store level

	return max;
}