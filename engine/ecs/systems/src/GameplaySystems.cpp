#include "GameplaySystems.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <utility>
#include <vector>

#include "Attachment.hpp"
#include "Bounce.hpp"
#include "Bounded.hpp"
#include "Choice.hpp"
#include "Collider.hpp"
#include "Destroyed.hpp"
#include "Gravity.hpp"
#include "HitFlash.hpp"
#include "Hovered.hpp"
#include "Lifetime.hpp"
#include "MusicWhen.hpp"
#include "ParticleEmitter.hpp"
#include "Pushable.hpp"
#include "RectShape.hpp"
#include "Solid.hpp"
#include "Tilemap.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"

namespace engine::ecs {

AttachmentSystem::AttachmentSystem(Registry &registry) : m_registry(registry)
{
}

void AttachmentSystem::update(float)
{
    for (std::size_t i : m_registry.view<Attachment, Transform>()) {
        const Entity child = m_registry.entityAt(i);
        const Attachment &link = m_registry.get<Attachment>(child);
        const Entity parent = link.parent();
        if (!link.attached() || !m_registry.isAlive(parent) || !m_registry.has<Transform>(parent))
            continue;
        const Transform &anchor = m_registry.get<Transform>(parent);
        m_registry.get<Transform>(child).setPosition(anchor.x() + link.offsetX(), anchor.y() + link.offsetY());
        if (m_registry.has<Velocity>(child))
            m_registry.get<Velocity>(child).set(0.f, 0.f);
    }
}

HitFlashSystem::HitFlashSystem(Registry &registry) : m_registry(registry)
{
}

void HitFlashSystem::update(float dt)
{
    for (std::size_t i : m_registry.view<HitFlash>()) {
        const Entity entity = m_registry.entityAt(i);
        HitFlash &flash = m_registry.get<HitFlash>(entity);
        flash.elapse(dt);
        if (flash.remaining() <= 0.f)
            m_registry.remove<HitFlash>(entity);
    }
}

SolidSystem::SolidSystem(Registry &registry) : m_registry(registry)
{
}

void SolidSystem::update(float)
{
    const auto walls = m_registry.view<Solid, Transform, Collider>();
    for (std::size_t mover : m_registry.view<Pushable, Transform, Collider>()) {
        for (std::size_t wall : walls)
            pushOut(mover, wall);
    }
}

void SolidSystem::pushOut(std::size_t mover, std::size_t wall)
{
    auto &transforms = m_registry.getComponents<Transform>();
    const auto &colliders = m_registry.getComponents<Collider>();
    Transform &body = transforms.at(mover);
    const Transform &block = transforms.at(wall);
    const Collider &bodySize = colliders.at(mover);
    const Collider &blockSize = colliders.at(wall);
    const float left = block.x() + blockSize.width() - body.x();
    const float right = body.x() + bodySize.width() - block.x();
    const float up = block.y() + blockSize.height() - body.y();
    const float down = body.y() + bodySize.height() - block.y();
    if (left <= 0.f || right <= 0.f || up <= 0.f || down <= 0.f)
        return;
    const float smallest = std::min({left, right, up, down});
    if (smallest == left)
        body.setPosition(body.x() + left, body.y());
    else if (smallest == right)
        body.setPosition(body.x() - right, body.y());
    else if (smallest == up)
        body.setPosition(body.x(), body.y() + up);
    else
        body.setPosition(body.x(), body.y() - down);
}

TilemapSystem::TilemapSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn, Vector2 area)
    : m_registry(registry), m_factory(factory), m_canSpawn(std::move(canSpawn)), m_area(area)
{
}

void TilemapSystem::update(float dt)
{
    for (std::size_t i : m_registry.view<Tilemap, Transform>()) {
        const Entity map = m_registry.entityAt(i);
        Tilemap &tiles = m_registry.get<Tilemap>(map);
        tiles.scroll(tiles.speed() * dt);
        const float originX = m_registry.get<Transform>(map).x();
        while (tiles.nextColumn() < tiles.columns()) {
            const float x = originX + static_cast<float>(tiles.nextColumn()) * tiles.tileSize() - tiles.scrolled();
            if (x > m_area.x() + tiles.tileSize() || !m_canSpawn())
                break;
            spawnColumn(map, x);
            m_registry.get<Tilemap>(map).advanceColumn();
        }
    }
}

