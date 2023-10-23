/* ========================================================================= */
/**
 * @file titlebar.h
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
#ifndef __WLMTK_TITLEBAR_H__
#define __WLMTK_TITLEBAR_H__

/** Forward declaration: Title bar. */
typedef struct _wlmtk_titlebar_t wlmtk_titlebar_t;

#include "element.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Creates a title bar, suitable as a window title.
 *
 * @return Pointer to the title bar state, or NULL on error. Must be free'd
 *     by calling @ref wlmtk_titlebar_destroy.
 */
wlmtk_titlebar_t *wlmtk_titlebar_create(void);

/**
 * Destroys the title bar.
 *
 * @param titlebar_ptr
 */
void wlmtk_titlebar_destroy(wlmtk_titlebar_t *titlebar_ptr);

/**
 * Returns the super Element of the titlebar.
 *
 * @param titlebar_ptr
 *
 * @return Pointer to the @ref wlmtk_element_t base instantiation to
 *     titlebar_ptr.
 */
wlmtk_element_t *wlmtk_titlebar_element(wlmtk_titlebar_t *titlebar_ptr);

/** Unit test cases. */
extern const bs_test_case_t wlmtk_titlebar_test_cases[];

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif /* __WLMTK_TITLEBAR_H__ */
/* == End of titlebar.h ==================================================== */
