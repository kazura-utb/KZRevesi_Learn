/***************************************************************************
* Name  : cpu.cpp
* Brief : �T���̏����S�ʂ��s��
* Date  : 2016/02/01
****************************************************************************/
#include "stdafx.h"
#include "bit64.h"
#include "board.h"
#include "move.h"
#include "rev.h"
#include "cpu.h"
#include "hash.h"
#include "eval.h"
#include "ordering.h"
#include "count_last_flip_carry_64.h"
#include "mpc_learn.h"


#include <stdio.h>

#define NO_PASS 0
#define ABORT 0x80000000

#define MPC_MIN_DEPTH 3

/***************************************************************************
*
* Global
*
****************************************************************************/
// CPU�ݒ�i�[�p
BOOL g_mpcFlag;
BOOL g_tableFlag;
INT32 g_empty;
INT32 g_limitDepth;
UINT64 g_casheSize;

// CPU AI���
BOOL g_AbortFlag;
UINT64 g_countNode;
UINT32 g_move;

HashTable *g_hash = NULL;

MPCINFO mpcInfo[22];
double MPC_CUT_VAL;

char g_cordinates_table[64][4];
SetMessageToGUI g_set_message_funcptr;

/***************************************************************************
*
* ProtoType(private)
*
****************************************************************************/
INT32 SearchMiddle(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);
INT32 SearchWinLoss(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);
INT32 SearchExact(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);
INT32 SearchEmpty_4(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 parity, INT32 alpha, UINT32 passed);

INT32 PVS_SearchDeep(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed);
INT32 AlphaBetaSearch(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed);

INT32 PVS_SearchDeepWinLoss(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed);
INT32 AlphaBetaSearchWinLoss(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 *quad_parity, UINT32 color, INT32 alpha, INT32 beta, UINT32 passed);

INT32 PVS_SearchDeepExact(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed);
INT32 AlphaBetaSearchExact(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 *quad_parity, UINT32 color, INT32 alpha, INT32 beta, UINT32 passed);

/***************************************************************************
* Name  : CreateCpuMessage
* Brief : CPU�̒�������t�h�ɑ��M����
* Args  : msg    : ���b�Z�[�W�i�[��
*         wh     : ���b�Z�[�W�i�[��̑傫��
*         eval   : �]���l
*         move   : ����ԍ�
*         cnt    : �[��
*         flag   : middle or winloss or exact
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
void CreateCpuMessage(char *msg, int msglen, int eval, int move, int cnt, int flag)
{
	if (flag == ON_MIDDLE){
		if (eval > 1280000)
		{
			sprintf_s(msg, msglen, "%s[WIN:%+d](depth = %d)", g_cordinates_table[move],
				eval - 1280000, cnt);
		}
		else if (eval < -1280000)
		{
			sprintf_s(msg, msglen, "%s[LOSS:%d](depth = %d)", g_cordinates_table[move],
				eval + 1280000, cnt);
		}
		else if (eval == 1280000)
		{
			sprintf_s(msg, msglen, "%s[DRAW](depth = %d)", g_cordinates_table[move], cnt);
		}
		else if (eval >= 0)
		{
			sprintf_s(msg, msglen, "%s[%.3f](depth = %d)", g_cordinates_table[move],
				eval / (double)EVAL_ONE_STONE, cnt);
		}
		else
		{
			sprintf_s(msg, msglen, "%s[%.3f](depth = %d)", g_cordinates_table[move],
				eval / (double)EVAL_ONE_STONE, cnt);
		}
	}
	else if (flag == ON_WINLOSS)
	{
		if (cnt == -1)
		{
			if (eval == WIN)
			{
				sprintf_s(msg, msglen, "guess... move %s [ WIN? ]", g_cordinates_table[move]);
			}
			else if (eval == LOSS)
			{
				sprintf_s(msg, msglen, "guess... move %s [ LOSS? ]", g_cordinates_table[move]);
			}
			else
			{
				sprintf_s(msg, msglen, "guess... move %s [ DRAW? ]", g_cordinates_table[move]);
			}
		}
		else
		{
			if (eval == WIN)
			{
				sprintf_s(msg, msglen, "%s [ WIN ]", g_cordinates_table[move]);
			}
			else if (eval == LOSS)
			{
				sprintf_s(msg, msglen, "%s [ LOSS ]", g_cordinates_table[move]);
			}
			else
			{
				sprintf_s(msg, msglen, "%s [ DRAW ]", g_cordinates_table[move]);
			}
		}

	}
	else
	{
		if (cnt == -2)
		{
			sprintf_s(msg, msglen, "guess... move %s [ %+d�`%+d ]", g_cordinates_table[move], eval - 8, eval + 8);
		}
		else if (cnt == -1)
		{
			if (eval > 0)
			{
				sprintf_s(msg, msglen, "guess... move %s [ WIN:%+d? ]", g_cordinates_table[move], eval);
			}
			else if (eval < 0)
			{
				sprintf_s(msg, msglen, "guess... move %s [ LOSS:%d? ]", g_cordinates_table[move], eval);
			}
			else
			{
				sprintf_s(msg, msglen, "guess... move %s [ DRAW:+0? ]", g_cordinates_table[move]);
			}
		}
		else
		{
			if (eval > 0)
			{
				sprintf_s(msg, msglen, "%s [ WIN:%+d ]", g_cordinates_table[move], eval);
			}
			else if (eval < 0)
			{
				sprintf_s(msg, msglen, "%s [ LOSS:%d ]", g_cordinates_table[move], eval);
			}
			else
			{
				sprintf_s(msg, msglen, "%s [ DRAW:+0 ]", g_cordinates_table[move]);
			}
		}
	}
}

/**
* @brief Stability Cutoff (SC).
*
* @param search Current position.
* @param alpha Alpha bound.
* @param beta Beta bound, to adjust if necessary.
* @param score Score to return in case of a cutoff is found.
* @return 'true' if a cutoff is found, false otherwise.
*/
bool search_SC_PVS(UINT64 bk, UINT64 wh, INT32 empty,
	volatile INT32 *alpha, volatile INT32 *beta, INT32 *score)
{
	if (*beta >= PVS_STABILITY_THRESHOLD[empty]) {
		*score = (64 - 2 * get_stability(wh, bk)) * EVAL_ONE_STONE;
		if (*score <= *alpha) {
			return true;
		}
		else if (*score < *beta) *beta = *score;
	}
	return false;
}

/**
* @brief Stability Cutoff (TC).
*
* @param search Current position.
* @param alpha Alpha bound.
* @param score Score to return in case of a cutoff is found.
* @return 'true' if a cutoff is found, false otherwise.
*/
bool search_SC_NWS(UINT64 bk, UINT64 wh, INT32 empty, INT32 alpha, INT32 *score)
{
	if (alpha >= NWS_STABILITY_THRESHOLD[empty]) {
		*score = 64 - 2 * get_stability(wh, bk);
		if (*score <= alpha) {
			return true;
		}
	}
	return false;
}

/***************************************************************************
* Name  : SetAbortFlag
* Brief : CPU�̏����𒆒f����
****************************************************************************/
void SetAbortFlag(){
	g_AbortFlag = TRUE;
}

