#include "myRtos.h"
#include "ARMCM3.h"
tTask *currentTask;
tTask *nextTask;

tBitmap taskPrioBitmap;
tList taskTable[TINYOS_PRO_COUNT];

// ��ʱ����
tList tTaskDelayedList;
uint8_t schedLockCount;

tTask *tTaskHighestReady(void)
{
    uint32_t higestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
    tNode *node = tListFirst(&(taskTable[higestPrio]));
    return tNodeParent(node, tTask, linkNode);
}

// ��ʼ�����������
void tTaskSchedInit(void)
{
    schedLockCount = 0;
    tBitmapInit(&taskPrioBitmap);
    for (int i = 0; i < TINYOS_PRO_COUNT; i++)
    {
        tListInit(&taskTable[i]);
    }
}

// ��ֹ�������
void tTaskSchedDisable(void)
{
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount < 255)
    {
        schedLockCount++;
    }

    tTaskExitCritical(status);
}

// �����������
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

// ��ʼ����ʱ����
void tTaskDelayedInit(void)
{
    tListInit(&tTaskDelayedList);
}

// ���뵽��ʱ�б��������������ʱ״̬
void tTimeTaskWait(tTask *task, uint32_t ticks)
{
    task->delayTicks = ticks;
    tListAddLast(&tTaskDelayedList, &(task->delayNode));
    task->state |= TINYOS_TASK_STATE_DELAYED;
}

// ����ʱ�б��Ƴ�������������뻽��״̬
void tTimeTaskWakeUp(tTask *task)
{
    tListRemove(&tTaskDelayedList, &(task->delayNode));
    task->state &= ~TINYOS_TASK_STATE_DELAYED;
}
// ����ʱ���������ʱ�������Ƴ�
void tTimeTaskRemove(tTask *task)
{
    tListRemove(&tTaskDelayedList, &(task->delayNode));
}
// ���뵽�����б���λͼ�����ø������λ
void tTaskSchedRdy(tTask *task)
{
    tListAddFirst(&(taskTable[task->prio]), &(task->linkNode));
    tBitmapSet(&taskPrioBitmap, task->prio);
}
// �Ӿ����б��Ƴ�����λͼ���Ƴ��������λ
void tTaskSchedUnRdy(tTask *task)
{
    tListRemove(&taskTable[task->prio], &(task->linkNode));
    // ��������ȼ���û�б�������ˣ���ȥ��bitmap�е�λ
    if (tListCount(&taskTable[task->prio]) == 0)
    {
        tBitmapClear(&taskPrioBitmap, task->prio);
    }
}
// ������Ӿ����б����Ƴ�
// ��tTaskSchedUnRdy()����һ������
// Ϊ����չ��
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
            // ����������ڵȴ�ĳ���¼���
            // ������ʱ���ڣ��������¼�����ɾ��
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

    // ��������ֱ�ӷ���
    if (schedLockCount > 0)
    {
        tTaskExitCritical(status);
        return;
    }
    // �鿴������ȼ�������
    // ���뵱ǰ������ͬ���򷵻�
    // ��ͬ������ȡ�
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
    // ��ʼ����ʱ��ģ��
    tTimerModuleInit();
    tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0x2222222, TINYOS_PRO_COUNT - 1, &taskIdleEnv[TINYOS_IDLETASK_STACK_SIZE]);

    idleTask = &tTaskIdle;

    nextTask = tTaskHighestReady();
    tTaskRunFirst();
    return 0;
}
