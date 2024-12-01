#include "tFlagGroup.h"
#include "myRtos.h"
// 初始化事件标志组
void tFlagGroupInit(tFlagGroup *flagGroup, uint32_t flags)
{
    tEventInit(&flagGroup->event, tEventTypeFlagGroup);
    flagGroup->flags = flags;
}

// 辅助函数。检查并消耗掉事件标志
static uint32_t tFlagGroupCheckAndConsume(tFlagGroup *flagGroup, uint32_t type, uint32_t *flags)
{
    // 获取flags
    uint32_t srcFlags = *flags;
    uint32_t isSet = type & TFLAGGROUP_SET;
    uint32_t isAll = type & TFLAGGROUP_ALL;
    uint32_t isConsume = type & TFLAGGROUP_CONSUME;

    // 有哪些类型的标志位出现
    // flagGroup->flags & flags：计算出哪些位为1
    // ~flagGroup->flags & flags:计算出哪位为0
    uint32_t calcFlag;
    if (isSet)
    {
        calcFlag = flagGroup->flags & srcFlags;
    }
    else
    {
        calcFlag = ~flagGroup->flags & srcFlags;
    }
    // 所有标志位出现 || 任意标志位出现，满足条件
    if (((isAll != 0) && (calcFlag == srcFlags)) || ((isAll == 0) && (calcFlag != 0)))
    {
        // 是否消耗掉标志位
        if (isConsume)
        {
            if (isSet)
            {
                // 清除为1的标志位，变成0
                flagGroup->flags &= ~srcFlags;
            }
            else
            {
                // 清除为0的标志位，变成1
                flagGroup->flags |= srcFlags;
            }
        }
        // 修改flags为计算出来的标志位，方便任务查看到底哪些条件被满足了
        *flags = calcFlag;
        return tErrorNoError;
    }

    *flags = calcFlag;
    return tErrorResourceUnavaliable;
}

// 等待事件标志组中特定的标志
uint32_t tFlagGroupWait(tFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag,
                        uint32_t *resultFlag, uint32_t waitTicks)
{
    uint32_t result;
    uint32_t flags = requestFlag;

    uint32_t status = tTaskEnterCritical();
    result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    // 如果不满足
    if (result != tErrorNoError)
    {
        // 如果事件标志不满足条件，则插入到等待队列中
        currentTask->waitFlagsType = waitType;
        currentTask->eventFlags = requestFlag;
        tEventWait(&flagGroup->event, currentTask, (void *)0, tEventTypeFlagGroup, waitTicks);
        tTaskExitCritical(status);
        // 再执行一次事件调度，以便于切换到其它任务
        tTaskSched();

        *resultFlag = currentTask->eventFlags;
        result = currentTask->waitEventResult;
    }
    else
    {
        *resultFlag = flags;
        tTaskExitCritical(status);
    }
    return result;
}

// 获取事件标志组中特定的标志,若不符合，直接返回，不做等待
uint32_t tFlagGroupNoWaitGet(tFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag)
{
    uint32_t flags = requestFlag;

    uint32_t status = tTaskEnterCritical();
    uint32_t result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    tTaskExitCritical(status);

    *resultFlag = flags;
    return status;
}

// 通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
void tFlagGroupNotify(tFlagGroup *flagGroup, uint8_t isSet, uint32_t flags)
{
    tList *waitList;
    tNode *node;
    tNode *nextNode;
    uint8_t sched = 0;

    uint32_t status = tTaskEnterCritical();

    if (isSet)
    {
        flagGroup->flags |= flags; // 置1事件
    }
    else
    {
        flagGroup->flags &= ~flags; // 清0事件
    }

    // 遍历所有的等待任务, 获取满足条件的任务，加入到待移除列表中
    waitList = &flagGroup->event.waitList;
    for (node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode)
    {
        uint32_t result;
        tTask *task = tNodeParent(node, tTask, linkNode);
        uint32_t flags = task->eventFlags;
        nextNode = node->nextNode;

        // 检查标志
        result = tFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &flags);
        if (result == tErrorNoError)
        {
            // 唤醒任务
            task->eventFlags = flags;
            tEventWakeUpSpecific(&flagGroup->event, task, (void *)0, tErrorNoError);
            sched = 1;
        }
    }

    // 如果有任务就绪，则执行一次调度
    if (sched)
    {
        tTaskSched();
    }

    tTaskExitCritical(status);
}
// 查询事件标志组的状态信息
void tFlagGroupGetInfo(tFlagGroup *flagGroup, tFlagGroupInfo *info)
{
    uint32_t status = tTaskEnterCritical();

    // 拷贝需要的信息
    info->flags = flagGroup->flags;
    info->taskCount = tEventWaitCount(&flagGroup->event);

    tTaskExitCritical(status);
}

// 销毁事件标志组,,清除等待事件
uint32_t tFlagGroupDestroy(tFlagGroup *flagGroup)
{
    uint32_t status = tTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = tEventRemoveAll(&flagGroup->event, (void *)0, tErrorDel);

    tTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}
