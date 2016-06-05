
#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <Windows.h>
#include <nmmintrin.h>

#include "learningTools.h"
#include "bit64.h"
#include "type.h"
#include "rev.h"
#include "eval.h"
#include "cpu.h"
#include "move.h"
#include "hash.h"
#include "GetPattern.h"

int MPC_INFO_NUM = 0;
int END_MPC_INFO_NUM = 0;
int deep_level;

int sharrow_level[] =
{
	1, 1, 2, 2, 2, 2, 3, 3, 4, 4, 4, 4, 4, 4, 4
};


void swap(UINT64 *bk, UINT64 *wh)
{
	UINT64 temp = *bk;
	*bk = *wh;
	*wh = temp;
}

int RemainSt1ULL(UINT64 black, UINT64 white)
{
	UINT64 blank = ~(black | white);
	return CountBit(blank);
}

int ConvertAsciiToNum(char* whorData, char* kifuData)
{
	int color = BLACK;
	int move;
	int byteCounter = 0;
	UINT64 bk = FIRST_BK, wh = FIRST_WH;
	UINT64 rev;
	bool badFlag = false;
	int turn = 0;
	int offset = 0;
	int whorCounter = 0;

	while (1)
	{
		while (kifuData[byteCounter] == ' '){
			byteCounter++;
		}

		/* �p����ǂݍ��� */
		/* �I�V�}�C */
		if (!isalpha(kifuData[byteCounter]))
		{
			break;
		}
		/* ������ǂݍ��� */
		/* �Ȃ����������Ȋ����f�[�^�����\���� */
		if (!isdigit(kifuData[byteCounter + 1]))
		{
			badFlag = true;
			break;
		}

		move = ((kifuData[byteCounter] - 'A') * 8) + (kifuData[byteCounter + 1] - '1');
		/* a9 �Ƃ����炩�ɊԈ��������܂�ł������������ */
		if (move < 0 || move >= 64)
		{
			badFlag = true;
			break;
		}
		/* �Ȃ������łɒu�����}�X�ɒu���Ă������������ */
		if (bk & (1ULL << move) || wh & (1ULL << move))
		{
			badFlag = true;
			break;
		}

		if (color == BLACK)
		{
			rev = GetRev[move](bk, wh);
			/* ���p�X�H */
			if (rev == 0)
			{
				rev = GetRev[move](wh, bk);
				/* �ꉞ���@��ɂȂ��Ă��邩�̃`�F�b�N */
				if (rev == 0)
				{
					badFlag = true;
					break;
				}
				bk ^= rev;
				wh ^= ((1ULL << move) | rev);
			}
			else
			{
				bk ^= ((1ULL << move) | rev);
				wh ^= rev;
				color ^= 1;
			}
		}
		else
		{
			rev = GetRev[move](wh, bk);
			/* ���p�X�H */
			if (rev == 0)
			{
				rev = GetRev[move](bk, wh);
				/* �ꉞ���@��ɂȂ��Ă��邩�̃`�F�b�N */
				if (rev == 0)
				{
					badFlag = true;
					break;
				}
				bk ^= ((1ULL << move) | rev);
				wh ^= rev;
			}
			else
			{
				bk ^= rev;
				wh ^= ((1ULL << move) | rev);
				color ^= 1;
			}
		}

		whorData[whorCounter++] = move;

		byteCounter += 2;
		turn++;
	}

	if ((turn != 60 && turn != 59) || badFlag == true)
	{
		return -1;
	}

	return 0;

}

void getKifuLine(char buf[][512])
{
	int i = 0;
	char line[512];

	FILE *rfp;
	if (fopen_s(&rfp, "kifu\\01E4.gam.1.new", "rb") != 0){
		return;
	}

	while (i < 500 && fgets(line, 512, rfp) != NULL)
	{
		if (line[121] != ' ')
		{
			// 60�蒅��ȊO�̊����̓o�O���Ă�\����Ȃ̂ŃX���[
			continue;
		}

		if (rand() % 8 == 0)
		{
			if (ConvertAsciiToNum(buf[i], line) == 0)
			{
				i++;
			}
		}
	}
}

