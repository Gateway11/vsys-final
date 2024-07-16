#ifndef PLATFORM_INFO_H
#define PLATFORM_INFO_H

#include <stdio.h>

#define MAX_CARDS 16
#define MAX_BUS_NUM 16
#define MAX_TDM_SLOT_NUM 16
#define MAX_NAME_LEN 64
typedef struct card_info {
    char name[MAX_NAME_LEN];
    struct pcm_config pcm_cfg;
    int32_t card;
    int32_t device;
    int32_t bus_id;
    char address[MAX_NAME_LEN];
} card_info_t;

enum tdm_slot {
    SLOT_0,
    SLOT_1,
    SLOT_2,
    SLOT_3,
    SLOT_4,
    SLOT_5,
    SLOT_6,
    SLOT_7,
    SLOT_8,
    SLOT_9,
    SLOT_10,
    SLOT_11,
    SLOT_12,
    SLOT_13,
    SLOT_14,
    SLOT_15,
};

enum sw_src { INPUT_SRC, OUTPUT_SRC };

enum method { SOFTWARE, HARDWARE };

typedef struct slot_map_t {
    int32_t bus_num;
    int32_t len;
    char slots[MAX_TDM_SLOT_NUM];
    bool overlay;  // true : overlay , false : mix
} slot_map_t;

struct misc_feature {
    bool audio_route;
    bool sw_in_src;
    bool sw_out_src;
    bool sw_ecnr;
    bool sw_gain;
    bool btcall;
    bool ahub_en;
};
typedef struct au_platfrom_info {
    card_info_t cards[MAX_CARDS];
    int32_t card_num;
    struct misc_feature misc;
    slot_map_t output_maps[MAX_BUS_NUM];
    slot_map_t input_maps[MAX_BUS_NUM];
    bool am_en;
} au_platform_info_t;

int32_t platform_info_init(void *platform);

#endif  // PLATFORM_INFO_H
