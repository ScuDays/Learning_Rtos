#include "myRtos.h"
#include "ARMCM3.h"

#define NVIC_INT_CTRL 0xE000ED04   // 中断控制及状态寄存器
#define NVIC_PENDSVSET 0x10000000  // 触发软件中断的值
#define NVIC_SYSPRI2 0xE000ED22	   // 系统优先级寄存器
#define NVIC_PENDSV_PRI 0x000000FF // 配置优先级

#define MEM32(addr) *(volatile unsigned long *)(addr) // 转成特定指针再解指针以访问特定长度内容
#define MEM8(addr) *(volatile unsigned char *)(addr)

uint32_t tTaskEnterCritical (void) 
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();        // CPSID I
    return primask;
}
void tTaskExitCritical (uint32_t status) {
    __set_PRIMASK(status);
}
__asm void PendSV_Handler(){
    IMPORT currentTask
    IMPORT nextTask
    
    MRS R0, PSP
    CBZ R0, PendSVHander_nosave
    
    STMDB R0!, {R4-R11}
    
    LDR R1, =currentTask
    LDR R1, [R1]
    STR R0, [R1]

PendSVHander_nosave
    LDR R0, =currentTask
    LDR R1, =nextTask
    LDR R2, [R1]
    STR R2, [R0]

    LDR R0, [R2]
    LDMIA R0!, {R4-R11}

    MSR PSP, R0
    ORR LR, LR, #0x04
    BX LR
}

void tTaskRunFirst(void){
    // 用于后续PendSV处理函数判断是否是第一次启动进程
    __set_PSP(0);

	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;  // 向NVIC_SYSPRI2写NVIC_PENDSV_PRI，设置其为NVIC_PENDSV_PRI的值的优先级（最低优先级）
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于触发PendSV中断
}

void tTaskSwitch(void){
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于触发PendSV中断
}
