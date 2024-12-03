#include "rv_plic.h"

<%
    from x_heep_gen.peripherals.rv_plic import RvPlicPeripheral
    handlers = set()
    for domain in xheep.iter_peripheral_domains():
        for periph in domain.iter_peripherals():
            if isinstance(periph, RvPlicPeripheral):
                handlers = handlers.union(periph.make_handler_set(xheep.get_rh()))
%>


% for domain in xheep.iter_peripheral_domains():
% for periph in domain.iter_peripherals():
% if isinstance(periph, RvPlicPeripheral):
extern rv_plic_inf_t ${periph.full_name}_inf = {
    .rv_plic_peri = ${periph.full_name}_peri,
    .default_handlers = {${periph.make_handler_array(xheep.get_rh())}}
};

% endif
% endfor
% endfor


% for handler in handlers:
__attribute__((weak, optimize("O0"))) void ${handler}(uint32_t id) {}
% endfor
