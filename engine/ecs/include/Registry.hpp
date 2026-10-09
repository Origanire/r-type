#pragma once

#include <any>
#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "EcsError.hpp"
#include "Entity.hpp"
#include "SparseArray.hpp"

namespace engine::ecs {

class Registry
{
public:
    template <typename Component>
    SparseArray<Component> &registerComponent()
    {
        auto [it, inserted] = m_components.try_emplace(std::type_index(typeid(Component)), SparseArray<Component>());
        if (inserted) {
            m_eraseFunctions.emplace_back([](Registry &registry, const Entity &entity) {
                registry.getComponents<Component>().erase(entity);
            });
        }
        return std::any_cast<SparseArray<Component> &>(it->second);
    }

    template <typename Component>
    SparseArray<Component> &getComponents()
    {
        auto it = m_components.find(std::type_index(typeid(Component)));
        if (it == m_components.end())
            throw EcsError(std::string("Registry: component not registered: ") + typeid(Component).name());
        return std::any_cast<SparseArray<Component> &>(it->second);
    }

    template <typename Component>
    const SparseArray<Component> &getComponents() const
    {
        auto it = m_components.find(std::type_index(typeid(Component)));
        if (it == m_components.end())
            throw EcsError(std::string("Registry: component not registered: ") + typeid(Component).name());
        return std::any_cast<const SparseArray<Component> &>(it->second);
    }

    Entity spawnEntity();
    Entity entityFromIndex(std::size_t idx) const;
    void killEntity(const Entity &entity);
    bool isAlive(const Entity &entity) const;

    template <typename Component>
    typename SparseArray<std::decay_t<Component>>::reference_type addComponent(const Entity &to, Component &&component)
    {
        checkAlive(to);
        return getComponents<std::decay_t<Component>>().insertAt(to, std::forward<Component>(component));
    }

    template <typename Component, typename... Params>
    typename SparseArray<Component>::reference_type emplaceComponent(const Entity &to, Params &&...params)
    {
        checkAlive(to);
        return getComponents<Component>().emplaceAt(to, std::forward<Params>(params)...);
    }

    template <typename Component>
    void removeComponent(const Entity &from)
    {
        if (isAlive(from))
            getComponents<Component>().erase(from);
    }

private:
    void checkAlive(const Entity &entity) const;

    std::unordered_map<std::type_index, std::any> m_components;
    std::vector<std::function<void(Registry &, const Entity &)>> m_eraseFunctions;    std::vector<Entity::Generation> m_generations;
    std::vector<bool> m_alive;
    std::vector<std::size_t> m_freeIndexes;
};

} // namespace engine::ecs