/***************************************************************************
* Name  : GetMoveFromAI
* Brief : CPU�̒����T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
UINT64 GetMoveFromAI(UINT64 bk, UINT64 wh, UINT32 emptyNum, CPUCONFIG *cpuConfig)
{
	UINT64 move;

	if (cpuConfig->color != BLACK && cpuConfig->color != WHITE)
	{
		// �ォ��n���ꂽ�p�����[�^���s��
		return ILLIGAL_ARGUMENT;
	}

	// �L���b�V����������΁A�L���b�V�����������m��(1MB�����͖�������)
	if (cpuConfig->tableFlag == TRUE && cpuConfig->casheSize >= 1024)
	{
		if (g_hash == NULL)
		{
			g_hash = HashNew(cpuConfig->casheSize);
			g_casheSize = cpuConfig->casheSize;
		}
		else if (g_casheSize != cpuConfig->casheSize){
			HashDelete(g_hash);
			g_hash = HashNew(cpuConfig->casheSize);
			g_casheSize = cpuConfig->casheSize;
		}
	}

	g_mpcFlag = cpuConfig->mpcFlag;
	g_tableFlag = cpuConfig->tableFlag;

	// ���̋ǖʂ̒u���\�����������Ă���
	int key = KEY_HASH_MACRO(bk, wh);

	UINT32 temp;
	// CPU�̓p�X
	if (CreateMoves(bk, wh, &temp) == 0){
		return MOVE_PASS;
	}

	g_empty = emptyNum;

	// ���Ղ��ǂ������`�F�b�N
	if (emptyNum <= cpuConfig->exactDepth)
	{
		g_limitDepth = emptyNum;
		g_evaluation = SearchExact(bk, wh, emptyNum, cpuConfig->color);
	}
	else if (emptyNum <= cpuConfig->winLossDepth)
	{
		g_limitDepth = emptyNum;
		g_evaluation = SearchWinLoss(bk, wh, emptyNum, cpuConfig->color);
	}
	else
	{
		g_limitDepth = cpuConfig->searchDepth;
		g_evaluation = SearchMiddle(bk, wh, emptyNum, cpuConfig->color);
	}

	g_AbortFlag = FALSE;
#if 0
	// �u���\���璅����擾
	if (g_tableFlag)
	{
		move = 1ULL << (g_hash->entry[key].deepest.bestmove);
	}
	else
	{
		move = 1ULL << g_move;
	}
#endif
	return 0;
}

/***************************************************************************
* Name  : SearchMiddle
* Brief : ���Ձ`���Ղ�CPU�̒����T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����]���l
****************************************************************************/
INT32 SearchMiddle(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color)
{
	INT32 alpha = NEGAMIN;
	INT32 beta = NEGAMAX;
	INT32 eval = 0;
	INT32 eval_b = 0;
	INT32 limit = g_limitDepth;
	UINT32 key = KEY_HASH_MACRO(bk, wh);

	/* ���OAI�ݒ�g���p(���͉����Ȃ�) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	// �����[���[���D��T��
	for (int count = 1; count <= limit; count ++)
	{
		eval_b = eval;
		g_limitDepth = count;
		eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, alpha, beta, NO_PASS);

		if (eval == ABORT)
		{
			break;
		}

		// �ݒ肵�������]���l���Ⴂ���H
		if (eval <= alpha)
		{
			// �Ⴂ�Ȃ烿�������ɍĐݒ肵�Č���
			eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, NEGAMIN, eval, NO_PASS);
		}
		// �ݒ肵�������]���l���������H
		else if (eval >= beta)
		{
			// �����Ȃ��������ɍĐݒ肵�Č���
			eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, eval, NEGAMAX, NO_PASS);
		}

		// ���̕����}8�ɂ��Č��� (��,��) ---> (eval - 8, eval + 8)
		alpha = eval - (8 * EVAL_ONE_STONE);
		beta = eval + (8 * EVAL_ONE_STONE);

	}

	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT){
		return eval_b;
	}

	return eval;
}

/***************************************************************************
* Name  : SearchExact
* Brief : CPU�̒�������s�T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����]���l
****************************************************************************/
INT32 SearchExact(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color)
{
	INT32 alpha = -INF_SCORE;
	INT32 beta = INF_SCORE;
	INT32 eval = 0;
	INT32 eval_b = 0;
	UINT32 key = KEY_HASH_MACRO(bk, wh);
	INT32 aVal;
	INT32 bVal;
	char msg[128];
	INT32 move;

	g_limitDepth = emptyNum - 2;
#if 1
	if (g_limitDepth % 2) g_limitDepth--;
	if (g_limitDepth > 24)
	{
		g_limitDepth = 24;
	}

	if (g_limitDepth >= 12)
	{
		// ���O���s�T��
		eval = SearchMiddle(bk, wh, emptyNum, color);

		eval /= EVAL_ONE_STONE;
		eval -= eval % 2;

		// ���炩�ɍ�������ꍇ�͑S�Ŏ�̌���
		if (eval + 8 > 48) eval = 56;
		if (eval - 8 < -48) eval = -56;

		aVal = eval - 8;
		bVal = eval + 8;

		CreateCpuMessage(msg, sizeof(msg), eval, g_hash->entry[key].deepest.bestmove, -2, ON_EXACT);
		g_set_message_funcptr(msg);

		// �u���\��΍��T���p�ɏ�����
		FixTableToExact(g_hash);
		//HashClear(g_hash);
	}
	else
	{
		HashClear(g_hash);
		aVal = -INF_SCORE;
		bVal = INF_SCORE;
	}

#endif

	g_empty = emptyNum;
	// PVS�΍��T��
	eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, aVal, bVal, NO_PASS);
	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT)
	{
		return eval_b;
	}

	if (eval == 64)
	{
		return eval;
	}
	if (eval == -64)
	{
		return -64;
	}

	eval_b = eval;

	if (eval <= aVal)
	{
		move = g_hash->entry[key].deepest.bestmove;
		sprintf_s(msg, "guess move %s < %+d", g_cordinates_table[move], eval);
		g_set_message_funcptr(msg);
		// ���l����������̂ōĒT��
		eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, -INF_SCORE, eval, NO_PASS);

	}
	else if (eval >= bVal)
	{
		move = g_hash->entry[key].deepest.bestmove;
		sprintf_s(msg, "guess move %s > %+d", g_cordinates_table[move], eval);
		g_set_message_funcptr(msg);
		// ���l���������̂ōĒT��
		eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, eval, INF_SCORE, NO_PASS);

	}

	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT)
	{
		return eval_b;
	}

	return eval;
}

