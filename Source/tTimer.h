#ifndef TTIMER_H
#define TTIMER_H

#include "tConfig.h"
#include "tEvent.h"
// 定时器状态
typedef enum _tTimerState
{
    tTimerCreated,  // 定时器已经创建
    tTimerStarted,  // 定时器已经启动
    tTimerRunning,  // 定时器正在执行回调函数
    tTimerStopped,  // 定时器已经停止
    tTimerDestroyed // 定时器已经销毁
} tTimerState;

// 软定时器结构
typedef struct _tTimer
{
    // 链表结点
    tNode linkNode;

    // 初次启动延后的ticks数
    uint32_t startDelayTicks;

    // 周期定时时的周期tick数
    uint32_t durationTicks;

    // 当前定时递减计数值
    uint32_t delayTicks;

    // 定时回调函数
    void (*timerFunc)(void *arg);

    // 传递给回调函数的参数
    void *arg;

    // 定时器配置参数
    uint32_t config;

    // 定时器状态
    tTimerState state;
} tTimer;
// 软定时器状态
typedef struct _tTimerInfo
{
    // 初次启动延后的ticks数
    uint32_t startDelayTicks;

    // 周期定时时的周期tick数
    uint32_t durationTicks;

    // 定时回调函数
    void (*timerFunc)(void *arg);

    // 传递给回调函数的参数
    void *arg;

    // 定时器配置参数
    uint32_t config;

    // 定时器状态
    tTimerState state;
} tTimerInfo;

// 软硬定时器
// 硬的，在systick中处理，延迟小，要求处理函数耗时短，避免中断处理函数事件过长。
#define TIMER_CONFIG_TYPE_HARD (1 << 0)
// 软的，在定时器任务中处理，延迟大，处理函数可以耗时较长。
#define TIMER_CONFIG_TYPE_SOFT (0 << 0)

// delayTicks：定时器初始启动的延时ticks数
// durationTicks 给周期性定时器用的周期tick数
void tTimerInit(tTimer *timer, uint32_t delayTicks, uint32_t durationTicks,
                void (*timerFunc)(void *arg), void *arg, uint32_t config);

// 启动定时器
void tTimerStart(tTimer *timer);

// 终止定时器
void tTimerStop(tTimer *timer);

// 通知定时模块进行处理
void tTimerModuleTickNotify(void);

// 定时器模块初始化
void tTimerModuleInit(void);

// 销毁定时器
void tTimerDestroy(tTimer *timer);

// 查询信息
void tTimerGetInfo(tTimer *timer, tTimerInfo *info);
#endif
