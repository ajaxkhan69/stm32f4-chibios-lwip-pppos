#include <hal.h>
#include "lwip/tcpip.h"
#include "lwip/udp.h"
#include "lwip/ip.h"
#include "ppp/ppp.h"

u32_t sys_jiffies(void) {
	static u32_t jiffies = 0;
#if OSAL_ST_FREQUENCY == 1000
	jiffies += (u32_t)osalOsGetSystemTimeX();
#elif (OSAL_ST_FREQUENCY / 1000) >= 1 && (OSAL_ST_FREQUENCY % 1000) == 0
	jiffies += ((u32_t)osalOsGetSystemTimeX() - 1) / (OSAL_ST_FREQUENCY / 1000) + 1;
#elif (1000 / OSAL_ST_FREQUENCY) >= 1 && (1000 % OSAL_ST_FREQUENCY) == 0
	jiffies += ((u32_t)osalOsGetSystemTimeX() - 1) * (1000 / OSAL_ST_FREQUENCY) + 1;
#else
	jiffies += (u32_t)(((u64_t)(osalOsGetSystemTimeX() - 1) * 1000) / OSAL_ST_FREQUENCY) + 1;
#endif

	return jiffies;
}
