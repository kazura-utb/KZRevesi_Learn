/***************************************************************************
* Name  : book.cpp
* Brief : �ǖʂƒ�΃f�[�^���Ƃ炵���킹�Ē���
* Date  : 2016/02/01
****************************************************************************/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bit64.h"
#include "board.h"
#include "rev.h"
#include "move.h"
#include "eval.h"
#include "board.h"
#include "book.h"
#include "fio.h"

INT32 max_change_num[2];
BOOL g_book_done;

/***************************************************************************
*
* Global
*
****************************************************************************/
/* �w����̉�]�E�Ώ̕ϊ��t���O */
int TRANCE_MOVE;


BooksNode g_bookTree;
BooksNode *g_bestNode;

/***************************************************************************
* 
* ProtoType(private)
* 
****************************************************************************/
INT32 SearchBooks(BooksNode *book_root, UINT64 bk, UINT64 wh,
	UINT32 color, UINT32 change, INT32 turn);
BooksNode *SearchBookInfo(BooksNode *book_header, BooksNode *before_book_header,
	UINT64 bk, UINT64 wh, INT32 turn);
INT32 book_alphabeta(BooksNode *book_header, UINT32 depth, INT32 alpha, INT32 beta,
	UINT32 color, UINT32 change, INT32 turn);
VOID SortBookNode(BooksNode *best_node[], INT32 e_list[], INT32 cnt);
INT32 SelectNode(INT32 e_list[], INT32 cnt, UINT32 change, INT32 turn);

