# OLCPixelRayTracer

<!-- markdownlint-disable MD033 MD026 -->
<!-- cSpell:words raytracer -->

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

</details>

### 2. Add PGE header and create a game from template.

We copy in the `olcPixelGameEngine.h` file and add it to our solution. We also add a blank `main.cpp` and populate it
with the contents of the template available in the `olcPixelGameEngine.h` header, taking care to rename our game class
to match our needs.

> Running our project will render a default PixelGameEngine scene: a 256x240 canvas of random pixels, magnified 4x.
