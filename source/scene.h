#ifndef __SCENE_H__
#define __SCENE_H__

#include "particle.h"

class ParticleType;

class Scene {
public:
    Scene(int width, int height) {
        this->width = width;
        this->height = height;
        this->particleCount = 0;
        this->particles = new ParticleType*[this->width * this->height];
        this->moved = new bool[this->width * this->height];
        std::fill(this->moved, this->moved + (this->width * this->height), false);
    }

    ~Scene() {
        delete(particles);
        delete(moved);
    }

    int GetWidth() {
        return this->width;
    }

    int GetHeight() {
        return this->height;
    }

    int GetParticleCount() {
        return this->particleCount;
    }

    ParticleType* GetParticle(int x, int y);
    void SetParticle(int x, int y, ParticleType* type);
    void SetMoved(int x, int y, bool moved);
    void Clear();

    void CreateParticles(int xpos, int ypos, int radius, ParticleType* type);
    void CreateLine(int newx, int newy, int oldx, int oldy, int size, ParticleType* type);
    void Emit(int x, int width, ParticleType* type, float density);

    void Update();
    void Draw();
private:
    int width;
    int height;
    int particleCount;
    ParticleType** particles;
    bool* moved;

    bool HasMoved(int x, int y);
    void UpdateParticle(int x, int y);
};

#endif