/***************************************************************************
* Name  : hash.cpp
* Brief : �u���\�֘A�̏������s��
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#include <stdlib.h>
#include "hash.h"
#include "eval.h"

int freeFlag = TRUE;

static void HashFinalize(HashTable *hash)
{
	if (freeFlag == TRUE){
		return;
	}
	if (hash->data) {
		free(hash->data);
	}
	freeFlag = TRUE;
}

void HashDelete(HashTable *hash)
{
	HashFinalize(hash);
	free(hash);
}

void HashClear(HashTable *hash)
{
	memset(hash->data, 0, sizeof(HashInfo) * hash->num);
	hash->getNum = 0;
	hash->hitNum = 0;
}

static int HashInitialize(HashTable *hash, int in_size)
{
	memset(hash, 0, sizeof(HashTable));
	hash->num = in_size;
	hash->data = (HashInfo *)malloc(sizeof(HashInfo) * hash->num);
	if (!hash->data) {
		return FALSE;
	}

	HashClear(hash);

	return TRUE;
}

HashTable *HashNew(UINT32 in_size)
{
	HashTable *hash;
	freeFlag = FALSE;
	hash = (HashTable *)malloc(sizeof(HashTable));
	if (hash) {
		if (!HashInitialize(hash, in_size)) {
			HashDelete(hash);
			hash = NULL;
		}
	}
	return hash;
}

void HashSet(HashTable *hash, int hashValue, const HashInfo *in_info)
{
	memcpy(&hash->data[hashValue], in_info, sizeof(HashInfo));
}

INT32 HashGet(HashTable *hash, int hashValue, UINT64 b_board, UINT64 w_board, HashInfo *out_info)
{
	if (hash == NULL){
		return FALSE;
	}
	//hash->getNum++;
	if (hash->data[hashValue].b_board == b_board && hash->data[hashValue].w_board == w_board)
	{
		memcpy(out_info, &hash->data[hashValue], sizeof(HashInfo));
		//hash->hitNum++;
		return TRUE;
	}
	else if (hash->data[hashValue].bestLocked == LOCKED){
		return LOCKED;
	}
	else if (hash->data[hashValue].bestLocked == PREPARE_LOCKED){
		return PREPARE_LOCKED;
	}
	return FALSE;
}

void HashClearInfo(HashTable *hash)
{
	hash->getNum = 0;
	hash->hitNum = 0;
}

int HashCountGet(HashTable *hash)
{
	return hash->getNum;
}

int HashCountHit(HashTable *hash)
{
	return hash->hitNum;
}

void HashUpdate(
	HashInfo *hash_info, 
	INT8 bestmove, 
	UINT32 depth,
	INT32 max, 
	INT32 alpha,
	INT32 beta,
	INT32 lower,
	INT32 upper){

	hash_info->locked = FALSE;
	hash_info->bestmove = bestmove;
	hash_info->depth = depth;

	if (max >= beta)
	{
		hash_info->lower = max;
		hash_info->upper = upper;
	}
	else if (max <= alpha)
	{
		hash_info->lower = lower;
		hash_info->upper = max;
	}
	else
	{
		hash_info->lower = max;
		hash_info->upper = max;
	}
}

void HashCreate(
	HashInfo *hash_info,
	UINT64 b_board, 
	UINT64 w_board,
	INT32 bestmove,
	INT32 move_cnt,
	UINT32 depth,
	INT32 max,
	INT32 alpha, INT32 beta,
	INT32 lower, INT32 upper){

	/* �u���\�ɓo�^ */
	hash_info->b_board = b_board;
	hash_info->w_board = w_board;
	hash_info->depth = depth;
	/* ���݂̋ǖʂ̎w����D�揇�ʂƒ���\����ۑ� */
	hash_info->move_cnt = move_cnt;
	hash_info->bestmove = bestmove;

	hash_info->locked = FALSE;

	if (max >= beta)
	{
		hash_info->lower = max;
		hash_info->upper = upper;
	}
	else if (max <= alpha)
	{
		hash_info->lower = lower;
		hash_info->upper = max;
	}
	else
	{
		hash_info->lower = max;
		hash_info->upper = max;
	}
}

void FixTableToMiddle(HashTable *hash)
{
	for (int i = 0; i < hash->num; i++)
	{
		hash->data[i].lower = NEGAMIN;
		hash->data[i].upper = NEGAMAX;
	}

}

void FixTableToWinLoss(HashTable *hash)
{
	for (int i = 0; i < hash->num; i++)
	{
		hash->data[i].lower = LOSS;
		hash->data[i].upper = WIN;
	}
}

void FixTableToExact(HashTable *hash)
{
	//UINT32 lower;
	//UINT32 upper;

	for (int i = 0; i < hash->num; i++)
	{
#if 0
		lower = hash->data[i].lower;
		upper = hash->data[i].upper;

		if (lower == LOSS && upper == LOSS)
		{
			hash->data[i].lower = -64;
			hash->data[i].upper = -2;
		}
		else if (lower == LOSS && upper == DRAW)
		{
			hash->data[i].lower = -64;
			hash->data[i].upper = 0;
		}
		else if (lower == LOSS && upper == WIN)
		{
			hash->data[i].lower = -64;
			hash->data[i].upper = 64;
		}
		else if (lower == DRAW && upper == DRAW)
		{
			hash->data[i].lower = 0;
			hash->data[i].upper = 0;
		}
		else if (lower == DRAW && upper == WIN)
		{
			hash->data[i].lower = 0;
			hash->data[i].upper = 64;
		}
		else if (lower == WIN && upper == WIN)
		{
			hash->data[i].lower = 2;
			hash->data[i].upper = 64;
		}
		else
		{
			hash->data[i].lower = -64;
			hash->data[i].upper = 64;
		}
#else
		hash->data[i].lower = -64;
		hash->data[i].upper = 64;
#endif

	}
}