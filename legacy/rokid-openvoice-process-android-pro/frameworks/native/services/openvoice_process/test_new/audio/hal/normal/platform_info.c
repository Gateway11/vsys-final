#define LOG_TAG "audio_hw_platform_info"
// #define LOG_NDEBUG 0

#include <audio_hw.h>
#include <expat.h>
#include <log/log.h>
#include <platform_info.h>
#include <platform_interface.h>
#include <unistd.h>

#define MAX_SLOT_MAPS 16

#define STR_BOOL(str, ret)       \
    if (!strcmp(str, "false")) { \
        *ret = false;            \
    }                            \
    if (!strcmp(str, "true")) {  \
        *ret = true;             \
    }

#define PLATFORM_INFO_PATH "/vendor/etc/"
#define PLATFORM_INFO_NAME "audio_platform_info.xml"

void *g_platform;

extern struct pcm_config *pcm_main_config;
typedef void (*section_process_fn)(const XML_Char **attr);
static void process_card_playback_config(const XML_Char **attr);
static void process_card_capture_config(const XML_Char **attr);
static void process_misc_config(const XML_Char **attr);
static void process_bus_slot_map_config(const XML_Char **attr);
static void process_bt_connect_config(const XML_Char **attr);

typedef enum {
    SECTION_CARD_PLAYBACK_CONFIG,
    SECTION_CARD_CAPTURE_CONFIG,
    SECTION_FEATURE_MISC,
    SECTION_BUS_SLOT_MAP,
    SECTION_BT_CONNECTION,
} section_t;

static section_process_fn section_table[] = {
    [SECTION_CARD_PLAYBACK_CONFIG] = process_card_playback_config,
    [SECTION_CARD_CAPTURE_CONFIG] = process_card_capture_config,
    [SECTION_FEATURE_MISC] = process_misc_config,
    [SECTION_BUS_SLOT_MAP] = process_bus_slot_map_config,
    [SECTION_BT_CONNECTION] = process_bt_connect_config,
};
static bool parse_bus_mix = false;
static bool parse_bus_slotmap = false;
static int32_t card_num;
static char card[8][128] = {};

void parse_slots(char *str, slot_map_t *map) {
    int32_t cnt = 0;
    char *cur = strtok(str, ",");
    map->slots[cnt] = atoi(cur);
    cnt++;
    while (1) {
        cur = strtok(NULL, ",");
        // AALOGI("cur: %s", cur);
        if (cur) {
            map->slots[cnt] = atoi(cur);
            cnt++;
        } else
            break;
    }
    map->len = cnt;
}

static void process_bus_slot_map_config(const XML_Char **attr) {
    DEBUG_FUNC_PRT
    slot_map_t map;
    const XML_Char *ptr = attr[0];
    if (NULL != attr[0]) {
        AALOGV("attr %s", ptr);
        memset(&map, 0, sizeof(map));
        ptr = attr[1];
        if (ptr) {
            parse_slots((char *)ptr, &map);
            if (!strcmp(attr[2], "type") && !strcmp(attr[3], "overlay"))
                map.overlay = true;
            else
                map.overlay = false;
            platform_set_slotmap_config(g_platform, atoi(attr[0] + 3), &map);
        }
    }
}

static void process_card_config(const XML_Char **attr, uint32_t direct) {
    DEBUG_FUNC_PRT
    card_info_t card_info;
    int i = 0;
    char * attr_ptr = NULL;
    memcpy(&card_info.pcm_cfg, pcm_main_config, sizeof(struct pcm_config));

    while (attr[i]) {
        AALOGV("process_card_config: %s", attr[i]);
        if (!strcmp(attr[i], "bit_width")) {
            switch (atoi(attr[i + 1])) {
                case 16:
                    card_info.pcm_cfg.format = PCM_FORMAT_S16_LE;
                    break;

                default:
                    AALOGE("Unsupport format");
                    card_info.pcm_cfg.format = PCM_FORMAT_INVALID;
                    // goto done;
            }
        } else if (!strncmp(attr[i], "rate", strlen("rate"))) {
            card_info.pcm_cfg.rate = atoi(attr[i + 1]);
        } else if (!strncmp(attr[i], "channels", strlen("channels"))) {
            card_info.pcm_cfg.channels = atoi(attr[i + 1]);
        } else if (!strncmp(attr[i], "address", strlen("address"))) {
            attr_ptr = (char *)attr[i + 1];
            AALOGI("xml addr %s", attr_ptr);
            strncpy(card_info.address, attr_ptr,
                    strlen(attr_ptr) < MAX_NAME_LEN ? strlen(attr_ptr) : MAX_NAME_LEN);
        } else if (attr[i] && !strcmp(attr[i], "bind_bus_id")) {
            char *cur = strtok((char *)attr[i + 1], ",");
            if (cur) {
                platform_set_bind_bus(g_platform, card[card_num], atoi(cur));
                card_info.bus_id = atoi(cur);
            }
            while (1) {
                cur = strtok(NULL, ",");
                if (cur)
                    platform_set_bind_bus(g_platform, card[card_num], atoi(cur));
                else
                    break;
            }
        } else if (!strncmp(attr[i], "card", strlen("card"))) {
            card_info.card = atoi(attr[i + 1]);
        } else if (!strncmp(attr[i], "device", strlen("device"))) {
            card_info.device = atoi(attr[i + 1]);
        } else {
            ALOGW("attr %s is not processed", attr[i]);
        }
        i += 2;
    }

    AALOGV("card num: %d", card_num);
    platform_set_card_info(g_platform, &card_info, direct);

    return;
}

static void process_card_playback_config(const XML_Char **attr) {
    process_card_config(attr, AUDIO_HAL_PLAYBACK);
}

