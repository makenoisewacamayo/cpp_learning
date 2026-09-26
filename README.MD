# cpp_learning

Learning repository for building **native Node.js addons in C++** using [node-addon-api](https://github.com/nodejs/node-addon-api) (N-API) and [cmake-js](https://github.com/cmake-js/cmake-js), with a TypeScript frontend.

## Projects

### `demo_myObject`

A demo native addon that exposes a C++ class (`MyObject`) to JavaScript via `Napi::ObjectWrap`, with instance methods like `greet` and `add`.

```
demo_myObject/
├── CMakeLists.txt   # CMake build config (consumed by cmake-js)
├── addons/          # C++ addon sources (myObject.cc / myObject.h)
├── src/             # TypeScript sources (bindings + entry point)
├── package.json
└── tsconfig.json
```

## Requirements

- **Node.js 22.x** (enforced via `engines` + `engine-strict`)
- **CMake** and a C++17 compiler (AppleClang/Xcode Command Line Tools on macOS)
- **Ninja** (optional, used automatically if available)

## Usage

```bash
cd demo_myObject
npm install        # installs deps (fails if Node isn't 22.x)
npm run build      # compiles the native addon with cmake-js
npm start          # compiles TypeScript and runs dist/index.js
```

The compiled addon is emitted at `build/Release/*.node` and loaded from the TypeScript code through `src/bindings.ts`.

## Topics covered

- N-API / node-addon-api class wrapping (`Napi::ObjectWrap`)
- Argument validation and throwing JavaScript exceptions from C++
- CMake configuration for Node addons (cmake-js, `NAPI_VERSION`, macOS `@rpath`)
- TypeScript interop with native modules
