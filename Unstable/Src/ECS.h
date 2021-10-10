#pragma once

#include <flecs.h>

#include "Engine.Forward.h"
#include "Utility.h"
#include "s7.h"
#include <vector>

namespace Unstable
{
	namespace ECS
	{
		struct ECSWorld
		{
			flecs::world* world;

			std::vector<flecs::entity> EarlyUpdateSystems;
			std::vector<flecs::entity> UpdateSystems;
			std::vector<flecs::entity> PrePhysicsSystem;
			std::vector<flecs::entity> PostPhysicsSystem;

		};

		struct DestroyTimer
		{
			f32 Value;
			f32 Elapsed;
		};

		struct SpawnTimer
		{
			f32 Value;
			f32 Elapsed;
			//flecs::prefab * prefab;
			std::string spawn;
		};

		struct OverlapDamage
		{
			f32 damage;
			f32 radius;
			flecs::entity* ignoreEnt;
		};

		struct SpriteComponent
		{
			sf::Sprite* sprite;
			s32 depthOffset;
			s32 depth;
			f32 offsetX;
			f32 offsetY;
		};

		struct SpriteSheet
		{
			sf::Sprite** sprites;
			u32 length;
			f32 timePerFrame;
			f32 offsetX;
			f32 offsetY;
		};

		struct AnimatedSprite
		{
			SpriteSheet* sprites;
			u32 currentFrame;
			f32 accumlator;

			bool haveCThunk;
			void (*c_thunk)(flecs::entity*);

			bool haveThunk;
			s7_pointer thunk;
		};

		struct AnimatedSpriteOnce
		{
			SpriteSheet* sprites;
			u32 currentFrame;
			f32 accumlator;

			bool haveCThunk;
			void (*c_thunk)(flecs::entity*);

			bool haveThunk;
			s7_pointer thunk;
		};

		struct Renderable
		{
			sf::Shape* drawable;
			s32 depthOffset;
			s32 depth;
		};

		struct CameraFocus
		{
			f32 offsetX;
			f32 offsetY;
		};

		struct Position
		{
			f32 x, y;
		};

		struct Rotation
		{
			f32 value;
		};

		struct SimPosition
		{
			f32 x, y, last_x, last_y;
		};

		struct SimCopy
		{

		};

		struct Velocity
		{
			f32 x, y;
		};

		struct Friction
		{
			f32 Value, StopThreshold;
		};

		struct Bounds
		{
			f32 width, height;
		};

		struct PhysicsBody
		{
			b2Body* Body;
		};

		struct AIState
		{
			u32 State;

			f32 moveTowardsX;
			f32 moveTowardsY;
			
			flecs::entity* targetEnt; //has a SimPosition

			//Stored path
			//etc
		};

		enum class NPCMoveState
		{
			Idle,
			Run,
			Attack,
			Dying,
			Dead,
		};

		struct NPCMovement
		{
			f32 speed;

			f32 range;
			f32 targetX;
			f32 targetY;
			NPCMoveState state;
			bool IsFacingLeft;
		};

		struct NPCCombat
		{
			f32 range;
			f32 attackRange;
			f32 attackTime;
			f32 attackDamageTime;
			f32 damage;

			bool didAttack;
			f32 accumlator;
			bool haveTarget;
			flecs::entity* target;
		};

		struct PlayerInput
		{
			f32 UpDown, LeftRight;
			f32 MaxMoveSpeed;
			bool Fire;
			bool Grenade;
			bool UsePowerup;
			bool Dodge;
			bool Activate;
			bool Reload;
		};

		enum class PlayerMoveState
		{
			Idle,
			IdleSpecial,
			Run,
			StartDodge,
			Dodging,
			GetUp,
			Dying,
			Dead,
			UnDie,
			UnDying,
		};

		const char* playerMoveStateString(PlayerMoveState s);

		struct PlayerMovement
		{
			bool IsFacingLeft;
			bool WasMoving;
			PlayerMoveState state;

			f32 dodgeX;
			f32 dodgeY;
		};

		struct Health
		{
			f32 Value;
		};

		enum class GunState
		{
			Idle,
			Reloading
		};

		enum class PowerUp
		{
			None,
			QuadDamage
		};

		struct GunCombat
		{
			u32 Bullets;
			u32 MagBullets;
			PowerUp PowerUp;
			f32 PowerUpTime;
			GunState gunState;
			f32 fireTimer;
			f32 reloadTimer;
		};

		struct AnimationSet
		{
			std::map<std::string, SpriteSheet*> animations;
			std::string index;
			std::string lastIndex;
			SpriteSheet* default;
			SpriteSheet* current;
			f32 elapsedTime;
			f32 timeUntilNext;
		};

		struct PlayerAttachment
		{
			flecs::entity* ent;
			f32 offsetX;
			f32 offsetY;
		};

		struct PlaySoundOnce
		{
			f32 volume;
			//fmod handle
		};

		struct PlaySoundLoop
		{
			f32 volume;
			//fmod handle
		};



		struct SchemeMarker
		{
			std::string formatString;
		};

		struct Tileset
		{
			std::vector<sf::Sprite*> sprites;
		};

		struct LevelEdit
		{
			u32 currentTileIndex;
			u32 currentTilesheetIndex;
			std::string currentTilesheet;
			std::map<std::string, Tileset*> tileSets;
		};

		void Init(Unstable::Engine* engine);
		void PrePhysicsUpdate(Unstable::Engine* engine);
		void PostPhysicsUpdate(Unstable::Engine* engine);

		void EarlyFrameUpdate(Unstable::Engine* engine);
		void Update(Unstable::Engine* engine);

		SpriteSheet* MakeSpriteSheet(std::vector<sf::Sprite*> sprites, f32 timePerFrame, f32 offsetX, f32 offsetY);
	}
}