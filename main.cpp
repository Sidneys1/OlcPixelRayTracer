#include <cmath>
#include <vector>
#include <memory>
#include <numeric>
#include <optional>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


#include "TPDThreadPool.h"

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

	// Addition: vf3d + vf3d = vf3d
	const vf3d operator+(const vf3d right) const {
		return { x + right.x, y + right.y, z + right.z };
	}

	// Subtraction: vf3d - vf3d = vf3d
	const vf3d operator-(const vf3d right) const {
		return { x - right.x, y - right.y, z - right.z };
	}

	// Division: vf3d / float = vf3d
	const vf3d operator/(float divisor) const {
		return { x / divisor, y / divisor, z / divisor };
	}

	// Multiplication: vf3d * float = vf3d
	const vf3d operator*(float factor) const {
		return { x * factor, y * factor, z * factor };
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

	// Return the length of this vf3d.
	const float length() const {
		return sqrtf(x * x + y * y + z * z);
	}
};

// Use a type alias to use vf3d and color3 interchangeably.
using color3 = vf3d;

// Struct to describe a 3D floating point ray (vector with origin point).
struct ray {
	vf3d origin, direction;

	/* CONSTRUCTORS */

	// Default constructor.
	ray() = default;

	// Add explicit constructor that initializes origin and direction.
	constexpr ray(const vf3d origin, const vf3d direction) : origin(origin), direction(direction) {}

	/* OPERATORS */

	// Multiplication: ray * float = ray
	const ray operator*(float right) const {
		return { origin, direction * right };
	}

	/* METHODS */

	// Return a normalized version of this ray (magnitude == 1).
	const ray normalize() const {
		return { origin, direction.normalize() };
	}

	// Return the vf3d at the end of this ray.
	const vf3d end() const {
		return origin + direction;
	}
};

// Class to describe any kind of object we want to add to our scene.
class Shape {
public:
	vf3d origin;
	color3 fill;
	float reflectivity;

	/* CONSTRUCTORS */

	// Delete the default constructor (we'll never have a Shape with a default origin and fill).
	Shape() = delete;

	// Add explicit constructor that initializes origin and fill.
	Shape(vf3d origin, color3 fill, float reflectivity = 0.0f) : origin(origin), fill(fill), reflectivity(reflectivity) {}

	/* METHODS */

	// Get the color of this Shape (when intersecting with a given ray).
	virtual color3 sample(ray sample_ray) const { return fill; }

	// Determin how far along a given ray this Shape intersects (if at all).
	virtual std::optional<float> intersection(ray r) const = 0;

	// Determine the surface normal of this Shape at a given intersection point.
	virtual ray normal(vf3d incident) const = 0;
};

// Subclass of Shape that represents a Sphere.
class Sphere : public Shape {
public:
	float radius;

	/* CONSTRUCTORS */

	// Delete the default constructor (see "Shape() = delete;").
	Sphere() = delete;

	// Add explicit constructor that initializes Shape::origin, Shape::fill, and Sphere::radius.
	Sphere(vf3d origin, color3 fill, float radius, float reflectivity = 0.0f) : Shape(origin, fill, reflectivity), radius(radius) {}

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

	// Return the surface normal of this Sphere at a given intersection point.
	ray normal(vf3d incident) const override {
		return { incident, (incident - origin).normalize() };
	}
};

// Subclass of Shape that represents a flat Plane.
class Plane : public Shape {
public:
	vf3d direction;
	color3 check_color;

	/* CONSTRUCTORS */

	// Delete the default construcotr (see "Shape() = delete;").
	Plane() = delete;

	// Add explicit constructor that initializes
	Plane(vf3d origin, vf3d direction, color3 fill, color3 check_color) : Shape(origin, fill), direction(direction), check_color(check_color) {}

	/* METHODS */

	// Determine how far along a given ray this Plane intersects (if at all).
	std::optional<float> intersection(ray sample_ray) const override {
		auto denom = direction * sample_ray.direction;
		if (fabs(denom) > 0.001f) {
			auto ret = (origin - sample_ray.origin) * direction / denom;
			if (ret > 0) return ret;
		}
		return {};
	}

	// Get the color of this Plane (when intersecting with a given ray).
	// We're overriding this to provide a checkerboard pattern.
	color3 sample(ray sample_ray) const override {
		// Get the point of intersection.
		auto intersect = (sample_ray * intersection(sample_ray).value_or(0.0f)).end();

		// Get the distances along the X and Z axis from the origin to the intersection.
		float diffX = origin.x - intersect.x;
		float diffZ = origin.z - intersect.z;

		// Get the XOR the signedness of the differences along X and Z.
		// This allows us to "invert" the +X,-Z and -X,+Z quadrants.
		bool color = (diffX < 0) ^ (diffZ < 0);

		// Flip the "color" boolean if diff % 100 < 50 (e.g., flip one half of each 100-unit span.
		if (fmod(fabs(diffZ), 100) < 50) color = !color;
		if (fmod(fabs(diffX), 100) < 50) color = !color;

		// If we're coloring this pixel, return the fill - otherwise return DARK_GREY.
		if (color)
			return fill;
		return check_color;
	}

