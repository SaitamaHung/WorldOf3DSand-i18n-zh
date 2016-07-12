#include "particle.h"

#include <iomanip>
#include <sstream>

#include "default_shbin.h"
#include "gui_bin.h"

static C3D_Tex* glyphSheets;

static void drawQuad(float x1, float y1, float x2, float y2, float tx1, float ty1, float tx2, float ty2) {
    C3D_ImmDrawBegin(GPU_TRIANGLES);

    C3D_ImmSendAttrib(x1, y1, 0.5f, 0.0f);
    C3D_ImmSendAttrib(tx1, ty1, 0.0f, 0.0f);

    C3D_ImmSendAttrib(x2, y2, 0.5f, 0.0f);
    C3D_ImmSendAttrib(tx2, ty2, 0.0f, 0.0f);

    C3D_ImmSendAttrib(x2, y1, 0.5f, 0.0f);
    C3D_ImmSendAttrib(tx2, ty1, 0.0f, 0.0f);

    C3D_ImmSendAttrib(x1, y1, 0.5f, 0.0f);
    C3D_ImmSendAttrib(tx1, ty1, 0.0f, 0.0f);

    C3D_ImmSendAttrib(x1, y2, 0.5f, 0.0f);
    C3D_ImmSendAttrib(tx1, ty2, 0.0f, 0.0f);

    C3D_ImmSendAttrib(x2, y2, 0.5f, 0.0f);
    C3D_ImmSendAttrib(tx2, ty2, 0.0f, 0.0f);

    C3D_ImmDrawEnd();
}

static void drawTexture(C3D_Tex* tex, float x, float y, float width, float height, float tx, float ty, float twidth, float theight) {
    float tx1 = tx / tex->width;
    float ty1 = ty / tex->height;
    float tx2 = (tx + twidth) / tex->width;
    float ty2 = (ty + theight) / tex->height;

    C3D_TexBind(0, tex);

    drawQuad(x, y, x + width, y + height, tx1, ty1, tx2, ty2);
}

static void drawString(const char* text, float x, float y, float scaleX, float scaleY) {
    if(glyphSheets == NULL || text == NULL) {
        return;
    }

    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_RGB, GPU_CONSTANT, 0, 0);
    C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, GPU_CONSTANT, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
    C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);
    C3D_TexEnvColor(env, 0xFFFFFFFF);

    float currX = x;
    int lastSheet = -1;

    const uint8_t* p = (const uint8_t*) text;
    u32 code = 0;
    ssize_t units = -1;
    while(*p && (units = decode_utf8(&code, p)) != -1 && code > 0) {
        p += units;

        if(code == '\n') {
            currX = x;
            y += scaleY * fontGetInfo()->lineFeed;
        } else {
            fontGlyphPos_s data;
            fontCalcGlyphPos(&data, fontGlyphIndexFromCodePoint(code), GLYPH_POS_CALC_VTXCOORD, scaleX, scaleY);

            if(data.sheetIndex != lastSheet) {
                lastSheet = data.sheetIndex;
                C3D_TexBind(0, &glyphSheets[lastSheet]);
            }

            drawQuad(currX + data.vtxcoord.left, y + data.vtxcoord.top, currX + data.vtxcoord.right, y + data.vtxcoord.bottom, data.texcoord.left, data.texcoord.top, data.texcoord.right, data.texcoord.bottom);

            currX += data.xAdvance;
        }
    }

    env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
}

