#ifndef TCONFIG_H
#define TCONFIG_H

// 支持的最大优先级数量
#define TINYOS_PRO_COUNT                32
// 每个任务最大运行的时间片计数
#define TINYOS_SLICE_MAX				10			

// 空闲任务的堆栈单元数
#define TINYOS_IDLETASK_STACK_SIZE      1024

// 定时器任务的堆栈单元数
#define TINYOS_TIMERTASK_STACK_SIZE		1024					
// 定时器任务的优先级
#define TINYOS_TIMERTASK_PRIO           1            


#endif
