#include "chip8.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

chip8 programChip;

// Override base class with your custom functionality
class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		// Name your application
		sAppName = "Example";
	}

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (programChip.drawFlag) {
			for(int y = 0; y < 32; ++y)
				for (int x = 0; x < 64; ++x) {
					if (programChip.gfx[(y * 64) + x] == 0)
						Draw(x, y, olc::Pixel(255,255,255));	// Disabled
					else
						Draw(x, y, olc::Pixel(0,0,0));  // Enabled
				}
		}
		programChip.emulateCycle();
		return true;
	}
};

int main(int argc, char** argv) {
	programChip.loadFile("./IBMLogo.c8");
	Example demo;
	if (demo.Construct(64, 32, 20, 20))
		demo.Start();
	return 0;
}
