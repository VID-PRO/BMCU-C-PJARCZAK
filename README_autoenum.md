# BMCU-C-PJARCZAK – Automatic AMS Slot Enumeration

This is a modified version of the BMCU-C-PJARCZAK firmware (V10.5 base,
by jarczakpawel) that adds **automatic AMS slot discovery at runtime**.
A single firmware binary discovers and claims its AMS slot (1–3) without
any compile-time constant, so you never have to build or flash different
firmware per unit.

---

## Slot assignment policy

| Slot | Device |
|------|--------|
| **0 (AMS A)** | Always reserved for the original Bambu AMS |
| **1 (AMS B)** | First BMCU |
| **2 (AMS C)** | Second BMCU |
| **3 (AMS D)** | Third BMCU |

Slot 0 is **permanently skipped** by the auto-enum firmware. If no original
Bambu AMS is present on your bus, slot 0 simply remains unclaimed and the
first BMCU takes slot 1. If you want a BMCU on slot 0 you must use the
legacy `bmcu_ams_a` build instead.

---

## How it works

### First boot (no slot saved in flash)
1. Device starts with candidate slot **1** (slot 0 is reserved for the real AMS).
2. During the Bambu Bus online-detect handshake the device sends its candidate
   slot + a unique serial number derived from the MCU's factory UID.
3. The printer echoes back the serial for whichever device it wants to confirm.
4. If our serial is echoed for slot 1 → **confirmed**, slot 1 written to flash.
5. If another device's serial is echoed for slot 1 → slot taken.
   We increment to slot 2 and retry. This repeats through slots 1→2→3→1
   until a free slot is found.

### Subsequent boots
The assigned slot is read from flash before the first packet is sent.
Enumeration completes in the very first handshake cycle — no delay.

### Force re-enumeration
Erase the saved slot by calling `Flash_AMS_num_clear()` (e.g. from a debug
command or by erasing the CFG flash page). On the next boot the device
re-negotiates its slot from scratch, starting at slot 1.

---

## What changed vs the original firmware

| File | Change |
|------|--------|
| `Flash_saves.h` | Added `FLASH_NVM_CFG_ADDR` (NVM page 15), `Flash_AMS_num_read/write/clear` |
| `Flash_saves.cpp` | Implemented CFG page R/W; reduced STA journal 10→9 pages to avoid overlap with CFG page |
| `bambu_bus_ams.h` | Exported `bambubus_get_ams_num()`, `bambubus_set_ams_num()`, `bambubus_is_registered()` |
| `bambu_bus_ams.cpp` | Full auto-enum state machine in `get_package_online_detect()`; starts at slot 1, cycles 1→2→3→1; slot bump triggers only on confirmed serial collision, not phase-prefix mismatch |
| `ahub_bus.cpp` | All runtime comparisons use `bambubus_get_ams_num()` instead of the compile-time constant |
| `ams.cpp` | `ams_init()` marks `ams[0]` online (motor wiring is always physical slot 0) |
| `main.cpp` | Loads persisted AMS slot before `bambubus_init()` |
| `Motion_control.h` | Range check exempts `0xFF` sentinel; `motion_control_ams_num` resolves to `0` for auto-enum builds (physical wiring unchanged) |
| `platformio.ini` | `[env:bmcu_auto]` with `-DBAMBU_BUS_AMS_NUM=0xFF`; variants `bmcu_auto_010`–`bmcu_auto_090` for different retraction settings; legacy per-slot envs retained |

---

## NVM layout (flash sector at 0x0800F000, 4 KB)

| Page | Address | Size | Contents |
|------|---------|------|----------|
| 0 | +0x000 | 256 B | Calibration (CAL) |
| 1 | +0x100 | 256 B | Motion params (MOT) |
| 2–5 | +0x200 | 4×256 B | Filament data (FIL) |
| 6–14 | +0x600 | 9×256 B | State log / loaded-channel journal |
| **15** | **+0xF00** | **256 B** | **Device config – AMS slot number (CFG)** |

---

## Building

```bash
# Build the universal auto-enumeration firmware (recommended)
pio run -e bmcu_auto

# Flash
pio run -e bmcu_auto --target upload
```

### Retraction variants
If you need a specific retraction distance, use one of the pre-configured
environments instead of `bmcu_auto`:

| Environment | Retraction |
|-------------|-----------|
| `bmcu_auto_010` | 1.0 mm |
| `bmcu_auto_020` | 2.0 mm |
| …  | … |
| `bmcu_auto_090` | 9.0 mm |
| `bmcu_auto` | default (0.95 mm) |

### Legacy per-slot builds
Only use these if you deliberately want a BMCU on a fixed slot and do **not**
have an original Bambu AMS (or if you're putting a BMCU on slot 0):

```bash
pio run -e bmcu_ams_a   # fixed slot 0 (AMS A)
pio run -e bmcu_ams_b   # fixed slot 1 (AMS B)
pio run -e bmcu_ams_c   # fixed slot 2 (AMS C)
pio run -e bmcu_ams_d   # fixed slot 3 (AMS D)
```

---

## Mixed setup (original AMS + BCMUs)

This is the primary intended use case:

1. Connect the original Bambu AMS — it self-assigns slot 0 (AMS A) as normal.
2. Flash each BMCU with `bmcu_auto` and connect them one at a time.
3. Each BMCU finds the lowest free slot in 1–3 and saves it to flash.
4. All three BMCU units remember their slots across power cycles.

You do **not** need to flash different firmware per BMCU unit.
