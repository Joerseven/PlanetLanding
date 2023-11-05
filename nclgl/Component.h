    //
// Created by jdhyd on 11/5/2023.
//

#ifndef PLANETLANDING_SPARSESET_H
#define PLANETLANDING_SPARSESET_H

#include <vector>
#include <typeinfo>
#include <unordered_map>
#include <memory>

#define MAX_ENTITIES 4000

using Entity = std::uint32_t;

class wtf {};

template<class T>
class Component : wtf {
public:
    Component<T>() = default;
    void AddComponent(Entity entity, T component);
private:
    std::vector<Entity> sparse;
    std::vector<Entity> packed;
    std::vector<T> items;
};

class Registry {
public:
    Entity CreateEntity();
    template <typename T>
    void RegisterComponent() {
        const char* typeName = typeid(T).name();
        components.insert({typeName, (void*)new Component<T>()});

    }
    std::uint32_t entityCount;
    std::unordered_map<const char*, void*> components;
};


#endif //PLANETLANDING_SPARSESET_H
