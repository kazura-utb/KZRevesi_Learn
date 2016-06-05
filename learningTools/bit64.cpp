/***************************************************************************
* Name  : bit64.cpp
* Brief : �ǖʂƒ�΃f�[�^���Ƃ炵���킹�Ē���
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"
#include <intrin.h>
#include "bit64.h"

/***************************************************************************
* Name  : CountBit
* Brief : �P�������Ă���r�b�g���𐔂���
* Args  : bit �P�������Ă���r�b�g�𐔂���Ώۂ̃r�b�g��
* Return: �P�������Ă���r�b�g��
****************************************************************************/
INT32 CountBit(UINT64 bit)
{
	int l_moves = bit & 0x00000000FFFFFFFF;
	int h_moves = (bit & 0xFFFFFFFF00000000) >> 32;

	int count = _mm_popcnt_u32(l_moves);
	count += _mm_popcnt_u32(h_moves);

	return count;
}