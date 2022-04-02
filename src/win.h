#pragma once

#include "log.h"

#include <stdlib.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>

// unfortunately, you can't use pure XCB with EGL/GLX:
// https://xcb.freedesktop.org/opengl/

#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>

#include <EGL/egl.h>
#include <GL/gl.h>

typedef struct {
	bool running;
	uint32_t x_res, y_res;

	// X11 stuff

	Display* display;
	int default_screen;

	xcb_connection_t* connection;
	xcb_screen_t* screen;

	xcb_drawable_t win;

	xcb_atom_t wm_delete_window_atom;
	xcb_ewmh_connection_t ewmh;

	// EGL stuff

	gl_funcs_t* gl;

	EGLDisplay* egl_display;
	EGLContext* egl_context;
	EGLSurface* egl_surface;

	// timing stuff

	struct timespec last_exposure;

	// mouse event stuff

	bool exclusive_mouse;
	int mouse_dx, mouse_dy;
} win_t;

static const char* egl_error_str(void) {
	EGLint error = eglGetError();

	#define ERROR_CASE(error) \
		case error: return #error;

	switch (error) {
		ERROR_CASE(EGL_SUCCESS            )
		ERROR_CASE(EGL_NOT_INITIALIZED    )
		ERROR_CASE(EGL_BAD_ACCESS         )
		ERROR_CASE(EGL_BAD_ALLOC          )
		ERROR_CASE(EGL_BAD_ATTRIBUTE      )
		ERROR_CASE(EGL_BAD_CONTEXT        )
		ERROR_CASE(EGL_BAD_CONFIG         )
		ERROR_CASE(EGL_BAD_CURRENT_SURFACE)
		ERROR_CASE(EGL_BAD_DISPLAY        )
		ERROR_CASE(EGL_BAD_SURFACE        )
		ERROR_CASE(EGL_BAD_MATCH          )
		ERROR_CASE(EGL_BAD_PARAMETER      )
		ERROR_CASE(EGL_BAD_NATIVE_PIXMAP  )
		ERROR_CASE(EGL_BAD_NATIVE_WINDOW  )
		ERROR_CASE(EGL_CONTEXT_LOST       )

	default:
		return "unkown EGL error; consider setting 'EGL_LOG_LEVEL=debug'";
	}

	#undef ERROR_CASE
}

void win_set_caption(win_t* self, char* caption) {
	xcb_change_property(self->connection, XCB_PROP_MODE_REPLACE, self->win, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(caption) /* don't need to include null */, caption);

	xcb_ewmh_set_wm_name(&self->ewmh, self->win, strlen(caption) + 1, caption);
	xcb_ewmh_set_wm_visible_name(&self->ewmh, self->win, strlen(caption) + 1, caption);

	xcb_flush(self->connection);
}

void win_set_mouse_pos(win_t* self, uint32_t x, uint32_t y) {
	if (x == -1 || y == -1) {
		x = self->x_res / 2;
		y = self->y_res / 2;
	}

	y = self->y_res - y; // side-note: systems which consider top-left to be the origin are dumb

	xcb_warp_pointer(self->connection, 0, self->win, 0, 0, 0, 0, x, y);
}

void win_set_exclusive_mouse(win_t* self, bool exclusive) {
	if (self->exclusive_mouse == exclusive) {
		return;
	}

	self->exclusive_mouse = exclusive;

	// grab/ungrab pointer & show/hide cursor
	// yes, this is needlessly complicated and hacky, thank you X11 😞
	// basically, what I'm doing is create a new cursor with an empty pixmap in it

	if (exclusive) {
		xcb_pixmap_t empty_pixmap = xcb_generate_id(self->connection);
		xcb_create_pixmap(self->connection, 1, empty_pixmap, self->win, 1, 1);

		xcb_cursor_t cursor = xcb_generate_id(self->connection);
		xcb_create_cursor(self->connection, cursor, empty_pixmap, empty_pixmap, 0, 0, 0, 0, 0, 0, 0, 0);

		xcb_grab_pointer(self->connection, true, self->win, 0, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, self->win, cursor, XCB_CURRENT_TIME);

		xcb_free_pixmap(self->connection, empty_pixmap);
		xcb_free_cursor(self->connection, cursor);

		// grab focus if the WM doesn't do it for us (bug in aquaBSD WM)

		xcb_set_input_focus(self->connection, XCB_INPUT_FOCUS_PARENT, self->win, XCB_CURRENT_TIME);
	}

	else {
		xcb_ungrab_pointer(self->connection, XCB_CURRENT_TIME);
		xcb_change_window_attributes(self->connection, self->win, XCB_CW_CURSOR, (uint32_t[]) { 0 });
	}

	win_set_mouse_pos(self, -1, -1);
	xcb_flush(self->connection);
}

