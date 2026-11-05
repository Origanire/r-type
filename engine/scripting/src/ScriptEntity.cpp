#include "ScriptEntity.hpp"

#include <cmath>
#include <numbers>

#include "Attachment.hpp"
#include "Collider.hpp"
#include "Destroyed.hpp"
#include "Health.hpp"
#include "Invulnerable.hpp"
#include "Owner.hpp"
#include "Player.hpp"
#include "PlayerControl.hpp"
#include "Rotation.hpp"
#include "Script.hpp"
#include "ScriptError.hpp"
#include "ScriptWorld.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "Weapon.hpp"

namespace engine::scripting {

ScriptEntity::ScriptEntity(ScriptWorld &world, ecs::Entity entity)
    : m_world(&world), m_entity(entity), m_age(0.f), m_timers()
{
}

void ScriptEntity::advance(float dt)
{
    m_age += dt;
    for (auto &timer : m_timers)
        timer.second += dt;
}

bool ScriptEntity::alive() const
{
    const ecs::Registry &registry = m_world->registry();
    return registry.isAlive(m_entity) && !registry.has<ecs::Destroyed>(m_entity);
}

bool ScriptEntity::exists() const
{
    return m_world->registry().isAlive(m_entity);
}

float ScriptEntity::age() const
{
    return m_age;
}

bool ScriptEntity::every(const std::string &name, float seconds)
{
    const auto [found, created] = m_timers.try_emplace(name, 0.f);
    if (created || found->second < seconds)
        return false;
    found->second -= seconds;
    return true;
}

float ScriptEntity::x() const
{
    const ecs::Registry &registry = m_world->registry();
    return exists() && registry.has<ecs::Transform>(m_entity) ? registry.get<ecs::Transform>(m_entity).x() : 0.f;
}

float ScriptEntity::y() const
{
    const ecs::Registry &registry = m_world->registry();
    return exists() && registry.has<ecs::Transform>(m_entity) ? registry.get<ecs::Transform>(m_entity).y() : 0.f;
}

float ScriptEntity::width() const
{
    const ecs::Registry &registry = m_world->registry();
    return exists() && registry.has<ecs::Collider>(m_entity) ? registry.get<ecs::Collider>(m_entity).width() : 0.f;
}

float ScriptEntity::height() const
{
    const ecs::Registry &registry = m_world->registry();
    return exists() && registry.has<ecs::Collider>(m_entity) ? registry.get<ecs::Collider>(m_entity).height() : 0.f;
}

float ScriptEntity::centerX() const
{
    return x() + width() / 2.f;
}

float ScriptEntity::centerY() const
{
    return y() + height() / 2.f;
}

float ScriptEntity::vx() const
{
    const ecs::Registry &registry = m_world->registry();
    return exists() && registry.has<ecs::Velocity>(m_entity) ? registry.get<ecs::Velocity>(m_entity).x() : 0.f;
}

float ScriptEntity::vy() const
{
    const ecs::Registry &registry = m_world->registry();
    return exists() && registry.has<ecs::Velocity>(m_entity) ? registry.get<ecs::Velocity>(m_entity).y() : 0.f;
}

void ScriptEntity::setPosition(float x, float y)
{
    ecs::Registry &registry = m_world->registry();
    if (!alive())
        return;
    if (registry.has<ecs::Transform>(m_entity))
        registry.get<ecs::Transform>(m_entity).setPosition(x, y);
    else
        registry.emplace<ecs::Transform>(m_entity, x, y);
}

void ScriptEntity::setVelocity(float vx, float vy)
{
    ecs::Registry &registry = m_world->registry();
    if (!alive())
        return;
    if (registry.has<ecs::Velocity>(m_entity))
        registry.get<ecs::Velocity>(m_entity).set(vx, vy);
    else
        registry.emplace<ecs::Velocity>(m_entity, vx, vy);
}

int ScriptEntity::health() const
{
    const ecs::Registry &registry = m_world->registry();
    return registry.isAlive(m_entity) && registry.has<ecs::Health>(m_entity)
        ? registry.get<ecs::Health>(m_entity).points()
        : 0;
}

int ScriptEntity::maxHealth() const
{
    const ecs::Registry &registry = m_world->registry();
    return registry.isAlive(m_entity) && registry.has<ecs::Health>(m_entity)
        ? registry.get<ecs::Health>(m_entity).maximum()
        : 0;
}

float ScriptEntity::number(const std::string &name, float fallback) const
{
    const ecs::Registry &registry = m_world->registry();
    if (!registry.isAlive(m_entity) || !registry.has<ecs::Script>(m_entity))
        return fallback;
    return registry.get<ecs::Script>(m_entity).number(name, fallback);
}

std::string ScriptEntity::text(const std::string &name, const std::string &fallback) const
{
    const ecs::Registry &registry = m_world->registry();
    if (!registry.isAlive(m_entity) || !registry.has<ecs::Script>(m_entity))
        return fallback;
    return registry.get<ecs::Script>(m_entity).text(name, fallback);
}

void ScriptEntity::fire(const std::string &prefab, float angle, float speed, float dx, float dy)
{
    if (!exists() || !m_world->canSpawn())
        return;
    const float originX = centerX() + dx;
    const float originY = centerY() + dy;
    ScriptEntity shot = m_world->spawn(prefab, originX, originY);
    shot.setPosition(originX - shot.width() / 2.f, originY - shot.height() / 2.f);
    const float radians = angle * std::numbers::pi_v<float> / 180.f;
    shot.setVelocity(std::cos(radians) * speed, std::sin(radians) * speed);
    m_world->registry().emplace<ecs::Owner>(shot.m_entity, m_entity);
}

void ScriptEntity::fireSpread(const std::string &prefab, int count, float spread, float angle, float speed)
{
    if (count <= 0)
        return;
    if (count == 1) {
        fire(prefab, angle, speed, 0.f, 0.f);
        return;
    }
    const float step = spread / static_cast<float>(count - 1);
    const float first = angle - spread / 2.f;
    for (int index = 0; index < count; ++index)
        fire(prefab, first + step * static_cast<float>(index), speed, 0.f, 0.f);
}

void ScriptEntity::fireAtPlayer(const std::string &prefab, float speed)
{
    fire(prefab, angleToPlayer(), speed, 0.f, 0.f);
}

std::optional<std::pair<float, float>> ScriptEntity::nearestPlayer() const
{
    const ecs::Registry &registry = m_world->registry();
    std::optional<std::pair<float, float>> nearest;
    float best = 0.f;
    for (std::size_t i : registry.view<ecs::PlayerControl, ecs::Transform>()) {
        const ecs::Entity player = registry.entityAt(i);
        if (registry.has<ecs::Destroyed>(player))
            continue;
        ScriptEntity target(*m_world, player);
        const float distanceX = target.centerX() - centerX();
        const float distanceY = target.centerY() - centerY();
        const float distance = distanceX * distanceX + distanceY * distanceY;
        if (!nearest || distance < best) {
            nearest = std::make_pair(target.centerX(), target.centerY());
            best = distance;
        }
    }
    return nearest;
}

float ScriptEntity::angleToPlayer() const
{
    const auto target = nearestPlayer();
    if (!target)
        return DEFAULT_ANGLE;
    const float radians = std::atan2(target->second - centerY(), target->first - centerX());
    return radians * 180.f / std::numbers::pi_v<float>;
}

ScriptEntity ScriptEntity::spawn(const std::string &prefab, float dx, float dy)
{
    return m_world->spawn(prefab, x() + dx, y() + dy);
}

void ScriptEntity::setInvulnerable(float seconds)
{
    ecs::Registry &registry = m_world->registry();
    if (!alive())
        return;
    if (registry.has<ecs::Invulnerable>(m_entity))
        registry.remove<ecs::Invulnerable>(m_entity);
    registry.emplace<ecs::Invulnerable>(m_entity, seconds);
}

void ScriptEntity::destroy()
{
    if (alive())
        m_world->registry().emplace<ecs::Destroyed>(m_entity);
}

void ScriptEntity::kill()
{
    m_world->kill(m_entity);
}

ScriptWorld &ScriptEntity::world() const
{
    return *m_world;
}

ecs::Entity ScriptEntity::entity() const
{
    return m_entity;
}

bool ScriptEntity::isPlayer() const
{
    return exists() && m_world->registry().has<ecs::PlayerControl>(m_entity);
}

int ScriptEntity::playerSlot() const
{
    const ecs::Registry &registry = m_world->registry();
    if (!exists() || !registry.has<ecs::Owner>(m_entity))
        return NO_SLOT;
    const ecs::Entity owner = registry.get<ecs::Owner>(m_entity).owner();
    if (!registry.isAlive(owner) || !registry.has<ecs::Player>(owner))
        return NO_SLOT;
    return static_cast<int>(registry.get<ecs::Player>(owner).slot());
}

void ScriptEntity::heal(int amount)
{
    ecs::Registry &registry = m_world->registry();
    if (alive() && registry.has<ecs::Health>(m_entity))
        registry.get<ecs::Health>(m_entity).heal(amount);
}

void ScriptEntity::setHealth(int points, int maximum)
{
    ecs::Registry &registry = m_world->registry();
    if (!alive())
        return;
    if (registry.has<ecs::Health>(m_entity))
        registry.get<ecs::Health>(m_entity).set(points, maximum);
    else
        registry.emplace<ecs::Health>(m_entity, points);
}

void ScriptEntity::setWeapon(const std::string &prefab, float cooldown, int count, float spread)
{
    ecs::Registry &registry = m_world->registry();
    if (!alive() || !registry.has<ecs::Weapon>(m_entity))
        return;
    registry.get<ecs::Weapon>(m_entity).configure(prefab, cooldown, count, spread);
}

float ScriptEntity::inputHeld(const std::string &name) const
{
    const ecs::Registry &registry = m_world->registry();
    if (!alive() || !registry.has<ecs::PlayerControl>(m_entity))
        return 0.f;
    const ecs::PlayerControl &control = registry.get<ecs::PlayerControl>(m_entity);
    if (name == INPUT_FIRE)
        return control.fireHeld();
    if (name == INPUT_ACTION)
        return control.action() ? 1.f : 0.f;
    throw ScriptError("ScriptEntity: unknown input " + name);
}

bool ScriptEntity::inputPressed(const std::string &name) const
{
    const ecs::Registry &registry = m_world->registry();
    if (!alive() || !registry.has<ecs::PlayerControl>(m_entity))
        return false;
    const ecs::PlayerControl &control = registry.get<ecs::PlayerControl>(m_entity);
    if (name == INPUT_ACTION)
        return control.actionPressed();
    if (name == INPUT_FIRE)
        return control.firing();
    throw ScriptError("ScriptEntity: unknown input " + name);
}

float ScriptEntity::inputReleased(const std::string &name) const
{
    const ecs::Registry &registry = m_world->registry();
    if (!alive() || !registry.has<ecs::PlayerControl>(m_entity))
        return 0.f;
    if (name != INPUT_FIRE)
        throw ScriptError("ScriptEntity: only fire reports a release, not " + name);
    return registry.get<ecs::PlayerControl>(m_entity).fireReleased();
}

void ScriptEntity::attachTo(const ScriptEntity &parent, float dx, float dy)
{
    ecs::Registry &registry = m_world->registry();
    if (!alive() || !parent.exists())
        return;
    if (registry.has<ecs::Attachment>(m_entity) && registry.get<ecs::Attachment>(m_entity).parent() == parent.m_entity)
        registry.get<ecs::Attachment>(m_entity).attach(dx, dy);
    else {
        if (registry.has<ecs::Attachment>(m_entity))
            registry.remove<ecs::Attachment>(m_entity);
        registry.emplace<ecs::Attachment>(m_entity, parent.m_entity, dx, dy);
    }
}

void ScriptEntity::detach()
{
    ecs::Registry &registry = m_world->registry();
    if (exists() && registry.has<ecs::Attachment>(m_entity))
        registry.get<ecs::Attachment>(m_entity).detach();
}

bool ScriptEntity::attached() const
{
    const ecs::Registry &registry = m_world->registry();
    return exists() && registry.has<ecs::Attachment>(m_entity) && registry.get<ecs::Attachment>(m_entity).attached();
}

std::optional<ScriptEntity> ScriptEntity::parent() const
{
    const ecs::Registry &registry = m_world->registry();
    if (!exists() || !registry.has<ecs::Attachment>(m_entity))
        return std::nullopt;
    const ecs::Entity parent = registry.get<ecs::Attachment>(m_entity).parent();
    if (!registry.isAlive(parent))
        return std::nullopt;
    return ScriptEntity(*m_world, parent);
}

void ScriptEntity::setAngle(float degrees)
{
    ecs::Registry &registry = m_world->registry();
    if (!exists())
        return;
    if (registry.has<ecs::Rotation>(m_entity))
        registry.get<ecs::Rotation>(m_entity).setDegrees(degrees);
    else
        registry.emplace<ecs::Rotation>(m_entity, degrees);
}

void ScriptEntity::setPlayerValue(const std::string &name, int value)
{
    m_world->setPlayerValue(m_entity, name, value);
}

} // namespace engine::scripting
