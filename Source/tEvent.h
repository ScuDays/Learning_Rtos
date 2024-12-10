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

// 在指定事件控制块上等待事件发生
// 若等待超时，则不等了，继续恢复运行
void tEventWait(tEvent *event, tTask *task, void *msg, uint32_t state, uint32_t timeout);

// 从特定事件控制块中唤醒首个等待的任务
tTask *tEventWakeUp(tEvent *event, void *msg, uint32_t result);

// 从事件控制块中唤醒指定任务
void tEventWakeUpSpecific(tEvent *event, tTask *task, void *msg, uint32_t result);

// 将任务从其等待队列中强制移除
void tEventRemoveTask(tTask *task, void *msg, uint32_t result);

// 清除所有等待中的任务，将事件发送给所有任务
uint32_t tEventRemoveAll(tEvent *event, void *msg, uint32_t result);

// 返回事件控制块中等待的任务数量
uint32_t tEventWaitCount(tEvent *event);

#endif