void TilemapSystem::spawnColumn(const Entity &map, float x)
{
    const Tilemap tiles = m_registry.get<Tilemap>(map);
    const float originY = m_registry.get<Transform>(map).y();
    for (std::size_t row = 0; row < tiles.rows().size(); ++row) {
        const std::string &line = tiles.rows()[row];
        if (tiles.nextColumn() >= line.size())
            continue;
        const auto prefab = tiles.legend().find(line[tiles.nextColumn()]);
        if (prefab == tiles.legend().end())
            continue;
        const Entity tile = m_factory.spawn(prefab->second);
        const float y = originY + static_cast<float>(row) * tiles.tileSize();
        if (m_registry.has<Transform>(tile))
            m_registry.get<Transform>(tile).setPosition(x, y);
        else
            m_registry.emplace<Transform>(tile, x, y);
        if (m_registry.has<Velocity>(tile))
            m_registry.get<Velocity>(tile).set(-tiles.speed(), 0.f);
        else
            m_registry.emplace<Velocity>(tile, -tiles.speed(), 0.f);
    }
}

ParticleSystem::ParticleSystem(Registry &registry) : m_registry(registry), m_random(std::random_device{}())
{
}

void ParticleSystem::update(float)
{
    for (std::size_t i : m_registry.view<ParticleEmitter, Transform>()) {
        const Entity source = m_registry.entityAt(i);
        if (!m_registry.get<ParticleEmitter>(source).emitted())
            emit(source);
    }
}

void ParticleSystem::emit(const Entity &source)
{
    const ParticleEmitter emitter = m_registry.get<ParticleEmitter>(source);
    const Transform origin = m_registry.get<Transform>(source);
    m_registry.get<ParticleEmitter>(source).markEmitted();
    std::uniform_real_distribution<float> angle(0.f, 2.f * std::numbers::pi_v<float>);
    std::uniform_real_distribution<float> speed(emitter.speed() * 0.3f, emitter.speed());
    for (int index = 0; index < emitter.count(); ++index) {
        const float direction = angle(m_random);
        const float velocity = speed(m_random);
        const Entity particle = m_registry.spawn();
        m_registry.emplace<Transform>(particle, origin.x(), origin.y());
        m_registry.emplace<Velocity>(particle, std::cos(direction) * velocity, std::sin(direction) * velocity);
        m_registry.emplace<RectShape>(particle, emitter.size(), emitter.size(), emitter.color());
        m_registry.emplace<Lifetime>(particle, emitter.lifetime());
    }
}

LifetimeSystem::LifetimeSystem(Registry &registry) : m_registry(registry)
{
}

void LifetimeSystem::update(float dt)
{
    std::vector<Entity> expired;
    for (std::size_t i : m_registry.view<Lifetime>()) {
        const Entity entity = m_registry.entityAt(i);
        Lifetime &life = m_registry.get<Lifetime>(entity);
        life.elapse(dt);
        if (life.remaining() <= 0.f) {
            expired.push_back(entity);
            continue;
        }
        if (m_registry.has<RectShape>(entity) && life.total() > 0.f) {
            RectShape &shape = m_registry.get<RectShape>(entity);
            const Color color = shape.color();
            const float ratio = life.remaining() / life.total();
            shape.setColor(Color(color.r(), color.g(), color.b(), static_cast<std::uint8_t>(ratio * MAX_ALPHA)));
        }
    }
    for (const Entity &entity : expired)
        m_registry.kill(entity);
}

MusicSystem::MusicSystem(Registry &registry, const UiStore &store, audio::IAudio &audio)
    : m_registry(registry), m_store(store), m_audio(audio), m_current()
{
}

void MusicSystem::update(float)
{
    const auto &rules = m_registry.getComponents<MusicWhen>();
    for (std::size_t i : m_registry.view<MusicWhen>()) {
        const MusicWhen &rule = rules.at(i);
        if (m_store.value(rule.key()) != rule.value())
            continue;
        if (rule.music() != m_current) {
            m_current = rule.music();
            m_audio.playMusic(m_current);
        }
        return;
    }
}

ShakeSystem::ShakeSystem(const UiStore &store, render::IRenderer &renderer, std::string key, float strength)
    : m_store(store), m_renderer(renderer), m_key(std::move(key)), m_strength(strength),
      m_random(std::random_device{}())
{
}