win_t* create_win(uint32_t x_res, uint32_t y_res) {
	win_t* self = calloc(1, sizeof *self);

	self->running = true;

	self->x_res = x_res;
	self->y_res = y_res;

	// get connection to the X server

	self->display = XOpenDisplay(NULL /* default to 'DISPLAY' environment variable */);

	if (!self->display) {
		FATAL_ERROR("Failed to open X display\n")
	}

	self->default_screen = DefaultScreen(self->display);
	self->connection = XGetXCBConnection(self->display);

	if (!self->connection) {
		FATAL_ERROR("Failed to get XCB connection from X display\n")
	}

	XSetEventQueueOwner(self->display, XCBOwnsEventQueue);

	xcb_screen_iterator_t it = xcb_setup_roots_iterator(xcb_get_setup(self->connection));
	for (int i = self->default_screen; it.rem && i > 0; i--, xcb_screen_next(&it));

	self->screen = it.data;

	// create window

	const uint32_t window_attribs[] = {
		XCB_EVENT_MASK_STRUCTURE_NOTIFY |
		XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE,
	};

	self->win = xcb_generate_id(self->connection);

	xcb_create_window(
		self->connection, XCB_COPY_FROM_PARENT, self->win, self->screen->root,
		0, 0, x_res, y_res, 0, // window geometry
		XCB_WINDOW_CLASS_INPUT_OUTPUT, self->screen->root_visual,
		XCB_CW_EVENT_MASK, window_attribs);

	xcb_map_window(self->connection, self->win);

	// setup 'WM_DELETE_WINDOW' protocol (yes this is dumb, thank you XCB & X11)

	xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(self->connection, 1, 12 /* strlen("WM_PROTOCOLS") */, "WM_PROTOCOLS");
	xcb_atom_t wm_protocols_atom = xcb_intern_atom_reply(self->connection, wm_protocols_cookie, 0)->atom;

	xcb_intern_atom_cookie_t wm_delete_window_cookie = xcb_intern_atom(self->connection, 0, 16 /* strlen("WM_DELETE_WINDOW") */, "WM_DELETE_WINDOW");
	self->wm_delete_window_atom = xcb_intern_atom_reply(self->connection, wm_delete_window_cookie, 0)->atom;

	xcb_icccm_set_wm_protocols(self->connection, self->win, wm_protocols_atom, 1, &self->wm_delete_window_atom);

	// EWMH

	xcb_intern_atom_cookie_t* cookies = xcb_ewmh_init_atoms(self->connection, &self->ewmh);

	if (!xcb_ewmh_init_atoms_replies(&self->ewmh, cookies, NULL)) {
		FATAL_ERROR("Failed to get EWMH atoms\n")
	}

	// set sensible minimum and maximum sizes for the window

	xcb_size_hints_t hints = { 0 };

	xcb_icccm_size_hints_set_min_size(&hints, 320, 200);
	// no maximum size

	xcb_icccm_set_wm_size_hints(self->connection, self->win, XCB_ATOM_WM_NORMAL_HINTS, &hints);

	// extra window setup

	win_set_caption(self, "Gamejam 2022");
	win_set_exclusive_mouse(self, false);

	// create context with EGL

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		FATAL_ERROR("Failed to bind EGL API (%s)\n", egl_error_str())
	}

	self->egl_display = eglGetDisplay(self->display);

	if (self->egl_display == EGL_NO_DISPLAY) {
		FATAL_ERROR("Failed to get EGL display from X11 display (%s)\n", egl_error_str())
	}

	__attribute__((unused)) int major;
	__attribute__((unused)) int minor;

	if (!eglInitialize(self->egl_display, &major, &minor)) {
		FATAL_ERROR("Failed to initialize EGL (%s)\n", egl_error_str())
	}

	const int config_attribs[] = {
		EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
		EGL_BUFFER_SIZE,       32,
		EGL_RED_SIZE,          8,
		EGL_GREEN_SIZE,        8,
		EGL_BLUE_SIZE,         8,
		EGL_ALPHA_SIZE,        8,
		EGL_DEPTH_SIZE,        16,

		EGL_SAMPLE_BUFFERS,    1,
		EGL_SAMPLES,           4,

		EGL_SURFACE_TYPE,      EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE,   EGL_OPENGL_BIT,

		EGL_NONE
	};

	EGLConfig config;
	EGLint config_count;

	if (!eglChooseConfig(self->egl_display, config_attribs, &config, 1, &config_count) || !config_count) {
		FATAL_ERROR("Failed to find a suitable EGL config (%s)\n", egl_error_str())
	}

	const int context_attribs[] = {
		// OpenGL ES 3.0

		EGL_CONTEXT_MAJOR_VERSION, 3,
		EGL_CONTEXT_MINOR_VERSION, 0,
		EGL_NONE
	};

	self->egl_context = eglCreateContext(self->egl_display, config, EGL_NO_CONTEXT, context_attribs);

	if (!self->egl_context) {
		FATAL_ERROR("Failed to create EGL context (%s)\n", egl_error_str())
	}

	const int surface_attribs[] = {
		EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
		EGL_NONE
	};

	self->egl_surface = eglCreateWindowSurface(self->egl_display, config, self->win, surface_attribs);

	if (!self->egl_surface) {
		FATAL_ERROR("Failed to create EGL surface (%s)\n", egl_error_str())
	}

	if (!eglMakeCurrent(self->egl_display, self->egl_surface, self->egl_surface, self->egl_context)) {
		FATAL_ERROR("Failed to make the EGL context the current GL context (%s)\n", egl_error_str())
	}

	EGLint render_buffer;

	if (!eglQueryContext(self->egl_display, self->egl_context, EGL_RENDER_BUFFER, &render_buffer) || render_buffer == EGL_SINGLE_BUFFER) {
		WARN("EGL surface is single buffered (%s)\n", egl_error_str())
	}

	return self;
}

