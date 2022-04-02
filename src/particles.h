#pragma once
#include "object.h"

typedef struct 
{
    object_t* object;
    float lifetime;
}particle_t;

typedef struct
{
    particle_t* particles;
    int particlesCount;
    //Todo autres paralètes
} emmiter_t;