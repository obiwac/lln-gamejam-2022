#!/bin/sh
set -e

mkdir -p bin

# not using something "proper" like 'make', because it'll make things simpler (this is a smaller project so it doesn't really matter)

cc -g -std=c99 src/main.c -o bin/out \
	-I/usr/local/include -L/usr/local/lib \
	-lm -pthread \
	-lxcb -lxcb-ewmh -lxcb-icccm -lX11 -lX11-xcb -lEGL