/***************************************************************************
* Name  : SearchWinLoss
* Brief : CPU�̒�������s�T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����]���l
****************************************************************************/
INT32 SearchWinLoss(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color)
{
	INT32 eval = 0;
	INT32 eval_b = 0;
	UINT32 key = KEY_HASH_MACRO(bk, wh);
	INT32 aVal;
	INT32 bVal;

	g_limitDepth = emptyNum;
	if (g_limitDepth % 2) g_limitDepth--;
	if (g_limitDepth > 24)
	{
		g_limitDepth = 24;
	}
	if (g_limitDepth >= 12)
	{
		// ���O�T��
		eval = SearchMiddle(bk, wh, emptyNum, color);
		// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
		if (g_AbortFlag)
		{
			return eval;
		}

		eval_b = eval;
		eval /= EVAL_ONE_STONE;
		if (eval % 2)
		{
			eval++;
		}

		if (eval >= 4)
		{
			aVal = DRAW;
			bVal = WIN;
		}
		else if (eval <= -4)
		{
			aVal = LOSS;
			bVal = DRAW;
		}
		else
		{
			aVal = DRAW;
			bVal = WIN;
		}
		// �u���\��΍��T���p�ɏ�����
		FixTableToWinLoss(g_hash);
	}
	else
	{
		aVal = LOSS;
		bVal = WIN;
	}

	g_empty = emptyNum;
	// PVS�΍��T��
	eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, aVal, bVal, NO_PASS);
	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT)
	{
		return eval_b;
	}

	eval_b = eval;

	// �����ߑł��T���Ɏ��s�������͕������Ɋg�債�čēx�T��
	if (eval <= aVal)
	{
		// PVS�΍��T��
		eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, LOSS, eval, NO_PASS);

	}
	else if (eval >= bVal)
	{
		// PVS�΍��T��
		eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, eval, WIN, NO_PASS);

	}

	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT)
	{
		return eval_b;
	}

	return eval;
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
INT32 PVS_SearchDeepExact(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	int score, bestscore, lower, upper, bestmove;
	UINT32 key;
	MoveList movelist[32 + 2], *iter;
	Move *move;
	HashInfo *hashInfo;

	bestmove = NOMOVE;
	lower = alpha;
	upper = beta;

	if (g_empty > 12 && empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
	{
		UINT64 blank = ~(bk | wh);
		UINT32 quad_parity[4];
		create_quad_parity(quad_parity, blank);
		return AlphaBetaSearchExact(bk, wh, blank, empty, quad_parity, 
			color, alpha, beta, passed);
	}

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &score)) return score;

	/************************************************************
	*
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/
	/* �L�[�𐶐� */
	key = KEY_HASH_MACRO(bk, wh);
	/* transposition cutoff ? */
	if (g_tableFlag)
	{
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->empty < empty)
			{
				if (upper > hashInfo->upper)
				{
					upper = hashInfo->upper;
					if (upper <= lower) return upper;
				}
				if (lower < hashInfo->lower)
				{
					lower = hashInfo->lower;
					if (lower >= upper) return lower;
				}
			}
			hashInfo->empty = empty;
			bestmove = hashInfo->bestmove;
		}
	}

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) �t�F�[�Y
	*
	*************************************************************/

	// ��������\��


	/************************************************************
	*
	* Principal Variation Search(PVS) �t�F�[�Y
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;

	// �����flip-bit�����߂�move�\���̂ɕۑ�
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL)
	{
		if (passed) {
			// game end...
			if (bk == 0)
			{
				bestscore = -64;
			}
			else if (wh == 0)
			{
				bestscore = 64;
			}
			else
			{
				bestscore = CountBit(bk) - CountBit(wh);
				// �󂫂������ԂŏI������̂ŋ󂫃}�X�������Ă�����ɉ��Z����
				if (bestscore > 0)
				{
					bestscore += empty;
				}
				else if (bestscore < 0)
				{
					bestscore -= empty;
				}
			}
			bestmove = NOMOVE;
		}
		else
		{
			bestscore = -PVS_SearchDeepExact(wh, bk, empty, color ^ 1, hash, -upper, -lower, 1);
			bestmove = PASS;
		}
	}
	else {

#if 1
		HashEntry *hash_entry;
		UINT32 hashKey;
		UINT64 bk_after, wh_after;
		bool pv_flag = true;

		/* enhanced transposition cutoff */
		if (hash != NULL) {
			if (bestmove != NOMOVE) SortMoveListTableMoveFirst(movelist, bestmove);
			for (iter = movelist->next; iter != NULL; iter = iter->next) {
				move = &(iter->move);
				bk_after = bk ^ ((1ULL << move->pos) | move->rev);
				wh_after = wh ^ move->rev;

				hashKey = KEY_HASH_MACRO(wh_after, bk_after);
				hash_entry = &(hash->entry[hashKey]);

				if (hash_entry->deepest.bk == wh_after &&
					hash_entry->deepest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->deepest.upper >= upper)
					return -hash_entry->deepest.upper;
				if (hash_entry->newest.bk == wh_after &&
					hash_entry->newest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->newest.upper >= upper)
					return -hash_entry->newest.upper;
			}
		}
#endif

		if (moveCount > 1)
		{
			if (empty > 15)
			{
				SortMoveListMiddle(movelist, bk, wh, hash, empty, alpha, beta, color);
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

		bestscore = -INF_SCORE;

		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;

			if (pv_flag)
			{
				score = -PVS_SearchDeepExact(move_w, move_b,
					empty - 1, color ^ 1, hash, -upper, -lower, 0);
			}
			else
			{
				score = -PVS_SearchDeepExact(move_w, move_b,
					empty - 1, color ^ 1, hash, -lower - 1, -lower, 0);
				if (lower < score && score < upper)
				{
					score = -PVS_SearchDeepExact(move_w, move_b,
						empty - 1, color ^ 1, hash, -upper, -lower, 0);
				}
			}

			if (score >= beta)
			{
				bestscore = score;
				bestmove = move->pos;
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (empty == g_empty)
				{
					char msg[64];
					// UI�Ƀ��b�Z�[�W�𑗐M
					CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, -1, ON_EXACT);
					g_set_message_funcptr(msg);
				}
				if (bestscore > lower)
				{
					pv_flag = false;
					lower = bestscore;

				}
			}
		}
	}
	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);

	if (empty == g_empty)
	{
		char msg[64];
		// UI�Ƀ��b�Z�[�W�𑗐M
		CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, 0, ON_EXACT);
		g_set_message_funcptr(msg);
	}

	return bestscore;
}

/**
* @brief Get the final score.
*
* Get the final score, when no move can be made.
*
* @param board Board.
* @param n_empties Number of empty squares remaining on the board.
* @return The final score, as a disc difference.
*/
INT32 get_exact_score(UINT64 bk, UINT64 wh, INT32 empty)
{
	const int n_discs_p = CountBit(bk);
	const int n_discs_o = 64 - empty - n_discs_p;
	int score = n_discs_p - n_discs_o;

	if (score < 0) score -= empty;
	else if (score > 0) score += empty;

	return score;
}


/**
* @brief Get the final score.
*
* Get the final score, when no move can be made.
*
* @param board Board.
* @param n_empties Number of empty squares remaining on the board.
* @return The final score, as a disc difference.
*/
INT32 get_winloss_score(UINT64 bk, UINT64 wh, INT32 empty)
{
	int score;
	const int n_discs_p = CountBit(bk);
	const int n_discs_o = 64 - empty - n_discs_p;

	if (n_discs_p > n_discs_o)
	{
		score = WIN;
	}
	else if (n_discs_p < n_discs_o)
	{
		score = LOSS;
	}
	else
	{
		score = DRAW;
	}

	return score;
}

/**
* @brief Get the final score.
*
* Get the final score, when 1 empty squares remain.
* The following code has been adapted from Zebra by Gunnar Anderson.
*
* @param board  Board to evaluate.
* @param beta   Beta bound.
* @param x      Last empty square to play.
* @return       The final opponent score, as a disc difference.
*/
INT32 SearchEmpty_1(UINT64 bk, UINT64 wh, INT32 pos)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	//g_countNode++;

	INT32 score = 2 * CountBit(bk);
	INT32 n_flips = count_last_flip[pos](bk);


	if (n_flips)
	{
		score += n_flips + 2;
	}
	else
	{
		// opponent turn...
		n_flips = count_last_flip[pos](wh);
		if (n_flips)
		{
			score -= n_flips;
		}
		else
		{
			// empty = 1 end...win(31, 30) loss(30, 31) win > 30*2 = 60
			if (score > 60) score += 2;
		}
	}

	return 	score - 64;
}

