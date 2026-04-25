# Empty_Pointer

Empty_Pointer is a small Raylib/C++ grid action game built for WebAssembly and hosted as static files on GitHub Pages.

## Play flow

- Press `ENTER` to start.
- Move with `WASD` or arrow keys.
- Avoid enemies that spawn from the grid edges.
- Press `ENTER` after game over to restart.

## GitHub Pages deployment

This repo is set up so GitHub Actions builds the WebAssembly version and publishes the generated static files to GitHub Pages.

1. Push the project to a GitHub repository.
2. In GitHub, open `Settings > Pages`.
3. Set `Build and deployment` source to `GitHub Actions`.
4. Push to the `main` branch.

The workflow builds Raylib for `PLATFORM_WEB`, compiles the game with Emscripten, writes the final site into `public/`, and deploys it to Pages.

## Local web build

If you have Emscripten active and a Raylib checkout available:

```sh
make -C raylib/src PLATFORM=PLATFORM_WEB
make web RAYLIB_SRC=raylib/src
```

The output will be in `public/`. Any static file server can host that folder.

## Native build

If Raylib is installed on your system:

```sh
make native
```
