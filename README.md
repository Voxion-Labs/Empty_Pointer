<p align="center">
  <img src="assets/logo.svg" alt="Empty_Pointer pixel E logo" width="140">
</p>

<h1 align="center">EMPTY_POINTER</h1>

<p align="center">
  A fast-paced 2D grid-based action game built natively with C++, Raylib, and WebAssembly. 👾
</p>

<p align="center">
  <a href="LICENSE">MIT License</a>
  |
  <a href="https://github.com/liambrooks-lab">Author GitHub</a>
</p>

<p align="center">
  <a href="https://voxion-labs.github.io/Empty_Pointer/"><strong>✨ CLICK HERE TO PLAY THE LIVE WEB BUILD ✨</strong></a>
</p>

---

##  About

**Empty_Pointer** is a lightweight arcade survival game where you navigate across a clean, minimalist grid while enemies spawn from the edges and close in on you. 

Built as an experimental project under Voxion Labs, it is designed to run directly on GitHub Pages as a static WebAssembly build—zero backend, zero installations, and instant browser gameplay.

---

## Features

- **Fluid Grid Movement:** Interpolated player rendering for buttery-smooth grid transitions.
- **Dynamic Threat Spawning:** Enemies continuously generate from the edges, increasing the pressure.
- **Pulse Attack:** Unleash an AoE (Area of Effect) pulse to clear nearby enemies when cornered.
- **Boss Encounters:** Face a challenging purple boss every 20 kills, complete with escort enemies, warning banners, and alert sounds.
- **Juicy Game Feel:** Includes AABB collision detection, satisfying player death particles, and screen shake impacts.
- **Run Analytics:** Tracks survival time, kills, current score, and personal best records.
- **Cross-Platform Controls:** Fully optimized for keyboard, mouse, and mobile touch displays.
- **Procedural Audio:** Built-in synthesized sound effects for movement, UI interactions, hits, and game over states.

---

## 🎮 Controls

| Action | Keyboard | Mouse / Touch |
| :--- | :--- | :--- |
| **Move** | `WASD` / Arrow Keys | Tap/Click a grid cell |
| **Pulse Attack** | `E` | Tap `PULSE` button |
| **Start / Restart** | `ENTER` / `SPACE` | Tap `START / RESTART` |
| **Pause** | `P` | Tap `PAUSE` |
| **Main Menu** | Pause first, then `MAIN MENU` | Tap `MAIN MENU` |
| **Guide** | `G` | Tap `GUIDE` |

*Note for Mobile:* Use the browser control dock below the game canvas for reliable button inputs.

---

## How To Play

Survive as long as possible! Your player occupies a single grid cell. Use movement to maintain your distance, and trigger your **PULSE** attack when enemies swarm too close. 

**Boss Fights:**
Every 20 kills triggers a boss encounter. A red warning banner will flash, and a dark purple boss will spawn alongside five escort enemies. 
* The boss is slower but features a visible aura that causes instant death on contact. 
* It takes **two pulse hits** to destroy the boss.

**Pro Tips :**
- **Conserve your Pulse:** Don't spam it when enemies are far; it has a cooldown. Save it for tight clusters.
- **Boss Strategy:** When the red warning appears, create distance immediately before the escorts spread out. Stay outside the boss's aura and pulse when it gets in range.
- **Diagonal Movement:** Alternate horizontal and vertical moves to zig-zag across the grid efficiently.
- **Audio Check:** On mobile browsers, tap any control button once to register a user gesture so the procedural audio can start playing.

---

## Build & Deployment

### GitHub Pages (Automated)
This repository uses GitHub Actions to compile the C++ code into WebAssembly and publish the static files directly to GitHub Pages.

**If GitHub Pages shows this README instead of the game:**
1. Go to your repository `Settings > Pages`.
2. Under `Build and deployment`, ensure `Source` is set to **GitHub Actions**.
3. Go to the `Actions` tab and run the `Build and Deploy Web Game` workflow.
4. Hard refresh (`Ctrl + F5`) the live URL once the build turns green.

### Local Web Build
Requires the Emscripten SDK and a local Raylib checkout:
```sh
make -C raylib/src PLATFORM=PLATFORM_WEB
make web RAYLIB_SRC=raylib/src
```
The output generates in the `public/` directory. Use any static file server (like `npx serve`) to host it locally.

### Native Desktop Build
If Raylib is installed natively on your OS:
```sh
make native
```

---

##  Contributing

Contributions are always welcome! Whether you want to polish the visuals, add new enemy types, fix bugs, or optimize the WebAssembly pipeline, feel free to jump in.

1. Fork the repository.
2. Create a new feature branch.
3. Commit your changes.
4. Open a Pull Request with a clear description of your upgrades.

---

## ⭐ Support

If you enjoy playing **Empty_Pointer** or found the C++ to WebAssembly pipeline helpful for your own learning, please consider dropping a **Star** on the repository! It keeps the motivation high and helps others discover the project.

---

##  Author

<p align="center">
  <img src="assets/author.jpg" alt="Rudranarayan Jena" width="150" style="border-radius: 50%;">
</p>

<p align="center">
  <strong>Crafted by Rudranarayan Jena</strong><br>
  <em>Founder @ Voxion Labs</em><br>
  Building deterministic systems, WebAssembly kernels, and fun native experiments.<br>
  <a href="https://github.com/liambrooks-lab">GitHub: @liambrooks-lab</a>
</p>

---

## 📄 License

This project is licensed under the **[MIT License](LICENSE)**.

You are free to use, copy, modify, merge, publish, and distribute this software, provided the original MIT license notice is included. Built for the community, as-is, without warranty.