	// Return the surface normal of this Sphere at a given intersection point.
	ray normal(vf3d incident) const override {
		return { incident, direction };
	}
};

/***** CONSTANTS *****/

// Game width and height (in pixels).
constexpr int WIDTH = 512;
constexpr int HEIGHT = 512;
constexpr int RENDERTHREADS = 8;

// Half the game width and height (to identify the center of the screen).
constexpr float HALF_WIDTH = WIDTH / 2.0f;
constexpr float HALF_HEIGHT = HEIGHT / 2.0f;

// Colors

color3 LIGHT_GRAY(0.8f);
color3 DARK_GRAY(0.5f);
color3 GREY(0.75f);
color3 RED(1.0f, 0.0f, 0.0f);
color3 GREEN(0.0f, 1.0f, 0.0f);

// Fog distance and reciprocal (falloff).
constexpr float FOG_INTENSITY_INVERSE = 3000;
constexpr float FOG_INTENSITY = 1 / FOG_INTENSITY_INVERSE;

// A color representing scene fog.
color3 FOG = DARK_GRAY;

// Lighting
constexpr float AMBIENT_LIGHT = 0.5f;

#ifdef DEBUG
constexpr int BOUNCES = 2;
constexpr int SAMPLES = 2;
#else
constexpr int BOUNCES = 5;
constexpr int SAMPLES = 4;
#endif

/***** PIXEL GAME ENGINE CLASS *****/

// Override base class with your custom functionality
class OlcPixelRayTracer : public olc::PixelGameEngine {
private:
	TPDThreadPool mThreadPool;

public:
	OlcPixelRayTracer() : light_point(0, -500, -500), mThreadPool(RENDERTHREADS - 1) {
		// Name your application
		sAppName = "RayTracer";
	}

public:
	bool OnUserCreate() override {
		// Called once at the start, so create things here

		// Create a new Sphere and add it to our scene.
		shapes.emplace_back(std::make_unique<Sphere>(vf3d(0, 0, 200), GREY, 100, 0.9f));

		// Add some additional Spheres at different positions.
		shapes.emplace_back(std::make_unique<Sphere>(vf3d(-150, +75, +300), RED, 100, 0.5f));
		shapes.emplace_back(std::make_unique<Sphere>(vf3d(+150, -75, +100), GREEN, 100));

		// Add a "floor" Plane
		shapes.emplace_back(std::make_unique<Plane>(vf3d(0, 200, 0 ), vf3d(0, -1, 0), LIGHT_GRAY, DARK_GRAY));

		return true;
	}

	void SampleRow(int row) {
		// Render a single row of pixels on our canvas

			for (int x = 0; x < WIDTH; x++) {
				// Create an array of colors - we'll be sampling this pixel multiple
				// times with varying offsets to create a multisample, and then
				// rendering the average of these samples.
				std::array<color3, SAMPLES> samples;

				// For each sample...
				for (auto i = 0; i < SAMPLES; i++) {
					// Create random offset within this pixel
					float offsetX = rand() / (float)RAND_MAX;
					float offsetY = rand() / (float)RAND_MAX;

					// Sample the color at that offset (converting screen coordinates to
					// scene coordinates).
				samples[i] = Sample(x - HALF_WIDTH + offsetX, row - HALF_HEIGHT + offsetY);
				}

				// Calculate the average color and draw it.
				color3 color = std::accumulate(samples.begin(), samples.end(), color3()) / SAMPLES;
			Draw(x, row, olc::PixelF(color.x, color.y, color.z));
			}
	}

	void DoInterleavedSample(int base, int interleave) {
		// Loops over all the rows of our image, rendering only certain ones.

		for (int y = base; y < HEIGHT; y += interleave)
			SampleRow(y);
	}

	void DoSampling() {
		for (int y = 0; y < RENDERTHREADS - 1; y++)
			mThreadPool.PushFunction(&OlcPixelRayTracer::DoInterleavedSample, this, y, RENDERTHREADS);

		mThreadPool.RunAll();
		DoInterleavedSample(RENDERTHREADS - 1, RENDERTHREADS);
		mThreadPool.WaitAll();
		}

	bool OnUserUpdate(float fElapsedTime) override {
		// Called once per frame

		// Create some static storage to accumulate elapsed time...
		static float accumulated_time = 0.0f;

		// ...and accumulate elapsed time into it.
		accumulated_time += fElapsedTime;

		// Update the position of our first Circle every update.
		// sin/cos = easy, cheap motion.
		Shape& shape = *shapes.at(0);
		shape.origin.y = sinf(accumulated_time) * 100 - 100;
		shape.origin.z = cosf(accumulated_time) * 100 + 100;

		// Update the position of our light_point relative to the mouse position.
		light_point.x = ((GetMouseX() / (float)WIDTH) - 0.5f) * 1000;
		light_point.y = ((GetMouseY() / (float)HEIGHT) - 0.5f) * 1000 - 700;

		DoSampling();

		return true;
	}

