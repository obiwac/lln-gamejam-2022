#pragma once

#include "log.h"

#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_event.h>

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

	// EGL stuff

	EGLDisplay* egl_display;
	EGLContext* egl_context;
	EGLSurface* egl_surface;
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

	// TODO set window caption & other stuff

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
		WARNING("EGL surface is single buffered (%s)\n", egl_error_str())
	}

	return self;
}
