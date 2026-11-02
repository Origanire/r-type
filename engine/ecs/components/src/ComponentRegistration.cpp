#include "ComponentRegistration.hpp"

#include "Anchor.hpp"
#include "AutoFire.hpp"
#include "Bounded.hpp"
#include "Collider.hpp"
#include "ContactDamage.hpp"
#include "DeathEffect.hpp"
#include "DespawnOffscreen.hpp"
#include "Destroyed.hpp"
#include "Health.hpp"
#include "Invulnerable.hpp"
#include "Lives.hpp"
#include "Match.hpp"
#include "MovementPattern.hpp"
#include "Owner.hpp"
#include "Parallax.hpp"
#include "Persistent.hpp"
#include "Player.hpp"
#include "PlayerControl.hpp"
#include "PlayerIndex.hpp"
#include "PlayerLabel.hpp"
#include "RectShape.hpp"
#include "Replicated.hpp"
#include "Respawn.hpp"
#include "Score.hpp"
#include "ScoreValue.hpp"
#include "SoundCue.hpp"
#include "Spawner.hpp"
#include "Sprite.hpp"
#include "SpriteAnimation.hpp"
#include "StatusText.hpp"
#include "Text.hpp"
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
    registry.registerComponent<AutoFire>();
    registry.registerComponent<SpriteAnimation>();
    registry.registerComponent<SoundCue>();
    registry.registerComponent<Text>();
    registry.registerComponent<Anchor>();
    registry.registerComponent<Player>();
    registry.registerComponent<Lives>();
    registry.registerComponent<Score>();
    registry.registerComponent<Respawn>();
    registry.registerComponent<Match>();
    registry.registerComponent<Persistent>();
    registry.registerComponent<DeathEffect>();
    registry.registerComponent<PlayerIndex>();
    registry.registerComponent<PlayerLabel>();
    registry.registerComponent<StatusText>();
}

} // namespace engine::ecs
