#include "particle.h"

#include <string.h>

#include <iomanip>
#include <sstream>

#include <citrus/core.hpp>
#include <citrus/gpu.hpp>
#include <citrus/gput.hpp>
#include <citrus/hid.hpp>

#include "gui_bin.h"

using namespace ctr;

#define VERSION "1.3.3"

int main(int argc, char **argv) {
    if(!core::init(argc)) {
        return 0;
    }

    srand((unsigned int) core::time());

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
    if(core::launcher()) {
        controlStream << "Press Start to exit." << "\n";
    }

    controlStream << "Press Select to take a screenshot." << "\n";
    controlStream << "Press B to clear the screen." << "\n";
    controlStream << "Press Up/Down to modify the pen size." << "\n";
    controlStream << "Press Left/Right to modify the emitter density." << "\n";
    const std::string controls = controlStream.str();

    u32 guiTexture = 0;
    void* gpuGuiTexture = gpu::galloc(gui_bin_size);
    memcpy(gpuGuiTexture, gui_bin, gui_bin_size);
    gpu::createTexture(&guiTexture);
    gpu::setTextureData(guiTexture, gpuGuiTexture, 512, 64, gpu::PIXEL_RGBA8, TEXTURE_MIN_FILTER(gpu::FILTER_NEAREST) | TEXTURE_MAG_FILTER(gpu::FILTER_NEAREST));
    gpu::gfree(gpuGuiTexture);

    u32 dashboardVbo;
    gpu::createVbo(&dashboardVbo);
    gpu::setVboAttributes(dashboardVbo, ATTRIBUTE(0, 3, gpu::ATTR_FLOAT) | ATTRIBUTE(1, 2, gpu::ATTR_FLOAT) | ATTRIBUTE(2, 4, gpu::ATTR_FLOAT), 3);

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

    gpu::setVboData(dashboardVbo, dashboardVboData, 6 * 9, gpu::PRIM_TRIANGLES);

    u32 selectorVbo;
    gpu::createVbo(&selectorVbo);
    gpu::setVboAttributes(selectorVbo, ATTRIBUTE(0, 3, gpu::ATTR_FLOAT) | ATTRIBUTE(1, 2, gpu::ATTR_FLOAT) | ATTRIBUTE(2, 4, gpu::ATTR_FLOAT), 3);

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

    gpu::setVboData(selectorVbo, selectorVboData, 6 * 9, gpu::PRIM_TRIANGLES);

    u32 sceneVbo = 0;
    gpu::createVbo(&sceneVbo);
    gpu::setVboAttributes(sceneVbo, ATTRIBUTE(0, 3, gpu::ATTR_FLOAT) | ATTRIBUTE(1, 2, gpu::ATTR_FLOAT) | ATTRIBUTE(2, 4, gpu::ATTR_FLOAT), 3);

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

    gpu::setVboData(sceneVbo, vboData, 6 * 9, gpu::PRIM_TRIANGLES);

    int fpsCounter = 0;
    int fps = 0;
    u64 lastfps = core::time() - 1000;
    while(core::running()) {
        hid::poll();
        if(core::launcher() && hid::pressed(hid::BUTTON_START)) {
            // Exit by breaking out of the main loop.
            break;
        }

        // Check for scene clear.
        if(hid::pressed(hid::BUTTON_B)) {
            scene->Clear();
        }

        // Check for pen size increase.
        if(hid::pressed(hid::BUTTON_UP)) {
            penSize++;
            if(penSize > 40) {
                penSize = 40;
            }
        }

        // Check for pen size decrease.
        if(hid::pressed(hid::BUTTON_DOWN)) {
            penSize--;
            if(penSize < 1) {
                penSize = 1;
            }
        }

        // Check for emitter density increase.
        if(hid::pressed(hid::BUTTON_LEFT)) {
            emitDensity -= 0.05f;
            if(emitDensity < 0.05f) {
                emitDensity = 0.05f;
            }
        }

        // Check for emitter density decrease.
        if(hid::pressed(hid::BUTTON_RIGHT)) {
            emitDensity += 0.05f;
            if(emitDensity > 1) {
                emitDensity = 1;
            }
        }

        // If a touch has just been started, reset the old x and y variables to its position and do GUI interaction.
        if(hid::pressed(hid::BUTTON_TOUCH)) {
            hid::Touch touch = hid::touch();
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
        if(hid::held(hid::BUTTON_TOUCH)) {
            hid::Touch touch = hid::touch();
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
        gpu::setViewport(gpu::SCREEN_BOTTOM, 0, 0, BOTTOM_WIDTH, BOTTOM_HEIGHT);
        gput::setOrtho(0, BOTTOM_WIDTH, 0, BOTTOM_HEIGHT, -1, 1);
        gpu::clear();

        // Draw the game scene.
        gpu::bindTexture(gpu::TEXUNIT0, scene->GetTexture());
        gpu::drawVbo(sceneVbo);

        // Draw GUI.
        gpu::bindTexture(gpu::TEXUNIT0, guiTexture);
        gpu::drawVbo(dashboardVbo);

        // Draw particle selector.
        int selectIndex = 0;
        for(std::vector<ParticleType *>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
            ParticleType *type = *i;
            if(type->IsSelectable()) {
                if(selectedType == type) {
                    int selectX = 35 + ((selectIndex % 10) * 25);
                    int selectY = ((2 - (selectIndex / 10)) * 13) + 1;

                    gput::pushModelView();
                    gput::translate(selectX, selectY, 0);
                    gpu::drawVbo(selectorVbo);
                    gput::popModelView();

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
                gput::pushModelView();
                gput::translate(emitterX, emitterY, 0);
                gpu::drawVbo(selectorVbo);
                gput::popModelView();
            }
        }

        // Finish drawing.
        gpu::flushCommands();
        gpu::flushBuffer();

        // Prepare to draw on-screen info.
        gpu::setViewport(gpu::SCREEN_TOP, 0, 0, TOP_WIDTH, TOP_HEIGHT);
        gput::setOrtho(0, TOP_WIDTH, 0, TOP_HEIGHT, -1, 1);
        gpu::clear();

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
        gput::drawString(stream.str(), 0, TOP_HEIGHT - 1 - gput::getStringHeight(stream.str(), 8), 8, 8);

        // Finish drawing.
        gpu::flushCommands();
        gpu::flushBuffer();

        if(hid::pressed(hid::BUTTON_SELECT)) {
            // Take a screenshot.
            gput::takeScreenshot();
        }

        // Swap buffers, putting the newly drawn frame on-screen.
        gpu::swapBuffers(true);

        // Perform FPS counting logic.
        fpsCounter++;
        u64 since = core::time() - lastfps;
        if(since >= 1000) {
            fps = fpsCounter;
            fpsCounter = 0;
            lastfps = core::time();
        }
    }

    // Delete the scene and clean up.
    delete(scene);

    core::exit();
    return 0;
}
