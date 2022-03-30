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
	uint32_t x_res, y_res;

	// X11 stuff

	Display* display;
	int default_screen;

	xcb_connection_t* connection;
	xcb_screen_t* screen;

	xcb_drawable_t window;
	
	xcb_atom_t wm_delete_window_atom;
	xcb_ewmh_connection_t ewmh;

	// EGL stuff

	EGLDisplay* egl_display;
	EGLContext* egl_context;
	EGLSurface* egl_surface;

	// timing stuff

	struct timespec last_exposure;
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
	xcb_ewmh_set_wm_name(&self->ewmh, self->window, strlen(caption) + 1, caption);
	xcb_flush(self->connection);
}

win_t* create_win(uint32_t x_res, uint32_t y_res) {
	win_t* self = calloc(1, sizeof *self);

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
		XCB_EVENT_MASK_EXPOSURE,
	};

	self->window = xcb_generate_id(self->connection);

	xcb_create_window(
		self->connection, XCB_COPY_FROM_PARENT, self->window, self->screen->root,
		0, 0, x_res, y_res, 0, // window geometry
		XCB_WINDOW_CLASS_INPUT_OUTPUT, self->screen->root_visual,
		XCB_CW_EVENT_MASK, window_attribs);
	
	xcb_map_window(self->connection, self->window);

	// setup 'WM_DELETE_WINDOW' protocol (yes this is dumb, thank you XCB & X11)

	xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(self->connection, 1, 12 /* strlen("WM_PROTOCOLS") */, "WM_PROTOCOLS");
	xcb_atom_t wm_protocols_atom = xcb_intern_atom_reply(self->connection, wm_protocols_cookie, 0)->atom;

	xcb_intern_atom_cookie_t wm_delete_window_cookie = xcb_intern_atom(self->connection, 0, 16 /* strlen("WM_DELETE_WINDOW") */, "WM_DELETE_WINDOW");
	self->wm_delete_window_atom = xcb_intern_atom_reply(self->connection, wm_delete_window_cookie, 0)->atom;

	xcb_icccm_set_wm_protocols(self->connection, self->window, wm_protocols_atom, 1, &self->wm_delete_window_atom);

	// EWMH

	xcb_intern_atom_cookie_t* cookies = xcb_ewmh_init_atoms(self->connection, &self->ewmh);

	if (!xcb_ewmh_init_atoms_replies(&self->ewmh, cookies, NULL)) {
		FATAL_ERROR("Failed to get EWMH atoms\n")
	}

	win_set_caption(self, "Gamejam 2022");

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

	self->egl_surface = eglCreateWindowSurface(self->egl_display, config, self->window, surface_attribs);

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

void win_loop(win_t* self, int (*draw_cb) (void* param, float dt), void* param) {
	for (xcb_generic_event_t* event; (event = xcb_wait_for_event(self->connection)); free(event)) {
		int type = event->response_type & XCB_EVENT_RESPONSE_TYPE_MASK;

		if (type == XCB_EXPOSE) {
			// get time between two frames

			struct timespec now = { 0, 0 };
			clock_gettime(CLOCK_MONOTONIC, &now);

			float last_seconds = (float) self->last_exposure.tv_sec + 1.0e-9 * self->last_exposure.tv_nsec;
			float now_seconds = (float) now.tv_sec + 1.0e-9 * now.tv_nsec;

			memcpy(&self->last_exposure, &now, sizeof self->last_exposure);
			float dt = now_seconds - last_seconds;

			// draw & swap buffers

			draw_cb(param, dt);
			eglSwapBuffers(self->egl_display, self->egl_surface);

			// invalidate by sending another expose event
			// the X11 spec is as usual really unclear about how we're supposed to do this, but whatever, nothing new to see here

			xcb_expose_event_t invalidate_event;

			invalidate_event.window = self->window;
			invalidate_event.response_type = XCB_EXPOSE;

			xcb_send_event(self->connection, 0, self->window, XCB_EVENT_MASK_EXPOSURE, (const char*) &invalidate_event);
			xcb_flush(self->connection);
		}

		else if (type == XCB_CLIENT_MESSAGE) {
			xcb_client_message_event_t* specific = (void*) event;

			if (specific->data.data32[0] == self->wm_delete_window_atom) {
				break; // quit
			}
		}
	}
}