/***************************************************************************
* Name  : GetMoveFromBooks
* Brief : ��΂���CPU�̒�������肷��
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
UINT64 GetMoveFromBooks(UINT64 bk, UINT64 wh, UINT32 color, UINT32 change, INT32 turn)
{
	INT64 move;
	if (turn == 0)
	{
		srand((UINT32)time(NULL));
		// ���ڂ̒���͂ǂ��ɒ��肵�Ă������Ȃ̂Ń����_���Ƃ���
		int cnt;
		INT64 enumMove = CreateMoves(bk, wh, (UINT32 *)&cnt);
		int rnd = rand() % cnt;

		while (rnd)
		{
			enumMove &= enumMove - 1;
			rnd--;
		}
		move = CountBit((enumMove & (-enumMove)) - 1);
	}
	else
	{
		move = SearchBooks(g_bookTree.child, bk, wh, color, change, turn);
	}
	
	if (move == MOVE_NONE)
	{
		return move;
	}

	return 1ULL << move;

}

/***************************************************************************
* Name  : SearchBooks
* Brief : ��΂₩��CPU�̒�������肷��
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 SearchBooks(BooksNode *book_root, UINT64 bk, UINT64 wh, 
	UINT32 color, UINT32 change, INT32 turn)
{
	INT32 move = MOVE_NONE;
	ULONG eval = 0;
	BooksNode *book_header;

	srand((unsigned int)time(NULL));

	/* �ǖʂ���Y���̒�΂�T�� */
	book_header = SearchBookInfo(book_root, NULL, bk, wh, turn);

	if (book_header != NULL)
	{
		/* �]���l�ɂ�莟�̎���΂���I�� */
		eval = book_alphabeta(book_header, 0, NEGAMIN, NEGAMAX, color, change, turn);
		book_header = g_bestNode;
		g_evaluation = eval;

		/* �w����̑Ώ̉�]�ϊ��̏ꍇ���� */
		switch (TRANCE_MOVE)
		{
		case 0:
			move = book_header->move;
			break;
		case 1:
		{
			UINT64 t_move = rotate_90(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 2:
		{
			UINT64 t_move = rotate_180(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 3:
		{
			UINT64 t_move = rotate_270(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 4:
		{
			UINT64 t_move = symmetry_x(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 5:
		{
			UINT64 t_move = symmetry_y(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 6:
		{
			UINT64 t_move = symmetry_b(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 7:
		{
			UINT64 t_move = symmetry_w(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		default:
			move = book_header->move;
			break;
		}
	}

	return move;
}

/***************************************************************************
* Name  : SearchBookInfo
* Brief : ��΂���CPU�̒�������肷��
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
BooksNode *SearchBookInfo(BooksNode *book_header, BooksNode *before_book_header, 
	UINT64 bk, UINT64 wh, INT32 turn)
{
	/* �t�m�[�h�܂Ō������Č�����Ȃ��ꍇ */
	if (book_header == NULL)
	{
		return NULL;
	}
	if (book_header->depth > turn)
	{
		return NULL;
	}
	if (book_header->depth == turn)
	{
		if (turn == 0)
		{
			before_book_header = &g_bookTree;
		}
		/* �Y���̒�΂𔭌�(��]�E�Ώ̂��l����) */
		if (book_header->bk == bk && book_header->wh == wh)
		{
			/* �w����̉�]�E�Ώ̕ϊ��Ȃ� */
			TRANCE_MOVE = 0;
			return before_book_header;
		}
		/* 90�x�̉�]�` */
		if (book_header->bk == rotate_90(bk) && book_header->wh == rotate_90(wh))
		{
			TRANCE_MOVE = 1;
			return before_book_header;
		}
		/* 180�x�̉�]�` */
		if (book_header->bk == rotate_180(bk) && book_header->wh == rotate_180(wh))
		{
			TRANCE_MOVE = 2;
			return before_book_header;
		}
		/* 270�x�̉�]�` */
		if (book_header->bk == rotate_270(bk) && book_header->wh == rotate_270(wh))
		{
			TRANCE_MOVE = 3;
			return before_book_header;
		}
		/* X���̑Ώ̌` */
		if (book_header->bk == symmetry_x(bk) && book_header->wh == symmetry_x(wh))
		{
			TRANCE_MOVE = 4;
			return before_book_header;
		}
		/* Y���̑Ώ̌` */
		if (book_header->bk == symmetry_y(bk) && book_header->wh == symmetry_y(wh))
		{
			TRANCE_MOVE = 5;
			return before_book_header;
		}
		/* �u���b�N���C���̑Ώ̌` */
		if (book_header->bk == symmetry_b(bk) && book_header->wh == symmetry_b(wh))
		{
			TRANCE_MOVE = 6;
			return before_book_header;
		}
		/* �z���C�g���C���̑Ώ̌` */
		if (book_header->bk == symmetry_w(bk) && book_header->wh == symmetry_w(wh))
		{
			TRANCE_MOVE = 7;
			return before_book_header;
		}
	}

	BooksNode *ret;
	if ((ret = SearchBookInfo(book_header->child, book_header, bk, wh, turn)) != NULL)
	{
		return ret;
	}
	if ((ret = SearchBookInfo(book_header->next, book_header, bk, wh, turn)) != NULL)
	{
		return ret;
	}

	return NULL;
}

/***************************************************************************
* Name  : book_alphabeta
* Brief : ��΂̌���̂����A��X�ɕ]���l���ł������Ȃ���̂��Z�o����
* Return: ��΂̕]���l
****************************************************************************/
INT32 book_alphabeta(BooksNode *book_header, UINT32 depth, INT32 alpha, INT32 beta,
	UINT32 color, UINT32 change, INT32 turn)
{
	if (book_header->child == NULL)
	{
		if (color == WHITE)
		{
			return -book_header->eval;
		}
		return book_header->eval;
	}

	int i;

	if (depth == 0)
	{
		int e_list[24] =
		{
			NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN,
			NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN
		};
		int eval, max = NEGAMIN - 1;
		BooksNode *best_node[24] =
		{
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
		};

		i = 0;
		do{
			if (i == 0)
			{
				book_header = book_header->child;
			}
			else
			{
				book_header = book_header->next;
			}
			eval = -book_alphabeta(book_header, depth + 1, 
				-beta, -alpha, color ^ 1, change, turn);
			e_list[i] = eval;
			best_node[i] = book_header;
			if (eval > max)
			{
				max = eval;
			}
			i++;
		} while (book_header->next != NULL);

		SortBookNode(best_node, e_list, i);
		/* �m�[�h�ԍ����Z�o */
		INT32 node_num = SelectNode(e_list, i, change, turn);
		g_bestNode = best_node[node_num];

		return e_list[node_num];
	}
	else
	{
		int eval, max = NEGAMIN - 1;
		i = 0;

		do{
			if (i == 0)
			{
				book_header = book_header->child;
			}
			else
			{
				book_header = book_header->next;
			}
			eval = -book_alphabeta(book_header, depth + 1, -beta, -alpha, 
				color ^ 1, change, turn);
			if (eval > max)
			{
				max = eval;
				if (max > alpha)
				{
					alpha = max;   //�����l���X�V
					/* �A���t�@�J�b�g */
					if (beta <= alpha)
					{
						break;
					}
				}
			}
			i++;
		} while (book_header->next != NULL);

		return max;
	}
}

/***************************************************************************
* Name  : SortBookNode
* Brief : ��΂̌���̂����A�]���l�̍������Ƀ\�[�g
****************************************************************************/
void SortBookNode(BooksNode *best_node[], int e_list[], int cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int swaps;
	BooksNode *temp;
	int int_temp;
	if (cnt == 1){ return; }
	while (1)
	{
		swaps = 0;
		for (i = 0; i + h < cnt; i++)
		{
			if (e_list[i] < e_list[i + h])
			{
				temp = best_node[i];
				best_node[i] = best_node[i + h];
				best_node[i + h] = temp;
				int_temp = e_list[i];
				e_list[i] = e_list[i + h];
				e_list[i + h] = int_temp;
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

/***************************************************************************
* Name  : SelectNode
* Brief : ��΂̕ω��x�ɂ���Č�������肷��
* Return: ��Δԍ�
****************************************************************************/
INT32 SelectNode(int e_list[], int cnt, UINT32 change, INT32 turn)
{
	int ret;
	srand((UINT32)time(NULL));
	if (change == NOT_CHANGE)
	{
		int count;
		int max = e_list[0];
		for (count = 1; count < cnt; count++)
		{
			if (e_list[count] < max)
			{
				break;
			}
		}
		ret = rand() % count;
	}
	/* ���P�肩��-2�ȏ�܂ŋ��� */
	else if (change == CHANGE_LITTLE)
	{
		int count;
		int flag = 0;
		int max = e_list[0];
		for (count = 1; count < cnt; count++)
		{
			/* ��x���P���I��ł���A
			�܂��͎��P�肪���̕]���l���2000�ȏ�Ⴂ�Ƃ���break */
			if (max_change_num[turn] || max - e_list[count] >= 2000)
			{
				break;
			}
			if (e_list[count] < max)
			{
				if (flag)
				{
					break;
				}
				flag++;
			}

		}
		ret = rand() % count;
		/* ���P���I�� */
		if (e_list[ret] != max)
		{
			max_change_num[turn]++;
		}
	}
	else if (change == CHANGE_MIDDLE)
	{
		int count;
		int flag = 0;
		int max = e_list[0];
		for (count = 1; count < cnt; count++)
		{
			/* 2�x���P���I��ł���A
			�܂��͎��P�肪���̕]���l���4000�ȏ�Ⴂ�Ƃ���break */
			if (max_change_num[turn] > 1 || max - e_list[count] >= 4000)
			{
				break;
			}
			if (e_list[count] < max)
			{
				if (flag > 2)
				{
					break;
				}
				flag++;
			}
		}
		ret = rand() % count;
		/* ���P���I�� */
		if (ret != 0 && e_list[ret] != max)
		{
			max_change_num[turn]++;
		}
	}
	else
	{
		// ���S�����_��
		ret = rand() % cnt;
	}

	return ret;
}

/* �m�[�h��ڑ� */
void Append(BooksNode *parent, BooksNode *node)
{
	//node->book_name = name;
	//node->move = move;
	node->child = NULL;
	node->next = NULL;
	if (parent == NULL)
	{
		return;
	}
	if (parent->child == NULL)
	{
		parent->child = node;
	}
	else
	{
		BooksNode *last = parent->child;
		while (last->next != NULL)
		{
			last = last->next;
		}
		last->next = node;
	}
}

BooksNode *SearchChild(BooksNode *head, int move)
{
	if (head->child == NULL)
	{
		return NULL;
	}
	head = head->child;
	while (head != NULL)
	{
		if (head->move == move)
		{
			return head;
		}
		head = head->next;
	}

	return NULL;
}

void StTreeFromLine(BooksNode *head, char *line, int eval)
{
	short depth = 0;
	char move_str[3];
	UINT64 bk = BK_FIRST;
	UINT64 wh = WH_FIRST;
	UINT64 rev;
	size_t line_len = strlen(line);
	BooksNode *head_child;

	while (depth < line_len)
	{
		//move_str[0] = 'a' + line[depth] - 'A';
		move_str[0] = line[depth];
		move_str[1] = line[depth + 1];
		move_str[2] = '\0';
		/* a1��0 �Ȃ� ���l�ɕϊ� */
		int move = (move_str[0] - 'a') * 8 + move_str[1] - '1';

		/* ���łɓo�^����Ă���m�[�h�����邩�T�� */
		head_child = SearchChild(head, move);

		/* �V�K�̏ꍇ */
		if (head_child == NULL)
		{
			BooksNode *node = (BooksNode *)malloc(sizeof(BooksNode));
			if (node == NULL) return;
			node->move = (short)move;
			if (depth % 4)
			{
				rev = GetRev[move](wh, bk);
				node->bk = bk;
				node->wh = wh;
				wh ^= (1ULL << move) | rev;
				bk ^= rev;
			}
			else
			{
				rev = GetRev[move](bk, wh);
				node->bk = bk;
				node->wh = wh;
				bk ^= (1ULL << move) | rev;
				wh ^= rev;
			}
			node->eval = eval;
			node->depth = depth / 2;
			Append(head, node);
			head = node;
		}
		/* ���o�̏ꍇ */
		else
		{
			if (depth % 4)
			{
				rev = GetRev[move](wh, bk);
				wh ^= (1ULL << move) | rev;
				bk ^= rev;
			}
			else
			{
				rev = GetRev[move](bk, wh);
				bk ^= (1ULL << move) | rev;
				wh ^= rev;
			}
			head = head_child;
		}
		depth += 2;
	}
}

void StructionBookTree(BooksNode *head, char *filename)
{
	char *decode_sep, *line_data, *eval_str;
	char *next_str = NULL, *next_line = NULL;
	UCHAR* decodeData;
	size_t decodeDataLen;

	decodeData = DecodeBookData(&decodeDataLen, filename);
	if (decodeDataLen == -1)
	{
		return;
	}

	decode_sep = strtok_s((char *)decodeData, "\n", &next_line);
	do
	{
		/* �t�@�C������1�s�Âǂݍ���Ŗ؍\�����쐬 */
		line_data = strtok_s(decode_sep, ";", &next_str);
		eval_str = strtok_s(next_str, ";", &next_str);
		StTreeFromLine(head, line_data, (int)(atof(eval_str) * EVAL_ONE_STONE));

	} while ((decode_sep = strtok_s(next_line, "\n", &next_line)) != NULL);

	free(decodeData);
}

/***************************************************************************
* Name  : OpenBook
* Brief : ��΃f�[�^���J��
* Return: TRUE/FALSE
****************************************************************************/
BOOL OpenBook(char *filename)
{
	BooksNode *root = &g_bookTree;
	root->bk = BK_FIRST;
	root->wh = WH_FIRST;
	root->move = 64;
	root->eval = 0;
	root->depth = 0;
	StructionBookTree(root, filename);

	if (root->child == NULL)
	{
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************
* Name  : BookFree
* Brief : ��΃f�[�^�̂��߂̃����������
****************************************************************************/
void BookFree(BooksNode *head)
{
	// �t�m�[�h�H
	if (head->next == NULL && head->child == NULL)
	{
		free(head);
		return;
	}

	if (head->child)
	{
		BookFree(head->child);
	}
	if (head->next)
	{
		BookFree(head->next);
	}

	free(head);

	return;
}