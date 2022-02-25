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

	/* OPERATORS */

	// Subtraction: vf3d - vf3d = vf3d
	const vf3d operator-(const vf3d right) const {
		return { x - right.x, y - right.y, z - right.z };
	}

	// Division: vf3d / float = vf3d
	const vf3d operator/(float divisor) const {
		return { x / divisor, y / divisor, z / divisor };
	}

	// Dot product (multiplication): vf3d * vf3d = float
	const float operator* (const vf3d right) const {
		return (x * right.x) + (y * right.y) + (z * right.z);
	}

	/* METHODS */

	// Return a normalized version of this vf3d (magnitude == 1).
	const vf3d normalize() const {
		return (*this) / sqrtf((*this) * (*this));
	}
};

// Struct to describe a 3D floating point ray (vector with origin point).
struct ray {
	vf3d origin, direction;

	/* CONSTRUCTORS */

	// Default constructor.
	ray() = default;

	// Add explicit constructor that initializes origin and direction.
	constexpr ray(const vf3d origin, const vf3d direction) : origin(origin), direction(direction) {}

	/* METHODS */

	// Return a normalized version of this ray (magnitude == 1).
	const ray normalize() const {
		return { origin, direction.normalize() };
	}
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

	/* METHODS */

	// Get the color of this Shape (when intersecting with a given ray).
	virtual olc::Pixel sample(ray sample_ray) const { return fill; }

	// Determin how far along a given ray this Shape intersects (if at all).
	virtual std::optional<float> intersection(ray r) const = 0;
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

	/* METHODS */

	// Determine how far along a given ray this Circle intersects (if at all).
	std::optional<float> intersection(ray r) const override {
		vf3d oc = r.origin - origin;

		float a = r.direction * r.direction;
		float b = 2.0f * (oc * r.direction);
		float c = (oc * oc) - (radius * radius);
		float discriminant = powf(b, 2) - 4 * a * c;

		if (discriminant < 0)
			return {};

		auto ret = (-b - sqrtf(discriminant)) / (2.0f * a);
		if (ret < 0)
			return {};

		return ret;
	}
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

		// Add some additional Spheres at different positions.
		shapes.emplace_back(std::make_unique<Sphere>(vf3d(-150, +75, +300), olc::RED, 100));
		shapes.emplace_back(std::make_unique<Sphere>(vf3d(+150, -75, +100), olc::GREEN, 100));

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
		ray sample_ray({ 0, 0, -800 }, { (x / (float)WIDTH) * 100, (y / (float)HEIGHT) * 100, 200});

		// Sample this ray - if the ray doesn't hit anything, use the color of
		// the surrounding fog.
		return SampleRay(sample_ray.normalize()).value_or(FOG);
	}

	std::optional<olc::Pixel> SampleRay(const ray r) const {
		// Called to get the color produced by a specific ray.

		// This will be the color we (eventually) return/
		olc::Pixel final_color;

		// Store a pointer to the Shape this ray intersects with.
		auto intersected_shape_iterator = shapes.end();

		// Also store the distance along the ray that the intersection occurs.
		float intersection_distance = INFINITY;

		/* Determine the Shape this ray intersects with(if any). */ {
			// Iterate over all of the Shapes in our scene.
			for (auto it = shapes.begin(); it != shapes.end(); it++) {
				// If the distance is not undefined (meaning no intersection)...
				if (std::optional<float> distance = (*it)->intersection(r).value_or(INFINITY);
						distance < intersection_distance) {
					// Save the current Shape as the intersected Shape!
					intersected_shape_iterator = it;
					// Also save the distance along the ray that this intersection occurred.
					intersection_distance = distance.value();
				}
			}

			// If we didn't intersect with any Shapes, return an empty optional.
			if (intersected_shape_iterator == shapes.end())
				return {};
		}

		// Get the shape we discovered
		const Shape &intersected_shape = **intersected_shape_iterator;

		// Set our color to the sampled color of the Shape this ray with.
		final_color = intersected_shape.sample(r);

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
