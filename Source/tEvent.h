#ifndef TEVENT_H
#define TEVENT_H

#include "tLib.h"
#include "tTask.h"
// Event类型
typedef enum _tEventType
{
    tEventTypeUnknown = 0,   // 未知类型
    tEventTypeSem = 1,       // 等待信号量
    tEventTypeMbox = 2,      // 邮箱
    tEventTypeMemBlock = 3,  // 内存块
    tEventTypeFlagGroup = 4, // 标志组
    tEventTypeMutex = 5,     //  互斥信号量
} tEventType;

// Event控制结构
typedef struct _tEvent
{
    tEventType type; // 代表事件块用于什么类型

    tList waitList; // 任务等待列表
} tEvent;

// 初始化事件控制块
void tEventInit(tEvent *event, tEventType type);
void tEventWait(tEvent *event, tTask *task, void *msg, uint32_t state, uint32_t timeout);

tTask *tEventWakeUp(tEvent *event, void *msg, uint32_t result);
void tEventWakeUpSpecific(tEvent *event, tTask *task, void *msg, uint32_t result);

void tEventRemoveTask(tTask *task, void *msg, uint32_t result);
uint32_t tEventRemoveAll(tEvent *event, void *msg, uint32_t result);
uint32_t tEventWaitCount(tEvent *event);

#endif
