# lln-gamejam-2022

Preparations for the 2022 Louvain-li-Nux gamejam.

## Idea

We want to build this game as much from the ground up as possible, so it'll be entirely written in standard C11 with a custom OpenGL loader & engine, direct communication with the X11 windowing system through XCB (and context creation through EGL), and a custom physics engine.

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
Id est, any version of OpenGL ES greater or equal to 2.0 (this is subject to change as we consider using compute shaders).
Your driver must also support EGL (there is no GLX backend), but any recent versions of the NVIDIA or MESA drivers should be fine wrt to this.

### Note on keymaps

The WASD keyboard layout is hard-coded.
If e.g. you are using a belgian AZERTY keyboard layout on an X11 windowing system, you can run something like:

```sh
% setxkbmap us && bin/out && setxkbmap be
```

(Obviously, make sure `setxkbmap` is installed.)
I keep on forgetting how you can get a list of all keymaps, so if you use another layout, you're on your own :P

## Code structure

Organizing source files into individual translation units is for weak developers; instead, our team prefers subjecting itself to long compile times and namespace conflicts by putting everything into ungodly header files.
