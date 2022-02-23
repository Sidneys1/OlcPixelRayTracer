#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

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
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		// Called once per frame, draws random coloured pixels
		for (int x = 0; x < ScreenWidth(); x++)
			for (int y = 0; y < ScreenHeight(); y++)
				Draw(x, y, olc::Pixel(rand() % 256, rand() % 256, rand() % 256));
		return true;
	}
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
