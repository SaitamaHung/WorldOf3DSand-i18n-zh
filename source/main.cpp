#include "particle.h"

#include <string.h>

#include <iomanip>
#include <sstream>

#include <ctrcommon/gpu.hpp>
#include <ctrcommon/input.hpp>
#include <ctrcommon/platform.hpp>

#include "gui_bin.h"

#define VERSION "1.3.1"

int main(int argc, char **argv) {
    if(!platformInit(argc)) {
        return 0;
    }

    srand((unsigned int) platformGetTime());

    ParticleType::InitParticles();

    Scene *scene = new Scene(BOTTOM_WIDTH, BOTTOM_HEIGHT - 40);
    ParticleType *selectedType = ParticleType::WALL;

    int penSize = 2;
    int oldx = 0;
    int oldy = 0;

    int emitters = 4;
    int emitterSpacing = scene->GetWidth() / emitters;
    bool emit[emitters] = {true, true, true, true};
    ParticleType *emitTypes[emitters] = {ParticleType::WATER, ParticleType::SAND, ParticleType::SALT, ParticleType::OIL};
    float emitDensity = 0.3f;

    std::stringstream controlStream;
    controlStream << "Touch to draw particles." << "\n";
    if(platformHasLauncher()) {
        controlStream << "Press Start to exit." << "\n";
    }

    controlStream << "Press Select to take a screenshot." << "\n";
    controlStream << "Press B to clear the screen." << "\n";
    controlStream << "Press Up/Down to modify the pen size." << "\n";
    controlStream << "Press Left/Right to modify the emitter density." << "\n";
    const std::string controls = controlStream.str();

    u32 guiTexture = 0;
    void* gpuGuiTexture = gpuAlloc(gui_bin_size);
    memcpy(gpuGuiTexture, gui_bin, gui_bin_size);
    gpuCreateTexture(&guiTexture);
    gpuTextureData(guiTexture, gpuGuiTexture, 512, 64, PIXEL_RGBA8, TEXTURE_MIN_FILTER(FILTER_NEAREST) | TEXTURE_MAG_FILTER(FILTER_NEAREST));
    gpuFree(gpuGuiTexture);

    u32 dashboardVbo;
    gpuCreateVbo(&dashboardVbo);
    gpuVboAttributes(dashboardVbo, ATTRIBUTE(0, 3, ATTR_FLOAT) | ATTRIBUTE(1, 2, ATTR_FLOAT) | ATTRIBUTE(2, 4, ATTR_FLOAT), 3);

    const float dashTexX1 = 0.0f;
    const float dashTexY1 = (64.0f - 40.0f) / 64.0f;
    const float dashTexX2 = dashTexX1 + (320.0f / 512.0f);
    const float dashTexY2 = dashTexY1 + (40.0f / 64.0f);

    const float dashboardVboData[] = {
            0.0f, 0.0f, -0.1f, dashTexX1, dashTexY1, 1.0f, 1.0f, 1.0f, 1.0f,
            320.0f, 0.0f, -0.1f, dashTexX2, dashTexY1, 1.0f, 1.0f, 1.0f, 1.0f,
            320.0f, 40.0f, -0.1f, dashTexX2, dashTexY2, 1.0f, 1.0f, 1.0f, 1.0f,
            320.0f, 40.0f, -0.1f, dashTexX2, dashTexY2, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 40.0f, -0.1f, dashTexX1, dashTexY2, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 0.0f, -0.1f, dashTexX1, dashTexY1, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    gpuVboData(dashboardVbo, dashboardVboData, 6 * 9, PRIM_TRIANGLES);

    u32 selectorVbo;
    gpuCreateVbo(&selectorVbo);
    gpuVboAttributes(selectorVbo, ATTRIBUTE(0, 3, ATTR_FLOAT) | ATTRIBUTE(1, 2, ATTR_FLOAT) | ATTRIBUTE(2, 4, ATTR_FLOAT), 3);

    const float selectorTexX1 = 0.0f;
    const float selectorTexY1 = (64.0f - 52.0f) / 64.0f;
    const float selectorTexX2 = selectorTexX1 + (24.0f / 512.0f);
    const float selectorTexY2 = selectorTexY1 + (12.0f / 64.0f);

    const float selectorVboData[] = {
            0.0f, 0.0f, -0.1f, selectorTexX1, selectorTexY1, 1.0f, 1.0f, 1.0f, 1.0f,
            24.0f, 0.0f, -0.1f, selectorTexX2, selectorTexY1, 1.0f, 1.0f, 1.0f, 1.0f,
            24.0f, 12.0f, -0.1f, selectorTexX2, selectorTexY2, 1.0f, 1.0f, 1.0f, 1.0f,
            24.0f, 12.0f, -0.1f, selectorTexX2, selectorTexY2, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 12.0f, -0.1f, selectorTexX1, selectorTexY2, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 0.0f, -0.1f, selectorTexX1, selectorTexY1, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    gpuVboData(selectorVbo, selectorVboData, 6 * 9, PRIM_TRIANGLES);

    u32 sceneVbo = 0;
    gpuCreateVbo(&sceneVbo);
    gpuVboAttributes(sceneVbo, ATTRIBUTE(0, 3, ATTR_FLOAT) | ATTRIBUTE(1, 2, ATTR_FLOAT) | ATTRIBUTE(2, 4, ATTR_FLOAT), 3);

    const float sceneTexX1 = 0.0f;
    const float sceneTexY1 = (512.0f - scene->GetHeight()) / 512.0f;
    const float sceneTexX2 = sceneTexX1 + (scene->GetWidth() / 512.0f);
    const float sceneTexY2 = sceneTexY1 + (scene->GetHeight() / 512.0f);

    const float vboData[] = {
            0.0f, 40.0f, -0.1f, sceneTexX1, sceneTexY1, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f + scene->GetWidth(), 40.0f, -0.1f, sceneTexX2, sceneTexY1, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f + scene->GetWidth(), 40.0f + scene->GetHeight(), -0.1f, sceneTexX2, sceneTexY2, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f + scene->GetWidth(), 40.0f + scene->GetHeight(), -0.1f, sceneTexX2, sceneTexY2, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 40.0f + scene->GetHeight(), -0.1f, sceneTexX1, sceneTexY2, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 40.0f, -0.1f, sceneTexX1, sceneTexY1, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    gpuVboData(sceneVbo, vboData, 6 * 9, PRIM_TRIANGLES);

    int fpsCounter = 0;
    int fps = 0;
    u64 lastfps = platformGetTime() - 1000;
    while(platformIsRunning()) {
        inputPoll();
        if(inputIsPressed(BUTTON_START) && platformHasLauncher()) {
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
            if(dashboardTouchY > 0 && dashboardTouchY < 40) {
                if(touch.x > 35 && touch.x < 285 && dashboardTouchY > 0 && dashboardTouchY < 40) {
                    int index = 0;
                    for(std::vector<ParticleType *>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
                        ParticleType *type = *i;
                        if(type->IsSelectable()) {
                            int x = 35 + ((index % 10) * 25);
                            int y = ((2 - (index / 10)) * 13) + 1;
                            if(touch.x > x && dashboardTouchY > y && touch.x <= x + 24 && dashboardTouchY <= y + 12) {
                                selectedType = type;
                                break;
                            }

                            index++;
                        }
                    }
                } else if(dashboardTouchY > 0 && dashboardTouchY < 40) {
                    for(int emitter = 0; emitter < emitters; emitter++) {
                        int x = 4;
                        int y = ((2 - (emitter % 2)) * 13) + 1;
                        if(emitter >= emitters / 2) {
                            x = 320 - x - 24;
                        }

                        if(touch.x > x && dashboardTouchY > y && touch.x <= x + 24 && dashboardTouchY <= y + 12) {
                            emit[emitter] = !emit[emitter];
                            break;
                        }
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
        gpuViewport(BOTTOM_SCREEN, 0, 0, BOTTOM_WIDTH, BOTTOM_HEIGHT);
        gputOrtho(0, BOTTOM_WIDTH, 0, BOTTOM_HEIGHT, -1, 1);
        gpuClear();

        // Draw the game scene.
        gpuBindTexture(TEXUNIT0, scene->GetTexture());
        gpuDrawVbo(sceneVbo);

        // Draw GUI.
        gpuBindTexture(TEXUNIT0, guiTexture);
        gpuDrawVbo(dashboardVbo);

        // Draw particle selector.
        int selectIndex = 0;
        for(std::vector<ParticleType *>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
            ParticleType *type = *i;
            if(type->IsSelectable()) {
                if(selectedType == type) {
                    int selectX = 35 + ((selectIndex % 10) * 25);
                    int selectY = ((2 - (selectIndex / 10)) * 13) + 1;

                    gputPushModelView();
                    gputTranslate(selectX, selectY, 0);
                    gpuDrawVbo(selectorVbo);
                    gputPopModelView();

                    break;
                }

                selectIndex++;
            }
        }

        // Draw emitter selectors.
        for(int emitter = 0; emitter < emitters; emitter++) {
            int emitterX = 4;
            int emitterY = ((2 - (emitter % 2)) * 13) + 1;
            if(emitter >= emitters / 2) {
                emitterX = 320 - emitterX - 24;
            }

            if(emit[emitter]) {
                gputPushModelView();
                gputTranslate(emitterX, emitterY, 0);
                gpuDrawVbo(selectorVbo);
                gputPopModelView();
            }
        }

        // Finish drawing.
        gpuFlush();
        gpuFlushBuffer();

        // Prepare to draw on-screen info.
        gpuViewport(TOP_SCREEN, 0, 0, TOP_WIDTH, TOP_HEIGHT);
        gputOrtho(0, TOP_WIDTH, 0, TOP_HEIGHT, -1, 1);
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
        gputDrawString(stream.str(), 0, gpuGetViewportHeight() - 1 - gputGetStringHeight(stream.str(), 8), 8, 8);

        // Finish drawing.
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
