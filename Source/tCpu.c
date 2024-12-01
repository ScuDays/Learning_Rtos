#include "myRtos.h"
#include "ARMCM3.h"

/**
系统时钟节拍定时器System Tick配置
目前的环境（模拟器）中，系统时钟节拍为12MHz
*/

void tSetSysTickPeriod(uint32_t ms)
{
    SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}
void SysTick_Handler()
{
    tTaskSystemTickHandler();
}
