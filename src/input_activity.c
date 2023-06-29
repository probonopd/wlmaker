/* ========================================================================= */
/**
 * @file input_activity.c
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

#include "input_activity.h"

#include <libbase/libbase.h>

#include "wlmaker-input-activity-unstable-v1-server-protocol.h"

/* == Declarations ========================================================= */

/** State of the input activity manager. */
struct _wlmaker_input_activity_manager_t {
    /** The global holding the input activity manager's interface. */
    struct wl_global          *wl_global_ptr;
};

static void bind_input_activity_manager(
    struct wl_client *wl_client_ptr,
    void *data_ptr,
    uint32_t version,
    uint32_t id);
static void input_activity_manager_resource_destroy(
    struct wl_resource *wl_resource_ptr);

/* == Exported methods ===================================================== */

/* ------------------------------------------------------------------------- */
wlmaker_input_activity_manager_t *wlmaker_input_activity_manager_create(
    struct wl_display *wl_display_ptr)
{
    wlmaker_input_activity_manager_t *wia_activity_ptr = logged_calloc(
        1, sizeof(wlmaker_input_activity_manager_t));
    if (NULL == wia_activity_ptr) return NULL;

    wia_activity_ptr->wl_global_ptr = wl_global_create(
        wl_display_ptr,
        &zwlmaker_input_activity_manager_v1_interface,
        1,
        wia_activity_ptr,
        bind_input_activity_manager);
    if (NULL == wia_activity_ptr->wl_global_ptr) {
        wlmaker_input_activity_manager_destroy(wia_activity_ptr);
        return NULL;
    }

    bs_log(BS_INFO, "Created activity %p", wia_activity_ptr);
    return wia_activity_ptr;
}

/* ------------------------------------------------------------------------- */
void wlmaker_input_activity_manager_destroy(
    wlmaker_input_activity_manager_t *wia_activity_ptr)
{
    if (NULL != wia_activity_ptr->wl_global_ptr) {
        wl_global_destroy(wia_activity_ptr->wl_global_ptr);
        wia_activity_ptr->wl_global_ptr = NULL;
    }

    free(wia_activity_ptr);
}

/* == Local (static) methods =============================================== */

/* ------------------------------------------------------------------------- */
/**
 * Binds an input activity manager for the client.
 *
 * @param wl_client_ptr
 * @param data_ptr
 * @param version
 * @param id
 */
void bind_input_activity_manager(
    struct wl_client *wl_client_ptr,
    void *data_ptr,
    uint32_t version,
    uint32_t id)
{
    bs_log(BS_DEBUG, "Bind input activity manager: client %p, data %p, "
           "version %"PRIu32", id %"PRIu32,
           wl_client_ptr, data_ptr, version, id);

    wlmaker_input_activity_manager_t *wia_activity_ptr = data_ptr;

    struct wl_resource *wl_resource_ptr = wl_resource_create(
        wl_client_ptr,
        &zwlmaker_input_activity_manager_v1_interface,
        version, id);
    bs_log(BS_INFO, "Created resource %p for activity manager %p",
           wl_resource_ptr, wia_activity_ptr);

    wl_resource_set_implementation(
        wl_resource_ptr,
        NULL,  // implementation.
        NULL,  // data
        input_activity_manager_resource_destroy);
}

/* ------------------------------------------------------------------------- */
/**
 * Cleans up what's associated with the activity manager resource.
 *
 * @param wl_resource_ptr
 */
void input_activity_manager_resource_destroy(
    struct wl_resource *wl_resource_ptr)
{
    bs_log(BS_INFO, "Destroying resource %p", wl_resource_ptr);
}

/* == End of input_activity.c ============================================== */
