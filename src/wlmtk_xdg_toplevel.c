/* ========================================================================= */
/**
 * @file xdg_toplevel.c
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

#include "xdg_toplevel.h"

/* == Declarations ========================================================= */

/** State of the content for an XDG toplevel surface. */
typedef struct {
    /** Super class. */
    wlmtk_surface_t           super_surface;
    /** The... other super class. FIXME. */
    wlmtk_content_t           super_content;

    /** Back-link to server. */
    wlmaker_server_t          *server_ptr;

    /** The corresponding wlroots XDG surface. */
    struct wlr_xdg_surface    *wlr_xdg_surface_ptr;
    /** Whether this surface is currently activated. */
    bool                      activated;

    /** Listener for the `destroy` signal of the `wlr_xdg_surface::events`. */
    struct wl_listener        destroy_listener;
    /** Listener for the `new_popup` signal of the `wlr_xdg_surface`. */
    struct wl_listener        new_popup_listener;
    /** Listener for the `map` signal of the `wlr_surface`. */
    struct wl_listener        surface_map_listener;
    /** Listener for the `unmap` signal of the `wlr_surface`. */
    struct wl_listener        surface_unmap_listener;
    /** Listener for the `commit` signal of the `wlr_surface`. */
    struct wl_listener        surface_commit_listener;

    /** Listener for `maximize` signal of `wlr_xdg_toplevel::events`. */
    struct wl_listener        toplevel_request_maximize_listener;
    /** Listener for `request_move` signal of `wlr_xdg_toplevel::events`. */
    struct wl_listener        toplevel_request_move_listener;
    /** Listener for `request_resize` signal of `wlr_xdg_toplevel::events`. */
    struct wl_listener        toplevel_request_resize_listener;
    /** Listener for the `set_title` signal of the `wlr_xdg_toplevel`. */
    struct wl_listener        toplevel_set_title_listener;
} wlmtk_xdg_toplevel_surface_t;

static wlmtk_xdg_toplevel_surface_t *xdg_toplevel_surface_create(
    struct wlr_xdg_surface *wlr_xdg_surface_ptr,
    wlmaker_server_t *server_ptr);
static void xdg_toplevel_surface_destroy(
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr);

static void handle_destroy(
    struct wl_listener *listener_ptr,
    void *data_ptr);
static void handle_new_popup(
    struct wl_listener *listener_ptr,
    void *data_ptr);
static void handle_surface_map(
    struct wl_listener *listener_ptr,
    void *data_ptr);
static void handle_surface_unmap(
    struct wl_listener *listener_ptr,
    void *data_ptr);
static void handle_surface_commit(
    struct wl_listener *listener_ptr,
    void *data_ptr);
static void handle_toplevel_request_maximize(
    struct wl_listener *listener_ptr,
    void *data_ptr);
static void handle_toplevel_request_move(
    struct wl_listener *listener_ptr,
    void *data_ptr);
static void handle_toplevel_request_resize(
    struct wl_listener *listener_ptr,
    void *data_ptr);
static void handle_toplevel_set_title(
    struct wl_listener *listener_ptr,
    void *data_ptr);

static void surface_element_destroy(wlmtk_element_t *element_ptr);
static struct wlr_scene_node *surface_element_create_scene_node(
    wlmtk_element_t *element_ptr,
    struct wlr_scene_tree *wlr_scene_tree_ptr);
static void surface_request_close(
    wlmtk_surface_t *surface_ptr);
static uint32_t surface_request_size(
    wlmtk_surface_t *surface_ptr,
    int width,
    int height);
static void surface_set_activated(
    wlmtk_surface_t *surface_ptr,
    bool activated);

/* == Data ================================================================= */

/** Virtual methods for XDG toplevel surface, for the Element superclass. */
const wlmtk_element_vmt_t     _wlmtk_xdg_toplevel_element_vmt = {
    .destroy = surface_element_destroy,
    .create_scene_node = surface_element_create_scene_node,
};

