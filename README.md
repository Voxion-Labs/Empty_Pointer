<p align="center">
  <img src="assets/logo.svg" alt="Empty_Pointer pixel E logo" width="140">
</p>

<h1 align="center">EMPTY_POINTER</h1>

<p align="center">
  A fast 2D grid-based action game built with C++, Raylib, and WebAssembly. 🎮
</p>

<p align="center">
  <a href="LICENSE">MIT License</a>
  |
  <a href="https://github.com/liambrooks-lab">Author GitHub</a>
</p>

## About 🎯

Empty_Pointer is a lightweight arcade survival game where the player moves across a clean grid while enemies spawn from the edges and chase inward. It is designed to run directly on GitHub Pages as a static WebAssembly build, with no backend and no complicated deploy setup.

## Features ✨

- Smooth grid movement using interpolated player rendering.
- Dynamic enemy spawning from the edges of the playfield.
- Menu, gameplay, and game-over state flow.
- AABB collision detection.
- Player death particles for extra impact.
- Static GitHub Pages deployment through GitHub Actions.

## Controls 🕹️

- `ENTER`, `SPACE`, or `START / RESTART` button: start or restart.
- `WASD` or arrow keys: move.
- Avoid enemies for as long as possible.

## GitHub Pages Deployment 🚀

This repo is set up so GitHub Actions builds the WebAssembly version and publishes the generated static files to GitHub Pages.

1. Push the project to a GitHub repository.
2. Open `Settings > Pages`.
3. Set `Build and deployment` source to `GitHub Actions`.
4. Push to the `main` or `master` branch.

The workflow builds Raylib for `PLATFORM_WEB`, compiles the game with Emscripten, writes the final site into `public/`, and deploys it to Pages.

## Local Web Build 🧪

If you have Emscripten active and a Raylib checkout available:

```sh
make -C raylib/src PLATFORM=PLATFORM_WEB
make web RAYLIB_SRC=raylib/src
```

The output will be in `public/`. Any static file server can host that folder.

## Native Build 💻

If Raylib is installed on your system:

```sh
make native
```

## Author 👤

<p align="center">
  <img src="assets/author.jpg" alt="Rudranarayan Jena portrait" width="170">
</p>

<p align="center">
  <strong>Rudranarayan Jena</strong><br>
  C++ and game-development learner building small, focused projects with clean code and web-friendly deployment in mind.<br>
  GitHub: <a href="https://github.com/liambrooks-lab">https://github.com/liambrooks-lab</a>
</p>

## Contributing 🤝

Contributions are welcome. You can help by improving gameplay, polishing visuals, fixing bugs, or making the WebAssembly deployment smoother.

To contribute:

1. Fork the repository.
2. Create a new branch.
3. Make your changes.
4. Open a pull request with a clear description.

## Support ⭐

If you like Empty_Pointer, please support the project by giving it a star on GitHub. It helps the project look alive and makes future improvements easier to discover.

## License 📄

This project is licensed under the [MIT License](LICENSE).

That means you can use, copy, modify, merge, publish, distribute, and build on this project, as long as the original MIT license notice is included. The software is provided as-is, without warranty.
