#include <stdlib.h>

#include "particle.h"

// Still
ParticleType* ParticleType::NOTHING = new ParticleType("Erase", {0, 0, 0}, STILL | SELECTABLE | NO_DRAW);
ParticleType* ParticleType::WALL = new ParticleType("Wall", {100, 100, 100}, STILL | SELECTABLE);
ParticleType* ParticleType::IRONWALL = new ParticleType("Iron Wall", {140, 140, 140}, STILL | SELECTABLE);
ParticleType* ParticleType::TORCH = new ParticleType("Torch", {139, 69, 19}, STILL | SELECTABLE);
ParticleType* ParticleType::STOVE = new ParticleType("Stove", {74, 74, 74}, STILL | SELECTABLE);
ParticleType* ParticleType::ICE = new ParticleType("Ice", {175, 238, 238}, STILL | SELECTABLE);
ParticleType* ParticleType::RUST = new ParticleType("Rust", {110, 40, 10}, STILL);
ParticleType* ParticleType::EMBER = new ParticleType("Ember", {127, 25, 25}, STILL);
ParticleType* ParticleType::PLANT = new ParticleType("Plant", {0, 150, 0}, STILL | BURNABLE | BURNS_TO_EMBER | SELECTABLE);
ParticleType* ParticleType::VOID = new ParticleType("Void", {60, 60, 60}, STILL | SELECTABLE);

// Spouts
ParticleType* ParticleType::WATERSPOUT = new ParticleType("Water Spout", {0, 0, 128}, STILL | SELECTABLE);
ParticleType* ParticleType::SANDSPOUT = new ParticleType("Sand Spout", {240, 230, 140}, STILL | SELECTABLE);
ParticleType* ParticleType::SALTSPOUT = new ParticleType("Salt Spout", {238, 233, 233}, STILL | SELECTABLE);
ParticleType* ParticleType::OILSPOUT = new ParticleType("Oil Spout", {108, 44, 44}, STILL | SELECTABLE);

// Elemental
ParticleType* ParticleType::WATER = new ParticleType("Water", {32, 32, 255}, SELECTABLE);
ParticleType* ParticleType::DIRT = new ParticleType("Dirt", {205, 175, 149}, SELECTABLE);
ParticleType* ParticleType::SALT = new ParticleType("Salt", {255, 255, 255}, SELECTABLE);
ParticleType* ParticleType::OIL = new ParticleType("Oil", {128, 64, 64}, BURNABLE | SELECTABLE);
ParticleType* ParticleType::SAND = new ParticleType("Sand", {238, 204, 128}, SELECTABLE);
ParticleType* ParticleType::ASH = new ParticleType("Ash", {200, 200, 200});

// Combined
ParticleType* ParticleType::SALTWATER = new ParticleType("Salt Water", {65, 105, 225});
ParticleType* ParticleType::MUD = new ParticleType("Mud", {139, 69, 19});
ParticleType* ParticleType::ACID = new ParticleType("Acid", {173, 255, 47}, SELECTABLE);

// Floating
ParticleType* ParticleType::STEAM = new ParticleType("Steam", {95, 158, 160}, FLOATING);
ParticleType* ParticleType::FIRE = new ParticleType("Fire", {255, 50, 50}, FLOATING | SELECTABLE);

// Electricity
ParticleType* ParticleType::ELECTRICITY = new ParticleType("Electricity", {255, 255, 0}, SELECTABLE);

// Explosive
ParticleType* ParticleType::EXPLOSION = new ParticleType("Explosion", {150, 150, 150}, STILL | DATA);
ParticleType* ParticleType::GUNPOWDER = new ParticleType("Gunpowder", {50, 50, 50}, SELECTABLE);

std::vector<ParticleType *>* ParticleType::TYPES = new std::vector<ParticleType*>();

void move_in_random_dir(int* x, int* y) {
    switch(rand() % 4) {
        case 0:
            *x -= 1;
            break;
        case 1:
            *y -= 1;
            break;
        case 2:
            *x += 1;
            break;
        case 3:
            *y += 1;
            break;
        default:
            break;
    }
}

