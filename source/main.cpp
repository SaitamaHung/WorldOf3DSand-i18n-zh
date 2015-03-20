#include "particle.h"

#include <stdlib.h>

#include <sstream>

#include <ctrcommon/input.hpp>
#include <ctrcommon/platform.hpp>
#include <ctrcommon/screen.hpp>

#define VERSION "1.2"

int main(int argc, char **argv) {
    if(!platformInit()) {
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

    screenBeginDraw(BOTTOM_SCREEN);
    u16 sceneScreenWidth = screenGetWidth();
    u16 sceneScreenHeight = screenGetHeight();
    screenEndDraw();

    u8 dashboardColor = 155;
    u8 dashboardSideColor = 50;

    int buttonsTotal = 0;
    for(std::vector<ParticleType *>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
        ParticleType *type = *i;
        if(type->IsSelectable()) {
            buttonsTotal++;
        }
    }

    u16 buttonsPerRow = 10;
    u16 buttonsPerColumn = 3;
    u16 buttonPadding = 1;
    u16 buttonWidth = 24;
    u16 buttonHeight = 12;
    u16 paddedButtonWidth = buttonWidth + buttonPadding;
    u16 paddedButtonHeight = buttonHeight + buttonPadding;

    u16 dashboardWidth = buttonsPerRow * paddedButtonWidth + buttonPadding;
    u16 dashboardHeight = buttonsPerColumn * paddedButtonHeight + buttonPadding;
    int dashboardX = (sceneScreenWidth - dashboardWidth) / 2;
    int dashboardY = sceneScreenHeight - dashboardHeight;

    int buttonBaseX = dashboardX + buttonPadding;
    int buttonBaseY = dashboardY + buttonPadding;

    Scene *scene = new Scene(sceneScreenWidth, sceneScreenHeight - dashboardHeight);
    ParticleType *selectedType = ParticleType::WALL;

    int penSize = 2;
    int oldx = 0;
    int oldy = 0;

    int emitters = 4;
    bool emit[emitters] = {true, true, true, true};
    ParticleType *emitTypes[emitters] = {ParticleType::WATER, ParticleType::SAND, ParticleType::SALT, ParticleType::OIL};
    float emitDensity = 0.3f;

    srand((unsigned int) platformGetTime());

    int fpsCounter = 0;
    int fps = 0;
    u64 lastfps = platformGetTime() - 1000;
    while(platformIsRunning()) {
        inputPoll();
        if(inputIsPressed(BUTTON_START)) {
            // Exit by breaking out of the main loop.
            break;
        }

        // Check for scene clear.
        if(inputIsPressed(BUTTON_B)) {
            scene->Clear();
        }

        // Check for pen size increase.
        if(inputIsPressed(BUTTON_UP)) {
            penSize++;
            if(penSize > 40) {
                penSize = 40;
            }
        }

        // Check for pen size decrease.
        if(inputIsPressed(BUTTON_DOWN)) {
            penSize--;
            if(penSize < 1) {
                penSize = 1;
            }
        }

        // Check for emitter density increase.
        if(inputIsPressed(BUTTON_LEFT)) {
            emitDensity -= 0.05f;
            if(emitDensity < 0.05f) {
                emitDensity = 0.05f;
            }
        }

        // Check for emitter density decrease.
        if(inputIsPressed(BUTTON_RIGHT)) {
            emitDensity += 0.05f;
            if(emitDensity > 1) {
                emitDensity = 1;
            }
        }

        // If a touch has just been started, reset the old x and y variables to its position and do GUI interaction.
        if(inputIsPressed(BUTTON_TOUCH)) {
            Touch touch = inputGetTouch();
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
        if(inputIsHeld(BUTTON_TOUCH)) {
            Touch touch = inputGetTouch();
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
        screenBeginDraw(BOTTOM_SCREEN);
        screenClear(0, 0, 0);

        // Draw the game scene.
        scene->Draw();

        // Draw GUI.
        screenFill(0, dashboardY, (u16) ((sceneScreenWidth - dashboardWidth) / 2), dashboardHeight, dashboardSideColor, dashboardSideColor, dashboardSideColor);
        screenFill(dashboardX, dashboardY, dashboardWidth, dashboardHeight, dashboardColor, dashboardColor, dashboardColor);
        screenFill(dashboardX + dashboardWidth, dashboardY, (u16) ((sceneScreenWidth - dashboardWidth) / 2), dashboardHeight, dashboardColor, dashboardColor, dashboardColor);
        int index = 0;
        for(std::vector<ParticleType *>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
            ParticleType *type = *i;
            if(type->IsSelectable()) {
                int x = ((index % buttonsPerRow) * paddedButtonWidth) + buttonBaseX;
                int y = ((index / buttonsPerRow) * paddedButtonHeight) + buttonBaseY;
                screenFill(x, y, buttonWidth, buttonHeight, type->GetRed(), type->GetGreen(), type->GetBlue());
                if(type == selectedType) {
                    screenFill(x, y, buttonWidth, buttonPadding, 0, 0, 0);
                    screenFill(x + buttonWidth - buttonPadding, y, buttonPadding, buttonHeight, 0, 0, 0);
                    screenFill(x, y + buttonHeight - buttonPadding, buttonWidth, buttonPadding, 0, 0, 0);
                    screenFill(x, y, buttonPadding, buttonHeight, 0, 0, 0);
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

            screenFill(x, y, buttonWidth, buttonHeight, emitTypes[emitter]->GetRed(), emitTypes[emitter]->GetGreen(), emitTypes[emitter]->GetBlue());
            if(emit[emitter]) {
                screenFill(x, y, buttonWidth, buttonPadding, 0, 0, 0);
                screenFill(x + buttonWidth - buttonPadding, y, buttonPadding, buttonHeight, 0, 0, 0);
                screenFill(x, y + buttonHeight - buttonPadding, buttonWidth, buttonPadding, 0, 0, 0);
                screenFill(x, y, buttonPadding, buttonHeight, 0, 0, 0);
            }
        }

        // Clean up after drawing.
        screenEndDraw();

        // Prepare to draw on-screen info.
        screenBeginDraw(TOP_SCREEN);
        screenClear((u8) 0, (u8) 0, (u8) 0);

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
        screenDrawString(stream.str(), 0, 0, 255, 255, 255);

        // Clean up after drawing.
        screenEndDraw();

        if(inputIsPressed(BUTTON_SELECT)) {
            // Take a screenshot.
            screenTakeScreenshot();
        }

        // Swap buffers, putting the newly drawn frame on-screen.
        screenSwapBuffers();

        // Perform FPS counting logic.
        fpsCounter++;
        u64 since = platformGetTime() - lastfps;
        if(since >= 1000) {
            fps = fpsCounter;
            fpsCounter = 0;
            lastfps = platformGetTime();
        }
    }

    // Delete the scene and clean up.
    delete(scene);
    platformCleanup();
    return 0;
}