int main(int argc, char **argv) {
    gfxInitDefault();

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE * 4);

    C3D_RenderTarget* bottomTarget = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetClear(bottomTarget, C3D_CLEAR_ALL, 0, 0);
    C3D_RenderTargetSetOutput(bottomTarget, GFX_BOTTOM, GFX_LEFT, GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));

    C3D_RenderTarget* topTarget = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetClear(topTarget, C3D_CLEAR_ALL, 0, 0);
    C3D_RenderTargetSetOutput(topTarget, GFX_TOP, GFX_LEFT, GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));

    DVLB_s* dvlb = DVLB_ParseFile((u32*) default_shbin, default_shbin_len);

    shaderProgram_s program;
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &dvlb->DVLE[0]);
    C3D_BindProgram(&program);

    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3);
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2);

    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);

    C3D_Mtx bottomProjection;
    Mtx_OrthoTilt(&bottomProjection, 0.0, 320.0, 240.0, 0.0, 0.0, 1.0);

    C3D_Mtx topProjection;
    Mtx_OrthoTilt(&topProjection, 0.0, 400.0, 240.0, 0.0, 0.0, 1.0);

    if(R_SUCCEEDED(fontEnsureMapped())) {
        TGLP_s* glyphInfo = fontGetGlyphInfo();
        glyphSheets = (C3D_Tex*) calloc(glyphInfo->nSheets, sizeof(C3D_Tex));
        if(glyphSheets != NULL) {
            for(int i = 0; i < glyphInfo->nSheets; i++) {
                C3D_Tex* tex = &glyphSheets[i];
                tex->data = fontGetGlyphSheetTex(i);
                tex->fmt = (GPU_TEXCOLOR) glyphInfo->sheetFmt;
                tex->size = glyphInfo->sheetSize;
                tex->width = glyphInfo->sheetWidth;
                tex->height = glyphInfo->sheetHeight;
                tex->param = GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_LINEAR) | GPU_TEXTURE_WRAP_S(GPU_CLAMP_TO_EDGE) | GPU_TEXTURE_WRAP_T(GPU_CLAMP_TO_EDGE);
            }
        }
    }

    srand((unsigned int) time(NULL));

    ParticleType::InitParticles();

    Scene *scene = new Scene(320, 200);
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
    controlStream << "Press Start to exit." << "\n";
    controlStream << "Press B to clear the screen." << "\n";
    controlStream << "Press Up/Down to modify the pen size." << "\n";
    controlStream << "Press Left/Right to modify the emitter density." << "\n";
    const std::string controls = controlStream.str();

    void* gpuGuiTexture = linearAlloc(gui_bin_len);
    memcpy(gpuGuiTexture, gui_bin, gui_bin_len);

    C3D_Tex guiTexture;
    memset(&guiTexture, 0, sizeof(guiTexture));
    C3D_TexInit(&guiTexture, 512, 64, GPU_RGBA8);
    C3D_TexSetFilter(&guiTexture, GPU_NEAREST, GPU_NEAREST);
    C3D_SafeDisplayTransfer((u32*) gpuGuiTexture, GX_BUFFER_DIM(512, 64), (u32*) guiTexture.data, GX_BUFFER_DIM(512, 64), GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GPU_RGBA8) | GX_TRANSFER_OUT_FORMAT(GPU_RGBA8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));
    gspWaitForPPF();

    linearFree(gpuGuiTexture);

    int fpsCounter = 0;
    int fps = 0;
    time_t lastfps = time(NULL) - 1;
    while(aptMainLoop()) {
        hidScanInput();

        if(hidKeysDown() & KEY_START) {
            // Exit by breaking out of the main loop.
            break;
        }

        // Check for scene clear.
        if(hidKeysDown() & KEY_B) {
            scene->Clear();
        }

        // Check for pen size increase.
        if(hidKeysDown() & KEY_UP) {
            penSize++;
            if(penSize > 40) {
                penSize = 40;
            }
        }

        // Check for pen size decrease.
        if(hidKeysDown() & KEY_DOWN) {
            penSize--;
            if(penSize < 1) {
                penSize = 1;
            }
        }

        // Check for emitter density increase.
        if(hidKeysDown() & KEY_LEFT) {
            emitDensity -= 0.05f;
            if(emitDensity < 0.05f) {
                emitDensity = 0.05f;
            }
        }

        // Check for emitter density decrease.
        if(hidKeysDown() & KEY_RIGHT) {
            emitDensity += 0.05f;
            if(emitDensity > 1) {
                emitDensity = 1;
            }
        }

        // If a touch has just been started, reset the old x and y variables to its position and do GUI interaction.
        if(hidKeysDown() & KEY_TOUCH) {
            touchPosition touch;
            hidTouchRead(&touch);

            oldx = touch.px;
            oldy = touch.py;

            // Check for GUI interaction.
            int dashboardTouchY = 239 - touch.py;
            if(dashboardTouchY > 0 && dashboardTouchY < 40) {
                if(touch.px > 35 && touch.px < 285 && dashboardTouchY > 0 && dashboardTouchY < 40) {
                    int index = 0;
                    for(std::vector<ParticleType *>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
                        ParticleType *type = *i;
                        if(type->IsSelectable()) {
                            int x = 35 + ((index % 10) * 25);
                            int y = ((2 - (index / 10)) * 13) + 1;
                            if(touch.px > x && dashboardTouchY > y && touch.px <= x + 24 && dashboardTouchY <= y + 12) {
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

                        if(touch.px > x && dashboardTouchY > y && touch.px <= x + 24 && dashboardTouchY <= y + 12) {
                            emit[emitter] = !emit[emitter];
                            break;
                        }
                    }
                }
            }
        }

        // Draw a line from touch input.
        if(hidKeysHeld() & KEY_TOUCH) {
            touchPosition touch;
            hidTouchRead(&touch);

            if(touch.px < scene->GetWidth() && touch.py < scene->GetHeight()) {
                scene->CreateLine(touch.px, touch.py, oldx, oldy, penSize, selectedType);
            }

            oldx = touch.px;
            oldy = touch.py;
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
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        // Draw bottom screen.
        C3D_FrameDrawOn(bottomTarget);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(program.vertexShader, "projection"), &bottomProjection);

        // Draw the game scene.
        C3D_TexFlush(scene->GetTexture());
        drawTexture(scene->GetTexture(), 0, 0, scene->GetWidth(), scene->GetHeight(), 0, 0, scene->GetWidth(), scene->GetHeight());

        // Draw GUI.
        drawTexture(&guiTexture, 0, 200, 320, 40, 0, 0, 320, 40);

        // Draw particle selector.
        int selectIndex = 0;
        for(std::vector<ParticleType *>::iterator i = ParticleType::TYPES->begin(); i != ParticleType::TYPES->end(); i++) {
            ParticleType *type = *i;
            if(type->IsSelectable()) {
                if(selectedType == type) {
                    int selectX = 35 + ((selectIndex % 10) * 25);
                    int selectY = 200 + ((selectIndex / 10) * 13) + 1;

                    drawTexture(&guiTexture, selectX, selectY, 24, 12, 0, 40, 24, 12);

                    break;
                }

                selectIndex++;
            }
        }

        // Draw emitter selectors.
        for(int emitter = 0; emitter < emitters; emitter++) {
            int emitterX = 4;
            int emitterY = 200 + ((emitter % 2) * 13) + 1;
            if(emitter >= emitters / 2) {
                emitterX = 320 - emitterX - 24;
            }

            if(emit[emitter]) {
                drawTexture(&guiTexture, emitterX, emitterY, 24, 12, 0, 40, 24, 12);
            }
        }

        // Draw top screen.
        C3D_FrameDrawOn(topTarget);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(program.vertexShader, "projection"), &topProjection);

        // Draw on-screen info.
        std::stringstream stream;
        stream << "World of 3DSand " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_MICRO << "\n";
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
        drawString(stream.str().c_str(), 0, 0, 0.5f, 0.5f);

        // Finish drawing.
        C3D_FrameEnd(0);

        // Perform FPS counting logic.
        fpsCounter++;
        if(time(NULL) - lastfps >= 1) {
            fps = fpsCounter;
            fpsCounter = 0;
            lastfps = time(NULL);
        }
    }

    // Delete the scene and clean up.
    delete scene;

    C3D_TexDelete(&guiTexture);

    if(glyphSheets != NULL) {
        free(glyphSheets);
        glyphSheets = NULL;
    }

    shaderProgramFree(&program);
    DVLB_Free(dvlb);
    C3D_RenderTargetDelete(bottomTarget);
    C3D_RenderTargetDelete(topTarget);
    C3D_Fini();

    gfxExit();
    return 0;
}
