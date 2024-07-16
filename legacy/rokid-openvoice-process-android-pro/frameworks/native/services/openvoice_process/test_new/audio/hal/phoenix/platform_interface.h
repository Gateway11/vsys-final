#ifndef PLATFORM_INF_H
#define PLATFORM_INF_H

#include <tinyalsa/asoundlib.h>

void platform_set_card_info(void *platform, const card_info_t *card_info,
                             int32_t direct);
void platform_set_misc_config(void *platform, void *misc);
void platform_set_slotmap_config(void *platform, uint32_t bus_id, slot_map_t *map);
void platform_set_bind_bus(void *platform, const char *card_name, int32_t bind_bus);
void platform_set_bt_connect(void *platform, enum bt_chip_connect_e connect);

#endif  // PLATFORM_INF_H