/* ========================================================================= */
/**
 * @file workspace.c
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

#include "toolkit.h"

#define WLR_USE_UNSTABLE
#include <wlr/types/wlr_scene.h>
#undef WLR_USE_UNSTABLE

/* == Declarations ========================================================= */

/** State of the workspace. */
struct _wlmtk_workspace_t {
    /** Superclass: Container. */
    wlmtk_container_t         super_container;

    /**
     * The workspace's element map() method will expect a parent from where to
     * retrieve the wlroots scene graph tree from. As a toplevel construct,
     * there is not really a parent, so we use this fake class instead.
     *
     * TODO(kaeser@gubbe.ch): This should live in wlmaker_server_t; ultimately
     * that is the "container" that holds all workspaces.
     */
    wlmtk_container_t         fake_parent;
};

static void workspace_container_destroy(wlmtk_container_t *container_ptr);

/** Method table for the container's virtual methods. */
const wlmtk_container_impl_t  workspace_container_impl = {
    .destroy = workspace_container_destroy
};

/* == Exported methods ===================================================== */

/* ------------------------------------------------------------------------- */
wlmtk_workspace_t *wlmtk_workspace_create(
    struct wlr_scene_tree *wlr_scene_tree_ptr)
{
    wlmtk_workspace_t *workspace_ptr =
        logged_calloc(1, sizeof(wlmtk_workspace_t));
    if (NULL == workspace_ptr) return NULL;

    if (!wlmtk_container_init(&workspace_ptr->super_container,
                              &workspace_container_impl)) {
        wlmtk_workspace_destroy(workspace_ptr);
        return NULL;
    }

    workspace_ptr->fake_parent.wlr_scene_tree_ptr = wlr_scene_tree_ptr;
    wlmtk_element_set_parent_container(
        &workspace_ptr->super_container.super_element,
        &workspace_ptr->fake_parent);

    wlmtk_element_map(&workspace_ptr->super_container.super_element);

    return workspace_ptr;
}

/* ------------------------------------------------------------------------- */
void wlmtk_workspace_destroy(wlmtk_workspace_t *workspace_ptr)
{
    wlmtk_element_unmap(&workspace_ptr->super_container.super_element);

    wlmtk_element_set_parent_container(
        &workspace_ptr->super_container.super_element,
        NULL);

    wlmtk_container_fini(&workspace_ptr->super_container);
    free(workspace_ptr);
}

/* == Local (static) methods =============================================== */

/* ------------------------------------------------------------------------- */
/** Virtual destructor, in case called from container. Wraps to our dtor. */
void workspace_container_destroy(wlmtk_container_t *container_ptr)
{
    wlmtk_workspace_t *workspace_ptr = BS_CONTAINER_OF(
        container_ptr, wlmtk_workspace_t, super_container);
    wlmtk_workspace_destroy(workspace_ptr);
}

/* == Unit tests =========================================================== */

static void test_create_destroy(bs_test_t *test_ptr);

const bs_test_case_t wlmtk_workspace_test_cases[] = {
    { 1, "create_destroy", test_create_destroy },
    { 0, NULL, NULL }
};

/* ------------------------------------------------------------------------- */
/** Exercises workspace create & destroy methods. */
void test_create_destroy(bs_test_t *test_ptr)
{
    struct wlr_scene *wlr_scene_ptr = wlr_scene_create();

    wlmtk_workspace_t *workspace_ptr = wlmtk_workspace_create(
        &wlr_scene_ptr->tree);
    BS_TEST_VERIFY_NEQ(test_ptr, NULL, workspace_ptr);

    wlmtk_workspace_destroy(workspace_ptr);

    // Note: There is no destroy method for wlr_scene_ptr.
}

/* == End of workspace.c =================================================== */
