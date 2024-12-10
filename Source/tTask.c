#include "myRtos.h"
void tTaskInit(tTask *task, void (*entry)(void *), void *param, uint32_t prio, tTaskStack *stack)
{
  /*Ӳ�����Զ����浽ջ�ϵļĴ���*/
  // ˳���ܱ䣬Ҫ����PendSV_Handler�Լ�CPU���쳣�Ĵ�������
  *(--stack) = (unsigned long)(1 << 24); // XPSR, ������Thumbģʽ���ָ���Thumb״̬����ARM״̬����
  *(--stack) = (unsigned long)entry;     // �������ڵ�ַ
  *(--stack) = (unsigned long)0x14;      // R14(LR), ���񲻻�ͨ��return xxx�����Լ�������δ��
  *(--stack) = (unsigned long)0x12;      // R12, δ��
  *(--stack) = (unsigned long)0x3;       // R3, δ��
  *(--stack) = (unsigned long)0x2;       // R2, δ��
  *(--stack) = (unsigned long)0x1;       // R1, δ��
  *(--stack) = (unsigned long)param;     // R0 = param, �����������ں���

  /*��Ҫ���б���ļĴ���*/
  // δ���ϵļĴ���ֱ�����˼Ĵ����ţ�������IDE����ʱ�鿴Ч����
  *(--stack) = (unsigned long)0x11; // R11, δ��
  *(--stack) = (unsigned long)0x10; // R10, δ��
  *(--stack) = (unsigned long)0x9;  // R9, δ��
  *(--stack) = (unsigned long)0x8;  // R8, δ��
  *(--stack) = (unsigned long)0x7;  // R7, δ��
  *(--stack) = (unsigned long)0x6;  // R6, δ��
  *(--stack) = (unsigned long)0x5;  // R5, δ��
  *(--stack) = (unsigned long)0x4;  // R4, δ��

  // �������յ�ֵ
  task->stack = stack;
  task->delayTicks = 0;
  task->prio = prio;
  // ��ʼ��ʱ��Ƭ
  task->slice = MYRTOS_SLICE_MAX;
  task->state = MYRTOS_TASK_STATE_RDY;
  // ��ʼ���������
  task->suspendCount = 0;
  // ����������
  task->clean = (void (*)(void *))0;
  // ���ô��ݸ��������Ĳ���
  task->cleanParam = (void *)0;
  // ��ʼ�������־λ
  task->requestDeleteFlag = 0;

  task->waitEvent = (tEvent *)0; // û�еȴ��¼�
  task->eventMsg = (void *)0;    // û�еȴ��¼�
  task->waitEventResult = tErrorNoError;
  // ��ʼ����ʱ���нڵ�
  tNodeInit(&(task->delayNode));
  // ��ʼ�����ȼ����нڵ�
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
// ���ѹ�������
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

// ��������ɾ��ʱ���õ�������
void tTaskSetCleanCallFunc(tTask *task, void (*clean)(void *param), void *param)
{
  task->clean = clean;
  task->cleanParam = param;
}

// ǿ��ɾ��ָ��������
void tTaskForceDelete(tTask *task)
{
  // �����ٽ���
  uint32_t status = tTaskEnterCritical();

  // �����������ʱ״̬�������ʱ������ɾ��
  if (task->state & MYRTOS_TASK_STATE_DELAYED)
  {
    tTimeTaskRemove(task);
  }
  // ������񲻴��ڹ���״̬����ô���Ǿ���̬���Ӿ�������ɾ��
  else if (!(task->state & MYRTOS_TASK_STATE_SUSPEND))
  {
    tTaskSchedRemove(task);
  }

  // ɾ��ʱ������������������������������
  if (task->clean)
  {
    task->clean(task->cleanParam);
  }

  // ���ɾ�������Լ�����ô��Ҫ�л�����һ����������ִ��һ���������
  if (currentTask == task)
  {
    tTaskSched();
  }

  // �˳��ٽ���
  tTaskExitCritical(status);
}

// ����ɾ��ĳ�������������Լ������Ƿ�ɾ���Լ�
void tTaskRequestDelete(tTask *task)
{
  // �����ٽ���
  uint32_t status = tTaskEnterCritical();

  // �������ɾ�����
  task->requestDeleteFlag = 1;

  // �˳��ٽ���
  tTaskExitCritical(status);
}

// �鿴�Ƿ��Ѿ�������ɾ���Լ�
uint8_t tTaskIsRequestedDelete(void)
{
  uint8_t delete;

  // �����ٽ���
  uint32_t status = tTaskEnterCritical();

  // ��ȡ����ɾ�����
  delete = currentTask->requestDeleteFlag;

  // �˳��ٽ���
  tTaskExitCritical(status);

  return delete;
}
// ɾ���Լ�
void tTaskDeleteSelf(void)
{
  // �����ٽ���
  uint32_t status = tTaskEnterCritical();

  // �����ڵ��øú���ʱ�������Ǵ��ھ���״̬�������ܴ�����ʱ����������״̬
  // ���ԣ�ֻ��Ҫ�Ӿ����������Ƴ�����
  tTaskSchedRemove(currentTask);

  // ɾ��ʱ������������������������������
  if (currentTask->clean)
  {
    currentTask->clean(currentTask->cleanParam);
  }

  // ���������϶����л�����������ȥ����
  tTaskSched();

  // �˳��ٽ���
  tTaskExitCritical(status);
}
// ��ȡ���������Ϣ
void tTaskGetInfo(tTask *task, tTaskInfo *info)
{
  // �����ٽ���
  uint32_t status = tTaskEnterCritical();

  info->delayTicks = task->delayTicks;     // ��ʱ��Ϣ
  info->prio = task->prio;                 // �������ȼ�
  info->state = task->state;               // ����״̬
  info->slice = task->slice;               // ʣ��ʱ��Ƭ
  info->suspendCount = task->suspendCount; // ������Ĵ���

  // �˳��ٽ���
  tTaskExitCritical(status);
}
