#include "tMutex.h"
#include "myRtos.h"

// 初始化互斥信号量
void tMutexInit(tMutex *mutex)
{
    tEventInit(&mutex->event, tEventTypeMutex);

    mutex->lockedCount = 0;
    mutex->owner = (tTask *)0;
    // 拥有者的原始优先级
    mutex->ownerOriginalPrio = TINYOS_PRO_COUNT;
}

// 等待信号量
uint32_t tMutexWait(tMutex *mutex, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    if (mutex->lockedCount <= 0)
    {
        // 如果没有锁定，则使用当前任务锁定
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockedCount++;

        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        // 信号量已经被锁定
        if (mutex->owner == currentTask)
        {
            // 如果是信号量的拥有者再次wait，简单增加计数
            mutex->lockedCount++;

            tTaskExitCritical(status);
            return tErrorNoError;
        }
        else
        {
            // 如果是信号量拥有者之外的任务wait，则要检查下是否需要使用
            // 优先级继承方式处理
            if (currentTask->prio < mutex->owner->prio)
            {
                tTask *owner = mutex->owner;

                // 如果当前任务的优先级比拥有者优先级更高，则使用优先级继承
                // 提升原拥有者的优先
                if (owner->state == TINYOS_TASK_STATE_RDY)
                {
                    // 任务处于就绪状态时，更改任务在就绪表中的位置
                    tTaskSchedUnRdy(owner);
                    owner->prio = currentTask->prio;
                    tTaskSchedRdy(owner);
                }
                else
                {
                    // 其它状态，只需要修改优先级
                    owner->prio = currentTask->prio;
                }
            }

            // 当前任务进入等待队列中
            tEventWait(&mutex->event, currentTask, (void *)0, tEventTypeMutex, waitTicks);
            tTaskExitCritical(status);

            // 执行调度， 切换至其它任务
            tTaskSched();
            return currentTask->waitEventResult;
        }
    }
}

//  获取信号量，如果已经被锁定，立即返回
uint32_t tMutexNoWaitGet(tMutex *mutex)
{
    uint32_t status = tTaskEnterCritical();

    if (mutex->lockedCount <= 0)
    {
        // 如果没有锁定，则使用当前任务锁定
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockedCount++;

        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        // 信号量已经被锁定
        if (mutex->owner == currentTask)
        {
            // 如果是信号量的拥有者再次wait，简单增加计数
            mutex->lockedCount++;

            tTaskExitCritical(status);
            return tErrorNoError;
        }

        tTaskExitCritical(status);
        return tErrorResourceUnavaliable;
    }
}

//  通知互斥信号量可用
uint32_t tMutexNotify(tMutex *mutex)
{
    uint32_t status = tTaskEnterCritical();

    if (mutex->lockedCount <= 0)
    {
        // 锁定计数为0，信号量未被锁定，直接退出
        tTaskExitCritical(status);
        return tErrorNoError;
    }

    if (mutex->owner != currentTask)
    {
        // 不是拥有者释放，认为是非法
        tTaskExitCritical(status);
        return tErrorOwner;
    }

    if (--mutex->lockedCount > 0)
    {
        // 减1后计数仍不为0, 直接退出，不需要唤醒等待的任务
        tTaskExitCritical(status);
        return tErrorNoError;
    }

    // 是否有发生优先级继承
    if (mutex->ownerOriginalPrio != mutex->owner->prio)
    {
        // 有发生优先级继承，恢复拥有者的优先级
        if (mutex->owner->state == TINYOS_TASK_STATE_RDY)
        {
            // 任务处于就绪状态时，更改任务在就绪表中的位置
            tTaskSchedUnRdy(mutex->owner);
            currentTask->prio = mutex->ownerOriginalPrio;
            tTaskSchedRdy(mutex->owner);
        }
        else
        {
            // 其它状态，只需要修改优先级
            currentTask->prio = mutex->ownerOriginalPrio;
        }
    }

    // 检查是否有任务等待
    if (tEventWaitCount(&mutex->event) > 0)
    {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        tTask *task = tEventWakeUp(&mutex->event, (void *)0, tErrorNoError);

        mutex->owner = task;
        mutex->ownerOriginalPrio = task->prio;
        mutex->lockedCount++;

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    tTaskExitCritical(status);
    return tErrorNoError;
}
// 销毁信号量,销毁等待队列中的所有任务
uint32_t tMutexDestroy(tMutex *mutex)
{
    uint32_t count = 0;
    uint32_t status = tTaskEnterCritical();

    // 信号量是否已经被锁定，未锁定时没有任务等待，不必处理
    if (mutex->lockedCount > 0)
    {
        // 是否有发生优先级继承?如果发生，需要恢复拥有者的原优先级
        if (mutex->ownerOriginalPrio != mutex->owner->prio)
        {
            // 有发生优先级继承，恢复拥有者的优先级
            if (mutex->owner->state == TINYOS_TASK_STATE_RDY)
            {
                // 任务处于就绪状态，
                // 修改任务在就绪表中的位置，恢复优先级位置
                tTaskSchedUnRdy(mutex->owner);
                mutex->owner->prio = mutex->ownerOriginalPrio;
                tTaskSchedRdy(mutex->owner);
            }
            else
            {
                // 其它状态，只需要修改优先级
                mutex->owner->prio = mutex->ownerOriginalPrio;
            }
        }

        // 然后，清空事件控制块中的任务
        count = tEventRemoveAll(&mutex->event, (void *)0, tErrorDel);

        // 清空过程中可能有任务就绪，执行一次调度
        if (count > 0)
        {
            tTaskSched();
        }
    }

    tTaskExitCritical(status);
    return count;
}

// 查询状态信息
void tMutexGetInfo(tMutex *mutex, tMutexInfo *info)
{
    uint32_t status = tTaskEnterCritical();

    // 拷贝需要的信息
    info->taskCount = tEventWaitCount(&mutex->event);
    info->ownerPrio = mutex->ownerOriginalPrio;
    if (mutex->owner != (tTask *)0)
    {
        info->inheritedPrio = mutex->owner->prio;
    }
    else
    {
        info->inheritedPrio = TINYOS_PRO_COUNT;
    }
    info->owner = mutex->owner;
    info->lockedCount = mutex->lockedCount;

    tTaskExitCritical(status);
}
