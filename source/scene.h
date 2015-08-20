#ifndef __SCENE_H__
#define __SCENE_H__

#include "particle.h"

#include <algorithm>

#include <citrus/types.hpp>

class ParticleType;

class Scene {
public:
    Scene(int width, int height);

    ~Scene();

    int GetWidth() {
        return this->width;
    }

    int GetHeight() {
        return this->height;
    }

    int GetParticleCount() {
        return this->particleCount;
    }

    u32 GetTexture() {
        return this->texture;
    }

    ParticleType* GetParticle(int x, int y);
    u32 GetData(int x, int y);
    void SetParticle(int x, int y, ParticleType* type);
    void SetParticle(int x, int y, ParticleType* type, u32 data);
    void SetMoved(int x, int y, bool moved);
    void Clear();

    void CreateParticles(int xpos, int ypos, int radius, ParticleType* type);
    void CreateLine(int newx, int newy, int oldx, int oldy, int size, ParticleType* type);
    void Emit(int x, int width, ParticleType* type, float density);

    void Update();
private:
    int width;
    int height;
    int particleCount;
    ParticleType** particles;
    u32* data;
    u32 texture;
    u32* texturePixels;

    bool HasMoved(int x, int y);
    void UpdateParticle(int x, int y);
};

#endif