/** Virtual methods for XDG toplevel surface, for the Surface superclass. */
const wlmtk_surface_vmt_t     _wlmtk_xdg_toplevel_surface_vmt = {
    .request_close = surface_request_close,
    .request_size = surface_request_size,
    .set_activated = surface_set_activated,
};

/* == Exported methods ===================================================== */

/* ------------------------------------------------------------------------- */
wlmtk_window_t *wlmtk_window_create_from_xdg_toplevel(
    struct wlr_xdg_surface *wlr_xdg_surface_ptr,
    wlmaker_server_t *server_ptr)
{
    wlmtk_xdg_toplevel_surface_t *surface_ptr = xdg_toplevel_surface_create(
        wlr_xdg_surface_ptr, server_ptr);
    if (NULL == surface_ptr) return NULL;

    wlmtk_window_t *wlmtk_window_ptr = wlmtk_window_create_content(
        server_ptr->env_ptr, &surface_ptr->super_content);
    if (NULL == wlmtk_window_ptr) {
        surface_element_destroy(&surface_ptr->super_surface.super_element);
        return NULL;
    }
    wlmtk_surface_set_window(&surface_ptr->super_surface, wlmtk_window_ptr);

    return wlmtk_window_ptr;
}

/* == Local (static) methods =============================================== */

/* ------------------------------------------------------------------------- */
wlmtk_xdg_toplevel_surface_t *xdg_toplevel_surface_create(
    struct wlr_xdg_surface *wlr_xdg_surface_ptr,
    wlmaker_server_t *server_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = logged_calloc(
        1, sizeof(wlmtk_xdg_toplevel_surface_t));
    if (NULL == xdg_tl_surface_ptr) return NULL;

    if (!wlmtk_surface_init(
            &xdg_tl_surface_ptr->super_surface,
            wlr_xdg_surface_ptr->surface,
            server_ptr->env_ptr)) {
        xdg_toplevel_surface_destroy(xdg_tl_surface_ptr);
        return NULL;
    }
    wlmtk_element_extend(
        &xdg_tl_surface_ptr->super_surface.super_element,
        &_wlmtk_xdg_toplevel_element_vmt);
    wlmtk_surface_extend(
        &xdg_tl_surface_ptr->super_surface,
        &_wlmtk_xdg_toplevel_surface_vmt);
    xdg_tl_surface_ptr->wlr_xdg_surface_ptr = wlr_xdg_surface_ptr;
    xdg_tl_surface_ptr->server_ptr = server_ptr;

    if (!wlmtk_content_init(
            &xdg_tl_surface_ptr->super_content,
            &xdg_tl_surface_ptr->super_surface,
            server_ptr->env_ptr)) {
        xdg_toplevel_surface_destroy(xdg_tl_surface_ptr);
        return NULL;
    }

    wlmtk_util_connect_listener_signal(
        &wlr_xdg_surface_ptr->events.destroy,
        &xdg_tl_surface_ptr->destroy_listener,
        handle_destroy);
    wlmtk_util_connect_listener_signal(
        &wlr_xdg_surface_ptr->events.new_popup,
        &xdg_tl_surface_ptr->new_popup_listener,
        handle_new_popup);
    wlmtk_util_connect_listener_signal(
        &wlr_xdg_surface_ptr->surface->events.map,
        &xdg_tl_surface_ptr->surface_map_listener,
        handle_surface_map);
    wlmtk_util_connect_listener_signal(
        &wlr_xdg_surface_ptr->surface->events.unmap,
        &xdg_tl_surface_ptr->surface_unmap_listener,
        handle_surface_unmap);
    wlmtk_util_connect_listener_signal(
        &wlr_xdg_surface_ptr->surface->events.commit,
        &xdg_tl_surface_ptr->surface_commit_listener,
        handle_surface_commit);

    wlmtk_util_connect_listener_signal(
        &wlr_xdg_surface_ptr->toplevel->events.request_maximize,
        &xdg_tl_surface_ptr->toplevel_request_maximize_listener,
        handle_toplevel_request_maximize);
    wlmtk_util_connect_listener_signal(
        &wlr_xdg_surface_ptr->toplevel->events.request_move,
        &xdg_tl_surface_ptr->toplevel_request_move_listener,
        handle_toplevel_request_move);
    wlmtk_util_connect_listener_signal(
        &wlr_xdg_surface_ptr->toplevel->events.request_resize,
        &xdg_tl_surface_ptr->toplevel_request_resize_listener,
        handle_toplevel_request_resize);
    wlmtk_util_connect_listener_signal(
        &wlr_xdg_surface_ptr->toplevel->events.set_title,
        &xdg_tl_surface_ptr->toplevel_set_title_listener,
        handle_toplevel_set_title);

    xdg_tl_surface_ptr->wlr_xdg_surface_ptr->data =
        &xdg_tl_surface_ptr->super_surface;

    return xdg_tl_surface_ptr;
}

