# Contributing to Empty_Pointer

First off, thank you for considering contributing to **Empty_Pointer**! 

Since this is a lightweight, open-source arcade game built for the web, contributions of all sizes are super welcome—whether it's fixing a small typo, tweaking the game feel, optimizing the C++ code, or adding a whole new enemy type.

## Code of Conduct
Let's keep this community friendly, respectful, and fun. We are here to learn C++, Raylib, and WebAssembly together while building something cool. Be kind to other contributors!

## Found a Bug?
If you find a bug (like a weird collision glitch, screen scaling issue on mobile, or audio not playing), please open an **Issue**!
* Check if the issue already exists in the tracker.
* Provide clear steps to reproduce the glitch.
* Mention if you were playing on a desktop browser, mobile browser, or a native local build.

## Proposing a Feature
Got a wild idea for a new boss, a different power-up, or a cool visual effect? We'd love to hear it! 
Please open an issue to discuss your idea **before** you start writing massive amounts of code. This just ensures your feature fits the minimal, fast-paced arcade style of the game.

## Pull Request Process
Ready to submit code? Awesome! Here is the standard workflow:

1. **Fork** the repository and clone it locally.
2. **Branch** out for your feature or bugfix (`git checkout -b feature/cool-new-mechanic`).
3. **Commit** your changes with clear, descriptive messages.
4. **Push** to your fork and submit a **Pull Request (PR)** to our `main` branch.

### Technical Guidelines for PRs:
* **WebAssembly First:** Empty_Pointer is designed to be played instantly in the browser. Always test your changes using the Emscripten web build (`make web`) before submitting. If it breaks the browser build, we can't merge it!
* **Keep it Lightweight:** Avoid adding heavy external libraries. Stick to standard C++17 and Raylib functions to keep the final `.wasm` payload as small and fast as possible.
* **Performance Matters:** Ensure the game maintains a solid 60 FPS. Be careful with manual memory management—avoid creating memory leaks or heavy object allocations inside the main game loop!

Once your PR is submitted, I will review it as soon as possible. 

Happy coding, and let's make this game even more awesome! 