void ParticleType::InitParticles() {
    TYPES->push_back(NOTHING);
    TYPES->push_back(WALL);
    TYPES->push_back(IRONWALL);
    TYPES->push_back(TORCH);
    TYPES->push_back(STOVE);
    TYPES->push_back(ICE);
    TYPES->push_back(RUST);
    TYPES->push_back(EMBER);
    TYPES->push_back(PLANT);
    TYPES->push_back(VOID);
    TYPES->push_back(WATERSPOUT);
    TYPES->push_back(SANDSPOUT);
    TYPES->push_back(SALTSPOUT);
    TYPES->push_back(OILSPOUT);
    TYPES->push_back(WATER);
    TYPES->push_back(DIRT);
    TYPES->push_back(SALT);
    TYPES->push_back(OIL);
    TYPES->push_back(SAND);
    TYPES->push_back(ASH);
    TYPES->push_back(SALTWATER);
    TYPES->push_back(MUD);
    TYPES->push_back(ACID);
    TYPES->push_back(STEAM);
    TYPES->push_back(FIRE);
    TYPES->push_back(ELECTRICITY);
    TYPES->push_back(EXPLOSION);
    TYPES->push_back(GUNPOWDER);

    IRONWALL->SetPhysics([] (Scene* scene, int x, int y) {
        // Rust when in contact with other rust particles.
        if(rand() % 200 == 0 && (scene->GetParticle(x, y - 1) == RUST || scene->GetParticle(x - 1, y) == RUST || scene->GetParticle(x + 1, y) == RUST)) {
            scene->SetParticle(x, y, RUST);
        }
    });

    TORCH->SetPhysics([] (Scene* scene, int x, int y) {
        // Spread fire.
        if(rand() % 2 == 0) {
            if(scene->GetParticle(x, y - 1) == NOTHING) {
                scene->SetParticle(x, y - 1, FIRE);
            }

            if(scene->GetParticle(x - 1, y) == NOTHING) {
                scene->SetParticle(x - 1, y, FIRE);
            }

            if(scene->GetParticle(x + 1, y) == NOTHING) {
                scene->SetParticle(x + 1, y, FIRE);
            }
        }

        // Evaporate water.
        if(scene->GetParticle(x, y - 1) == WATER) {
            scene->SetParticle(x, y - 1, STEAM);
        }

        if(scene->GetParticle(x - 1, y) == WATER) {
            scene->SetParticle(x - 1, y, STEAM);
        }

        if(scene->GetParticle(x + 1, y) == WATER) {
            scene->SetParticle(x + 1, y, STEAM);
        }
    });

    STOVE->SetPhysics([] (Scene* scene, int x, int y) {
        ParticleType* above = scene->GetParticle(x, y - 1);
        // Boil water.
        if(above == WATER && rand() % 4 == 0) {
            scene->SetParticle(x, y - 1, STEAM);
        } else if(above == SALTWATER && rand() % 4 == 0) {
            // Separate saltwater.
            scene->SetParticle(x, y - 1, SALT);
            scene->SetParticle(x, y - 2, STEAM);
        } else if(above == OIL && rand() % 8 == 0) {
            // Light oil on fire.
            scene->SetParticle(x, y - 1, EMBER);
        }
    });

    RUST->SetPhysics([] (Scene* scene, int x, int y) {
        // Deteriorate rust
        if(rand() % 7000 == 0) {
            scene->SetParticle(x, y, NOTHING);
        }
    });

    EMBER->SetPhysics([] (Scene* scene, int x, int y) {
        ParticleType* below = scene->GetParticle(x, y + 1);
        // Spread fire.
        if(below == NOTHING || below->IsBurnable()) {
            scene->SetParticle(x, y + 1, FIRE);
        }

        // Set plants on fire.
        int rx = x;
        int ry = y;
        move_in_random_dir(&rx, &ry);
        if(scene->GetParticle(rx, ry) == PLANT) {
            scene->SetParticle(rx, ry, FIRE);
        }

        // Burn out slowly.
        if(rand() % 18 == 0) {
            scene->SetParticle(x, y, NOTHING);
        }
    });

    PLANT->SetPhysics([] (Scene* scene, int x, int y) {
        // Grow where there is water to feed it.
        if(rand() % 2 == 0) {
            int rx = x;
            int ry = y;
            move_in_random_dir(&rx, &ry);
            if(scene->GetParticle(rx, ry) == WATER) {
                scene->SetParticle(rx, ry, PLANT);
            }
        }
    });

    VOID->SetPhysics([] (Scene* scene, int x, int y) {
        // Delete adjacent particles.
        scene->SetParticle(x, y - 1, NOTHING);
        scene->SetParticle(x, y + 1, NOTHING);
        scene->SetParticle(x - 1, y, NOTHING);
        scene->SetParticle(x + 1, y, NOTHING);
    });

    WATERSPOUT->SetPhysics([] (Scene* scene, int x, int y) {
        // Emit particles.
        if(rand() % 6 == 0 && scene->GetParticle(x, y + 1) == NOTHING)  {
            scene->SetParticle(x, y + 1, WATER);
        }
    });

    SANDSPOUT->SetPhysics([] (Scene* scene, int x, int y) {
        // Emit particles.
        if(rand() % 6 == 0 && scene->GetParticle(x, y + 1) == NOTHING)  {
            scene->SetParticle(x, y + 1, SAND);
        }
    });

    SALTSPOUT->SetPhysics([] (Scene* scene, int x, int y) {
        // Emit salt and turn water into saltwater.
        if(rand() % 6 == 0)  {
            if(scene->GetParticle(x, y + 1) == NOTHING) {
                scene->SetParticle(x, y + 1, SALT);
            } else if(scene->GetParticle(x, y + 1) == WATER) {
                scene->SetParticle(x, y + 1, SALTWATER);
            }
        }
    });

    OILSPOUT->SetPhysics([] (Scene* scene, int x, int y) {
        // Emit particles.
        if(rand() % 6 == 0 && scene->GetParticle(x, y + 1) == NOTHING)  {
            scene->SetParticle(x, y + 1, OIL);
        }
    });

    WATER->SetPhysics([] (Scene* scene, int x, int y) {
        ParticleType* below = scene->GetParticle(x, y + 1);
        // Rust iron walls.
        if(rand() % 200 == 0 && below == IRONWALL) {
            scene->SetParticle(x, y + 1, RUST);
        }

        ParticleType* above = scene->GetParticle(x, y - 1);
        // Turn water into steam when exposed to fire.
        if(below == FIRE || above == FIRE || scene->GetParticle(x - 1, y) == FIRE || scene->GetParticle(x + 1, y) == FIRE) {
            scene->SetParticle(x, y, STEAM);
            scene->SetMoved(x, y, true);
        }

        //Making dirt and water into mud, and salt and water into saltwater.
        if(below == DIRT) {
            scene->SetParticle(x, y + 1, MUD);
            scene->SetMoved(x, y + 1, true);
            scene->SetParticle(x, y, NOTHING);
        } else if(below == SALT) {
            scene->SetParticle(x, y + 1, SALTWATER);
            scene->SetMoved(x, y + 1, true);
            scene->SetParticle(x, y, NOTHING);
        }

        if(above == DIRT) {
            scene->SetParticle(x, y - 1, MUD);
            scene->SetMoved(x, y - 1, true);
            scene->SetParticle(x, y, NOTHING);
        } else if(above == SALT) {
            scene->SetParticle(x, y - 1, SALTWATER);
            scene->SetMoved(x, y - 1, true);
            scene->SetParticle(x, y, NOTHING);
        }

        // Melt nearby ice.
        if(rand() % 60 == 0) {
            int rx = x;
            int ry = y;
            move_in_random_dir(&rx, &ry);
            if(scene->GetParticle(rx, ry) == ICE) {
                scene->SetParticle(rx, ry, WATER);
            }
        }

        // Apply density by moving water above more dense particles.
        if((above == SAND || above == MUD || above == SALTWATER) && rand() % 3 == 0) {
            scene->SetParticle(x, y, above);
            scene->SetParticle(x, y - 1, WATER);
            scene->SetMoved(x, y - 1, true);
            return;
        }
    });

    SALT->SetPhysics([] (Scene* scene, int x, int y) {
        // Melt ice.
        if(rand() % 20 == 0) {
            int rx = x;
            int ry = y;
            move_in_random_dir(&rx, &ry);
            if(scene->GetParticle(rx, ry) == ICE) {
                scene->SetParticle(rx, ry, WATER);
            }
        }
    });

    OIL->SetPhysics([] (Scene* scene, int x, int y) {
        // Light on fire when in contact with fire.
        int rx = x;
        int ry = y;
        move_in_random_dir(&rx, &ry);
        if(scene->GetParticle(rx, ry) == FIRE) {
            scene->SetParticle(x, y, FIRE);
        }

        // Apply density by moving oil above water.
        ParticleType* above = scene->GetParticle(x, y - 1);
        if(above == WATER && rand() % 3 == 0) {
            scene->SetParticle(x, y, above);
            scene->SetParticle(x, y - 1, OIL);
            scene->SetMoved(x, y - 1, true);
        }
    });

    SALTWATER->SetPhysics([] (Scene* scene, int x, int y) {
        // Melt ice, but do so more slowly than salt.
        if(rand()%40 == 0) {
            int rx = x;
            int ry = y;
            move_in_random_dir(&rx, &ry);
            if(scene->GetParticle(rx, ry) == ICE) {
                scene->SetParticle(rx, ry, WATER);
            }
        }

        // Apply density by moving saltwater above more dense particles.
        ParticleType* above = scene->GetParticle(x, y - 1);
        if((above == DIRT || above == MUD || above == SAND) && rand() % 3 == 0) {
            scene->SetParticle(x, y, above);
            scene->SetParticle(x, y - 1, SALTWATER);
            scene->SetMoved(x, y - 1, true);
        }
    });

    ACID->SetPhysics([] (Scene* scene, int x, int y) {
        // Burn through materials.
        int rx = x;
        int ry = y;
        move_in_random_dir(&rx, &ry);
        ParticleType* check = scene->GetParticle(rx, ry);
        if(check != WALL && check != IRONWALL && check != WATER && check != ACID) {
            scene->SetParticle(rx, ry, NOTHING);
        }
    });

    STEAM->SetPhysics([] (Scene* scene, int x, int y) {
        // Condense into water.
        if(rand() % 1000 == 0) {
            scene->SetParticle(x, y, WATER);
            scene->SetMoved(x, y, true);
            return;
        }

        // Evaporate into nothing.
        if(rand() % 500 == 0) {
            scene->SetParticle(x, y, NOTHING);
            return;
        }

        // Pass upwards through certain particles.
        ParticleType* above = scene->GetParticle(x, y - 1);
        if(!above->IsStill() && !above->IsFloating()) {
            if(rand() % 15 == 0) {
                scene->SetParticle(x, y, NOTHING);
            } else {
                scene->SetParticle(x, y, above);
                scene->SetParticle(x, y - 1, STEAM);
                scene->SetMoved(x, y - 1, true);
            }
        }

        // Pass through nothing.
        int sign = rand() % 2 == 0 ? -1 : 1;
        if (scene->GetParticle(x - sign, y - 1) == NOTHING) {
            scene->SetParticle(x - sign, y - 1, STEAM);
            scene->SetMoved(x - sign, y - 1, true);
            scene->SetParticle(x, y, NOTHING);
        } else if(scene->GetParticle(x + sign, y - 1) == NOTHING) {
            scene->SetParticle(x + sign, y - 1, STEAM);
            scene->SetMoved(x + sign, y - 1, true);
            scene->SetParticle(x, y, NOTHING);
        } else if(scene->GetParticle(x - sign, y) == NOTHING) {
            scene->SetParticle(x - sign, y, STEAM);
            scene->SetMoved(x - sign, y, true);
            scene->SetParticle(x, y, NOTHING);
        } else if(scene->GetParticle(x + sign, y) == NOTHING) {
            scene->SetParticle(x + sign, y, STEAM);
            scene->SetMoved(x + sign, y, true);
            scene->SetParticle(x, y, NOTHING);
        }
    });

    FIRE->SetPhysics([] (Scene* scene, int x, int y) {
        ParticleType* above = scene->GetParticle(x, y - 1);
        // Extinguish fire.
        if(!above->IsBurnable() && rand() % 10 == 0) {
            scene->SetParticle(x, y, NOTHING);
            return;
        }

        // Melt ice.
        if(rand() % 4 == 0) {
            if(above == ICE) {
                scene->SetParticle(x, y - 1, WATER);
                scene->SetParticle(x, y, NOTHING);
            }

            if(scene->GetParticle(x, y + 1) == ICE) {
                scene->SetParticle(x, y + 1, WATER);
                scene->SetParticle(x, y, NOTHING);
            }

            if(scene->GetParticle(x - 1, y) == ICE) {
                scene->SetParticle(x - 1, y, WATER);
                scene->SetParticle(x, y, NOTHING);
            }

            if(scene->GetParticle(x + 1, y) == ICE) {
                scene->SetParticle(x + 1, y, WATER);
                scene->SetParticle(x, y, NOTHING);
            }
        }

        // Burn a random particle.
        int rx = x;
        int ry = y;
        move_in_random_dir(&rx, &ry);
        ParticleType* check = scene->GetParticle(rx, ry);
        if(check->IsBurnable()) {
            scene->SetParticle(rx, ry, check->BurnsToEmber() ? EMBER : FIRE);
            // Leave behind ash.
            if(scene->GetParticle(rx, ry - 1) == ParticleType::NOTHING && rand() % 2 == 0) {
                scene->SetParticle(rx, ry - 1, ParticleType::ASH);
            }
        }
    });

    ELECTRICITY->SetPhysics([] (Scene* scene, int x, int y) {
        // Disappear.
        if(rand() % 2 == 0) {
            scene->SetParticle(x, y, NOTHING);
        }
    });

    GUNPOWDER->SetPhysics([] (Scene* scene, int x, int y) {
        // Explode in contact with fire.
        bool explode = false;
        if(scene->GetParticle(x - 1, y) == FIRE) {
            explode = true;
        } else if(scene->GetParticle(x + 1, y) == FIRE) {
            explode = true;
        } else if(scene->GetParticle(x, y - 1) == FIRE) {
            explode = true;
        } else if(scene->GetParticle(x, y + 1) == FIRE) {
            explode = true;
        }

        if(explode) {
            scene->SetParticle(x, y, ParticleType::EXPLOSION, 100);
        }
    });

    EXPLOSION->SetPhysics([] (Scene* scene, int x, int y) {
        u32 power = scene->GetData(x, y);
        // Gradually fade out explosion particle.
        if(power <= 1) {
            if(power == 0) {
                scene->SetParticle(x, y, ParticleType::NOTHING);
            } else {
                scene->SetParticle(x, y, ParticleType::EXPLOSION, power - 1);
            }
        } else {
            // Spread explosion.
            if(power != 0 && (rand() % power) <= power * 0.75f) {
                if(scene->GetParticle(x, y + 1) != ParticleType::IRONWALL) {
                    scene->SetParticle(x, y + 1, ParticleType::EXPLOSION, power - 1);
                }

                if(scene->GetParticle(x, y - 1) != ParticleType::IRONWALL) {
                    scene->SetParticle(x, y - 1, ParticleType::EXPLOSION, power - 1);
                }

                if(scene->GetParticle(x + 1, y) != ParticleType::IRONWALL) {
                    scene->SetParticle(x + 1, y, ParticleType::EXPLOSION, power - 1);
                }

                if(scene->GetParticle(x - 1, y) != ParticleType::IRONWALL) {
                    scene->SetParticle(x - 1, y, ParticleType::EXPLOSION, power - 1);
                }
            }

            // Set explosion particle to fade out.
            scene->SetParticle(x, y, ParticleType::EXPLOSION, 1);
        }
    });
}