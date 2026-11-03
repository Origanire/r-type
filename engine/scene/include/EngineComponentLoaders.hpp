#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "ComponentFactory.hpp"
#include "DataNode.hpp"
#include "Entity.hpp"
#include "LayerTable.hpp"
#include "Match.hpp"
#include "MovementPattern.hpp"
#include "Registry.hpp"
#include "SpawnEntry.hpp"

namespace engine::scene {

class EngineComponentLoaders
{
public:
    static void registerAll(ComponentFactory &factory, const LayerTable &layers);

private:
    static void registerGameplay(ComponentFactory &factory, const LayerTable &layers);
    static void registerRules(ComponentFactory &factory);
    static void registerPresentation(ComponentFactory &factory);

    static void loadTransform(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadVelocity(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadCollider(
        const LayerTable &layers, ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadBounded(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadHealth(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadContactDamage(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadInvulnerable(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadMovementPattern(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadDespawnOffscreen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadPlayerControl(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadScoreValue(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadWeapon(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadAutoFire(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadSpawner(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadReplicated(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);

    static void loadLives(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadScore(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadRespawn(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadMatch(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadPersistent(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);

    static void loadSprite(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadRectShape(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadParallax(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadTiltAnimation(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadSpriteAnimation(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadSoundCue(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadDeathEffect(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadPlayerLabel(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadStatusText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);

    static ecs::MovementPattern::Kind readPatternKind(const DataNode &data);
    static std::vector<ecs::SpawnEntry> readSpawnEntries(const DataNode &data);
    static std::optional<ecs::Match::Phase> readPhase(const DataNode &data, const std::string &key);
    static std::optional<std::size_t> readSlot(const DataNode &data);
};

} // namespace engine::scene
