#include "particle.h"

#include <iomanip>
#include <sstream>

#include <ctrcommon/gpu.hpp>
#include <ctrcommon/input.hpp>
#include <ctrcommon/platform.hpp>

#define VERSION "1.3"

int main(int argc, char **argv) {
    if(!platformInit()) {
        return 0;
    }

    std::stringstream controlStream;
    controlStream << "Touch to draw particles." << "\n";
    if(platformIsNinjhax()) {
        controlStream << "Press Start to exit." << "\n";
    }

    controlStream << "Press Select to take a screenshot." << "\n";
    controlStream << "Press B to clear the screen." << "\n";
    controlStream << "Press Up/Down to modify the pen size." << "\n";
    controlStream << "Press Left/Right to modify the emitter density." << "\n";
    const std::string controls = controlStream.str();

    ParticleType::InitParticles();

    u16 sceneScreenWidth = 320;
    u16 sceneScreenHeight = 240;

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
    int dashboardY = 0;

    int buttonBaseX = dashboardX + buttonPadding;
    int buttonBaseY = dashboardY + buttonPadding;

    Scene *scene = new Scene(sceneScreenWidth, sceneScreenHeight - dashboardHeight);
    ParticleType *selectedType = ParticleType::WALL;

    int penSize = 2;
    int oldx = 0;
    int oldy = 0;

    int emitters = 4;
    int emitterSpacing = scene->GetWidth() / emitters;
    bool emit[emitters] = {true, true, true, true};
    ParticleType *emitTypes[emitters] = {ParticleType::WATER, ParticleType::SAND, ParticleType::SALT, ParticleType::OIL};
    float emitDensity = 0.3f;

    srand((unsigned int) platformGetTime());

    u32 sceneVbo = 0;
    gpuCreateVbo(&sceneVbo);
    gpuVboAttributes(sceneVbo, ATTRIBUTE(0, 3, ATTR_FLOAT) | ATTRIBUTE(1, 2, ATTR_FLOAT) | ATTRIBUTE(2, 4, ATTR_FLOAT), 3);

    const float x1 = 0;
    const float y1 = dashboardY + dashboardHeight;
    const float x2 = x1 + scene->GetWidth();
    const float y2 = y1 + scene->GetHeight();

    const float texX1 = 0.0f;
    const float texY1 = (512.0f - scene->GetHeight()) / 512.0f;
    const float texX2 = texX1 + (scene->GetWidth() / 512.0f);
    const float texY2 = texY1 + (scene->GetHeight() / 512.0f);

    const float vboData[] = {
            x1, y1, -0.1f, texX1, texY1, 1.0f, 1.0f, 1.0f, 1.0f,
            x2, y1, -0.1f, texX2, texY1, 1.0f, 1.0f, 1.0f, 1.0f,
            x2, y2, -0.1f, texX2, texY2, 1.0f, 1.0f, 1.0f, 1.0f,
            x2, y2, -0.1f, texX2, texY2, 1.0f, 1.0f, 1.0f, 1.0f,
            x1, y2, -0.1f, texX1, texY2, 1.0f, 1.0f, 1.0f, 1.0f,
            x1, y1, -0.1f, texX1, texY1, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    gpuVboData(sceneVbo, vboData, 6 * 9, PRIM_TRIANGLES);

    int fpsCounter = 0;
    int fps = 0;
    u64 lastfps = platformGetTime() - 1000;
    while(platformIsRunning()) {
        inputPoll();
        if(inputIsPressed(BUTTON_START) && platformIsNinjhax()) {
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
            int dashboardTouchY = 239 - touch.y;
            if(touch.x > dashboardX && touch.x < dashboardX + dashboardWidth && dashboardTouchY > dashboardY && dashboardTouchY < dashboardY + dashboardHeight) {
                int index = 0;
                for(std::vector<ParticleType *>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
                    ParticleType *type = *i;
                    if(type->IsSelectable()) {
                        int x = ((index % buttonsPerRow) * paddedButtonWidth) + buttonBaseX;
                        int y = ((2 - (index / buttonsPerRow)) * paddedButtonHeight) + buttonBaseY;
                        if(touch.x > x && dashboardTouchY > y && touch.x <= x + buttonWidth && dashboardTouchY <= y + buttonHeight) {
                            selectedType = type;
                            break;
                        }

                        index++;
                    }
                }
            } else if(dashboardTouchY > dashboardY && dashboardTouchY < dashboardY + dashboardHeight) {
                for(int emitter = 0; emitter < emitters; emitter++) {
                    int x = ((sceneScreenWidth - dashboardWidth) / 2 - (paddedButtonWidth + buttonPadding)) / 2;
                    int y = buttonBaseY + paddedButtonHeight * (2 - (emitter % 2));
                    if(emitter >= emitters / 2) {
                        x = sceneScreenWidth - x - buttonWidth;
                    }

                    if(touch.x > x && dashboardTouchY > y && touch.x <= x + buttonWidth && dashboardTouchY <= y + buttonHeight) {
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
        for(int emitter = 0; emitter < emitters; emitter++) {
            if(emit[emitter]) {
                scene->Emit(emitterSpacing * emitter + (emitterSpacing / 2), 20, emitTypes[emitter], emitDensity);
            }
        }

        // Update the scene.
        scene->Update();

        // Prepare to draw.
        gpuViewport(BOTTOM_SCREEN, 0, 0, 320, 240);
        gpuClear();

        // Draw the game scene.
        gpuBindTexture(TEXUNIT0, scene->GetTexture());
        gpuDrawVbo(sceneVbo);

        // Draw GUI.
        gputDrawRectangle(0, dashboardY, (u32) ((sceneScreenWidth - dashboardWidth) / 2), dashboardHeight, dashboardSideColor, dashboardSideColor, dashboardSideColor);
        gputDrawRectangle(dashboardX, dashboardY, dashboardWidth, dashboardHeight, dashboardColor, dashboardColor, dashboardColor);
        gputDrawRectangle(dashboardX + dashboardWidth, dashboardY, (u32) ((sceneScreenWidth - dashboardWidth) / 2), dashboardHeight, dashboardSideColor, dashboardSideColor, dashboardSideColor);
        int index = 0;
        for(std::vector<ParticleType *>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
            ParticleType *type = *i;
            if(type->IsSelectable()) {
                int x = ((index % buttonsPerRow) * paddedButtonWidth) + buttonBaseX;
                int y = ((2 - (index / buttonsPerRow)) * paddedButtonHeight) + buttonBaseY;
                gputDrawRectangle(x, y, buttonWidth, buttonHeight, type->GetRed(), type->GetGreen(), type->GetBlue());
                if(type == selectedType) {
                    gputDrawRectangle(x, y, buttonWidth, buttonPadding, 0, 0, 0);
                    gputDrawRectangle(x + buttonWidth - buttonPadding, y, buttonPadding, buttonHeight, 0, 0, 0);
                    gputDrawRectangle(x, y + buttonHeight - buttonPadding, buttonWidth, buttonPadding, 0, 0, 0);
                    gputDrawRectangle(x, y, buttonPadding, buttonHeight, 0, 0, 0);
                }

                index++;
            }
        }

        for(int emitter = 0; emitter < emitters; emitter++) {
            int x = ((sceneScreenWidth - dashboardWidth) / 2 - (paddedButtonWidth + buttonPadding)) / 2;
            int y = buttonBaseY + paddedButtonHeight * (2 - (emitter % 2));
            if(emitter >= emitters / 2) {
                x = sceneScreenWidth - x - buttonWidth;
            }

            gputDrawRectangle(x, y, buttonWidth, buttonHeight, emitTypes[emitter]->GetRed(), emitTypes[emitter]->GetGreen(), emitTypes[emitter]->GetBlue());
            if(emit[emitter]) {
                gputDrawRectangle(x, y, buttonWidth, buttonPadding, 0, 0, 0);
                gputDrawRectangle(x + buttonWidth - buttonPadding, y, buttonPadding, buttonHeight, 0, 0, 0);
                gputDrawRectangle(x, y + buttonHeight - buttonPadding, buttonWidth, buttonPadding, 0, 0, 0);
                gputDrawRectangle(x, y, buttonPadding, buttonHeight, 0, 0, 0);
            }
        }

        // Clean up after drawing.
        gpuFlush();
        gpuFlushBuffer();

        // Prepare to draw on-screen info.
        gpuViewport(TOP_SCREEN, 0, 0, 400, 240);
        gpuClear();

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
        gputDrawString(stream.str(), 0, gpuGetViewportHeight() - 1 - gputGetStringHeight(stream.str()));

        // Clean up after drawing.
        gpuFlush();
        gpuFlushBuffer();

        if(inputIsPressed(BUTTON_SELECT)) {
            // Take a screenshot.
            gputTakeScreenshot();
        }

        // Swap buffers, putting the newly drawn frame on-screen.
        gpuSwapBuffers(true);

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
