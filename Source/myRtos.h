#ifndef MYRTOS_H
#define MYRTOS_H

#include <stdint.h>
// 内核库文件
#include "tLib.h"
#include "tConfig.h"

#include "tTask.h"

#include "tEvent.h"
#include "tSem.h"
#include "tMbox.h"
#include "tMemBlock.h"
#include "tMutex.h"
#include "tTimer.h"

// 错误码
typedef enum _tError
{
    tErrorNoError = 0,         // 没有错误
    tErrorTimeout,             // 等待超时
    tErrorResourceUnavaliable, // 等待超时
    tErrorDel,                 // 被删除
    tErrorResourceFull,        // 邮箱资源满了
    tErrorOwner,               // 该进程不拥有该互斥量
} tError;
extern tTask *currentTask;
extern tTask *nextTask;

uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);

void tTaskRunFirst(void);
void tTaskSwitch(void);

// 初始化任务调度锁
void tTaskSchedInit(void);

// 禁止任务调度
void tTaskSchedDisable(void);
// 允许任务调度
void tTaskSchedEnable(void);
// 插入到就绪列表，在位图中设置该任务的位
void tTaskSchedRdy(tTask *task);
void tTaskSchedUnRdy(tTask *task);


void tTaskSched(void);



void tTimeTaskWait(tTask *task, uint32_t ticks);
void tTimeTaskWakeUp(tTask *task);


void tTaskSystemTickHandler(void);
void tTaskDelay(uint32_t delay);

void tSetSysTickPeriod(uint32_t ms);
void tInitApp(void);

void tTimeTaskRemove(tTask *task);
void tTaskSchedRemove(tTask *task);

#endif