int save_mpc(int info_num, const MPCINFO *in_info, char *file_name)
{
	FILE *fp;

	int error = fopen_s(&fp, file_name, "w");
	if (error != 0) {
		printf("mpc.dat ���J���܂���B\n");
		return 0;
	}

	for (int i = 0; i < info_num; i++){
		fprintf(fp, "%d\n", in_info[i].depth);
		fprintf(fp, "%d\n", in_info[i].offset);
		fprintf(fp, "%d\n", in_info[i].deviation);
	}

	fclose(fp);
	return 1;
}

void read_mpc_info(MPCINFO *mpc_info, int info_num, char *file_name){

	FILE *fp;
	char filename[32];

	//sprintf_s(filename, 32, "src\\mpc%02d.dat", i);
	sprintf_s(filename, 32, file_name);
	if (fopen_s(&fp, filename, "r")){
		printf("mpc.dat ���J���܂���B\n");
		return;
	}

	memset(mpc_info, 0, sizeof(MPCINFO) * info_num);

	for (int i = 0; i < info_num; i++){
		fscanf_s(fp, "%d", &(mpc_info[i].depth));
		fscanf_s(fp, "%d", &(mpc_info[i].offset));
		fscanf_s(fp, "%d", &(mpc_info[i].deviation));
	}

	fclose(fp);
}


