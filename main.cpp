#include <vector>
#include <memory>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

// Class to describe any kind of object we want to add to our scene.
class Shape {

};

// Subclass of Shape that represents a Sphere.
class Sphere : public Shape {

};

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

		Clear(olc::MAGENTA);

		return true;
	}

private:

	// A vector of Shape smart pointers representing our scene.
	// Because these are smart pointers we can point to subclasses of Shape.
	std::vector<std::unique_ptr<Shape>> shapes;
};

// Program entrypoint
int main() {
	// Create an instance of our PixelGameEngine
	OlcPixelRayTracer ray_tracer;

	// Construct and start it
	if (ray_tracer.Construct(256, 240, 4, 4))
		ray_tracer.Start();

	return 0;
}
