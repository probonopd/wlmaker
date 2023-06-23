/* ========================================================================= */
/**
 * @file activity.c
 *
 * @copyright
 * Copyright 2023 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "activity.h"

#include <libbase/libbase.h>
#include "wlm-input-activity-v1-server.h"


/* == Declarations ========================================================= */

struct _wlmaker_activity_t {
    struct wl_global          *wl_global_ptr;
};

static void activity_bind(
    struct wl_client *wl_client,
    void *data,
    uint32_t version,
    uint32_t id);

/* == Exported methods ===================================================== */

/* ------------------------------------------------------------------------- */
wlmaker_activity_t *wlmaker_activity_create(struct wl_display *wl_display_ptr)
{
    wlmaker_activity_t *activity_ptr = logged_calloc(
        1, sizeof(wlmaker_activity_t));
    if (NULL == activity_ptr) return NULL;

    activity_ptr->wl_global_ptr = wl_global_create(
        wl_display_ptr,
        &wlm_input_activity_v1_interface,
        1,
        activity_ptr,
        activity_bind);
    if (NULL == activity_ptr->wl_global_ptr) {
        wlmaker_activity_destroy(activity_ptr);
        return NULL;
    }

    return activity_ptr;
}

/* ------------------------------------------------------------------------- */
void wlmaker_activity_destroy(wlmaker_activity_t *activity_ptr)
{
    if (NULL != activity_ptr->wl_global_ptr) {
        wl_global_destroy(activity_ptr->wl_global_ptr);
        activity_ptr->wl_global_ptr = NULL;
    }

    free(activity_ptr);
}

/* == Local (static) methods =============================================== */

/* ------------------------------------------------------------------------- */
void activity_bind(
    struct wl_client *wl_client,
    void *data,
    uint32_t version,
    uint32_t id)
{
    bs_log(BS_INFO, "FIXME: Bind activity: client %p, data %p, "
           "version %"PRIu32", id %"PRIu32,
           wl_client, data, version, id);
}

/* == End of activity.c ==================================================== */
