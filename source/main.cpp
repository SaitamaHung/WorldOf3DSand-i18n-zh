#include <math.h>
#include <stdlib.h>

#include <string>
#include <sstream>

#include <ctrcommon/common.hpp>

#include "particle.h"

#define VERSION "1.2"

int main(int argc, char **argv) {
	if(!platform_init()) {
		return 0;
	}

	std::stringstream controlStream;
	controlStream << "Touch to draw particles." << "\n";
	controlStream << "Press Start to exit." << "\n";
	controlStream << "Press Select to take a screenshot." << "\n";
	controlStream << "Press B to clear the screen." << "\n";
	controlStream << "Press Up/Down to modify the pen size." << "\n";
	controlStream << "Press Left/Right to modify the emitter density." << "\n";
	const std::string controls = controlStream.str();

	ParticleType::InitParticles();

	screen_begin_draw(BOTTOM_SCREEN);
	int sceneScreenWidth = screen_get_width();
	int sceneScreenHeight = screen_get_height();
	screen_end_draw();

	Color dashboardColor = {155, 155, 155};
	Color dashboardSideColor = {50, 50, 50};

	int buttonsTotal = 0;
	for(std::vector<ParticleType*>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
		ParticleType* type = *i;
		if(type->IsSelectable()) {
			buttonsTotal++;
		}
	}

	int buttonsPerRow = 10;
	int buttonsPerColumn = 3;
	int buttonPadding = 1;
	int buttonWidth = 24;
	int buttonHeight = 12;
	int paddedButtonWidth = buttonWidth + buttonPadding;
	int paddedButtonHeight = buttonHeight + buttonPadding;

	int dashboardWidth = buttonsPerRow * paddedButtonWidth + buttonPadding;
	int dashboardHeight = buttonsPerColumn * paddedButtonHeight + buttonPadding;
	int dashboardX = (sceneScreenWidth - dashboardWidth) / 2;
	int dashboardY = sceneScreenHeight - dashboardHeight;

	int buttonBaseX = dashboardX + buttonPadding;
	int buttonBaseY = dashboardY + buttonPadding;

	Scene* scene = new Scene(sceneScreenWidth, sceneScreenHeight - dashboardHeight);
	ParticleType* selectedType = ParticleType::WALL;

	int penSize = 2;
	int oldx = 0;
	int oldy = 0;

	int emitters = 4;
	bool emit[emitters] = {true, true, true, true};
	ParticleType* emitTypes[emitters] = {ParticleType::WATER, ParticleType::SAND, ParticleType::SALT, ParticleType::OIL};
	float emitDensity = 0.3f;

	srand((unsigned int) platform_get_time());

	int fpsCounter = 0;
	int fps = 0;
	u64 lastfps = platform_get_time() - 1000;
	while(platform_is_running()) {
		input_poll();
		if(input_is_pressed(BUTTON_START)) {
			// Exit by breaking out of the main loop.
			break;
		}

		// Check for scene clear.
		if(input_is_pressed(BUTTON_B)) {
			scene->Clear();
		}

		// Check for pen size increase.
		if(input_is_pressed(BUTTON_UP)) {
			penSize++;
			if(penSize > 40) {
				penSize = 40;
			}
		}

		// Check for pen size decrease.
		if(input_is_pressed(BUTTON_DOWN)) {
			penSize--;
			if(penSize < 1) {
				penSize = 1;
			}
		}

		// Check for emitter density increase.
		if(input_is_pressed(BUTTON_LEFT)) {
			emitDensity -= 0.05f;
			if(emitDensity < 0.05f) {
				emitDensity = 0.05f;
			}
		}

		// Check for emitter density decrease.
		if(input_is_pressed(BUTTON_RIGHT)) {
			emitDensity += 0.05f;
			if(emitDensity > 1) {
				emitDensity = 1;
			}
		}

		// If a touch has just been started, reset the old x and y variables to its position and do GUI interaction.
		if(input_is_pressed(BUTTON_TOUCH)) {
			Touch touch = input_get_touch();
			oldx = touch.x;
			oldy = touch.y;

			// Check for GUI interaction.
			if(touch.x > dashboardX && touch.x < dashboardX + dashboardWidth && touch.y > dashboardY && touch.y < dashboardY + dashboardHeight) {
				int index = 0;
				for(std::vector<ParticleType *>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
					ParticleType *type = *i;
					if(type->IsSelectable()) {
						int x = ((index % buttonsPerRow) * paddedButtonWidth) + buttonBaseX;
						int y = ((index / buttonsPerRow) * paddedButtonHeight) + buttonBaseY;
						if(touch.x > x && touch.y > y && touch.x <= x + buttonWidth && touch.y <= y + buttonHeight) {
							selectedType = type;
							break;
						}

						index++;
					}
				}
			} else if(touch.y > dashboardY && touch.y < dashboardY + dashboardHeight) {
				for(int emitter = 0; emitter < emitters; emitter++) {
					int x = ((sceneScreenWidth - dashboardWidth) / 2 - (paddedButtonWidth + buttonPadding)) / 2;
					int y = buttonBaseY + paddedButtonHeight * (emitter % 2);
					if(emitter >= emitters / 2) {
						x = sceneScreenWidth - x - buttonWidth;
					}

					if(touch.x > x && touch.y > y && touch.x <= x + buttonWidth && touch.y <= y + buttonHeight) {
						emit[emitter] = !emit[emitter];
						break;
					}
				}
			}
		}

		// Draw a line from touch input.
		if(input_is_held(BUTTON_TOUCH)) {
			Touch touch = input_get_touch();
			if(touch.x < scene->GetWidth() && touch.y < scene->GetHeight()) {
				scene->CreateLine(touch.x, touch.y, oldx, oldy, penSize, selectedType);
			}

			oldx = touch.x;
			oldy = touch.y;
		}

		// Emit particles from enabled emitters.
		int emitterSpacing = scene->GetWidth() / emitters;
		for(int emitter = 0; emitter < emitters; emitter++) {
			if(emit[emitter]) {
				scene->Emit(emitterSpacing * emitter + (emitterSpacing / 2), 20, emitTypes[emitter], emitDensity);
			}
		}

		// Update the scene.
		scene->Update();

		// Prepare to draw.
		screen_begin_draw(BOTTOM_SCREEN);
		screen_clear(0, 0, 0);

		// Draw the game scene.
		scene->Draw();

		// Draw GUI.
		screen_fill(0, dashboardY, (sceneScreenWidth - dashboardWidth) / 2, dashboardHeight, dashboardSideColor.r, dashboardSideColor.g, dashboardSideColor.b);
		screen_fill(dashboardX, dashboardY, dashboardWidth, dashboardHeight, dashboardColor.r, dashboardColor.g, dashboardColor.b);
		screen_fill(dashboardX + dashboardWidth, dashboardY, (sceneScreenWidth - dashboardWidth) / 2, dashboardHeight, dashboardColor.r, dashboardColor.g, dashboardColor.b);
		int index = 0;
		for(std::vector<ParticleType*>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
			ParticleType* type = *i;
			if(type->IsSelectable()) {
				int x = ((index % buttonsPerRow) * paddedButtonWidth) + buttonBaseX;
				int y = ((index / buttonsPerRow) * paddedButtonHeight) + buttonBaseY;
				screen_fill(x, y, buttonWidth, buttonHeight, type->GetColor().r, type->GetColor().g, type->GetColor().b);
				if(type == selectedType) {
					screen_fill(x, y, buttonWidth, buttonPadding, 0, 0, 0);
					screen_fill(x + buttonWidth - buttonPadding, y, buttonPadding, buttonHeight, 0, 0, 0);
					screen_fill(x, y + buttonHeight - buttonPadding, buttonWidth, buttonPadding, 0, 0, 0);
					screen_fill(x, y, buttonPadding, buttonHeight, 0, 0, 0);
				}

				index++;
			}
		}

		for(int emitter = 0; emitter < emitters; emitter++) {
			int x = ((sceneScreenWidth - dashboardWidth) / 2 - (paddedButtonWidth + buttonPadding)) / 2;
			int y = buttonBaseY + paddedButtonHeight * (emitter % 2);
			if(emitter >= emitters / 2) {
				x = sceneScreenWidth - x - buttonWidth;
			}

			screen_fill(x, y, buttonWidth, buttonHeight, emitTypes[emitter]->GetColor().r, emitTypes[emitter]->GetColor().g, emitTypes[emitter]->GetColor().b);
			if(emit[emitter]) {
				screen_fill(x, y, buttonWidth, buttonPadding, 0, 0, 0);
				screen_fill(x + buttonWidth - buttonPadding, y, buttonPadding, buttonHeight, 0, 0, 0);
				screen_fill(x, y + buttonHeight - buttonPadding, buttonWidth, buttonPadding, 0, 0, 0);
				screen_fill(x, y, buttonPadding, buttonHeight, 0, 0, 0);
			}
		}

		// Clean up after drawing.
		screen_end_draw();

		// Prepare to draw on-screen info.
		screen_begin_draw(TOP_SCREEN);
		screen_clear((u8) 0, (u8) 0, (u8) 0);

		// Draw on-screen info.
		std::stringstream stream;
		stream << "World of 3DSand v" << VERSION << "\n";
		stream << "FPS: " << fps << "\n";
		stream << "Particle count: " << scene->GetParticleCount() << "\n";
		stream << "Selected Particle: " << selectedType->GetName() << "\n";
		stream << "Pen Size: " << penSize << "\n";
		stream << "Emitters: ";
		bool one = false;
		for(int emitter = 0; emitter < emitters; emitter++) {
			if(emit[emitter]) {
				if(one) {
					stream << ", ";
				}

				one = true;
				stream << emitTypes[emitter]->GetName();
			}
		}

		stream << "\n";
		stream << "Emitter Density: " << emitDensity << "\n";
		stream << "\n" << controls;
		screen_draw_string(stream.str(), 0, 0, 255, 255, 255);

		// Clean up after drawing.
		screen_end_draw();

		if(input_is_pressed(BUTTON_SELECT)) {
			// Take a screenshot.
			screen_take_screenshot();
		}

		// Swap buffers, putting the newly drawn frame on-screen.
		screen_swap_buffers();

		// Perform FPS counting logic.
		fpsCounter++;
		u64 since = platform_get_time() - lastfps;
		if(since >= 1000) {
			fps = fpsCounter;
			fpsCounter = 0;
			lastfps = platform_get_time();
		}
	}

	// Delete the scene and clean up.
	delete(scene);
	platform_cleanup();
	return 0;
}