void ShakeSystem::update(float)
{
    const std::string text = m_store.value(m_key);
    const float amount = text.empty() ? 0.f : std::strtof(text.c_str(), nullptr) * m_strength;
    if (amount <= 0.f) {
        m_renderer.setOffset({0.f, 0.f});
        return;
    }
    std::uniform_real_distribution<float> offset(-amount, amount);
    m_renderer.setOffset({offset(m_random), offset(m_random)});
}

GravitySystem::GravitySystem(Registry &registry) : m_registry(registry)
{
}

void GravitySystem::update(float dt)
{
    const auto &gravities = m_registry.getComponents<Gravity>();
    auto &velocities = m_registry.getComponents<Velocity>();
    for (std::size_t i : m_registry.view<Gravity, Velocity>()) {
        Velocity &velocity = velocities.at(i);
        velocity.setY(velocity.y() + gravities.at(i).force() * dt);
    }
}

BounceSystem::BounceSystem(Registry &registry) : m_registry(registry)
{
}

void BounceSystem::update(float)
{
    const auto walls = m_registry.view<Solid, Transform, Collider>();
    for (std::size_t i : m_registry.view<Bounce, Transform, Velocity, Collider>()) {
        if (m_registry.has<Bounded>(m_registry.entityAt(i)))
            bounceOnBounds(i);
        for (std::size_t wall : walls) {
            if (wall != i)
                bounceOnSolid(i, wall);
        }
    }
}

void BounceSystem::bounceOnBounds(std::size_t index)
{
    const Entity entity = m_registry.entityAt(index);
    const Transform &transform = m_registry.get<Transform>(entity);
    const Collider &collider = m_registry.get<Collider>(entity);
    const Bounded &area = m_registry.get<Bounded>(entity);
    const float restitution = m_registry.get<Bounce>(entity).restitution();
    Velocity &velocity = m_registry.get<Velocity>(entity);
    if ((transform.x() <= area.minX() && velocity.x() < 0.f)
        || (transform.x() + collider.width() >= area.maxX() && velocity.x() > 0.f))
        velocity.setX(-velocity.x() * restitution);
    if ((transform.y() <= area.minY() && velocity.y() < 0.f)
        || (transform.y() + collider.height() >= area.maxY() && velocity.y() > 0.f))
        velocity.setY(-velocity.y() * restitution);
}

void BounceSystem::bounceOnSolid(std::size_t index, std::size_t wall)
{
    auto &transforms = m_registry.getComponents<Transform>();
    const auto &colliders = m_registry.getComponents<Collider>();
    Transform &body = transforms.at(index);
    const Transform &block = transforms.at(wall);
    const Collider &bodySize = colliders.at(index);
    const Collider &blockSize = colliders.at(wall);
    const float left = block.x() + blockSize.width() - body.x();
    const float right = body.x() + bodySize.width() - block.x();
    const float up = block.y() + blockSize.height() - body.y();
    const float down = body.y() + bodySize.height() - block.y();
    if (left <= 0.f || right <= 0.f || up <= 0.f || down <= 0.f)
        return;
    const Entity entity = m_registry.entityAt(index);
    const float restitution = m_registry.get<Bounce>(entity).restitution();
    Velocity &velocity = m_registry.get<Velocity>(entity);
    const float smallest = std::min({left, right, up, down});
    if (smallest == left || smallest == right) {
        body.setPosition(body.x() + (smallest == left ? left : -right), body.y());
        velocity.setX(-velocity.x() * restitution);
    } else {
        body.setPosition(body.x(), body.y() + (smallest == up ? up : -down));
        velocity.setY(-velocity.y() * restitution);
    }
}

ChoiceSystem::ChoiceSystem(Registry &registry, const UiInput &input, UiStore &store)
    : m_registry(registry), m_input(input), m_store(store)
{
}

void ChoiceSystem::update(float)
{
    for (std::size_t i : m_registry.view<Choice>()) {
        const Entity entity = m_registry.entityAt(i);
        Choice &choice = m_registry.get<Choice>(entity);
        if (m_input.clicked() && m_registry.has<Hovered>(entity))
            choice.next();
        m_store.setValue(std::string(Choice::STORE_PREFIX) + choice.name(), choice.current());
    }
}

} // namespace engine::ecs
