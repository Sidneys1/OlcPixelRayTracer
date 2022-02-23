# OLCPixelRayTracer

<!-- markdownlint-disable MD033 MD026 -->
<!-- cSpell:words raytracer renderable -->

Welcome to the OLCPixelRayTracer repository. This project is designed to be a living tutorial on building up a raytracer
from scratch using the [OneLoneCoder PixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine).

## Using This Repository

The intention is for this repository to be read in Git commit-history order, with commit messages (and this readme)
documenting the step-by-step process of building up a functional ray tracer. Please see
[the commit history](https://github.com/Sidneys1/OlcPixelRayTracer/commits/main) to get started.

## Tutorial

<details><summary>Expand previous steps</summary>

### 1. Create new, empty Visual Studio project.

First, <kbd>Create a new project</kbd> in Visual Studio, selecting the <kbd>Empty Project</kbd>
(`C++`/`Windows`/`Console`) template. I also opted for the flat directory structure option (<kbd>☑ Place solution and
project in the same directory</kbd>).

![Create a new project dialog](./docs/images/create-a-new-project.png)

Our project cannot currently run (there is no main entrypoint)!

### 2. Add PGE header and create a game from template.

We copy in the `olcPixelGameEngine.h` file and add it to our solution. We also add a blank `main.cpp` and populate it
with the contents of the template available in the `olcPixelGameEngine.h` header, taking care to rename our game class
to match our needs.

> Running our project will render a default PixelGameEngine scene: a 256x240 canvas of random pixels, magnified 4x.

### 3. Add basic Shapes and a vector of shapes to render.

We create a base class `Shape` and derived class `Sphere` (blank for now) that we will use to define our renderable
objects in the future.

We also add a `std::vector` of `std::unique_ptr<Shape>` to our game class. This will allow us to add new `Shape`-derived
objects to our scene.

Finally, when the game exits, the memory we allocated will be freed (thanks, smart pointers).

> Running our project will now render a solid magenta canvas.

</details>

### 4. Add constants and a way to "Sample" single pixels.

We define a few constants for window geometry and begin implementing our rendering process by looping over the rows and
columns of the scene and calling a `Sample` function that takes a floating-point x,y position on the viewport and
returns a `olc::Pixel` for that location.

> Running our project will now render a 250x250 canvas at 2x magnification. Our magenta fill has been replaced with a
> color pattern converging in the center of the canvas.