void CulclationMpcValue()
{
	char buf[1000][512];
	UINT64 bk, wh, move, rev;

	int sw_depth[] = { 1, 2, 1, 2, 3, 4, 3, 4, 5, 6, 5, 6, 7, 8, 7, 8, 9, 10, 9, 10, 11, 12 };
	int i, j, byteCounter;
	//int k;
	int num, value_low, value_high;
	int color;
	int mean;
	UINT64 var;
	double div_mean, div_var;

	int elem[60000];

	// ��΃f�[�^�ƕ]���e�[�u���̃��[�h
	BOOL result = LoadData();

	if (result == FALSE)
	{
		printf("ERROR:�]���e�[�u���̓ǂݍ��݂Ɏ��s���܂���\r\n");
		return;
	}

	/* pos�ԍ�-->�w���蕶����ϊ��e�[�u�� */
	char cordinate[4];
	/* �w����̍��W�\�L�ϊ��p */
	for (int idx = 0; idx < 64; idx++)
	{
		sprintf_s(cordinate, "%c%d", idx / 8 + 'a', (idx % 8) + 1);
		strcpy_s(g_cordinates_table[idx], 4, cordinate);
	}

	//save_mpc(0, MPCInfo);
	//read_mpc_info(mpc_info, 5, "src\\mpc.dat");

	//�����̓ǂݍ��ݏ���
	getKifuLine(buf);

	int difficult_table[12];
	difficult_table[0] = 12;
	difficult_table[1] = 14;
	difficult_table[2] = 16;
	difficult_table[3] = 18;
	difficult_table[4] = 18;
	difficult_table[5] = 20;
	difficult_table[6] = 20;
	difficult_table[7] = 22;
	difficult_table[8] = 22;
	difficult_table[9] = 24;
	difficult_table[10] = 24;
	difficult_table[11] = 26;

	//int window[2];
	//int g, wld;

	//int score_array[500], result_array[500];

	CPUCONFIG cpuConfig;

	cpuConfig.bookFlag = 0;
	cpuConfig.bookVariability = 0;
	cpuConfig.casheSize = 1 << 21;
	cpuConfig.color = BLACK;
	cpuConfig.exactDepth = 0;
	cpuConfig.mpcFlag = TRUE;
	cpuConfig.tableFlag = TRUE;
	cpuConfig.winLossDepth = 0;

	for (i = 5; i < 22; i++) {
		num = 0;
		mean = 0;
		var = 0;
		MPC_INFO_NUM = i;
		read_mpc_info(mpcInfo, MPC_INFO_NUM, "src\\mpc.dat");

		//END_MPC_INFO_NUM = i;
		printf("MPC�v�Z�� %d / %d\r\n", i, 15);

		// 500�ǂ̕��U���v�Z
		for (j = 0; j < 500; j++) {

			color = BLACK;
			bk = FIRST_BK;
			wh = FIRST_WH;
			byteCounter = 0;

			// k = 1 �͐΍��f�[�^�������Ă��邽��
			for (byteCounter = 0;; byteCounter++) {

				int empty = RemainSt1ULL(bk, wh);
				if (empty >= (i + 3) + 2)
				{
					cpuConfig.color = color;
					if (color == WHITE)
					{
						swap(&bk, &wh);
					}
					
					cpuConfig.searchDepth = sw_depth[i];
					GetMoveFromAI(bk, wh, empty, &cpuConfig); //low
					value_low = g_evaluation;

					cpuConfig.searchDepth = i + 3;
					GetMoveFromAI(bk, wh, empty, &cpuConfig); // high
					value_high = g_evaluation;

					elem[num] = (value_high - value_low); // ��������
					mean += (value_high - value_low);
					num++;
					if (color == WHITE)
					{
						swap(&bk, &wh);
					}

				}
				else
				{
					break;
				}

				move = buf[j][byteCounter];
				if (move == -1)
				{
					printf("assert!!! move = -1!!\n");
					continue;
				}
				if (color == BLACK)
				{
					rev = GetRev[move](bk, wh);
					/* ���p�X�H */
					if (rev == 0)
					{
						rev = GetRev[move](wh, bk);
						/* �ꉞ���@��ɂȂ��Ă��邩�̃`�F�b�N */
						if (rev == 0)
						{
							printf("assert!!! END GAME before culc have finished!!\n");
							break;
						}
						bk ^= rev;
						wh ^= ((1ULL << move) | rev);
					}
					else
					{
						bk ^= ((1ULL << move) | rev);
						wh ^= rev;
						color ^= 1;
					}
				}
				else
				{
					rev = GetRev[move](wh, bk);
					/* ���p�X�H */
					if (rev == 0)
					{
						rev = GetRev[move](bk, wh);
						/* �ꉞ���@��ɂȂ��Ă��邩�̃`�F�b�N */
						if (rev == 0)
						{
							printf("assert!!! END GAME before culc have finished!!\n");
							break;
						}
						bk ^= ((1ULL << move) | rev);
						wh ^= rev;
					}
					else
					{
						bk ^= rev;
						wh ^= ((1ULL << move) | rev);
						color ^= 1;
					}
				}
				//byteCounter += 2;

			}

			if (j % 10 == 0){
				div_mean = mean / (double)num;
				div_var = 0;
				for (int ii = 0; ii < num; ii++)
				{
					div_var += (elem[ii] - div_mean) * (elem[ii] - div_mean);
				}
				div_var = sqrt(div_var / (double)num);
				printf("game:%d, prob depth:%d/%d, offset:%d, deviation:%d\n", j,
				((i + MPC_DEPTH_MIN) & 1) + 2 * ((i + MPC_DEPTH_MIN) / 4),
				i + MPC_DEPTH_MIN, (int)(div_mean), (int)(div_var));
				//printf("game:%d, prob depth:%d/%d, offset:%d, deviation:%d\n", j,
				//	sharrow_level[i],
				//	i + END_MPC_DEPTH_MIN, (int)(div_mean), (int)(div_var));

			}

		}

		div_mean = mean / (double)num;
		div_var = 0;
		for (int ii = 0; ii < num; ii++)
		{
			div_var += (elem[ii] - div_mean) * (elem[ii] - div_mean);
		}
		div_var = sqrt(div_var / (double)num);
		mpcInfo[i].depth = sw_depth[i];
		mpcInfo[i].offset = (int)div_mean;
		mpcInfo[i].deviation = (int)div_var;
		save_mpc(i + 1, mpcInfo, "src\\mpc.dat");
	}

	//culcExp(score_array, result_array);

	printf("�v�Z�������܂���\n");

}