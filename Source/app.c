#include "myRtos.h"

// 任务测试文件
tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack task4Env[1024];


void timerFunc (void * arg)
{

}

int task1Flag;
void task1Entry (void * param)
{

}

int task2Flag;
void task2Entry (void * param) 
{

}

int task3Flag;
void task3Entry (void * param) 
{

}

int task4Flag;
void task4Entry (void * param) 
{

}

// 初始化应用
void tInitApp (void) 
{
    // 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
    tTaskInit(&tTask1, task1Entry, (void *)0x11111111, 0, &task1Env[1024]);
    tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);
    tTaskInit(&tTask3, task3Entry, (void *)0x33333333, 1, &task3Env[1024]);
    tTaskInit(&tTask4, task4Entry, (void *)0x44444444, 1, &task4Env[1024]);
}