	color3 Sample(float x, float y) const {
		// Called to get the color of a specific point on the screen.

		// Create a ray casting into the scene from this "pixel".
		ray sample_ray({ 0, 0, -800 }, { (x / (float)WIDTH) * 100, (y / (float)HEIGHT) * 100, 200 });

		// Sample this ray - if the ray doesn't hit anything, use the color of
		// the surrounding fog.
		return SampleRay(sample_ray.normalize(), BOUNCES).value_or(FOG);
	}

	std::optional<color3> SampleRay(ray r, int bounces) const {
		bounces--;

		// Called to get the color produced by a specific ray.

		// This will be the color we (eventually) return/
		color3 final_color;

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

		// Quick check - if the intersection is further away than the furthest Fog point,
		// then we can save some time and not calculate anything further, since it would
		// be obscured by Fog regardless.
		if (intersection_distance >= FOG_INTENSITY_INVERSE)
			return FOG;

		// Set our color to the sampled color of the Shape this ray with.
		final_color = intersected_shape.sample(r);

		// Determine the point at which our ray intersects our Shape.
		vf3d intersection_point = (r * intersection_distance).end();
		// Calculate the normal of the given Shape at that point.
		ray normal = intersected_shape.normal(intersection_point);

		// Apply reflection
		if (bounces != 0 && intersected_shape.reflectivity > 0) {
			// Our reflection ray starts out as our normal...
			ray reflection = normal;

			// Apply a slight offset *along* the normal. This way our reflected ray will
			// start at some slight offset from the surface so that rounding errors don't
			// cause it to collide with the Shape it originated from!
			reflection.origin = reflection.origin + (normal.direction + 0.001f);

			// Reflect the direction around the normal with some simple geometry.
			reflection.direction = (normal.direction * (2 * ((r.direction * -1) * normal.direction)) + r.direction).normalize();

			// Recursion! Since SampleRay doesn't care if the ray is coming from the
			// canvas, we can use it to get the color that will be reflected by this Shape!
			std::optional<color3> reflected_color = SampleRay(reflection, bounces);

			// Finally, mix our Shape's color with the reflected color (or Fog color, in case
			// of a miss) according to the reflectivity.
			final_color = lerp(final_color, reflected_color.value_or(FOG), intersected_shape.reflectivity);
		}

		// Apply lighting

		// First we'll get the un-normalized ray from our intersection point to the light source.
		ray light_ray = ray(intersection_point, light_point - intersection_point);
		// Get the distance to the light (equal to the length of the un-normalized ray).
		float light_distance = light_ray.direction.length();
		// We'll also offset the origin of the light ray by a small amount along the
		// surface normal so the ray doesn't intersect with this Shape itself.
		light_ray.origin = light_ray.origin + (normal.direction * 0.001f);
		// And finally we'll normalize the light_ray.
 		light_ray.direction = light_ray.direction.normalize();

		// Then we'll search for any Shapes that is occluding the light_ray,
		// using more or less our existing search code.
		// We initialize closest_distance to our light distance, because we
		// don't care if any of the Shapes intersect the ray beyond the light.
		float closest_distance = light_distance;
		for (auto& shape : shapes) {
			if (float distance = shape->intersection(light_ray).value_or(INFINITY);
				distance < closest_distance) {
				closest_distance = distance;
			}
		}

		// Check if we had an intersection (the light is occluded).
		if (closest_distance < light_distance) {
			// Multiplying our final color by the ambient light darkens this surface "entirely".
			final_color = final_color * AMBIENT_LIGHT;
		}  else {
			// Next we'll compute the dot product between our surface normal and the light ray.
			// We need to clamp this between 0 and 1, because negative values have no meaning here.
			// Additionally, we'll add in our ambient light so no surfaces are entirely dark.
			float dot = std::clamp(AMBIENT_LIGHT + (light_ray.direction * normal.direction), 0.0f, 1.0f);

			// Multiplying our final color by this dot product darkens surfaces pointing away from the light.
			final_color = final_color * dot;
		}

		// Apply Fog
		if (FOG_INTENSITY)
			final_color = lerp(final_color, FOG, intersection_distance * FOG_INTENSITY);

		return final_color;
	}

private:

	// A vector of Shape smart pointers representing our scene.
	// Because these are smart pointers we can point to subclasses of Shape.
	std::vector<std::unique_ptr<Shape>> shapes;

	// Apply a linear interpolation between two colors:
	//  from |-------------------------------| to
	//                ^ by
	color3 lerp(color3 from, color3 to, float by) const {
		if (by <= 0.0f) return from;
		if (by >= 1.0f) return to;
		return color3(
			from.x * (1 - by) + to.x * by,
			from.y * (1 - by) + to.y * by,
			from.z * (1 - by) + to.z * by
		);
	}

	// The position of our point light.
	vf3d light_point;
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
