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
	float fTargetFrameTime = 1.0f / 600.0f; // Virtual FPS of 100fps
	float fAccumulatedTime = 0.0f;

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
						Draw(x, y, olc::Pixel(0, 0, 0));	// Disabled
					else
						Draw(x, y, olc::Pixel(255, 255, 255)); // Enabled
				}
		}

		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= fTargetFrameTime)
		{
			fAccumulatedTime -= fTargetFrameTime;
			fElapsedTime = fTargetFrameTime;
			handleUserInput();
			programChip.emulateCycle();
		}
		return true;
	}

	void handleUserInput() {
		if (GetKey(olc::Key::K1).bPressed) programChip.currentKey[0x1] = 1;
		if (GetKey(olc::Key::K2).bPressed) programChip.currentKey[0x2] = 1;
		if (GetKey(olc::Key::K3).bPressed) programChip.currentKey[0x3] = 1;
		if (GetKey(olc::Key::K4).bPressed) programChip.currentKey[0xC] = 1;
		if (GetKey(olc::Key::Q).bPressed) programChip.currentKey[0x4] = 1;
		if (GetKey(olc::Key::W).bPressed) programChip.currentKey[0x5] = 1;
		if (GetKey(olc::Key::E).bPressed) programChip.currentKey[0x6] = 1;
		if (GetKey(olc::Key::R).bPressed) programChip.currentKey[0xD] = 1;
		if (GetKey(olc::Key::A).bPressed) programChip.currentKey[0x7] = 1;
		if (GetKey(olc::Key::S).bPressed) programChip.currentKey[0x8] = 1;
		if (GetKey(olc::Key::D).bPressed) programChip.currentKey[0x9] = 1;
		if (GetKey(olc::Key::F).bPressed) programChip.currentKey[0xE] = 1;
		if (GetKey(olc::Key::Z).bPressed) programChip.currentKey[0xA] = 1;
		if (GetKey(olc::Key::X).bPressed) programChip.currentKey[0x0] = 1;
		if (GetKey(olc::Key::C).bPressed) programChip.currentKey[0xB] = 1;
		if (GetKey(olc::Key::V).bPressed) programChip.currentKey[0xF] = 1;

		if (GetKey(olc::Key::K1).bReleased) programChip.currentKey[0x1] = 0;
		if (GetKey(olc::Key::K2).bReleased) programChip.currentKey[0x2] = 0;
		if (GetKey(olc::Key::K3).bReleased) programChip.currentKey[0x3] = 0;
		if (GetKey(olc::Key::K4).bReleased) programChip.currentKey[0xC] = 0;
		if (GetKey(olc::Key::Q).bReleased) programChip.currentKey[0x4] = 0;
		if (GetKey(olc::Key::W).bReleased) programChip.currentKey[0x5] = 0;
		if (GetKey(olc::Key::E).bReleased) programChip.currentKey[0x6] = 0;
		if (GetKey(olc::Key::R).bReleased) programChip.currentKey[0xD] = 0;
		if (GetKey(olc::Key::A).bReleased) programChip.currentKey[0x7] = 0;
		if (GetKey(olc::Key::S).bReleased) programChip.currentKey[0x8] = 0;
		if (GetKey(olc::Key::D).bReleased) programChip.currentKey[0x9] = 0;
		if (GetKey(olc::Key::F).bReleased) programChip.currentKey[0xE] = 0;
		if (GetKey(olc::Key::Z).bReleased) programChip.currentKey[0xA] = 0;
		if (GetKey(olc::Key::X).bReleased) programChip.currentKey[0x0] = 0;
		if (GetKey(olc::Key::C).bReleased) programChip.currentKey[0xB] = 0;
		if (GetKey(olc::Key::V).bReleased) programChip.currentKey[0xF] = 0;

	}
};

int main(int argc, char** argv) {
	programChip.loadFile("./currGame.c8");
	Example demo;
	if (demo.Construct(64, 32, 20, 20))
		demo.Start();
	return 0;
}