INT32 SearchEmpty_2(UINT64 bk, UINT64 wh, INT32 x1, INT32 x2, 
	                INT32 alpha, INT32 beta, INT32 passed)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	INT32 eval, best;
	UINT64 pos_bit;
	UINT64 rev;

	pos_bit = 1ULL << x1;
	rev = GetRev[x1](bk, wh);
	if (rev)
	{
		best = -SearchEmpty_1(wh ^ rev, bk ^ (pos_bit | rev), x2);
		if (best >= beta) return best;
		alpha = max(best, alpha);
	}
	else best = -INF_SCORE;

	pos_bit = 1ULL << x2;
	rev = GetRev[x2](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_1(wh ^ rev, bk ^ (pos_bit | rev), x1);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			alpha = max(eval, alpha);
		}
	}

	// no move
	if (best == -INF_SCORE)
	{
		if (passed)
		{
			best = get_exact_score(bk, wh, 2);
		}
		else
		{
			best = -SearchEmpty_2(wh, bk, x1, x2, -beta, -alpha, 1);
		}
	}

	return best;
}

INT32 SearchEmpty_3(UINT64 bk, UINT64 wh, UINT64 blank, UINT32 parity, INT32 alpha, INT32 beta, INT32 passed)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	// 3 empties parity
	UINT64 temp_moves = blank;
	int x1 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x1);
	int x2 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x2);
	int x3 = CountBit((~temp_moves) & (temp_moves - 1));

	if (!(parity & board_parity_bit[x1])) {
		if (parity & board_parity_bit[x2]) { // case 1(x2) 2(x1 x3)
			int tmp = x1; x1 = x2; x2 = tmp;
		}
		else { // case 1(x3) 2(x1 x2)
			int tmp = x1; x1 = x3; x3 = x2; x2 = tmp;
		}
	}

	UINT64 pos_bit, rev;
	INT32 eval;
	INT32 best;

	pos_bit = 1ULL << x1;
	rev = GetRev[x1](bk, wh);
	if (rev)
	{
		best = -SearchEmpty_2(wh ^ rev, bk ^ (pos_bit | rev), x2, x3, -beta, -alpha, 0);
		if (best >= beta) return best;
		alpha = max(best, alpha);
	}
	else best = -INF_SCORE;

	pos_bit = 1ULL << x2;
	rev = GetRev[x2](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_2(wh ^ rev, bk ^ (pos_bit | rev), x1, x3, -beta, -alpha, 0);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			alpha = max(eval, alpha);
		}
	}

	pos_bit = 1ULL << x3;
	rev = GetRev[x3](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_2(wh ^ rev, bk ^ (pos_bit | rev), x1, x2, -beta, -alpha, 0);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			alpha = max(eval, alpha);
		}
	}

	// no move
	if (best == -INF_SCORE)
	{
		if (passed)
		{
			best = get_exact_score(bk, wh, 3);
		}
		else
		{
			best = -SearchEmpty_3(wh, bk, blank, parity, -beta, -alpha, 1);
		}
	}

	return best;

}

INT32 SearchEmpty_4(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 parity, INT32 alpha, INT32 beta, UINT32 passed)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	INT32 best;
	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &best)) return best;

	// 4 empties parity
	UINT64 temp_moves = blank;
	int x1 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x1);
	int x2 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x2);
	int x3 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x3);
	int x4 = CountBit((~temp_moves) & (temp_moves - 1));

	// parity...move sort
	//    4 - 1 3 - 2 2 - 1 1 2 - 1 1 1 1
	// Only the 1 1 2 case needs move sorting.
	if (!(parity & board_parity_bit[x1])) {
		if (parity & board_parity_bit[x2]) {
			if (parity & board_parity_bit[x3]) { // case 1(x2) 1(x3) 2(x1 x4)
				int tmp = x1; x1 = x2; x2 = x3; x3 = tmp;
			}
			else { // case 1(x2) 1(x4) 2(x1 x3)
				int tmp = x1; x1 = x2; x2 = x4; x4 = x3; x3 = tmp;
			}
		}
		else if (parity & board_parity_bit[x3]) { // case 1(x3) 1(x4) 2(x1 x2)
			int tmp = x1; x1 = x3; x3 = tmp; tmp = x2; x2 = x4; x4 = tmp;
		}
	}
	else {
		if (!(parity & board_parity_bit[x2])) {
			if (parity & board_parity_bit[x3]) { // case 1(x1) 1(x3) 2(x2 x4)
				int tmp = x2; x2 = x3; x3 = tmp;
			}
			else { // case 1(x1) 1(x4) 2(x2 x3)
				int tmp = x2; x2 = x4; x4 = x3; x3 = tmp;
			}
		}
	}

	UINT64 pos_bit, rev;
	INT32 eval;

	pos_bit = 1ULL << x1;
	rev = GetRev[x1](bk, wh);
	if (rev)
	{
		best = -SearchEmpty_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, parity ^ board_parity_bit[x1], -beta, -alpha, 0);
		if (best >= beta) return best;
		alpha = max(best, alpha);
	}
	else best = -INF_SCORE;

	pos_bit = 1ULL << x2;
	rev = GetRev[x2](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, parity ^ board_parity_bit[x2], -beta, -alpha, 0);
		if (eval >= beta) return eval;
		else if (eval > best) 
		{
			best = eval; 
			alpha = max(eval, alpha);
		}
	}

	pos_bit = 1ULL << x3;
	rev = GetRev[x3](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, parity ^ board_parity_bit[x3], -beta, -alpha, 0);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			alpha = max(eval, alpha);
		}
	}

	pos_bit = 1ULL << x4;
	rev = GetRev[x4](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, parity ^ board_parity_bit[x4], -beta, -alpha, 0);
		if (eval > best) best = eval;
	}

	// no move
	if (best == -INF_SCORE)
	{
		if (passed)
		{
			best = get_exact_score(bk, wh, 4);
		}
		else
		{
			best = -SearchEmpty_4(wh, bk, blank, empty, parity, -beta, -alpha, 1);
		}
	}


	return best;
}

