# lln-gamejam-2022

2022 Louvain-li-Nux gamejam.

## Idea

We wanted to build this game as much from the ground up as possible (within reason), so it'll be entirely written in standard C11 with a custom OpenGL loader & engine, direct communication with the X11 windowing system through XCB (and context creation through EGL), and a custom physics engine.

## Building

You need to have installed relevant development files & headers for your OS distribution first. (*TODO*: specify which packages for Debian?)
Then, build:

```sh
% sh build.sh
```

This will generate an executable for your OS (GNU/Linux & FreeBSD tested, other Unix-like OS' will probably work) in `bin/out`.

## Running

Just:

```sh
% bin/out
```

### Note on Wayland

You must have setup XWayland to run the game, as it is exclusively an X client.

### Note on hardware support

You must have a graphics adapter supporting at least modern OpenGL (or equivalent extensions).
Id est, any version of OpenGL ES greater or equal to 2.0.
Your driver must also support EGL (there is no GLX backend), but any recent versions of the NVIDIA or MESA drivers should be fine wrt to this.

## Code structure

Organizing source files into individual translation units is for weak developers; instead, our team prefers subjecting itself to long compile times and namespace conflicts by putting everything into ungodly header files.
