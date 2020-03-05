
#include "cpu.h"

extern int MPC_INFO_NUM;
extern int MPC_INFO_NUM_END;

void CulclationMpcValue();
void CulclationMpcValue_End();
int save_mpc(int info_num, const MPCINFO *in_info, char *file_name);