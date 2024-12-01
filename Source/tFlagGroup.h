#ifndef TFLAGGROUP_H
#define TFLAGGROUP_H

#include "tConfig.h"
#include "tEvent.h"

// 是否为0
#define TFLAGGROUP_CLEAR (0x0 << 0)
// 是否置1
#define TFLAGGROUP_SET (0x1 << 0)
// 任意一位满足
#define TFLAGGROUP_ANY (0x0 << 1)
// 全部位满足
#define TFLAGGROUP_ALL (0x1 << 1)

// 请求的flags中全部位置1
#define TFLAGGROUP_SET_ALL (TFLAGGROUP_SET | TFLAGGROUP_ALL)
// 请求的flags中任意一位置1
#define TFLAGGROUP_SET_ANY (TFLAGGROUP_SET | TFLAGGROUP_ANY)
// 请求的flags中全部位为0
#define TFLAGGROUP_CLEAR_ALL (TFLAGGROUP_CLEAR | TFLAGGROUP_ALL)
// 请求的flags中任意一位为0
#define TFLAGGROUP_CLEAR_ANY (TFLAGGROUP_CLEAR | TFLAGGROUP_ANY)
// 是否要消耗掉对应位
#define TFLAGGROUP_CONSUME (0x1 << 7)

// 事件标志组
typedef struct _tFlagGroup
{
    // 事件控制块
    tEvent event;
    // 事件标志
    uint32_t flags;
} tFlagGroup;

// 事件标志组信息
typedef struct _tFlagGroupInfo
{
    // 当前的事件标志
    uint32_t flags;

    // 当前等待的任务计数
    uint32_t taskCount;
} tFlagGroupInfo;

// 初始化事件标志组
void tFlagGroupInit(tFlagGroup *flagGroup, uint32_t flags);

// 等待事件标志组中特定的标志
uint32_t tFlagGroupWait(tFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag,
                        uint32_t *resultFlag, uint32_t waitTicks);

// 获取事件标志组中特定的标志,若不符合，返回
uint32_t tFlagGroupNoWaitGet(tFlagGroup *flagGroup, uint32_t waitType, uint32_t requstFlag, uint32_t *requestFlag);

// 通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
void tFlagGroupNotify(tFlagGroup *flagGroup, uint8_t isSet, uint32_t flags);

// 查询事件标志组的状态信息
void tFlagGroupGetInfo(tFlagGroup *flagGroup, tFlagGroupInfo *info);

// 销毁事件标志组,清除等待的任务
uint32_t tFlagGroupDestroy(tFlagGroup *flagGroup);
#endif
