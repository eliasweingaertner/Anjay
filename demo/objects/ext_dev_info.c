/*
 * Copyright 2017-2018 AVSystem <avsystem@avsystem.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../demo_utils.h"
#include "../objects.h"

#include <assert.h>
#include <time.h>

#include <anjay/stats.h>

#define EXT_DEV_RES_OBU_ID 0                        // string
#define EXT_DEV_RES_PLATE_NUMBER 1                  // string
#define EXT_DEV_RES_IMEI 2                          // string
#define EXT_DEV_RES_IMSI 3                          // string
#define EXT_DEV_RES_ICCID 4                         // string
#define EXT_DEV_RES_GPRS_RSSI 5                     // int
#define EXT_DEV_RES_GPRS_PLMN 6                     // int
#define EXT_DEV_RES_GPRS_ULMODULATION 7             // string
#define EXT_DEV_RES_GPRS_DLMODULATION 8             // string
#define EXT_DEV_RES_GPRS_ULFREQUENCY 9              // int
#define EXT_DEV_RES_GPRS_DLFREQUENCY 10             // int
#define EXT_DEV_RES_RX_BYTES 11                     // uint64
#define EXT_DEV_RES_TX_BYTES 12                     // uint64
#define EXT_DEV_RES_NUM_INCOMING_RETRANSMISSIONS 13 // uint64
#define EXT_DEV_RES_NUM_OUTGOING_RETRANSMISSIONS 14 // uint64
#define EXT_DEV_RES_UPTIME 15                       // double

typedef struct {
    const anjay_dm_object_def_t *def;
    avs_time_monotonic_t init_time;
} extdev_repr_t;

static inline extdev_repr_t *
get_extdev(const anjay_dm_object_def_t *const *obj_ptr) {
    assert(obj_ptr);
    return container_of(obj_ptr, extdev_repr_t, def);
}

static int generate_fake_rssi_value(void) {
    return 50 + (int) (time_to_rand() % 20);
}

static int dev_read(anjay_t *anjay,
                    const anjay_dm_object_def_t *const *obj_ptr,
                    anjay_iid_t iid,
                    anjay_rid_t rid,
                    anjay_output_ctx_t *ctx) {
    (void) anjay;
    (void) iid;

    switch (rid) {
    case EXT_DEV_RES_OBU_ID:
        return anjay_ret_string(ctx, "Dummy_OBU_ID");
    case EXT_DEV_RES_PLATE_NUMBER:
        return anjay_ret_string(ctx, "PL 473N0");
    case EXT_DEV_RES_IMEI:
        return anjay_ret_string(ctx, "01-345678-901234");
    case EXT_DEV_RES_IMSI:
        return anjay_ret_string(ctx, "26000007");
    case EXT_DEV_RES_ICCID:
        return anjay_ret_string(ctx, "8926000000073");
    case EXT_DEV_RES_GPRS_RSSI:
        return anjay_ret_i32(ctx, generate_fake_rssi_value());
    case EXT_DEV_RES_GPRS_PLMN:
        return anjay_ret_i32(ctx, 26001);
    case EXT_DEV_RES_GPRS_ULMODULATION:
        return anjay_ret_string(ctx, "GMSK");
    case EXT_DEV_RES_GPRS_DLMODULATION:
        return anjay_ret_string(ctx, "GMSK");
    case EXT_DEV_RES_GPRS_ULFREQUENCY:
        return anjay_ret_i32(ctx, 1950);
    case EXT_DEV_RES_GPRS_DLFREQUENCY:
        return anjay_ret_i32(ctx, 2140);
    case EXT_DEV_RES_RX_BYTES:
        return anjay_ret_i64(ctx, (int64_t) anjay_get_rx_bytes(anjay));
    case EXT_DEV_RES_TX_BYTES:
        return anjay_ret_i64(ctx, (int64_t) anjay_get_tx_bytes(anjay));
    case EXT_DEV_RES_NUM_INCOMING_RETRANSMISSIONS:
        return anjay_ret_i64(
                ctx, (int64_t) anjay_get_num_incoming_retransmissions(anjay));
    case EXT_DEV_RES_NUM_OUTGOING_RETRANSMISSIONS:
        return anjay_ret_i64(
                ctx, (int64_t) anjay_get_num_outgoing_retransmissions(anjay));
    case EXT_DEV_RES_UPTIME: {
        avs_time_duration_t diff =
                avs_time_monotonic_diff(avs_time_monotonic_now(),
                                        get_extdev(obj_ptr)->init_time);

        return anjay_ret_double(ctx, (double) diff.seconds
                                             + (double) diff.nanoseconds / 1e9);
    }
    default:
        return ANJAY_ERR_NOT_FOUND;
    }
}

static const anjay_dm_object_def_t EXT_DEV_INFO = {
    .oid = DEMO_OID_EXT_DEV_INFO,
    .supported_rids =
            ANJAY_DM_SUPPORTED_RIDS(EXT_DEV_RES_OBU_ID,
                                    EXT_DEV_RES_PLATE_NUMBER,
                                    EXT_DEV_RES_IMEI,
                                    EXT_DEV_RES_IMSI,
                                    EXT_DEV_RES_ICCID,
                                    EXT_DEV_RES_GPRS_RSSI,
                                    EXT_DEV_RES_GPRS_PLMN,
                                    EXT_DEV_RES_GPRS_ULMODULATION,
                                    EXT_DEV_RES_GPRS_DLMODULATION,
                                    EXT_DEV_RES_GPRS_ULFREQUENCY,
                                    EXT_DEV_RES_GPRS_DLFREQUENCY,
                                    EXT_DEV_RES_RX_BYTES,
                                    EXT_DEV_RES_TX_BYTES,
                                    EXT_DEV_RES_NUM_INCOMING_RETRANSMISSIONS,
                                    EXT_DEV_RES_NUM_OUTGOING_RETRANSMISSIONS,
                                    EXT_DEV_RES_UPTIME),
    .handlers = {
        .instance_it = anjay_dm_instance_it_SINGLE,
        .instance_present = anjay_dm_instance_present_SINGLE,
        .resource_present = anjay_dm_resource_present_TRUE,
        .resource_read = dev_read
    }
};

const anjay_dm_object_def_t **ext_dev_info_object_create(void) {
    extdev_repr_t *repr =
            (extdev_repr_t *) avs_calloc(1, sizeof(extdev_repr_t));
    if (!repr) {
        return NULL;
    }

    repr->def = &EXT_DEV_INFO;
    repr->init_time = avs_time_monotonic_now();

    return &repr->def;
}

void ext_dev_info_object_release(const anjay_dm_object_def_t **def) {
    if (def) {
        avs_free(get_extdev(def));
    }
}

void ext_dev_info_notify_time_dependent(anjay_t *anjay,
                                        const anjay_dm_object_def_t **def) {
    anjay_notify_changed(anjay, (*def)->oid, 0, EXT_DEV_RES_GPRS_RSSI);
    anjay_notify_changed(anjay, (*def)->oid, 0, EXT_DEV_RES_UPTIME);
}
