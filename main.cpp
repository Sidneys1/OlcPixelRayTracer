#include <cmath>
#include <vector>
#include <memory>
#include <optional>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

/***** TYPES *****/

// Struct to describe a 3D floating point vector.
struct vf3d {
	float x, y, z;

	/* CONSTRUCTORS */

	// Default constructor.
	vf3d() = default;

	// Explicit constructor that initializes x, y, and z.
	constexpr vf3d(float x, float y, float z) : x(x), y(y), z(z) {}

	// Explicit constructor that initializes x, y, and z to the same value.
	constexpr vf3d(float f) : x(f), y(f), z(f) {}
};

// Struct to describe a 3D floating point ray (vector with origin point).
struct ray {
	vf3d origin, direction;

	/* CONSTRUCTORS */

	// Default constructor.
	ray() = default;

	// Add explicit constructor that initializes origin and direction.
	constexpr ray(const vf3d origin, const vf3d direction) : origin(origin), direction(direction) {}
};

// Class to describe any kind of object we want to add to our scene.
class Shape {
public:
	vf3d origin;
	olc::Pixel fill;

	/* CONSTRUCTORS */

	// Delete the default constructor (we'll never have a Shape with a default origin and fill).
	Shape() = delete;

	// Add explicit constructor that initializes origin and fill.
	Shape(vf3d origin, olc::Pixel fill) : origin(origin), fill(fill) {}
};

// Subclass of Shape that represents a Sphere.
class Sphere : public Shape {
public:
	float radius;

	/* CONSTRUCTORS */

	// Delete the default constructor (see "Shape() = delete;").
	Sphere() = delete;

	// Add explicit constructor that initializes Shape::origin, Shape::fill, and Sphere::radius.
	Sphere(vf3d origin, olc::Pixel fill, float radius) : Shape(origin, fill), radius(radius) {}
};

/***** CONSTANTS *****/

// Game width and height (in pixels).
constexpr int WIDTH = 250;
constexpr int HEIGHT = 250;

// Half the game width and height (to identify the center of the screen).
constexpr float HALF_WIDTH = WIDTH / 2.0f;
constexpr float HALF_HEIGHT = HEIGHT / 2.0f;

// A color representing scene fog.
olc::Pixel FOG(128, 128, 128);

/***** PIXEL GAME ENGINE CLASS *****/

// Override base class with your custom functionality
class OlcPixelRayTracer : public olc::PixelGameEngine {
public:
	OlcPixelRayTracer() {
		// Name your application
		sAppName = "RayTracer";
	}

public:
	bool OnUserCreate() override {
		// Called once at the start, so create things here

		// Create a new Sphere and add it to our scene.
		shapes.emplace_back(std::make_unique<Sphere>(vf3d(0, 0, 200), olc::GREY, 100));

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		// Called once per frame

		// Iterate over the rows and columns of the scene
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				// Sample this specific pixel (converting screen coordinates
				// to scene coordinates).
				auto color = Sample(x - HALF_WIDTH, y - HALF_HEIGHT);
				Draw(x, y, color);
			}
		}

		return true;
	}

	olc::Pixel Sample(float x, float y) const {
		// Called to get the color of a specific point on the screen.

		// Create a ray casting into the scene from this "pixel".
		ray sample_ray({ x, y, 0 }, { 0, 0, 1 });

		// Sample this ray - if the ray doesn't hit anything, use the color of
		// the surrounding fog.
		return SampleRay(sample_ray).value_or(FOG);
	}

	std::optional<olc::Pixel> SampleRay(const ray r) const {
		// Called to get the color produced by a specific ray.

		// This will be the color we (eventually) return/
		olc::Pixel final_color;

		// For now we're returning a color based on the screen coordinates.
		final_color = olc::Pixel(std::abs(r.origin.x * 255), std::abs(r.origin.y * 255), 0);

		return final_color;
	}

private:

	// A vector of Shape smart pointers representing our scene.
	// Because these are smart pointers we can point to subclasses of Shape.
	std::vector<std::unique_ptr<Shape>> shapes;
};

/***** PROGRAM ENTRYPOINT *****/

int main() {
	// Create an instance of our PixelGameEngine
	OlcPixelRayTracer ray_tracer;

	// Construct and start it with our WIDTH and HEIGHT constants.
	if (ray_tracer.Construct(WIDTH, HEIGHT, 2, 2))
		ray_tracer.Start();

	return 0;
}
