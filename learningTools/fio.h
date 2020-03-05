/***************************************************************************
* Name  : fio.h
* Brief : �t�@�C��IO�֘A�̏������s��
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

typedef struct {
	int code;
	int codeSize;
}CodeInfo;

typedef struct{
	char chr;
	int occurrence;
	int parent;
	int left;
	int right;
}TreeNode;

UCHAR *DecodeBookData(size_t *decodeDataLen_p, char *filename);
UCHAR *DecodeEvalData(size_t *decodeDataLen_p, char *filename);
BOOL OpenMpcInfoData(char *filename);