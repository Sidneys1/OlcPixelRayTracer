# OLCPixelRayTracer

<!-- markdownlint-disable MD033 MD026 -->
<!-- cSpell:words raytracer renderable structs -->

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

### 4. Add constants and a way to "Sample" single pixels.

We define a few constants for window geometry and begin implementing our rendering process by looping over the rows and
columns of the scene and calling a `Sample` function that takes a floating-point x,y position on the viewport and
returns a `olc::Pixel` for that location.

> Running our project will now render a 250x250 canvas at 2x magnification. Our magenta fill has been replaced with a
> color pattern converging in the center of the canvas.

### 5. Add some geometry types, enhance Shape and Sphere.

We add structs for vectors and rays, and enhance our `Shape` and `Sphere` classes with properties that will allow us to
describe their size and position in our scene.

> Running our project produces no differences from the last commit.

### 6. Add fog color and a way to sample rays.

To prevent our scene from extending into infinity, and to have something to show when a ray doesn't hit *anything*, we
add a new constant: a "fog" color. Additionally, we add a more specific function, `SampleRay`, that is called by
`Sample` to return the color (or absence thereof) of a ray as it extends into our scene. For now, still, this returns a
color relative to the X and Y coordinate in our scene.

> Running our project produces no differences from our last commit.

### 7. Add intersection and sample methods to Shapes.

Our `SampleRay` function has been upgraded to search for a `Shape` that it intersects with. To do this, `Shape` has been
upgraded with two new virtual methods:

* `std::optional<float> intersection(ray)`
* `olc::Pixel sample(ray)`

These methods provide the ability to determine where along a ray a `Shape` intersects, and to provide the color of the
`Shape` at a give ray intersection. Finally, our `Sphere` class overrides the `intersection` method, though for now the
implementation only returns an empty optional.

> Running our project produces no difference from the last commit.

### 8. Implement ray-Sphere intersection.

We'll need to overload some operators for a `vf3d`: subtraction, and dot-product. A dot-product is a useful way of
comparing two vectors to determine if they are similar.

We'll also implement the equation for an intersection between a ray and a `Sphere`. I'm not going to go into depth
explaining the geometry here: this is a well-documented process and can be researched separately.

> Running our project will now render a (highly aliased and flatly-colored) `Sphere`!

### 9. Add perspective rendering and depth sorting.

First we'll add some additional `Sphere`s to our scene at different
Z-depths. If we run our project now, you'll see that the `Sphere`s added
to our scene last are drawn in front of the earlier ones, even if they
are further away.

To remedy that, we update our hit check in `SampleRay` to select the
`Shape` whose intersection is nearest to the ray origin.

> Now if we run our project, the `Sphere`s are properly sorted. **However**, you'll notice that all three `Sphere`s are
> the same size, despite being different distances from the camera.

<!-- TODO: enhance this section -->
To fix this, we'll need to add perspective to our camera. We'll do this in a very simplistic manner, by having all of
the rays originate from some point, and pointing towards what you can think of as a "virtual canvas" some distance in
front of that origin point. By normalizing this ray we get rays properly fanning out in a perspective.

> Running our project will now produce a proper perspective rendering of our three flat-shaded `Sphere`s, at the correct
> depths.

</details>

### 10. Add a Plane Shape, and apply Fog.

First we'll add a new type of `Shape`: a `Plane`. This is a flat surface extending infinitely in all directions. I'm not
going to go into depth about the intersection algorithm, as that's basic geometry and is better explained elsewhere.
Unlike a `Sphere`, orientation matters to a `Plane`, so we'll also add a "direction" `vf3d` that will indicate the
normal pointing away from the surface. We will also override the `sample` virtual method for the first time to provide a
checkerboard pattern that will make the perspective rendering we added last time really pop. To do this we'll also be
adding some new operator overloads to both `vf3d` and `ray`, and we'll also add a new method to `ray` that returns the
`vf3d` representing the endpoint of the `ray`. Finally, we'll add a new `Plane` to our scene just below our `Sphere`s.
Note that since we render our canvas top to bottom, +Y is down, while -Y is up.

> Running our project now you'll see the checkerboard pattern continue off to the horizon - **however**, it appears
> further up on the canvas than you might expect. **Additionally**, the checkerboard pattern gets very garbled as the
> checks gets smaller than a single pixel, creating a sort of unexpected and disorienting moire pattern. Perhaps drawing
> surfaces *that* far away isn't good...

To remedy this, we'll add the concept of Fog. We already have a Fog color, for when a ray doesn't hit anything. This new
concept applies the idea of there being some extremely translucency to the nothingness between a ray's origin and the
`Shape` it intersects with. We'll begin by adding two new constants, one to define the maximum distance at which an
`Shape` would be visible, and the other as the reciprocal of that. now when we're determining the color of a ray in
`SampleRay` we can check if the intersection distance is greater than that of the max Fog distance. If so, we'll
immediately return the Fog color and skip further calculation. If the distance is lower, however, we want to smoothly
transition between the `Shape`'s color and the Fog's color, depending on the distance.

To do this, we'll implement a function called `lerp` - short for "linear interpolation". This just smoothly mixes two
colors based on a floating point value between 0 and 1.

> Running our project now displays our `Sphere`s as before, plus the checkerboard `Plane` of the floor, smoothly fading
> into the distance.

Note that as our scene and renderer grow in complexity we'll begin to see lower and lower frame-rates when running our
project. Switching our compilation mode to Release and running without debugging can help, as the compiler will more
aggressively apply optimizations. Feel free to experiment with optimization strategies in the Release compilation
settings.
