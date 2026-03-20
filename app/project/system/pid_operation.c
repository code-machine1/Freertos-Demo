#include "PID_operation.h"

PID s1_210_pid;
PID s1_245_pid;
PID s2_210_pid;
PID s2_245_pid;

volatile float abs_limit(volatile float value, volatile float ABS_MAX) // �����޷����������ֵ��
{
    if (value > ABS_MAX)
    {
        value = ABS_MAX;
    }
    if (value < -ABS_MAX)
    {
        value = -ABS_MAX;
    }

    return value;
}


float PID_Position_Calc(PID *pid, float Target_val, float Actual_val) 
{
    pid->Error = Target_val - Actual_val; 

    pid->SumError += pid->Error; 

    if (pid->SumError >= pid->Integralmax)
    {
        pid->SumError = pid->Integralmax;
    }
    if (pid->SumError <= 0)
    {
        pid->SumError = 0;
    }

    pid->DError = pid->Error - pid->LastError; 
    pid->output = pid->Kp * pid->Error +
                  abs_limit(pid->Ki * pid->SumError, pid->Integralmax) +
                  pid->Kd * pid->DError;
    pid->LastError = pid->Error; 

    if (pid->output > pid->outputmax)
    {
        pid->output = pid->outputmax;
    }

    if (pid->output <= 0)
    {
        pid->output = 0;
    }

    return pid->output; 
}


void PID_Init(PID *pid, float Kp, float Ki, float Kd, float Limit_value)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->PrevError = pid->LastError = pid->Error = pid->SumError = pid->output = 0;
    pid->Integralmax = Limit_value;
    pid->outputmax = Limit_value;
}

void PID_Clear(PID *pid)
{
    pid->PrevError = pid->LastError = pid->Error = pid->SumError = pid->output = 0;
}

void PID_Clear_I(PID *pid)
{
 
	 pid->SumError = 0;
}
