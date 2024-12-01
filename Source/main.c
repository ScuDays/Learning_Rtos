#include "myRtos.h"
#include "ARMCM3.h"
tTask *currentTask;
tTask *nextTask;

tBitmap taskPrioBitmap;
tList taskTable[TINYOS_PRO_COUNT];

// 延时队列
tList tTaskDelayedList;
uint8_t schedLockCount;

tTask *tTaskHighestReady(void)
{
    uint32_t higestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
    tNode *node = tListFirst(&(taskTable[higestPrio]));
    return tNodeParent(node, tTask, linkNode);
}

// 初始化任务调度锁
void tTaskSchedInit(void)
{
    schedLockCount = 0;
    tBitmapInit(&taskPrioBitmap);
    for (int i = 0; i < TINYOS_PRO_COUNT; i++)
    {
        tListInit(&taskTable[i]);
    }
}

// 禁止任务调度
void tTaskSchedDisable(void)
{
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount < 255)
    {
        schedLockCount++;
    }

    tTaskExitCritical(status);
}

// 允许任务调度
void tTaskSchedEnable(void)
{
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount > 0)
    {
        if (--schedLockCount == 0)
        {
            tTaskSched();
        }
    }
    tTaskExitCritical(status);
}

// 初始化延时数组
void tTaskDelayedInit(void)
{
    tListInit(&tTaskDelayedList);
}

// 插入到延时列表，设置任务进入延时状态
void tTimeTaskWait(tTask *task, uint32_t ticks)
{
    task->delayTicks = ticks;
    tListAddLast(&tTaskDelayedList, &(task->delayNode));
    task->state |= TINYOS_TASK_STATE_DELAYED;
}

// 从延时列表移除，设置任务进入唤醒状态
void tTimeTaskWakeUp(tTask *task)
{
    tListRemove(&tTaskDelayedList, &(task->delayNode));
    task->state &= ~TINYOS_TASK_STATE_DELAYED;
}
// 将延时的任务从延时队列中移除
void tTimeTaskRemove(tTask *task)
{
    tListRemove(&tTaskDelayedList, &(task->delayNode));
}
// 插入到就绪列表，在位图中设置该任务的位
void tTaskSchedRdy(tTask *task)
{
    tListAddFirst(&(taskTable[task->prio]), &(task->linkNode));
    tBitmapSet(&taskPrioBitmap, task->prio);
}
// 从就绪列表移除，在位图中移除该任务的位
void tTaskSchedUnRdy(tTask *task)
{
    tListRemove(&taskTable[task->prio], &(task->linkNode));
    // 如果该优先级中没有别的任务了，则去掉bitmap中的位
    if (tListCount(&taskTable[task->prio]) == 0)
    {
        tBitmapClear(&taskPrioBitmap, task->prio);
    }
}
// 将任务从就绪列表中移除
// 和tTaskSchedUnRdy()不是一样的吗？
// 为了扩展性
void tTaskSchedRemove(tTask *task)
{
    tListRemove(&taskTable[task->prio], &(task->linkNode));

    if (tListCount(&taskTable[task->prio]) == 0)
    {
        tBitmapClear(&taskPrioBitmap, task->prio);
    }
}
void tTaskSystemTickHandler()
{

    uint32_t status = tTaskEnterCritical();

    tNode *node;
    for (node = tTaskDelayedList.headNode.nextNode; node != &(tTaskDelayedList.headNode); node = node->nextNode)
    {
        tTask *task = tNodeParent(node, tTask, delayNode);
        if (--task->delayTicks == 0)
        {
            // 如果该任务在等待某个事件，
            // 并且延时到期，则从这个事件块中删除
            if (task->waitEvent)
            {
                tEventRemoveTask(task, (void *)0, tErrorTimeout);
            }

            tTimeTaskWakeUp(task);

            tTaskSchedRdy(task);
        }
    }
    if (--currentTask->slice == 0)
    {
        if (tListCount(&taskTable[currentTask->prio]) > 0)
        {
            tListRemoveFirst(&taskTable[currentTask->prio]);
            tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));

            currentTask->slice = TINYOS_SLICE_MAX;
        }
    }
    tTaskExitCritical(status);
    tTimerModuleTickNotify();
    tTaskSched();
}

void tTaskSched()
{
    uint32_t status = tTaskEnterCritical();

    // 空闲任务直接返回
    if (schedLockCount > 0)
    {
        tTaskExitCritical(status);
        return;
    }
    // 查看最高优先级的任务
    // 若与当前任务相同，则返回
    // 不同，则调度。
    tTask *tempTask = tTaskHighestReady();
    if (currentTask != tempTask)
    {
        nextTask = tempTask;
        tTaskSwitch();
    }

    tTaskExitCritical(status);
}

tTask tTaskIdle;
tTaskStack taskIdleEnv[TINYOS_IDLETASK_STACK_SIZE];

tTask *idleTask;
void idleTaskEntry(void *param)
{
    for (;;)
    {
    }
}

int main()
{
    tTaskSchedInit();
    tTaskDelayedInit();

    tInitApp();
    // 初始化定时器模块
    tTimerModuleInit();
    tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0x2222222, TINYOS_PRO_COUNT - 1, &taskIdleEnv[TINYOS_IDLETASK_STACK_SIZE]);

    idleTask = &tTaskIdle;

    nextTask = tTaskHighestReady();
    tTaskRunFirst();
    return 0;
}
