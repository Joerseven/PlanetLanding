    //
// Created by jdhyd on 11/5/2023.
//

#ifndef PLANETLANDING_SPARSESET_H
#define PLANETLANDING_SPARSESET_H

#include <vector>
#include <typeinfo>
#include <unordered_map>
#include <memory>
#include "Matrix4.h"
#include "glad/glad.h"
#include <cassert>

#define MAX_ENTITIES 4000

using Entity = std::uint32_t;
using Transform = Matrix4;

class Shader;

struct ModelData {
    GLuint texture;
    Shader *shader;
    GLuint arrayObject;
    GLuint type;
    GLuint numIndices;
};

class wtf {};

template<class T>
class Component : wtf {
public:
    Component<T>() = default;
    void AddComponent(Entity entity, const T& component) {
        items.push_back(component);

        if (items.size() >= packed.size()) {
            packed.resize(items.size());
        }

        packed[items.size() - 1] = entity;

        if (entity >= sparse.size()) {
            sparse.resize(entity + 1);
        }

        sparse[entity] = items.size() - 1;
    };

    std::vector<Entity> sparse;
    std::vector<Entity> packed;
    std::vector<T> items;
};

class Registry {
public:
    Entity CreateEntity() { return entityCount++; }
    template <typename T>
    void RegisterComponent() {
        const char* typeName = typeid(T).name();
        assert(components.find(typeName) == components.end() && "Registering component type more than once.");
        components.insert({typeName, (void*)new Component<T>()});
        assert(components.find(typeName) != components.end() && "registered properly?");
    }

    template <typename T>
    void AddComponent(Entity entity, const T& component) {
        GetComponents<T>()->AddComponent(entity, component);
    }

    template <typename T>
    Component<T> *GetComponents() {
        return (Component<T>*)components[typeid(T).name()];
    }
    std::uint32_t entityCount = 0;
    std::unordered_map<const char*, void*> components;
};



#endif //PLANETLANDING_SPARSESET_H
