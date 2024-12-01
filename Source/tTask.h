#ifndef TTASK_H
#define TTASK_H

// 代表进程的延时状态
#define TINYOS_TASK_STATE_RDY 0
// 代表任务是否被删除
#define TINYOS_TASK_STATE_DESTROYED (1 << 0)
// 代表任务是否进入延时状态
#define TINYOS_TASK_STATE_DELAYED (1 << 1)
// 代表任务是否进入挂起状态
#define TINYOS_TASK_STATE_SUSPEND (1 << 2)

// 高16位为事件等待的控制位，标志任务在等待什么事件
#define TINYOS_TASK_WAIT_MASK (0xFF << 16)

struct _tEvent;

typedef uint32_t tTaskStack;
typedef struct _tTask
{

    tTaskStack *stack;
    int delayTicks;
    tNode linkNode;
    tNode delayNode;
    uint32_t state; // 表示是否延时的状态
    uint32_t prio;
    uint32_t slice;
    uint32_t suspendCount;

    // 任务被删除时调用的清理函数
    void (*clean)(void *param);
    // 传递给清理函数的参数
    void *cleanParam;
    // 请求删除标志，非0表示请求删除
    uint8_t requestDeleteFlag;

    // 任务等待的事件类型
    struct _tEvent *waitEvent;
    // 事件等待的消息存储的位置
    void *eventMsg;
    // 等待的事件的结果
    uint32_t waitEventResult;

    // 等待的事件是什么(任意或全部位 置1或置0)
    uint32_t waitFlagsType;
    // 等待的事件标志
    uint32_t eventFlags;
} tTask;
// 任务相关信息结构
typedef struct _tTaskInfo
{
    // 任务延时计数器
    uint32_t delayTicks;
    // 任务的优先级
    uint32_t prio;
    // 任务当前状态
    uint32_t state;
    // 当前剩余的时间片
    uint32_t slice;
    // 被挂起的次数
    uint32_t suspendCount;
} tTaskInfo;

void tTaskInit(tTask *task, void (*entry)(void *), void *param, uint32_t prio, tTaskStack *stack);
void tTaskSuspend(tTask *task);
void tTaskWakeUp(tTask *task);
// 设置任务被删除时调用的清理函数
void tTaskSetCleanCallFunc(tTask *task, void (*clean)(void *param), void *param);
// 强制删除指定的任务
void tTaskForceDelete(tTask *task);
// 请求删除某个任务，由任务自己决定是否删除自己
void tTaskRequestDelete(tTask *task);

// 查看是否已经被请求删除自己
uint8_t tTaskIsRequestedDelete(void);
// 删除自己
void tTaskDeleteSelf(void);
// 获取任务相关信息
void tTaskGetInfo(tTask *task, tTaskInfo *info);

#endif
