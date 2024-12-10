#include "myRtos.h"
void tTaskInit(tTask *task, void (*entry)(void *), void *param, uint32_t prio, tTaskStack *stack)
{
  /*硬件会自动保存到栈上的寄存器*/
  // 顺序不能变，要符合PendSV_Handler以及CPU对异常的处理流程
  *(--stack) = (unsigned long)(1 << 24); // XPSR, 设置了Thumb模式，恢复到Thumb状态而非ARM状态运行
  *(--stack) = (unsigned long)entry;     // 程序的入口地址
  *(--stack) = (unsigned long)0x14;      // R14(LR), 任务不会通过return xxx结束自己，所以未用
  *(--stack) = (unsigned long)0x12;      // R12, 未用
  *(--stack) = (unsigned long)0x3;       // R3, 未用
  *(--stack) = (unsigned long)0x2;       // R2, 未用
  *(--stack) = (unsigned long)0x1;       // R1, 未用
  *(--stack) = (unsigned long)param;     // R0 = param, 传给任务的入口函数

  /*需要自行保存的寄存器*/
  // 未用上的寄存器直接填了寄存器号，方便在IDE调试时查看效果；
  *(--stack) = (unsigned long)0x11; // R11, 未用
  *(--stack) = (unsigned long)0x10; // R10, 未用
  *(--stack) = (unsigned long)0x9;  // R9, 未用
  *(--stack) = (unsigned long)0x8;  // R8, 未用
  *(--stack) = (unsigned long)0x7;  // R7, 未用
  *(--stack) = (unsigned long)0x6;  // R6, 未用
  *(--stack) = (unsigned long)0x5;  // R5, 未用
  *(--stack) = (unsigned long)0x4;  // R4, 未用

  // 保存最终的值
  task->stack = stack;
  task->delayTicks = 0;
  task->prio = prio;
  // 初始化时间片
  task->slice = MYRTOS_SLICE_MAX;
  task->state = MYRTOS_TASK_STATE_RDY;
  // 初始化挂起次数
  task->suspendCount = 0;
  // 设置清理函数
  task->clean = (void (*)(void *))0;
  // 设置传递给清理函数的参数
  task->cleanParam = (void *)0;
  // 初始化清理标志位
  task->requestDeleteFlag = 0;

  task->waitEvent = (tEvent *)0; // 没有等待事件
  task->eventMsg = (void *)0;    // 没有等待事件
  task->waitEventResult = tErrorNoError;
  // 初始化延时队列节点
  tNodeInit(&(task->delayNode));
  // 初始化优先级队列节点
  tNodeInit(&(task->linkNode));
  tTaskSchedRdy(task);
}

void tTaskSuspend(tTask *task)
{
  uint32_t status = tTaskEnterCritical();
  if (!(task->state & MYRTOS_TASK_STATE_DELAYED))
  {
    if (++task->suspendCount <= 1)
    {
      task->state |= MYRTOS_TASK_STATE_SUSPEND;
      tTaskSchedUnRdy(task);
      if (task == currentTask)
      {
        tTaskSched();
      }
    }
  }
  tTaskExitCritical(status);
}
// 唤醒挂起任务
void tTaskWakeUp(tTask *task)
{
  uint32_t status = tTaskEnterCritical();
  if (task->state & MYRTOS_TASK_STATE_SUSPEND)
  {
    if (--task->suspendCount == 0)
    {
      task->state &= ~MYRTOS_TASK_STATE_SUSPEND;
      tTaskSchedRdy(task);
      tTaskSched();
    }
  }
  tTaskExitCritical(status);
}

// 设置任务被删除时调用的清理函数
void tTaskSetCleanCallFunc(tTask *task, void (*clean)(void *param), void *param)
{
  task->clean = clean;
  task->cleanParam = param;
}

// 强制删除指定的任务
void tTaskForceDelete(tTask *task)
{
  // 进入临界区
  uint32_t status = tTaskEnterCritical();

  // 如果任务处于延时状态，则从延时队列中删除
  if (task->state & MYRTOS_TASK_STATE_DELAYED)
  {
    tTimeTaskRemove(task);
  }
  // 如果任务不处于挂起状态，那么就是就绪态，从就绪表中删除
  else if (!(task->state & MYRTOS_TASK_STATE_SUSPEND))
  {
    tTaskSchedRemove(task);
  }

  // 删除时，如果有设置清理函数，则调用清理函数
  if (task->clean)
  {
    task->clean(task->cleanParam);
  }

  // 如果删除的是自己，那么需要切换至另一个任务，所以执行一次任务调度
  if (currentTask == task)
  {
    tTaskSched();
  }

  // 退出临界区
  tTaskExitCritical(status);
}

// 请求删除某个任务，由任务自己决定是否删除自己
void tTaskRequestDelete(tTask *task)
{
  // 进入临界区
  uint32_t status = tTaskEnterCritical();

  // 设置清除删除标记
  task->requestDeleteFlag = 1;

  // 退出临界区
  tTaskExitCritical(status);
}

// 查看是否已经被请求删除自己
uint8_t tTaskIsRequestedDelete(void)
{
  uint8_t delete;

  // 进入临界区
  uint32_t status = tTaskEnterCritical();

  // 获取请求删除标记
  delete = currentTask->requestDeleteFlag;

  // 退出临界区
  tTaskExitCritical(status);

  return delete;
}
// 删除自己
void tTaskDeleteSelf(void)
{
  // 进入临界区
  uint32_t status = tTaskEnterCritical();

  // 任务在调用该函数时，必须是处于就绪状态，不可能处于延时或挂起等其它状态
  // 所以，只需要从就绪队列中移除即可
  tTaskSchedRemove(currentTask);

  // 删除时，如果有设置清理函数，则调用清理函数
  if (currentTask->clean)
  {
    currentTask->clean(currentTask->cleanParam);
  }

  // 接下来，肯定是切换到其它任务去运行
  tTaskSched();

  // 退出临界区
  tTaskExitCritical(status);
}
// 获取任务相关信息
void tTaskGetInfo(tTask *task, tTaskInfo *info)
{
  // 进入临界区
  uint32_t status = tTaskEnterCritical();

  info->delayTicks = task->delayTicks;     // 延时信息
  info->prio = task->prio;                 // 任务优先级
  info->state = task->state;               // 任务状态
  info->slice = task->slice;               // 剩余时间片
  info->suspendCount = task->suspendCount; // 被挂起的次数

  // 退出临界区
  tTaskExitCritical(status);
}