/* ------------------------------------------------------------------------- */
void xdg_toplevel_surface_destroy(
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr)
{
    wl_list_remove(
        &xdg_tl_surface_ptr->toplevel_set_title_listener.link);
    wl_list_remove(&xdg_tl_surface_ptr->toplevel_request_resize_listener.link);
    wl_list_remove(&xdg_tl_surface_ptr->toplevel_request_move_listener.link);
    wl_list_remove(&xdg_tl_surface_ptr->toplevel_request_maximize_listener.link);

    wl_list_remove(&xdg_tl_surface_ptr->surface_commit_listener.link);
    wl_list_remove(&xdg_tl_surface_ptr->surface_map_listener.link);
    wl_list_remove(&xdg_tl_surface_ptr->surface_unmap_listener.link);
    wl_list_remove(&xdg_tl_surface_ptr->new_popup_listener.link);
    wl_list_remove(&xdg_tl_surface_ptr->destroy_listener.link);

    wlmtk_surface_fini(&xdg_tl_surface_ptr->super_surface);
    free(xdg_tl_surface_ptr);
}

/* ------------------------------------------------------------------------- */
/**
 * Destructor. Wraps to @ref wlmtk_xdg_toplevel_surface_destroy.
 *
 * @param surface_ptr
 */
void surface_element_destroy(wlmtk_element_t *element_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        element_ptr, wlmtk_xdg_toplevel_surface_t,
        super_surface.super_element);
    xdg_toplevel_surface_destroy(xdg_tl_surface_ptr);
}

/* ------------------------------------------------------------------------- */
/**
 * Creates the wlroots scene graph API node, attached to `wlr_scene_tree_ptr`.
 *
 * @param surface_ptr
 * @param wlr_scene_tree_ptr
 *
 * @return Scene graph API node that represents the surface.
 */
struct wlr_scene_node *surface_element_create_scene_node(
    wlmtk_element_t *element_ptr,
    struct wlr_scene_tree *wlr_scene_tree_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        element_ptr, wlmtk_xdg_toplevel_surface_t,
        super_surface.super_element);

    struct wlr_scene_tree *surface_wlr_scene_tree_ptr =
        wlr_scene_xdg_surface_create(
            wlr_scene_tree_ptr,
            xdg_tl_surface_ptr->wlr_xdg_surface_ptr);
    return &surface_wlr_scene_tree_ptr->node;
}

/* ------------------------------------------------------------------------- */
/**
 * Requests the surface to close: Sends a 'close' message to the toplevel.
 *
 * @param surface_ptr
 */
void surface_request_close(wlmtk_surface_t *surface_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        surface_ptr, wlmtk_xdg_toplevel_surface_t, super_surface);

    wlr_xdg_toplevel_send_close(
        xdg_tl_surface_ptr->wlr_xdg_surface_ptr->toplevel);
}

/* ------------------------------------------------------------------------- */
/**
 * Sets the dimensions of the element in pixels.
 *
 * @param surface_ptr
 * @param width               Width of surface.
 * @param height              Height of surface.
 *
 * @return The serial.
 */
