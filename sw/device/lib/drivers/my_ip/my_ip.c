#include "my_ip_structs.h"
#include "my_ip.h"


__attribute__((optimize("O0"))) uint32_t my_ip_is_ready()
{
    /* The transaction READY bit is read from the status register*/
    uint32_t ret = ( my_ip_peri->CONTROL & (1<<MY_IP_CONTROL_READY_BIT) );
    return ret;
}