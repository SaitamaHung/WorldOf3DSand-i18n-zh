#include "scene.h"

ParticleType *Scene::GetParticle(int x, int y) {
    if(x < 0 || y < 0 || x >= this->width || y >= this->height) {
        return ParticleType::NOTHING;
    }

    ParticleType *type = this->particles[x + (this->width * y)];
    if(type == NULL) {
        return ParticleType::NOTHING;
    }

    return type;
}

u32 Scene::GetData(int x, int y) {
    if(x < 0 || y < 0 || x >= this->width || y >= this->height) {
        return 0;
    }

    return this->data[x + (this->width * y)];
}

void Scene::SetParticle(int x, int y, ParticleType *type) {
    this->SetParticle(x, y, type, 0);
}

void Scene::SetParticle(int x, int y, ParticleType *type, u32 data) {
    if(x < 0 || y < 0 || x >= this->width || y >= this->height) {
        return;
    }

    ParticleType* previousType = this->GetParticle(x, y);
    int index = x + (this->width * y);
    this->particles[index] = type;
    this->data[index] = data;
    if(type != previousType) {
        if(type->IsDrawn()) {
            this->texturePixels[gpuTextureIndex((u32) x, (u32) y, 512, 512)] = (u32) ((type->GetRed() << 24) | (type->GetGreen() << 16) | (type->GetBlue() << 8) | 0xFF);
            if((!previousType->IsDrawn() || previousType->IsStill()) && !type->IsStill()) {
                this->particleCount++;
            }
        } else {
            this->texturePixels[gpuTextureIndex((u32) x, (u32) y, 512, 512)] = 0;
            if(previousType->IsDrawn() && !previousType->IsStill()) {
                this->particleCount--;
            }
        }
    }
}

void Scene::SetMoved(int x, int y, bool moved) {
    if(x < 0 || y < 0 || x >= this->width || y >= this->height) {
        return;
    }

    ParticleType *type = this->GetParticle(x, y);
    if(type->HasData()) {
        return;
    }

    this->SetParticle(x, y, type, (u32) moved);
}

void Scene::Clear() {
    for(int x = 0; x < this->width; x++) {
        for(int y = 0; y < this->height; y++) {
            SetParticle(x, y, ParticleType::NOTHING);
            SetMoved(x, y, false);
        }
    }
}

void Scene::CreateParticles(int xpos, int ypos, int radius, ParticleType *type) {
    for(int x = ((xpos - radius - 1) < 0) ? 0 : (xpos - radius - 1); x <= xpos + radius && x < this->width; x++) {
        for(int y = ((ypos - radius - 1) < 0) ? 0 : (ypos - radius - 1); y <= ypos + radius && y < this->height; y++) {
            if((x - xpos) * (x - xpos) + (y - ypos) * (y - ypos) <= radius * radius) {
                SetParticle(x, y, type);
            }
        }
    }
}

void Scene::CreateLine(int newx, int newy, int oldx, int oldy, int size, ParticleType *type) {
    if(newx == oldx && newy == oldy) {
        CreateParticles(newx, newy, size, type);
    } else {
        float step = 1.0f / ((abs(newx - oldx) > abs(newy - oldy)) ? abs(newx - oldx) : abs(newy - oldy));
        for(float a = 0; a < 1; a += step) {
            CreateParticles((int) (a * newx + (1 - a) * oldx), (int) (a * newy + (1 - a) * oldy), size, type);
        }
    }
}

void Scene::Emit(int x, int width, ParticleType *type, float density) {
    for(int i = x - width / 2; i < x + width / 2; i++) {
        if(rand() < (int) (RAND_MAX * density)) {
            SetParticle(i, 1, type);
        }
    }
}

void Scene::Update() {
    for(int y = 0; y < this->height; y++) {
        // Choose direction randomly per line to mix things up a little.
        if(rand() % 2 == 0) {
            for(int x = this->width - 1; x >= 0; x--) {
                UpdateParticle(x, y);
            }
        } else {
            for(int x = 0; x < this->width; x++) {
                UpdateParticle(x, y);
            }
        }
    }

    for(int i = 0; i < this->width * this->height; i++) {
        if(this->particles[i] != NULL && !this->particles[i]->HasData()) {
            this->data[i] = 0;
        }
    }
}

bool Scene::HasMoved(int x, int y) {
    if(x < 0 || y < 0 || x >= this->width || y >= this->height) {
        return false;
    }

    return !this->GetParticle(x, y)->HasData() && this->GetData(x, y) == 1;
}

void Scene::UpdateParticle(int x, int y) {
    ParticleType *type = GetParticle(x, y);
    if(type != ParticleType::NOTHING) {
        if(type->IsStill()) {
            if(type->GetPhysics() != NULL) {
                type->GetPhysics()(this, x, y);
            }
        } else if(!HasMoved(x, y)) {
            // Apply random condition to make particles move unevenly.
            if(rand() >= RAND_MAX / 13) {
                ParticleType *above = GetParticle(x, y - 1);
                ParticleType *below = GetParticle(x, y + 1);
                // Apply gravity to non-floating particles.
                if(!type->IsFloating()) {
                    if(below == ParticleType::NOTHING && rand() % 8) {
                        SetParticle(x, y + 1, type);
                        SetMoved(x, y + 1, true);
                        SetParticle(x, y, ParticleType::NOTHING);
                        return;
                    }
                } else {
                    // Make floating particles rise.
                    if(rand() % 3 != 0 && (above == ParticleType::NOTHING || above == ParticleType::FIRE) && (rand() % 8) && type != ParticleType::ELECTRICITY) {
                        if(type != ParticleType::FIRE || (rand() % 20) != 0) {
                            SetParticle(x, y - 1, type);
                        }

                        SetParticle(x, y, ParticleType::NOTHING);
                        return;
                    }
                }

                if(type->GetPhysics() != NULL) {
                    type->GetPhysics()(this, x, y);
                }

                // Make non-floating particles spread due to gravity.
                if(!type->IsFloating()) {
                    int sign = rand() % 2 == 0 ? -1 : 1;
                    if(GetParticle(x - sign, y + 1) == ParticleType::NOTHING) {
                        SetParticle(x - sign, y + 1, type);
                        SetMoved(x - sign, y + 1, true);
                        SetParticle(x, y, ParticleType::NOTHING);
                    } else if(GetParticle(x + sign, y + 1) == ParticleType::NOTHING) {
                        SetParticle(x + sign, y + 1, type);
                        SetMoved(x + sign, y + 1, true);
                        SetParticle(x, y, ParticleType::NOTHING);
                    } else if(GetParticle(x - sign, y) == ParticleType::NOTHING) {
                        SetParticle(x - sign, y, type);
                        SetMoved(x - sign, y, true);
                        SetParticle(x, y, ParticleType::NOTHING);
                    } else if(GetParticle(x + sign, y) == ParticleType::NOTHING) {
                        SetParticle(x + sign, y, type);
                        SetMoved(x + sign, y, true);
                        SetParticle(x, y, ParticleType::NOTHING);
                    }
                }
            }
        }
    }
}