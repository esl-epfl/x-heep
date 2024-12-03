#ifndef _RV_PLIC_GEN_H_
#define _RV_PLIC_GEN_H_

#include "rv_plic.h"
<%
    from x_heep_gen.peripherals.rv_plic import RvPlicPeripheral
    handlers = set()
    for domain in xheep.iter_peripheral_domains():
        for periph in domain.iter_peripherals():
            if isinstance(periph, RvPlicPeripheral):
                handlers = handlers.union(periph.make_handler_set(xheep.get_rh()))
%>

% for handler in handlers:
void ${handler}(uint32_t id);
% endfor

% for domain in xheep.iter_peripheral_domains():
% for periph in domain.iter_peripherals():
% if isinstance(periph, RvPlicPeripheral):
extern rv_plic_inf_t ${periph.full_name}_inf;
% endif
% endfor
% endfor

#endif