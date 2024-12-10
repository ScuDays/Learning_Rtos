#ifndef TSEM_H
#define TSEM_H

#include "tConfig.h"
#include "tEvent.h"

// 信号量
typedef struct _tSem
{
	// 事件控制块
	// 该结构被特意放到起始处，以实现tSem同时是一个tEvent的目的
	tEvent event;

	// 当前的计数
	uint32_t count;

	// 最大计数
	uint32_t maxCount;
} tSem;

// 信号量信息
typedef struct _tSemInfo
{
	// 当前信号量大小
	uint32_t count;
	// 信号量允许的最大值
	uint32_t maxCount;
	// 有多少任务在等待
	uint32_t taskCount;
} tSemInfo;

//  初始化信号量
void tSemInit(tSem *sem, uint32_t startCount, uint32_t maxCount);

// 等待信号量
// waitTicks 最大等待ticks数，若为0则可无限等待
uint32_t tSemWait(tSem *sem, uint32_t waitTicks);

// 获取信号量，如果信号量计数不可用，则立即退回
uint32_t tSemNoWaitGet(tSem *sem);

// 通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
void tSemNotify(tSem *sem);

// 获取该信号量有关的信息
void tSemGetInfo(tSem *sem, tSemInfo *info);

// 摧毁该信号量
uint32_t tSemDestroy(tSem *sem);

#endif
