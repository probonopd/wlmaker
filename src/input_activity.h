/* ========================================================================= */
/**
 * @file input_activity.h
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
#ifndef __INPUT_ACTIVITY_H__
#define __INPUT_ACTIVITY_H__

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/** Forward declaration: State of input activity manager. */
typedef struct _wlmaker_input_activity_manager_t wlmaker_input_activity_manager_t;

/**
 * Creates an input activity manager.
 *
 * @param wl_display_ptr
 *
 * @return Handle of the input activity manager or NULL on error. The pointer
 *     must be destroyed by @ref wlmaker_input_activity_manager_destroy.
 */
wlmaker_input_activity_manager_t *wlmaker_input_activity_manager_create(
    struct wl_display *wl_display_ptr);

/**
 * Destroys the input activity manager.
 *
 * @param wia_manager_ptr
 */
void wlmaker_input_activity_manager_destroy(
    wlmaker_input_activity_manager_t *wia_manager_ptr);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif /* __INPUT_ACTIVITY_H__ */
/* == End of input_activity.h ============================================== */
