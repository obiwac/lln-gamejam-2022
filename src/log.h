#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static bool verbose = true;

#define LOG_REGULAR "\033[0m"
#define LOG_RED     "\033[0;31m"
#define LOG_YELLOW  "\033[0;33m"

#define LOG(...) \
	if (verbose) { \
		printf(LOG_REGULAR __VA_ARGS__); \
	}

#define FATAL_ERROR(...) \
	fprintf(stderr, LOG_RED "[ERROR] " __VA_ARGS__); \
	exit(EXIT_FAILURE);

#define WARN(...) \
	fprintf(stderr, LOG_YELLOW "[WARNING] " __VA_ARGS__);