static inline void __process_event(win_t* self, int type, xcb_generic_event_t* event) {
	gl_funcs_t* gl = self->gl;

	if (type == XCB_CLIENT_MESSAGE) {
		xcb_client_message_event_t* specific = (void*) event;

		if (specific->data.data32[0] == self->wm_delete_window_atom) {
			self->running = false;
		}
	}

	else if (type == XCB_CONFIGURE_NOTIFY) {
		xcb_configure_notify_event_t* detail = (void*) event;

		self->x_res = detail->width;
		self->y_res = detail->height;

		gl->Viewport(0, 0, self->x_res, self->y_res);
	}

	else if (type == XCB_KEY_PRESS) {
		xcb_key_press_event_t* detail = (void*) event;
		xcb_keycode_t key = detail->detail;

		// you can get keycodes for this shit quite easily with the 'xev' tool

		if (key == 9 /* ESC */) {
			win_set_exclusive_mouse(self, false);
		}
	}

	else if (type == XCB_KEY_RELEASE) {
		xcb_key_release_event_t* detail = (void*) event;
		xcb_keycode_t key = detail->detail;
	}

	else if (type == XCB_BUTTON_PRESS) {
		xcb_button_press_event_t* detail = (void*) event;
		win_set_exclusive_mouse(self, true);
	}

	#define GENERIC_MOTION_EVENT(T, name) \
		else if (type == XCB_##name) { \
			T* detail = (void*) event; \
			\
			self->mouse_dx = detail->event_x - self->x_res / 2; \
			self->mouse_dy = detail->event_y - self->y_res / 2; \
		}

	GENERIC_MOTION_EVENT(xcb_motion_notify_event_t, MOTION_NOTIFY)
	GENERIC_MOTION_EVENT(xcb_enter_notify_event_t,  ENTER_NOTIFY )
	GENERIC_MOTION_EVENT(xcb_leave_notify_event_t,  LEAVE_NOTIFY )

	#undef GENERIC_MOTION_EVENT
}

void win_loop(win_t* self, int (*draw_cb) (void* param, float dt), void* param) {
	while (self->running) {
		// get time between two frames

		struct timespec now = { 0, 0 };
		clock_gettime(CLOCK_MONOTONIC, &now);

		float last_seconds = (float) self->last_exposure.tv_sec + 1.0e-9 * self->last_exposure.tv_nsec;
		float now_seconds = (float) now.tv_sec + 1.0e-9 * now.tv_nsec;

		memcpy(&self->last_exposure, &now, sizeof self->last_exposure);
		float dt = now_seconds - last_seconds;

		// continiously centre the mouse if it's exclusive to us
		// otherwise, ignore any mouse movement

		if (self->exclusive_mouse) {
			win_set_mouse_pos(self, -1, -1);
		}

		else {
			self->mouse_dx = 0;
			self->mouse_dy = 0;
		}

		// draw & swap buffers

		draw_cb(param, dt);
		eglSwapBuffers(self->egl_display, self->egl_surface);

		// process events

		for (xcb_generic_event_t* event; (event = xcb_poll_for_event(self->connection)); free(event)) {
			int type = event->response_type & XCB_EVENT_RESPONSE_TYPE_MASK;
			__process_event(self, type, event);
		}
	}
}