uint32_t surface_request_size(
    wlmtk_surface_t *surface_ptr,
    int width,
    int height)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        surface_ptr, wlmtk_xdg_toplevel_surface_t, super_surface);

    return wlr_xdg_toplevel_set_size(
        xdg_tl_surface_ptr->wlr_xdg_surface_ptr->toplevel, width, height);
}

/* ------------------------------------------------------------------------- */
/**
 * Sets the keyboard activation status for the surface.
 *
 * @param surface_ptr
 * @param activated
 */
void surface_set_activated(
    wlmtk_surface_t *surface_ptr,
    bool activated)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        surface_ptr, wlmtk_xdg_toplevel_surface_t, super_surface);
    // Early return, if nothing to be done.
    if (xdg_tl_surface_ptr->activated == activated) return;

    struct wlr_seat *wlr_seat_ptr =
        xdg_tl_surface_ptr->server_ptr->wlr_seat_ptr;
    wlr_xdg_toplevel_set_activated(
        xdg_tl_surface_ptr->wlr_xdg_surface_ptr->toplevel, activated);

    if (activated) {
        struct wlr_keyboard *wlr_keyboard_ptr = wlr_seat_get_keyboard(
            wlr_seat_ptr);
        if (NULL != wlr_keyboard_ptr) {
            wlr_seat_keyboard_notify_enter(
                wlr_seat_ptr,
                xdg_tl_surface_ptr->wlr_xdg_surface_ptr->surface,
                wlr_keyboard_ptr->keycodes,
                wlr_keyboard_ptr->num_keycodes,
                &wlr_keyboard_ptr->modifiers);
        }
    } else {
        BS_ASSERT(xdg_tl_surface_ptr->activated);
        // FIXME: This clears pointer focus. But, this is keyboard focus?
        if (wlr_seat_ptr->keyboard_state.focused_surface ==
            xdg_tl_surface_ptr->wlr_xdg_surface_ptr->surface) {
            wlr_seat_pointer_clear_focus(wlr_seat_ptr);
        }
    }

    xdg_tl_surface_ptr->activated = activated;
}

/* ------------------------------------------------------------------------- */
/**
 * Handler for the `destroy` signal of the `wlr_xdg_surface::events`.
 *
 * @param listener_ptr
 * @param data_ptr
 */
void handle_destroy(struct wl_listener *listener_ptr,
                    __UNUSED__ void *data_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        listener_ptr, wlmtk_xdg_toplevel_surface_t, destroy_listener);
    // Destroy the window -> also destroys the surface.
    wlmtk_window_destroy(xdg_tl_surface_ptr->super_surface.window_ptr);
}

/* ------------------------------------------------------------------------- */
/**
 * Handler for the `new_popup` signal.
 *
 * @param listener_ptr
 * @param data_ptr
 */
void handle_new_popup(
    struct wl_listener *listener_ptr,
    void *data_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        listener_ptr, wlmtk_xdg_toplevel_surface_t, new_popup_listener);

    bs_log(BS_WARNING, "FIXME: wlmtk_xdg_toplevel %p, New popup %p",
           xdg_tl_surface_ptr, data_ptr);
}

/* ------------------------------------------------------------------------- */
/**
 * Handler for the `map` signal.
 *
 * Issued when the XDG toplevel is fully configured and ready to be shown.
 * Will add it to the current workspace.
 *
 * @param listener_ptr
 * @param data_ptr
 */
void handle_surface_map(
    struct wl_listener *listener_ptr,
    __UNUSED__ void *data_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        listener_ptr, wlmtk_xdg_toplevel_surface_t, surface_map_listener);

    wlmtk_workspace_t *wlmtk_workspace_ptr = wlmaker_workspace_wlmtk(
        wlmaker_server_get_current_workspace(xdg_tl_surface_ptr->server_ptr));

    wlmtk_workspace_map_window(
        wlmtk_workspace_ptr,
        xdg_tl_surface_ptr->super_surface.window_ptr);
}

/* ------------------------------------------------------------------------- */
/**
 * Handler for the `unmap` signal. Removes it from the workspace.
 *
 * @param listener_ptr
 * @param data_ptr
 */
