#pragma once
#include <stdint.h>

enum class bambubus_package_type
{
    error = -1,
    none = 0,
    filament_motion_short,
    filament_motion_long,
    online_detect,
    REQx6,
    NFC_detect,
    set_filament_info,
    MC_online,
    read_filament_info,
    set_filament_info_type2,
    version,
    serial_number,
    heartbeat,
    ETC,

    __BambuBus_package_packge_type_size
};

void bambubus_init(void);
void bambubus_heartbeat_seen_fast(void);
extern bambubus_package_type bambubus_run();

// Runtime AMS slot helpers
uint8_t bambubus_get_ams_num(void);    // current bus slot 0-3
bool    bambubus_is_registered(void);  // true once printer confirmed our slot

// Map from bus-slot (0-3) to ams[] data index (0 = data store, 0xFF = not ours).
// Populated by bambubus_init(). Used by ahub_bus query handler to remap incoming
// slot addresses and ignore packets intended for other devices (e.g. original AMS).
extern uint8_t bambubus_ams_map[4];
