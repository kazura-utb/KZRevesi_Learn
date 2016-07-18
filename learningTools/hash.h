/***************************************************************************
* Name  : hash.h
* Brief : �u���\�֘A�̏������s��
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once


#define PREPARE_LOCKED 3
#define LOCKED 2

/*! Hash : item stored in the hash table */
typedef struct HashInfo {
	UINT64 bk;
	UINT64 wh;
	INT32 lower;        /*!< lower bound of the position score */
	INT32 upper;        /*!< upper bound of the position score */
	INT8 bestmove;       /*!< best move */
	INT8 empty;      /*!< depth of the analysis ( = board->n_empties) */
	INT8 locked;
} HashInfo;

/*! HashEntry: an entry, with two items, of the hash table */
typedef struct HashEntry {
	HashInfo deepest; /*!< entry for the highest cost search */
	HashInfo newest;  /*!< entry for the most recent search */
} HashEntry;

/*! HashTable : hash table */
typedef struct HashTable {
	HashEntry *entry; 
	INT32 size; 
} HashTable;

HashTable *HashNew(UINT32 in_size);
void HashDelete(HashTable *hash);
void HashClear(HashTable *hash);

void HashSet(HashTable *hash, int hashValue, const HashInfo *in_info);
HashInfo *HashGet(HashTable *hash, int key, UINT64 bk, UINT64 wh);

void HashUpdate(
	HashTable* hash_table,
	UINT32 key,
	UINT64 bk,
	UINT64 wh,
	INT32 alpha,
	INT32 beta,
	INT32 score,
	INT32 empty,
	INT8 move,
	INT32 inf_score);

void FixTableToMiddle(HashTable *hash);
void FixTableToWinLoss(HashTable *hash);
void FixTableToExact(HashTable *hash);