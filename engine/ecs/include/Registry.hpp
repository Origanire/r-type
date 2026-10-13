#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "EcsError.hpp"
#include "Entity.hpp"
#include "IComponentArray.hpp"
#include "SparseArray.hpp"

namespace engine::ecs {

class Registry
{
public:
    template <typename Component>
    SparseArray<Component> &registerComponent()
    {
        std::unique_ptr<IComponentArray> &array = m_components[std::type_index(typeid(Component))];
        if (!array)
            array = std::make_unique<SparseArray<Component>>();
        return static_cast<SparseArray<Component> &>(*array);
    }

    template <typename Component>
    SparseArray<Component> &getComponents()
    {
        auto it = m_components.find(std::type_index(typeid(Component)));
        if (it == m_components.end())
            throw EcsError(std::string("Registry: component not registered: ") + typeid(Component).name());
        return static_cast<SparseArray<Component> &>(*it->second);
    }

    template <typename Component>
    const SparseArray<Component> &getComponents() const
    {
        auto it = m_components.find(std::type_index(typeid(Component)));
        if (it == m_components.end())
            throw EcsError(std::string("Registry: component not registered: ") + typeid(Component).name());
        return static_cast<const SparseArray<Component> &>(*it->second);
    }

    template <typename Component>
    void addComponent(const Entity &to, Component component)
    {
        checkAlive(to);
        getComponents<Component>().insertAt(to.index(), std::move(component));
    }

    template <typename Component>
    void removeComponent(const Entity &from)
    {
        if (isAlive(from))
            getComponents<Component>().erase(from.index());
    }

    Entity spawnEntity();
    Entity entityFromIndex(std::size_t index) const;
    void killEntity(const Entity &entity);
    bool isAlive(const Entity &entity) const;

private:
    void checkAlive(const Entity &entity) const;

    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> m_components;
    std::vector<Entity::Generation> m_generations;
    std::vector<bool> m_alive;
    std::vector<std::size_t> m_freeIndexes;
};

} // namespace engine::ecs
