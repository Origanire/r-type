#include "ComponentRegistration.hpp"

#include "Bounded.hpp"
#include "Collider.hpp"
#include "ContactDamage.hpp"
#include "DespawnOffscreen.hpp"
#include "Destroyed.hpp"
#include "Health.hpp"
#include "Invulnerable.hpp"
#include "MovementPattern.hpp"
#include "Owner.hpp"
#include "Parallax.hpp"
#include "PlayerControl.hpp"
#include "RectShape.hpp"
#include "Replicated.hpp"
#include "ScoreValue.hpp"
#include "Spawner.hpp"
#include "Sprite.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "Weapon.hpp"

namespace engine::ecs {

void registerEngineComponents(Registry &registry)
{
    registry.registerComponent<Transform>();
    registry.registerComponent<Velocity>();
    registry.registerComponent<Collider>();
    registry.registerComponent<Bounded>();
    registry.registerComponent<Health>();
    registry.registerComponent<ContactDamage>();
    registry.registerComponent<Invulnerable>();
    registry.registerComponent<MovementPattern>();
    registry.registerComponent<DespawnOffscreen>();
    registry.registerComponent<PlayerControl>();
    registry.registerComponent<Owner>();
    registry.registerComponent<ScoreValue>();
    registry.registerComponent<Destroyed>();
    registry.registerComponent<Sprite>();
    registry.registerComponent<RectShape>();
    registry.registerComponent<Parallax>();
    registry.registerComponent<TiltAnimation>();
    registry.registerComponent<Weapon>();
    registry.registerComponent<Spawner>();
    registry.registerComponent<Replicated>();
}

} // namespace engine::ecs
