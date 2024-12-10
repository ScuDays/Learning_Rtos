#include "myRtos.h"
// 初始化事件控制块
void tEventInit(tEvent *event, tEventType type)
{
    event->type = type;
    tListInit(&event->waitList);
}

// 在指定事件控制块上等待事件发生
// 若等待超时，则不等了，继续恢复运行
void tEventWait(tEvent *event, tTask *task, void *msg, uint32_t state, uint32_t timeout)
{
    uint32_t status = tTaskEnterCritical();

    // 标记任务处于等待某种事件的状态
    // state 16位之后为事件等待标志
    task->state |= state << 16;
    // 设置任务等待的事件结构
    task->waitEvent = event;
    task->eventMsg = msg;
    task->waitEventResult = tErrorNoError;
    // 将任务从就绪队列中移除
    tTaskSchedUnRdy(task);
    // 将任务插入到等待队列中
    tListAddLast(&event->waitList, &task->linkNode);

    if (timeout)
    {
        tTimeTaskWait(task, timeout);
    }

    tTaskExitCritical(status);
}
// 从特定事件控制块中唤醒首个等待的任务
tTask *tEventWakeUp(tEvent *event, void *msg, uint32_t result)
{
    tNode *node;
    tTask *task = (tTask *)0;

    uint32_t status = tTaskEnterCritical();

    if ((node = tListRemoveFirst(&event->waitList)) != (tNode *)0)
    {
        task = (tTask *)tNodeParent(node, tTask, linkNode);
        task->waitEvent = (tEvent *)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~MYRTOS_TASK_WAIT_MASK;

        if (task->delayTicks != 0)
        {
            tTimeTaskWakeUp(task);
        }

        tTaskSchedRdy(task);
    }

    tTaskExitCritical(status);

    return task;
}

// 从事件控制块中唤醒指定任务
void tEventWakeUpSpecific(tEvent *event, tTask *task, void *msg, uint32_t result)
{
    // 进入临界区
    uint32_t status = tTaskEnterCritical();

    tListRemove(&event->waitList, &task->linkNode);

    // 设置收到的消息、结构，清除相应的等待标志位
    task->waitEvent = (tEvent *)0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~MYRTOS_TASK_WAIT_MASK;
    // 任务申请了超时等待，这里检查下，将其从延时队列中移除
    if (task->delayTicks != 0)
    {
        tTimeTaskWakeUp(task);
    }
    // 将任务加入就绪队列
    tTaskSchedRdy(task);
    // 退出临界区
    tTaskExitCritical(status);
}

// 将任务从其等待队列中强制移除
void tEventRemoveTask(tTask *task, void *msg, uint32_t result)
{
    uint32_t status = tTaskEnterCritical();

    tListRemove(&task->waitEvent->waitList, &task->linkNode);
    task->waitEvent = (tEvent *)0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    // 清除标志位
    task->state &= ~MYRTOS_TASK_WAIT_MASK;

    tTaskExitCritical(status);
}

// 清除所有等待中的任务，将事件发送给所有任务
uint32_t tEventRemoveAll(tEvent *event, void *msg, uint32_t result)
{
    tNode *node;
    uint32_t count;

    // 进入临界区
    uint32_t status = tTaskEnterCritical();

    // 获取等待中的任务数量
    count = tListCount(&event->waitList);

    // 遍历所有等待中的任务
    while ((node = tListRemoveFirst(&event->waitList)) != (tNode *)0)
    {
        // 转换为相应的任务结构
        tTask *task = (tTask *)tNodeParent(node, tTask, linkNode);

        // 设置消息、结构，清除相应的等待标志位
        task->waitEvent = (tEvent *)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~MYRTOS_TASK_WAIT_MASK;

        // 任务申请了超时等待，这里检查下，将其从延时队列中移除
        if (task->delayTicks != 0)
        {
            tTimeTaskWakeUp(task);
        }

        // 将任务加入就绪队列
        tTaskSchedRdy(task);
    }

    // 退出临界区
    tTaskExitCritical(status);

    return count;
}

// 返回事件控制块中等待的任务数量
uint32_t tEventWaitCount(tEvent *event)
{
    uint32_t count = 0;

    // 进入临界区
    uint32_t status = tTaskEnterCritical();

    count = tListCount(&event->waitList);

    // 退出临界区
    tTaskExitCritical(status);

    return count;
}