void handle_surface_unmap(
    struct wl_listener *listener_ptr,
    __UNUSED__ void *data_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        listener_ptr, wlmtk_xdg_toplevel_surface_t, surface_unmap_listener);

    wlmtk_window_t *window_ptr = xdg_tl_surface_ptr->super_surface.window_ptr;
    wlmtk_workspace_unmap_window(
        wlmtk_workspace_from_container(
            wlmtk_window_element(window_ptr)->parent_container_ptr),
        window_ptr);
}

/* ------------------------------------------------------------------------- */
/**
 * Handler for the `commit` signal.
 *
 * @param listener_ptr
 * @param data_ptr            Points to the const struct wlr_surface.
 */
void handle_surface_commit(
    struct wl_listener *listener_ptr,
    __UNUSED__ void *data_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        listener_ptr, wlmtk_xdg_toplevel_surface_t, surface_commit_listener);

    if (NULL == xdg_tl_surface_ptr->wlr_xdg_surface_ptr) return;

    wlmtk_content_commit_size(
        &xdg_tl_surface_ptr->super_content,
        xdg_tl_surface_ptr->wlr_xdg_surface_ptr->current.configure_serial,
        xdg_tl_surface_ptr->wlr_xdg_surface_ptr->current.geometry.width,
        xdg_tl_surface_ptr->wlr_xdg_surface_ptr->current.geometry.height);
}

/* ------------------------------------------------------------------------- */
/**
 * Handler for the `request_maximize` signal.
 *
 * @param listener_ptr
 * @param data_ptr
 */
void handle_toplevel_request_maximize(
    struct wl_listener *listener_ptr,
    __UNUSED__ void *data_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        listener_ptr,
        wlmtk_xdg_toplevel_surface_t,
        toplevel_request_maximize_listener);
    wlmtk_window_request_maximize(
        xdg_tl_surface_ptr->super_content.window_ptr,
        !wlmtk_window_maximized(xdg_tl_surface_ptr->super_content.window_ptr));
}

/* ------------------------------------------------------------------------- */
/**
 * Handler for the `request_move` signal.
 *
 * @param listener_ptr
 * @param data_ptr
 */
void handle_toplevel_request_move(
    struct wl_listener *listener_ptr,
    __UNUSED__ void *data_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        listener_ptr,
        wlmtk_xdg_toplevel_surface_t,
        toplevel_request_move_listener);
    wlmtk_window_request_move(xdg_tl_surface_ptr->super_surface.window_ptr);
}

/* ------------------------------------------------------------------------- */
/**
 * Handler for the `request_resize` signal.
 *
 * @param listener_ptr
 * @param data_ptr            Points to a struct wlr_xdg_toplevel_resize_event.
 */
void handle_toplevel_request_resize(
    struct wl_listener *listener_ptr,
    void *data_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        listener_ptr,
        wlmtk_xdg_toplevel_surface_t,
        toplevel_request_resize_listener);
    struct wlr_xdg_toplevel_resize_event *resize_event_ptr = data_ptr;
    wlmtk_window_request_resize(
        xdg_tl_surface_ptr->super_surface.window_ptr,
        resize_event_ptr->edges);
}

/* ------------------------------------------------------------------------- */
/**
 * Handler for the `set_title` signal.
 *
 * @param listener_ptr
 * @param data_ptr
 */
void handle_toplevel_set_title(
    struct wl_listener *listener_ptr,
    __UNUSED__ void *data_ptr)
{
    wlmtk_xdg_toplevel_surface_t *xdg_tl_surface_ptr = BS_CONTAINER_OF(
        listener_ptr,
        wlmtk_xdg_toplevel_surface_t,
        toplevel_set_title_listener);

    wlmtk_window_set_title(
        xdg_tl_surface_ptr->super_surface.window_ptr,
        xdg_tl_surface_ptr->wlr_xdg_surface_ptr->toplevel->title);
}

/* == End of xdg_toplevel.c ================================================ */
