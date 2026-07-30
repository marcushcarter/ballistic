#pragma once
#include <core/base/guid.h>
#include <core/base/error.h>
#include <vector>

namespace ballistic {

struct Scene
{

};

struct Scenes
{
    std::vector<Scene> scenes;
    Scene* active = nullptr;

    Error load() { return Error::Ok; }
    void unload() {}
};

}