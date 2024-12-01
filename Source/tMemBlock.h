#ifndef TMEMBLOCK_H
#define TMEMBLOCK_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tMemBlock
{
    // 事件控制块
    tEvent event;

    // 存储块的首地址
    void *memStart;

    // 每个存储块的大小
    uint32_t blockSize;

    // 总的存储块的个数
    uint32_t maxCount;

    // 存储块列表
    tList blockList;
} tMemBlock;
// 存储块信息
typedef struct _tMemBlockInfo
{
    // 当前存储块的计数
    uint32_t count;
    // 允许的最大计数
    uint32_t maxCount;
    // 每个存储块的大小
    uint32_t blockSize;
    // 当前等待的任务计数
    uint32_t taskCount;
} tMemBlockInfo;

// 初始化存储控制块
void tMemBlockInit(tMemBlock *memBlock, uint8_t *memStart, uint32_t blockSize, uint32_t blockCnt);

// 等待存储块，有最大等待时长
uint32_t tMemBlockWait(tMemBlock *memBlock, uint8_t **mem, uint32_t waitTicks);

// 获取存储块，如果没有存储块，则立即退回
uint32_t tMemBlockNoWaitGet(tMemBlock *memBlock, void **mem);

//  通知存储块可用，
// 	若等待队列中有任务唤醒等待队列中的一个任务，
//  没有任务则将存储块加入队列中
void tMemBlockNotify(tMemBlock *memBlock, uint8_t *mem);

// 查询存储控制块信息
void tMemBlockGetInfo(tMemBlock *memBlock, tMemBlockInfo *info);

// 销毁存储控制块, 清空任务
uint32_t tMemBlockDestroy(tMemBlock *memBlock);
#endif
