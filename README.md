<h1 align="left">
	<img style="vertical-align:middle" src="res/twist.png" title="Twist"/>
	Twist
</h1>
A node-based audio synthesizer written in C++

![Twist](res/screenshot.png?raw=true)

Twist is the _unexpected_ result of me trying to experiment with audio programming. I started August the 1st as a small toy, and it grew a _lot_.

It is a 100% visual tool to help you create a broad variety of sounds, packed with various effects and operators.

## Main Features
* Many useful nodes
* Recording (OGG)
* Sampling (WAV, OGG, AIFF and FLAC)
* Effects

## Building
Tools Needed:
* CMake
* Anything that can compile C++17 code

Dependencies:
* Linux
	* Debian-based: `sudo apt install libsdl2-dev libsdl2-2.0-0 libsndfile1-dev libsndfile1`
	* Arch: `sudo pacman -S sdl2 libsndfile`

* Windows
	* [SDL2](https://www.libsdl.org/download-2.0.php)
	* [libsndfile](http://www.mega-nerd.com/libsndfile/#Download)

> RtMidi is included as a Git submodule so do this before building:
> `git submodule update --init --recursive`

### Linux Build
```sh
$ mkdir build && cd build
$ cmake -G "Unix Makefiles" 
-DCMAKE_BUILD_TYPE=Release .
$ make -j2
```

### Windows Build
- Create a `build` folder in Twist's root dir and open CMake GUI.
- Set the `source path`. 
- Set the `build path` to the `build` folder you just created.
- Configure.
- Set the generator to `Visual Studio {YOUR VERSION}` or to any other IDE you use.
- Click Finish, wait for it to complete and then click Generate.
- Open the generated project files in your IDE and build.

***
