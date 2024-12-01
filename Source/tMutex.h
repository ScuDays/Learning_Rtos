#ifndef TMUTEX_H
#define TMUTEX_H

#include "tConfig.h"
#include "tEvent.h"

// 互斥信号量类型
typedef struct _tMutex
{
    // 事件控制块
    tEvent event;
    // 已被请求的的次数
    uint32_t lockedCount;
    // 信号量目前的拥有者
    tTask *owner;
    // 拥有者原始的优先级,这个属性是为了后面实现避免优先级翻转的问题
    uint32_t ownerOriginalPrio;
} tMutex;

// 互斥信号量信息
typedef struct _tMutexInfo
{
    // 等待的任务数量
    uint32_t taskCount;
    // 拥有者任务的优先级
    uint32_t ownerPrio;
    // 继承优先级
    uint32_t inheritedPrio;
    // 当前信号量的拥有者
    tTask *owner;
    // 锁定次数
    uint32_t lockedCount;
} tMutexInfo;
// 初始化互斥信号量
void tMutexInit(tMutex *mutex);

// 等待信号量
uint32_t tMutexWait(tMutex *mutex, uint32_t waitTicks);
//  获取信号量，如果已经被锁定，立即返回
uint32_t tMutexNoWaitGet(tMutex *mutex);
//  通知互斥信号量可用
uint32_t tMutexNotify(tMutex *mutex);
// 销毁信号量,销毁等待队列中的所有任务
uint32_t tMutexDestroy(tMutex *mutex);
// 查询状态信息
void tMutexGetInfo(tMutex *mutex, tMutexInfo *info);

#endif
