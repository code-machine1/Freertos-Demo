#ifndef	__PID_OPERATION_H__
#define __PID_OPERATION_H__
#include "at32f403a_407.h"

typedef struct PID {
		float  Kp;         //  Proportional Const  Pϵ��
		float  Ki;           //  Integral Const      Iϵ��
		float  Kd;         //  Derivative Const    Dϵ��
		
		volatile float  PrevError ;          //  Error[-2]  
		volatile float  LastError;          //  Error[-1]  
		volatile float  Error;              //  Error[0 ]  
		volatile float  DError;            //pid->Error - pid->LastError	
		volatile float  SumError;           //  Sums of Errors  
		
		volatile float  output;
		
		volatile float  Integralmax;      //����������ֵ
		volatile float  outputmax;        //���������ֵ
} PID;

extern PID s1_210_pid;
extern PID s1_245_pid;
extern PID s2_210_pid;
extern PID s2_245_pid;
void PID_Init(PID *pid, float Kp , float Ki , float Kd , float Limit_value);
void PID_Clear(PID *pid);
void PID_Clear_I(PID *pid);
float PID_Position_Calc(PID *pid, float Target_val, float Actual_val);
#endif
