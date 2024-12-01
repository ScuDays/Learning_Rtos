#include "tMemBlock.h"
#include "myRtos.h"

//  初始化存储控制块
//  memStart:存储区的起始地址
void tMemBlockInit(tMemBlock *memBlock, uint8_t *memStart, uint32_t blockSize, uint32_t blockCnt)
{
    uint8_t *memBlockStart = (uint8_t *)memStart;
    uint8_t *memBlockEnd = memBlockStart + blockSize * blockCnt;

    // 每个存储块需要来放置链接指针，所以空间至少要比tNode大
    // 即便如此，实际用户可用的空间并没有少
    if (blockSize < sizeof(tNode))
    {
        return;
    }
    // 初始化事件块
    tEventInit(&memBlock->event, tEventTypeMemBlock);

    memBlock->memStart = memStart;
    memBlock->blockSize = blockSize;
    memBlock->maxCount = blockCnt;

    tListInit(&memBlock->blockList);
    while (memBlockStart < memBlockEnd)
    {
        // memBlockStart指向的地址被初始化为一个tNode节点来维护内存块
        // 在维护的时候会占用一个tNode结构体大小的内存
        // 但是在实际使用的时候，内存块分配出去了，不需要tNode节点维护
        // 所以用户实际可以使用的内存块大小不变。
        tNodeInit((tNode *)memBlockStart);
        tListAddLast(&memBlock->blockList, (tNode *)memBlockStart);

        memBlockStart += blockSize;
    }
}
// 等待存储块，有最大等待时长
uint32_t tMemBlockWait(tMemBlock *memBlock, uint8_t **mem, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查是否有空闲的存储块
    if (tListCount(&memBlock->blockList) > 0)
    {
        // 如果有的话，取出一个
        *mem = (uint8_t *)tListRemoveFirst(&memBlock->blockList);
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        // 然后将任务插入事件队列中
        tEventWait(&memBlock->event, currentTask, (void *)0, tEventTypeMemBlock, waitTicks);
        tTaskExitCritical(status);

        // 最后再执行一次事件调度，以便于切换到其它任务
        tTaskSched();

        // 当切换回来时，从tTask中取出获得的消息
        *mem = currentTask->eventMsg;

        // 取出等待结果
        return currentTask->waitEventResult;
    }
}

// 获取存储块，如果没有存储块，则立即退回
uint32_t tMemBlockNoWaitGet(tMemBlock *memBlock, void **mem)
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查是否有空闲的存储块
    if (tListCount(&memBlock->blockList) > 0)
    {
        // 如果有的话，取出一个
        *mem = (uint8_t *)tListRemoveFirst(&memBlock->blockList);
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        // 否则，返回资源不可用
        tTaskExitCritical(status);
        return tErrorResourceUnavaliable;
    }
}

//  通知存储块可用，
// 	若等待队列中有任务唤醒等待队列中的一个任务，
//  没有任务则将存储块加入队列中
void tMemBlockNotify(tMemBlock *memBlock, uint8_t *mem)
{
    uint32_t status = tTaskEnterCritical();

    // 检查是否有任务等待
    if (tEventWaitCount(&memBlock->event) > 0)
    {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        tTask *task = tEventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);

        // 如果唤醒的这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    else
    {
        // 如果没有任务等待的话，将存储块插入到队列中
        tListAddLast(&memBlock->blockList, (tNode *)mem);
    }
    tTaskExitCritical(status);
}
// 查询存储控制块信息
void tMemBlockGetInfo(tMemBlock *memBlock, tMemBlockInfo *info)
{
    uint32_t status = tTaskEnterCritical();

    // 拷贝信息
    info->count = tListCount(&memBlock->blockList);
    info->maxCount = memBlock->maxCount;
    info->blockSize = memBlock->blockSize;
    info->taskCount = tEventWaitCount(&memBlock->event);

    tTaskExitCritical(status);
}

// 销毁存储控制块, 清空任务
uint32_t tMemBlockDestroy(tMemBlock *memBlock)
{
    uint32_t status = tTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = tEventRemoveAll(&memBlock->event, (void *)0, tErrorDel);

    tTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}