INT32 AlphaBetaSearchExact(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 *quad_parity, UINT32 color, INT32 alpha, INT32 beta, UINT32 passed)
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
	UINT64 moves;

	// parity moving
	if (empty == 4)
	{
		UINT32 parity =
			((CountBit(blank & quad_parity_bitmask[3]) % 2) << 3) |
			((CountBit(blank & quad_parity_bitmask[2]) % 2) << 2) |
			((CountBit(blank & quad_parity_bitmask[1]) % 2) << 1) |
			(CountBit(blank & quad_parity_bitmask[0]) % 2);
		return SearchEmpty_4(bk, wh, blank, empty, parity, alpha, beta, 0);
	}

	g_countNode++;

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &max)) return max;

	max = -INF_SCORE;

	// First, move odd parity empties
	for (int i = 0; i < 4; i++)
	{
		if (quad_parity[i])
		{
			// odd parity
			moves = blank & quad_parity_bitmask[i];
			// parity�T���J�n
			while (moves)
			{
				/*
			�@	 �����ɗ���̂͂U�}�X�ȉ��̋󂫂Ȃ̂ŁACreateMoves���ĂԂ��
				 ���]�f�[�^�擾�ƍ��@����󂫃}�X���璼�ڃ`�F�b�N�����ق������|�I�ɑ���
				 */
				pos = CountBit((~moves) & (moves - 1));
				pos_bit = 1ULL << pos;
				rev = GetRev[pos](bk, wh);

				if (rev)
				{
					// reverse parity
					quad_parity[i] ^= 1;
					eval = -AlphaBetaSearchExact(wh ^ rev, bk ^ (pos_bit | rev),
						blank ^ pos_bit, empty - 1, quad_parity, color ^ 1, -beta, -alpha, 0);
					// restore parity
					create_quad_parity(quad_parity, blank);
					if (beta <= eval)
					{
						return eval;
					}

					/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
					if (eval > max)
					{
						max = eval;
						alpha = max(alpha, eval);
					}
				}
				moves ^= pos_bit;
			}
		}
	}

	// after, even parity
	for (int i = 0; i < 4; i++)
	{
		if (quad_parity[i] == 0)
		{
			// even parity
			moves = blank & quad_parity_bitmask[i];
			// parity�T���J�n
			while (moves)
			{
				/*
			�@	 �����ɗ���̂͂U�}�X�ȉ��̋󂫂Ȃ̂ŁACreateMoves���ĂԂ��
				 ���]�f�[�^�擾�ƍ��@����󂫃}�X���璼�ڃ`�F�b�N�����ق������|�I�ɑ���
				 */
				pos = CountBit((~moves) & (moves - 1));
				pos_bit = 1ULL << pos;
				rev = GetRev[pos](bk, wh);

				if (rev)
				{
					// reverse parity
					quad_parity[i] ^= 1;
					eval = -AlphaBetaSearchExact(wh ^ rev, bk ^ (pos_bit | rev),
						blank ^ pos_bit, empty - 1, quad_parity, color ^ 1, -beta, -alpha, 0);
					// restore parity
					create_quad_parity(quad_parity, blank);
					if (beta <= eval)
					{
						return eval;
					}

					/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
					if (eval > max)
					{
						max = eval;
						alpha = max(alpha, eval);
					}
				}
				moves ^= pos_bit;
			}
		}
	}

	if (max == -INF_SCORE)
	{
		// �łĂȂ�����
		if (passed)
		{
			max = get_exact_score(bk, wh, empty);
		}
		else
		{
			max = -AlphaBetaSearchExact(wh, bk, blank, empty, quad_parity,
				color ^ 1, -beta, -alpha, 1);
			// restore parity
			create_quad_parity(quad_parity, blank);
		}
	}

	return max;

}

/***************************************************************************
* Name  : PVS_SearchDeepWinLoss
* Brief : PVS���s���A�]���l����ɍőP����擾
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
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	int score, bestscore, lower, upper, bestmove;
	UINT32 key;
	MoveList movelist[32 + 2], *iter;
	Move *move;
	HashInfo *hashInfo;

	bestmove = NOMOVE;
	lower = alpha;
	upper = beta;

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &score))
	{
		if (score > 0)
		{
			score = WIN;
		}
		else if (score < 0)
		{
			score = LOSS;
		}
		else
		{
			score = DRAW;
		}

		return score;
	}

	UINT32 quad_parity[4];
	if (empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
	{
		create_quad_parity(quad_parity, ~(bk | wh));
		return AlphaBetaSearchWinLoss(bk, wh, ~(bk | wh), empty,
			quad_parity, color, alpha, beta, passed);
	}

	/************************************************************
	*
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/
	/* �L�[�𐶐� */
	key = KEY_HASH_MACRO(bk, wh);
	/* transposition cutoff ? */
	if (g_tableFlag)
	{
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->empty < empty)
			{
				if (upper > hashInfo->upper)
				{
					upper = hashInfo->upper;
					if (upper <= lower) return upper;
				}
				if (lower < hashInfo->lower)
				{
					lower = hashInfo->lower;
					if (lower >= upper) return lower;
				}
			}
			hashInfo->empty = empty;
			bestmove = hashInfo->bestmove;
		}
	}

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) �t�F�[�Y
	*
	*************************************************************/

	// ��������\��


	/************************************************************
	*
	* Principal Variation Search(PVS) �t�F�[�Y
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;


	// �����flip-bit�����߂�move�\���̂ɕۑ�
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL)
	{
		if (passed) {
			// game end...
			bestscore = get_winloss_score(bk, wh, empty);
			bestmove = NOMOVE;
		}
		else {
			bestscore = -PVS_SearchDeepWinLoss(wh, bk, empty, color ^ 1, hash, -upper, -lower, 1);
			bestmove = PASS;
		}
	}
	else {

#if 1
		HashEntry *hash_entry;
		UINT32 hashKey;
		UINT64 bk_after, wh_after;

		/* enhanced transposition cutoff */
		if (hash != NULL) {
			if (bestmove != NOMOVE) SortMoveListTableMoveFirst(movelist, bestmove);
			for (iter = movelist->next; iter != NULL; iter = iter->next) {
				move = &(iter->move);
				bk_after = bk ^ ((1ULL << move->pos) | move->rev);
				wh_after = wh ^ move->rev;

				hashKey = KEY_HASH_MACRO(wh_after, bk_after);
				hash_entry = &(hash->entry[hashKey]);

				if (hash_entry->deepest.bk == wh_after &&
					hash_entry->deepest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->deepest.upper >= upper)
					return -hash_entry->deepest.upper;
				if (hash_entry->newest.bk == wh_after &&
					hash_entry->newest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->newest.upper >= upper)
					return -hash_entry->newest.upper;
			}
		}
#endif

		if (moveCount > 1)
		{
			// ��̕��בւ�
			SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
		}

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		/* �ŏ��ɍőP��Ƃ��ڂ������ł��ĉ��ɍőP��Ƃ��� */
		iter = movelist->next;
		move = &(iter->move);

		move_b = bk ^ ((1ULL << move->pos) | move->rev);
		move_w = wh ^ move->rev;

		bestscore = -PVS_SearchDeepWinLoss(move_w, move_b,
			empty - 1, color ^ 1, hash, -upper, -lower, 0);

		// �őP��Ƃ���
		bestmove = move->pos;
		if (bestscore >= beta)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);
			if (empty == g_empty)
			{
				char msg[64];
				// UI�Ƀ��b�Z�[�W�𑗐M
				CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, 0, ON_WINLOSS);
				g_set_message_funcptr(msg);
			}
			return bestscore;   // fail-hard beta-cutoff
		}

		if (bestscore > lower) lower = bestscore;

		if (empty == g_empty)
		{
			char msg[64];
			// UI�Ƀ��b�Z�[�W�𑗐M
			CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, -1, ON_WINLOSS);
			g_set_message_funcptr(msg);
		}

		/* other moves : try to refute the first/best one */
		for (iter = iter->next; lower < upper && iter != NULL; iter = iter->next)
		{
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;

			score = -PVS_SearchDeepWinLoss(move_w, move_b,
				empty - 1, color ^ 1, hash, -lower - 1, -lower, 0);
			if (lower < score && score < upper)
				score = -PVS_SearchDeepWinLoss(move_w, move_b,
				empty - 1, color ^ 1, hash, -upper, -score, 0);

			if (score >= beta)
			{
				bestscore = score;
				bestmove = move->pos;
				HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (empty == g_empty)
				{
					char msg[64];
					// UI�Ƀ��b�Z�[�W�𑗐M
					CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, -1, ON_WINLOSS);
					g_set_message_funcptr(msg);
				}
				if (bestscore > lower) lower = bestscore;
			}
		}
	}
	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);

	if (empty == g_empty)
	{
		char msg[64];
		// UI�Ƀ��b�Z�[�W�𑗐M
		CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, 0, ON_WINLOSS);
		g_set_message_funcptr(msg);
	}

	return bestscore;


}

