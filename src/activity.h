/* ========================================================================= */
/**
 * @file activity.h
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
#ifndef __ACTIVITY_H__
#define __ACTIVITY_H__

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct _wlmaker_activity_t wlmaker_activity_t;

wlmaker_activity_t *wlmaker_activity_create(
    struct wl_display *wl_display_ptr);
void wlmaker_activity_destroy(wlmaker_activity_t *activity_ptr);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif /* __ACTIVITY_H__ */
/* == End of activity.h ================================================== */