static void process_card_capture_config(const XML_Char **attr) {
    process_card_config(attr, AUDIO_HAL_CAPTURE);
}

static void process_misc_config(const XML_Char **attr) {
    DEBUG_FUNC_PRT

    struct misc_feature feature;
    int i = 0;

    while (attr[i]) {
        if (strncmp(attr[i], "audio_route_en", strlen("audio_route_en")) == 0) {
            STR_BOOL(attr[i + 1], &feature.audio_route);
        } else if (strncmp(attr[i], "in_src_en", strlen("in_src_en")) == 0) {
            STR_BOOL(attr[i + 1], &feature.sw_in_src);
        } else if (strncmp(attr[i], "out_src_en", strlen("out_src_en")) == 0) {
            STR_BOOL(attr[i + 1], &feature.sw_out_src);
        } else if (strncmp(attr[i], "sw_ecnr_en", strlen("sw_ecnr_en")) == 0) {
            STR_BOOL(attr[i + 1], &feature.sw_ecnr);
        } else if (strncmp(attr[i], "sw_gain_en", strlen("sw_gain_en")) == 0) {
            STR_BOOL(attr[i + 1], &feature.sw_gain);
        } else if (strncmp(attr[i], "btcall_en", strlen("btcall_en")) == 0) {
            STR_BOOL(attr[i + 1], &feature.btcall);
        } else if (strncmp(attr[i], "ahub_en", strlen("ahub_en")) == 0) {
            STR_BOOL(attr[i + 1], &feature.ahub_en);
        } else {
            ALOGW("attr %s is not processed", attr[i]);
        }
        i += 2;
    }

    platform_set_misc_config(g_platform, &feature);

    return;
}

static void process_bt_connect_config(const XML_Char **attr) {
    DEBUG_FUNC_PRT

    if (strncmp(attr[0], "sink", strlen("sink")) != 0) {
        AALOGE("'sink' not found!");
        goto done;
    }
    if (!strcmp(attr[1], "soc"))
        platform_set_bt_connect(g_platform, BT_CONNECT_WITH_SOC);
    else if (!strcmp(attr[1], "dsp"))
        platform_set_bt_connect(g_platform, BT_CONNECT_WITH_DSP);
done:
    return;
}

static void start_tag(void *userdata __unused, const XML_Char *tag_name, const XML_Char **attr) {
    AALOGV("tag_name: %s", tag_name);
    int32_t fn_indx = -1;
    if (!strcmp(tag_name, "card") && !strcmp(attr[0], "name")) {
        strcpy(card[card_num], attr[1]);
    } else if (!strcmp(tag_name, "playback")) {
        fn_indx = SECTION_CARD_PLAYBACK_CONFIG;
    } else if (!strcmp(tag_name, "capture")) {
        fn_indx = SECTION_CARD_CAPTURE_CONFIG;
    } else if (!strcmp(tag_name, "misc")) {
        fn_indx = SECTION_FEATURE_MISC;
    } else if (!strcmp(tag_name, "bt_connection")) {
        fn_indx = SECTION_BT_CONNECTION;
    } else if (!parse_bus_mix && !strcmp(tag_name, "slot_map")) {
        AALOGI("------");
        if (!strcmp(attr[0], "enable") && !strcmp(attr[1], "true"))
            parse_bus_slotmap = true;
    } else if (parse_bus_slotmap && !strcmp(tag_name, "out")) {
        fn_indx = SECTION_BUS_SLOT_MAP;
    }

    if (0 <= fn_indx && section_table[fn_indx])
        section_table[fn_indx](attr);
}

static void end_tag(void *userdata __unused, const XML_Char *tag_name) {
    AALOGV("tag_name: %s", tag_name);
    if (!strcmp(tag_name, "card"))
        card_num++;
}

int32_t platform_info_init(void *platform) {
    int32_t ret = 0;
    int32_t bytes_read = 0;
    uint32_t buf_sz = 2048;
    char xml_path[128] = {0};
    void *buf = NULL;
    FILE *file = NULL;
    XML_Parser parser = NULL;

    if (!platform) {
        AALOGE("Invalid arguments");
        ret = -EINVAL;
        goto error;
    }
    g_platform = platform;

    sprintf(xml_path, "%s%s", PLATFORM_INFO_PATH, PLATFORM_INFO_NAME);
    file = fopen(xml_path, "r");

    if (!file) {
        AALOGE("%s: Failed to open %s, using defaults.", __func__, xml_path);
        ret = -ENODEV;
        goto error;
    }

    parser = XML_ParserCreate(NULL);
    if (!parser) {
        AALOGE("%s: Failed to create XML parser!", __func__);
        ret = -ENODEV;
        goto error;
    }

    XML_SetElementHandler(parser, start_tag, end_tag);

    while (1) {
        AALOGI("parse xml");
        buf = XML_GetBuffer(parser, buf_sz);
        if (buf == NULL) {
            AALOGE("%s: XML_GetBuffer failed", __func__);
            ret = -ENOMEM;
            goto error;
        }

        bytes_read = fread(buf, 1, buf_sz, file);
        if (bytes_read < 0) {
            AALOGE("%s: fread failed, bytes read = %d", __func__, bytes_read);
            ret = bytes_read;
            ret = -ENOMEM;
            goto error;
        }

        if (XML_ParseBuffer(parser, bytes_read, bytes_read == 0) == XML_STATUS_ERROR) {
            AALOGE("%s: XML_ParseBuffer failed, for %s", __func__, xml_path);
            ret = -EINVAL;
            goto error;
        }

        if (bytes_read == 0)
            break;
    }

error:
    if (file)
        fclose(file);
    if (parser)
        XML_ParserFree(parser);
    return ret;
}