/***************************************************************************
* Name  : SearchEmptyWinLoss_1
* Brief : ���s�T�����A�c��1�}�X�̎��̏������s��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �󂫃}�X�̃r�b�g��
* Return: WIN or LOSS or DRAW
****************************************************************************/
/**
* @brief Get the final score.
*
* Get the final score, when 1 empty squares remain.
* The following code has been adapted from Zebra by Gunnar Anderson.
*
* @param board  Board to evaluate.
* @param beta   Beta bound.
* @param x      Last empty square to play.
* @return       The final opponent score, as a disc difference.
*/
INT32 SearchEmptyWinLoss_1(UINT64 bk, UINT64 wh, INT32 pos, INT32 beta)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	INT32 score, n_flips;

	score = 2 * CountBit(wh) - 64;
	UINT64 rev = GetRev[pos](bk, wh);

	if ((n_flips = count_last_flip[pos](bk)) != 0)
	{
		score -= n_flips;
	}
	else
	{
		if (score >= 0) {
			score += 2;
			if (score < beta) { // lazy cut-off
				rev = GetRev[pos](wh, bk);
				if ((n_flips = count_last_flip[pos](wh)) != 0) {
					score += n_flips;
				}
			}
		}
		else {
			if (score < beta) { // lazy cut-off
				rev = GetRev[pos](wh, bk);
				if ((n_flips = count_last_flip[pos](wh)) != 0) {
					score += n_flips + 2;
				}
			}
		}
	}

	if (score > 0)
	{
		score = WIN;
	}
	else if (score < 0)
	{
		score = LOSS;
	}
	else
	{
		score = DRAW;
	}

	return score;
}

INT32 SearchEmptyWinLoss_2(UINT64 bk, UINT64 wh, UINT64 blank,
	INT32 pos1, INT32 pos2, INT32 alpha)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	INT32 eval, best;
	UINT64 pos_bit = 1ULL << pos1;
	UINT64 rev = GetRev[pos1](bk, wh);
	INT32 beta = alpha + 1;

	if (rev)
	{
		best = SearchEmptyWinLoss_1(wh ^ rev, bk ^ (pos_bit | rev), pos2, beta);
	}
	else best = -INF_SCORE;

	if (best < beta)
	{
		pos_bit = 1ULL << pos2;
		rev = GetRev[pos2](bk, wh);
		if (rev)
		{
			eval = SearchEmptyWinLoss_1(wh ^ rev, bk ^ (pos_bit | rev), pos1, beta);
			if (eval > best) best = eval;
		}

		// pass
		if (best == -INF_SCORE) {
			pos_bit = 1ULL << pos1;
			rev = GetRev[pos1](wh, bk);
			if (rev)
			{
				best = -SearchEmptyWinLoss_1(bk ^ rev, wh ^ (pos_bit | rev), pos2, -alpha);
			}
			else best = INF_SCORE;

			if (best > alpha)
			{
				pos_bit = 1ULL << pos2;
				rev = GetRev[pos2](wh, bk);
				if (rev)
				{
					eval = -SearchEmptyWinLoss_1(bk ^ rev, wh ^ (pos_bit | rev), pos1, -alpha);
					if (eval < best) best = eval;
				}

				// gameover
				if (best == INF_SCORE)
				{
					return get_winloss_score(bk, wh, 2);
				}
			}
		}

	}

	return best;
}

INT32 SearchEmptyWinLoss_3(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty, UINT32 parity, INT32 alpha)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	// 3 empties parity
	UINT64 temp_moves = blank;
	int x1 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x1);
	int x2 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x2);
	int x3 = CountBit((~temp_moves) & (temp_moves - 1));

	if (!(parity & board_parity_bit[x1])) {
		if (parity & board_parity_bit[x2]) { // case 1(x2) 2(x1 x3)
			int tmp = x1; x1 = x2; x2 = tmp;
		}
		else { // case 1(x3) 2(x1 x2)
			int tmp = x1; x1 = x3; x3 = x2; x2 = tmp;
		}
	}

	UINT64 pos_bit, rev;
	UINT64 move_b, move_w;
	INT32 eval;
	INT32 best;
	INT32 beta = alpha + 1;

	// �����̒���
	pos_bit = 1ULL << x1;
	rev = GetRev[x1](bk, wh);

	if (rev)
	{
		move_b = bk ^ (pos_bit | rev);
		move_w = wh ^ rev;
		best = -SearchEmptyWinLoss_2(move_w, move_b, blank ^ pos_bit, x2, x3, -beta);
		if (best >= beta) return best;
	}
	else best = -INF_SCORE;

	pos_bit = 1ULL << x2;
	rev = GetRev[x2](bk, wh);

	if (rev)
	{
		move_b = bk ^ (pos_bit | rev);
		move_w = wh ^ rev;
		eval = -SearchEmptyWinLoss_2(move_w, move_b, blank ^ pos_bit, x1, x3, -beta);
		if (eval >= beta) return eval;
		else if (eval > best) best = eval;
	}

	pos_bit = 1ULL << x3;
	rev = GetRev[x3](bk, wh);

	if (rev)
	{
		move_b = bk ^ (pos_bit | rev);
		move_w = wh ^ rev;
		eval = -SearchEmptyWinLoss_2(move_w, move_b, blank ^ pos_bit, x1, x2, -beta);
		if (eval > best) best = eval;
	}

	// �������łĂȂ������̂ő���̒���
	if (best == -INF_SCORE)
	{
		pos_bit = 1ULL << x1;
		rev = GetRev[x1](wh, bk);

		if (rev)
		{
			move_b = bk ^ rev;
			move_w = wh ^ (pos_bit | rev);
			best = SearchEmptyWinLoss_2(move_b, move_w, blank ^ pos_bit, x2, x3, alpha);
			if (best <= alpha) return best;
		}
		else best = INF_SCORE;

		pos_bit = 1ULL << x2;
		rev = GetRev[x2](wh, bk);


		if (rev)
		{
			move_b = bk ^ rev;
			move_w = wh ^ (pos_bit | rev);
			eval = SearchEmptyWinLoss_2(move_b, move_w, blank ^ pos_bit, x1, x3, alpha);
			if (eval <= alpha) return eval;
			else if (eval < best) best = eval;
		}

		pos_bit = 1ULL << x3;
		rev = GetRev[x3](wh, bk);

		if (rev)
		{
			move_b = bk ^ rev;
			move_w = wh ^ (pos_bit | rev);
			eval = SearchEmptyWinLoss_2(move_b, move_w, blank ^ pos_bit, x1, x2, alpha);
			if (eval < best) best = eval;
		}

		// gameover
		if (best == INF_SCORE)
		{
			return get_winloss_score(bk, wh, 3);
		}
	}

	return best;

}

