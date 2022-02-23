#include <cmath>
#include <vector>
#include <memory>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

/***** TYPES *****/

// Class to describe any kind of object we want to add to our scene.
class Shape {

};

// Subclass of Shape that represents a Sphere.
class Sphere : public Shape {

};

/***** CONSTANTS *****/

// Game width and height (in pixels).
constexpr int WIDTH = 250;
constexpr int HEIGHT = 250;

// Half the game width and height (to identify the center of the screen).
constexpr float HALF_WIDTH = WIDTH / 2.0f;
constexpr float HALF_HEIGHT = HEIGHT / 2.0f;

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
		shapes.emplace_back(std::make_unique<Sphere>());

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

		// For now we're returning a color based on the screen coordinates.
		return olc::Pixel(std::abs(x * 255), std::abs(y * 255), 0);
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
