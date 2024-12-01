#ifndef TMBOX_H
#define TMBOX_H

#include "tConfig.h"
#include "tEvent.h"

#define tMBOXSendNormal 0x00 // 正常发送发送至缓冲区
#define tMBOXSendFront 0x01  // 消息发送至缓冲区头部
// 邮箱
typedef struct _tMbox
{
    // 事件控制块
    tEvent event;
    // 当前的消息数量
    uint32_t count;
    // 读取消息的索引
    uint32_t read;
    // 写消息的索引
    uint32_t write;
    // 最大容纳消息数量
    uint32_t maxCount;
    // 消息存储缓冲区，使用循环数组进行维护
    void **msgBuffer;
} tMbox;

// 邮箱状态
typedef struct _tMboxInfo
{
    // 当前的消息数量
    uint32_t count;

    // 最大允许容纳的消息数量
    uint32_t maxCount;

    // 当前等待的任务计数
    uint32_t taskCount;
} tMboxInfo;

// 初始化邮箱
void tMboxInit(tMbox *mbox, void **msgBuffer, uint32_t maxCount);

// 等待邮箱, 获取一则消息,waitTicks 最大等待的ticks数，为0表示无限等待
uint32_t tMboxWait(tMbox *mbox, void **msg, uint32_t waitTicks);

// 获取一则消息，如果没有消息，则立即返回
uint32_t tMboxNoWaitGet(tMbox *mbox, void **msg);

// 通知消息可用，唤醒等待队列中的一个任务，或者将消息插入到邮箱中
uint32_t tMboxNotify(tMbox *mbox, void *msg, uint32_t notifyOption);

// 清空邮箱中所有消息
void tMboxFlush(tMbox *mbox);

// 销毁邮箱,清空里面所有任务
uint32_t tMboxDestroy(tMbox *mbox);

// 查询状态信息
void tMboxGetInfo(tMbox *mbox, tMboxInfo *info);
#endif