INT32 SearchEmptyWinLoss_4(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 parity, INT32 alpha, UINT32 passed)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	INT32 best;
	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &best))
	{
		if (best > 0)
		{
			best = WIN;
		}
		else if (best < 0)
		{
			best = LOSS;
		}
		else
		{
			best = DRAW;
		}

		return best;
	}

	// 4 empties parity
	UINT64 temp_moves = blank;
	int x1 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x1);
	int x2 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x2);
	int x3 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x3);
	int x4 = CountBit((~temp_moves) & (temp_moves - 1));
	// parity...move sort
	//    4 - 1 3 - 2 2 - 1 1 2 - 1 1 1 1
	// Only the 1 1 2 case needs move sorting.
	if (!(parity & board_parity_bit[x1])) {
		if (parity & board_parity_bit[x2]) {
			if (parity & board_parity_bit[x3]) { // case 1(x2) 1(x3) 2(x1 x4)
				int tmp = x1; x1 = x2; x2 = x3; x3 = tmp;
			}
			else { // case 1(x2) 1(x4) 2(x1 x3)
				int tmp = x1; x1 = x2; x2 = x4; x4 = x3; x3 = tmp;
			}
		}
		else if (parity & board_parity_bit[x3]) { // case 1(x3) 1(x4) 2(x1 x2)
			int tmp = x1; x1 = x3; x3 = tmp; tmp = x2; x2 = x4; x4 = tmp;
		}
	}
	else {
		if (!(parity & board_parity_bit[x2])) {
			if (parity & board_parity_bit[x3]) { // case 1(x1) 1(x3) 2(x2 x4)
				int tmp = x2; x2 = x3; x3 = tmp;
			}
			else { // case 1(x1) 1(x4) 2(x2 x3)
				int tmp = x2; x2 = x4; x4 = x3; x3 = tmp;
			}
		}
	}

	UINT64 pos_bit, rev;
	INT32 eval;
	INT32 beta = alpha + 1;

	pos_bit = 1ULL << x1;
	rev = GetRev[x1](bk, wh);
	if (rev)
	{
		best = -SearchEmptyWinLoss_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, empty - 1,
			parity ^ board_parity_bit[x1], -beta);
		if (best >= beta) return best;
	}
	else best = -INF_SCORE;

	pos_bit = 1ULL << x2;
	rev = GetRev[x2](bk, wh);
	if (rev)
	{
		eval = -SearchEmptyWinLoss_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, empty - 1,
			parity ^ board_parity_bit[x2], -beta);
		if (eval >= beta) return eval;
		else if (eval > best) best = eval;
	}

	pos_bit = 1ULL << x3;
	rev = GetRev[x3](bk, wh);
	if (rev)
	{
		eval = -SearchEmptyWinLoss_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, empty - 1,
			parity ^ board_parity_bit[x3], -beta);
		if (eval >= beta) return eval;
		else if (eval > best) best = eval;
	}

	pos_bit = 1ULL << x4;
	rev = GetRev[x4](bk, wh);
	if (rev)
	{
		eval = -SearchEmptyWinLoss_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, empty - 1,
			parity ^ board_parity_bit[x4], -beta);
		if (eval > best) best = eval;
	}

	// no move
	if (best == -INF_SCORE)
	{
		if (passed)
		{
			return get_winloss_score(bk, wh, 4);
		}
		else
		{
			best = -SearchEmptyWinLoss_4(wh, bk, blank, empty, parity, -beta, 1);
		}
	}


	return best;
}


/***************************************************************************
* Name  : AlphaBetaSearchWinLoss
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
INT32 AlphaBetaSearchWinLoss(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 *quad_parity, UINT32 color, INT32 alpha, INT32 beta, UINT32 passed)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	INT32 max;                    //���݂̍ō��]���l
	INT32 eval;                   //�]���l�̕ۑ�
	INT32 pos;
	UINT64 pos_bit, rev;
	UINT64 moves = blank;

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &max))
	{
		if (max > 0)
		{
			max = WIN;
		}
		else if (max < 0)
		{
			max = LOSS;
		}
		else
		{
			max = DRAW;
		}

		return max;
	}

	// parity moving
	if (empty == 4)
	{
		UINT32 parity =
			(CountBit(blank & quad_parity_bitmask[3]) % 2) << 3 |
			(CountBit(blank & quad_parity_bitmask[2]) % 2) << 2 |
			(CountBit(blank & quad_parity_bitmask[1]) % 2) << 1 |
			CountBit(blank & quad_parity_bitmask[0]) % 2;

		return SearchEmptyWinLoss_4(bk, wh, blank, empty, parity, alpha, passed);
	}

	max = -INF_SCORE;

	// First, move odd parity empties
	for (int i = 0; i < 4; i++)
	{
		if (quad_parity[i])
		{
			// odd parity
			moves = blank & quad_parity_bitmask[i];
			// parity�T���J�n
			while (moves)
			{
				/*
				�@	 �����ɗ���̂͂U�}�X�ȉ��̋󂫂Ȃ̂ŁACreateMoves���ĂԂ��
					 ���]�f�[�^�擾�ƍ��@����󂫃}�X���璼�ڃ`�F�b�N�����ق������|�I�ɑ���
					 */
				pos = CountBit((~moves) & (moves - 1));
				pos_bit = 1ULL << pos;
				rev = GetRev[pos](bk, wh);

				if (rev)
				{
					// reverse parity
					quad_parity[i] ^= 1;
					eval = -AlphaBetaSearchWinLoss(wh ^ rev, bk ^ (pos_bit | rev),
						blank ^ pos_bit, empty - 1, quad_parity, color ^ 1, -beta, -alpha, 0);
					// restore parity
					create_quad_parity(quad_parity, blank);
					if (beta <= eval)
					{
						return eval;
					}

					/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
					if (eval > max)
					{
						max = eval;
						alpha = max(alpha, eval);
					}
				}
				moves ^= pos_bit;
			}
		}
	}

	// after, even parity
	for (int i = 0; i < 4; i++)
	{
		if (!quad_parity[i])
		{
			// even parity
			moves = blank & quad_parity_bitmask[i];
			// parity�T���J�n
			while (moves)
			{
				/*
				�@	 �����ɗ���̂͂U�}�X�ȉ��̋󂫂Ȃ̂ŁACreateMoves���ĂԂ��
					 ���]�f�[�^�擾�ƍ��@����󂫃}�X���璼�ڃ`�F�b�N�����ق������|�I�ɑ���
					 */
				pos = CountBit((~moves) & (moves - 1));
				pos_bit = 1ULL << pos;
				rev = GetRev[pos](bk, wh);

				if (rev)
				{
					// reverse parity
					quad_parity[i] ^= 1;
					eval = -AlphaBetaSearchWinLoss(wh ^ rev, bk ^ (pos_bit | rev),
						blank ^ pos_bit, empty - 1, quad_parity, color ^ 1, -beta, -alpha, 0);
					// restore parity
					create_quad_parity(quad_parity, blank);
					if (beta <= eval)
					{
						return eval;
					}

					/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
					if (eval > max)
					{
						max = eval;
						alpha = max(alpha, eval);
					}
				}
				moves ^= pos_bit;
			}
		}
	}

	if (max == -INF_SCORE)
	{
		// �łĂȂ�����
		if (passed)
		{
			max = get_winloss_score(bk, wh, empty);
		}
		else
		{
			max = -AlphaBetaSearchWinLoss(wh, bk, blank, empty, quad_parity,
				color ^ 1, -beta, -alpha, 1);
			// restore parity
			create_quad_parity(quad_parity, blank);
		}
	}

	return max;
}

