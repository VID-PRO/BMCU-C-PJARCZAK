// ams.cpp – BMCU-C-PJARCZAK
// Changes vs. original V10.5:
//   • ams[0] is ALWAYS the data store for auto-enum builds.
//     Motion_control_init() sets ams[0].online = true and ams[0].ams_type = 0x03.
//     apply_slot() (called from bambubus_init) only updates bambubus_ams_map to map
//     the runtime bus slot → ams[0]; it never touches ams[].online.
//     The ahub heartbeat does a reverse-lookup through bambubus_ams_map to emit
//     the correct wire address (runtime_slot << 2) while reading from ams[0].

#include "ams.h"

_ams    ams[ams_max_number];
uint8_t bus_now_ams_num = 0;

void ams_init()
{
    for (uint8_t i = 0; i < ams_max_number; i++)
        ams[i].init();
    // ams[0].online and ams[0].ams_type are set by Motion_control_init(),
    // which is called after ams_init() in main(). Do not set them here.
}