/***************************************************************************
* Name  : PVS_SearchDeep
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
INT32 PVS_SearchDeep(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	if (depth < DEPTH_DEEP_TO_SHALLOW_SEARCH)
	{
		return AlphaBetaSearch(bk, wh, depth, empty, color, alpha, beta, passed);
	}

	g_countNode++;

	int score, bestscore, lower, upper, bestmove;
	UINT32 key;
	MoveList movelist[34], *iter;
	Move *move;
	HashInfo *hashInfo;

	bestmove = NOMOVE;
	lower = alpha;
	upper = beta;

	// stability cutoff
	if (search_SC_PVS(bk, wh, empty, &alpha, &beta, &score)) return score;

	/************************************************************
	*
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/

	/* transposition cutoff ? */
	if (g_tableFlag){
		/* �L�[�𐶐� */
		key = KEY_HASH_MACRO(bk, wh);
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->empty < empty)
			{
#if 1
				if (upper > hashInfo->upper) {
					upper = hashInfo->upper;
					if (upper <= lower) return upper;
				}
				if (lower < hashInfo->lower) {
					lower = hashInfo->lower;
					if (lower >= upper) return lower;
				}
#endif
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
	if (g_mpcFlag && depth >= MPC_MIN_DEPTH && depth <= MPC_INFO_NUM + MPC_MIN_DEPTH - 1)
	{
		if ((59 - empty) >= 36)
		{
			MPC_CUT_VAL = 1.4;
		}
		else
		{
			MPC_CUT_VAL = 1.0;
		}

		MPCINFO *mpcInfo_p = &mpcInfo[depth - MPC_MIN_DEPTH];
		INT32 value = (INT32)(alpha - (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		INT32 eval = AlphaBetaSearch(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed);
		if (eval < value)
		{
			return alpha;
		}

		value = (INT32)(beta + (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AlphaBetaSearch(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed);
		if (eval > value)
		{
			return beta;
		}
	}
#endif

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	bool pv_flag = true;

	// �����flip-bit�����߂�move�\���̂ɕۑ�
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL) {
		if (passed) {
			bestscore = CountBit(bk) - CountBit(wh);

			if (bestscore > 0)
			{
				bestscore += 1280000;
			}
			else if (bestscore < 0)
			{
				bestscore -= 1280000;
			}
			else
			{
				bestscore = 1280000;
			}

			bestmove = NOMOVE;
		}
		else {
			bestscore = -PVS_SearchDeep(wh, bk, depth, empty, color ^ 1, hash, -upper, -lower, 1);
			bestmove = PASS;
		}
	}
	else
	{
#if 1
		HashEntry *hash_entry;
		UINT32 hashKey;
		UINT64 bk_after, wh_after;
		/* enhanced transposition cutoff */
		if (g_tableFlag && hash != NULL) {
			if (bestmove != NOMOVE) SortMoveListTableMoveFirst(movelist, bestmove);
			for (iter = movelist->next; iter != NULL; iter = iter->next) {
				move = &(iter->move);
				bk_after = bk ^ ((1ULL << move->pos) | move->rev);
				wh_after = wh ^ move->rev;

				hashKey = KEY_HASH_MACRO(wh_after, bk_after);
				hash_entry = &(hash->entry[hashKey]);

				if (hash_entry->deepest.bk == wh_after &&
					hash_entry->deepest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->deepest.upper >= upper &&
					depth != g_limitDepth)
					return -hash_entry->deepest.upper;
				if (hash_entry->newest.bk == wh_after &&
					hash_entry->newest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->newest.upper >= upper &&
					depth != g_limitDepth)
					return -hash_entry->newest.upper;
			}
		}
#endif
		/* ���՗p�̃I�[�_�����O */
		SortMoveListMiddle(movelist, bk, wh, hash, empty, alpha, beta, color);

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		bestscore = NEGAMIN;
		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			move = &(iter->move);

			if (pv_flag)
			{
				score = -PVS_SearchDeep(wh ^ move->rev, bk ^ ((1ULL << move->pos) | move->rev),
					depth - 1, empty - 1, color ^ 1, hash, -upper, -lower, 0);
			}
			else
			{
				score = -PVS_SearchDeep(wh^move->rev, bk ^ ((1ULL << move->pos) | move->rev),
					depth - 1, empty - 1, color ^ 1, hash, -lower - 1, -lower, 0);
				if (lower < score && score < upper)
					score = -PVS_SearchDeep(wh ^ move->rev, bk ^ ((1ULL << move->pos) | move->rev),
						depth - 1, empty - 1, color ^ 1, hash, -upper, -lower, 0);
			}

			if (score >= beta)
			{
				bestscore = score;
				bestmove = move->pos;
				if (depth == g_limitDepth) g_move = bestmove;
				HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, NEGAMAX);
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (depth == g_limitDepth) g_move = bestmove;
				if (bestscore > lower)
				{
					lower = bestscore;
					pv_flag = false;
				}
			}
		}
	}

	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, NEGAMAX);

	return bestscore;

}

INT32 AlphaBetaSearch(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (depth == 0)
	{
		/* �t�m�[�h(�ǂ݂̌��E�l�̃m�[�h)�̏ꍇ�͕]���l���Z�o */
		InitIndexBoard(bk, wh);
		return Evaluation(g_board, bk, wh, color, 59 - empty);
	}

	int eval;
	// stability cutoff
	if (search_SC_PVS(bk, wh, empty, &alpha, &beta, &eval)) return eval;

	int move_cnt;
	int max;                    //���݂̍ō��]���l
	UINT64 rev;
	UINT64 moves;             //���@��̃��X�g�A�b�v

	/* ���@�萶���ƃp�X�̏��� */
	if ((moves = CreateMoves(bk, wh, (UINT32 *)(&move_cnt))) == 0)
	{
		if (passed)
		{
			max = CountBit(bk) - CountBit(wh);
			if (max > 0)
			{
				max += 1280000;
			}
			else if (max < 0)
			{
				max -= 1280000;
			}
			else
			{
				max = 1280000;
			}

			return max;
		}
		max = -AlphaBetaSearch(wh, bk, depth, empty, color ^ 1, -beta, -alpha, 1);
	}
	else
	{
		int pos;
		max = NEGAMIN;
		do
		{
			/* �ÓI�����Â��i���Ȃ��R�X�g�ő啝�ɍ���������݂����j */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);
			/* �^�[����i�߂čċA������ */
			eval = -AlphaBetaSearch(wh ^ rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, empty - 1, color ^ 1, -beta, -alpha, 0);

			if (beta <= eval)
			{
				return eval;
			}

			/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
			if (eval > max)
			{
				max = eval;
				alpha = max(alpha, eval);
			}

			moves ^= 1ULL << pos;

		} while (moves);
	}

	return max;

}

INT32 OrderingAlphaBeta(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (depth == 0){
		/* �t�m�[�h(�ǂ݂̌��E�l�̃m�[�h)�̏ꍇ�͕]���l���Z�o */
		InitIndexBoard(bk, wh);
		return Evaluation(g_board, bk, wh, color, 59 - empty);
	}

	UINT32 move_cnt;
	INT32 max;                    //���݂̍ō��]���l
	INT32 eval;                   //�]���l�̕ۑ�
	UINT64 rev;
	UINT64 moves;             //���@��̃��X�g�A�b�v

	/* ���@�萶���ƃp�X�̏��� */
	if ((moves = CreateMoves(bk, wh, &move_cnt)) == 0){
		if (passed)
		{
			max = CountBit(bk) - CountBit(wh);
			if (max > 0)
			{
				max += 1280000;
			}
			else if (max < 0)
			{
				max -= 1280000;
			}
			else
			{
				max = 1280000;
			}
		}
		else
		{
			max = -OrderingAlphaBeta(wh, bk, depth, empty, color ^ 1, -beta, -alpha, 1);
		}
	}
	else
	{
		int pos;
		max = NEGAMIN;
		do{
			/* �ÓI�����Â��i���Ȃ��R�X�g�ő啝�ɍ���������݂����j */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);

			eval = -OrderingAlphaBeta(wh^rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, empty - 1, color ^ 1, -beta, -alpha, 0);

			if (beta <= eval){
				return eval;
			}

			/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
			if (eval > max)
			{
				max = eval;
				alpha = max(alpha, eval);
			}

			moves ^= 1ULL << pos;
		} while (moves);
	}

	return max;

}