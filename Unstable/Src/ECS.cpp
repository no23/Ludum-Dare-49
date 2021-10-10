
#include "Engine.h"

#include "ECS.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "UI.h"

using std::cout;
using std::endl;
using std::string;
using flecs::entity;
using flecs::IsA;

using namespace Unstable::ECS;

namespace fs = std::filesystem;

namespace Unstable
{
	namespace ECS
	{
		Unstable::Engine* _engine;
		flecs::query<SimPosition, SimPosition> q;

		flecs::entity* GetEntityFromBody(b2Body* ppbod)
		{
			//Scheme function that creates PhysicsBody components, always makes the body user pointer
			//point towards the entity its on
			return (flecs::entity*)ppbod->GetUserData().pointer;
		}

		const char* playerMoveStateString(PlayerMoveState s)
		{
			switch (s)
			{
			case PlayerMoveState::Idle: return "Idle"; break;
			case PlayerMoveState::IdleSpecial: return "IdleSpecial"; break;
			case PlayerMoveState::Run: return "Run"; break;
			case PlayerMoveState::StartDodge: return "Dodging"; break;
			case PlayerMoveState::Dodging: return "Dodging"; break;
			case PlayerMoveState::GetUp: return "GetUp"; break;
			case PlayerMoveState::Dying: return "Dying"; break;
			case PlayerMoveState::Dead: return "Dead"; break;
			case PlayerMoveState::UnDie: return "UnDie"; break;
			case PlayerMoveState::UnDying: return "UnDying"; break;

			default: return "default"; break;
			}
		}

		//Simulation
		void LastPositionSystem(flecs::entity e, SimPosition& lp)
		{
			lp.last_x = lp.x;
			lp.last_y = lp.y;
		}

		void FrictionSystem(flecs::entity e, Velocity& v, const Friction f)
		{
			v.x = Utility::LerpClamped(v.x, 0, _engine->fixedDeltatime * f.Value);
			v.y = Utility::LerpClamped(v.y, 0, _engine->fixedDeltatime * f.Value);

			if (Utility::AlmostZero(v.x, f.StopThreshold * _engine->fixedDeltatime))
				v.x = 0;
			if (Utility::AlmostZero(v.y, f.StopThreshold * _engine->fixedDeltatime))
				v.y = 0;
		}

		void VelocitySystem(flecs::entity e, SimPosition& p, const Velocity& v)
		{
			p.x += v.x * _engine->fixedDeltatime;
			p.y += v.y * _engine->fixedDeltatime;
		}

		void CopyToPhysicsSystem(flecs::entity, const PhysicsBody& b, const SimPosition& p, const Velocity& v)
		{
			b.Body->SetTransform(b2Vec2(p.x, p.y), 0);
			//b.Body->ApplyForce({ 1, 1 }, {}, true);
		}

		void CopyFromPhysicsSystem(flecs::entity, const PhysicsBody& b, SimPosition& p, const Velocity& v, const SimCopy& c)
		{
			const b2Vec2 simPos = b.Body->GetPosition();

			p.x = simPos.x;
			p.y = simPos.y;
		}

		void PlayerInputSystem(flecs::entity e, PlayerInput& input)
		{
			input.UpDown = 0;
			input.LeftRight = 0;
			input.Fire = false;
			input.Grenade = false;
			input.UsePowerup = false;
			input.Dodge = false;
			input.Activate = false;
			input.Reload = false;

			//xbox 
			//a = 0 
			//b = 1 
			//x = 2 
			//y = 3

			if (sf::Joystick::isConnected(0))
			{
				input.UpDown = Utility::SnapToZero(_engine->controllerYPos, 0.15f); //LS Y
				input.LeftRight = Utility::SnapToZero(_engine->controllerXPos, 0.15f); //LS X

				input.Fire = _engine->controllerZPos < 0; //RT

				input.Grenade = sf::Joystick::isButtonPressed(0, 5); //RB
				input.Dodge = sf::Joystick::isButtonPressed(0, 1); //B
				input.UsePowerup = sf::Joystick::isButtonPressed(0, 2); //X
				input.Activate = sf::Joystick::isButtonPressed(0, 0); //A
				input.Reload = sf::Joystick::isButtonPressed(0, 3); //Y
			}

			if (_engine->KeyState[sf::Keyboard::W])
				input.UpDown += -1.0f;

			if (_engine->KeyState[sf::Keyboard::S])
				input.UpDown += 1.0f;

			if (_engine->KeyState[sf::Keyboard::D])
				input.LeftRight += 1.0f;

			if (_engine->KeyState[sf::Keyboard::A])
				input.LeftRight += -1.0f;

			input.Fire = input.Fire || _engine->MouseButtonState[sf::Mouse::Left];
			input.Grenade = input.Grenade || _engine->KeyState[sf::Keyboard::LAlt] || _engine->MouseButtonState[sf::Mouse::Right];
			input.UsePowerup = input.UsePowerup || _engine->KeyState[sf::Keyboard::Q];
			input.Dodge = input.Dodge || _engine->KeyState[sf::Keyboard::Space];
			input.Activate = input.Activate || _engine->KeyState[sf::Keyboard::F];
			input.Reload = input.Reload || _engine->KeyState[sf::Keyboard::R];

			input.UpDown = Utility::Clamp(input.UpDown, -1.0f, 1.0f);
			input.LeftRight = Utility::Clamp(input.LeftRight, -1.0f, 1.0f);

		}

		void PlayerMovementSystem(flecs::entity e, const PlayerInput& input, PlayerMovement& mov, AnimationSet& ani, SimPosition& lp, Velocity& v, Friction& f)
		{

			bool WantToMove = (input.UpDown != 0 || input.LeftRight != 0);
			switch (mov.state)
			{
			default:
			case PlayerMoveState::Idle:
			{
				ani.index = mov.IsFacingLeft ? "idle-left" : "idle-right";

				//High friction when not moving
				f.Value = 20;
				f.StopThreshold = 1.0f;

				auto c = Utility::GetMouseWorldPos(_engine);
				mov.IsFacingLeft = c.x < lp.x;

				//User wants to move
				//Switch to move
				if (WantToMove)
				{
					mov.state = PlayerMoveState::Run;

					if (Utility::SnapToZero(input.LeftRight, 0.01f) != 0)
						mov.IsFacingLeft = input.LeftRight < 0;


					ani.index = mov.IsFacingLeft ? "run-left" : "run-right";
					return;
				}
			}
			break;
			case PlayerMoveState::IdleSpecial:
			{
				ani.index = mov.IsFacingLeft ? "idle-special-left" : "idle-special-right";
				//High friction when not moving
				f.Value = 20;
				f.StopThreshold = 1.0f;

				if (ani.elapsedTime > ani.timeUntilNext)
				{
					e.remove<AnimatedSpriteOnce>();
					e.set<AnimatedSprite>({ });
					mov.state = PlayerMoveState::Idle;

					ani.index = mov.IsFacingLeft ? "idle-left" : "idle-right";
					return;
				}

				//User wants to move
				//if user wants to dodge, override moving with a dodge start instead
				if (WantToMove && input.Dodge)
				{
					mov.state = PlayerMoveState::StartDodge;
					return;
				}
				//Switch to move
				//Cleanup special once animation component
				if (WantToMove)
				{
					e.remove<AnimatedSpriteOnce>();
					e.set<AnimatedSprite>({ });
					mov.state = PlayerMoveState::Run;

					if (Utility::SnapToZero(input.LeftRight, 0.01f) != 0)
						mov.IsFacingLeft = input.LeftRight < 0;


					ani.index = mov.IsFacingLeft ? "run-left" : "run-right";
					return;
				}
			}
			break;
			case PlayerMoveState::Run:
			{
				if (!WantToMove)
				{
					mov.state = PlayerMoveState::Idle;

					ani.index = mov.IsFacingLeft ? "idle-left" : "idle-right";

					if (Utility::Random01() > 0.70f)
					{
						if (ani.animations.find("idle-special-right") != ani.animations.end())
						{
							e.remove<AnimatedSprite>();
							e.set<AnimatedSpriteOnce>({ });
							mov.state = PlayerMoveState::IdleSpecial;

							auto sheet = ani.animations["idle-special-right"];
							f32 time = sheet->length * sheet->timePerFrame;
							ani.timeUntilNext = time;


							ani.index = mov.IsFacingLeft ? "idle-special-left" : "idle-special-right";
							//printf("Playing Special idle for %fs\n", time);
						}
						else
						{
							//no special animation, just change to normal idle
							ani.timeUntilNext = 0;
						}
					}
					return;
				}

				if (Utility::SnapToZero(input.LeftRight, 0.01f) != 0)
					mov.IsFacingLeft = input.LeftRight < 0;

				ani.index = mov.IsFacingLeft ? "run-left" : "run-right";

				//When moving lower friction
				f.Value = 6.0f;
				f.StopThreshold = 0.0f;

				//scale back velocity as it reaches our move speed
				f32 distToMaxX = Utility::Clamp01(input.MaxMoveSpeed - abs(v.x));
				f32 distToMaxY = Utility::Clamp01(input.MaxMoveSpeed - abs(v.y));

				v.x += input.LeftRight * 800.0f * (f32)_engine->fixedDeltatime * distToMaxX;
				v.y += input.UpDown * 800.0f * (f32)_engine->fixedDeltatime * distToMaxY;

				//if user wants to dodge go to that next
				if (WantToMove && input.Dodge)
				{
					mov.state = PlayerMoveState::StartDodge;
					return;
				}
			}
			break;
			case PlayerMoveState::StartDodge:
			{
				printf("start dodge \n");
				//setup animation for play once
				e.remove<AnimatedSprite>();
				e.set<AnimatedSpriteOnce>({ });

				f32 time = 0.5f;
				if (ani.animations.find("dodge-right") != ani.animations.end())
				{
					auto sheet = ani.animations["dodge-right"];
					time = sheet->length * sheet->timePerFrame;
					ani.timeUntilNext = time;
				}
				ani.index = mov.IsFacingLeft ? "dodge-left" : "dodge-right";

				//change state to dodging
				mov.state = PlayerMoveState::Dodging;

				//Add velocity in dodge dir
				mov.dodgeX = input.LeftRight * 150;
				mov.dodgeY = input.UpDown * 150;

				v.x = Utility::Lerp(v.x, 0, 0.75f);
				v.y = Utility::Lerp(v.y, 0, 0.75f);

				//fall though into normal state
				[[fallthrough]];
			}
			case PlayerMoveState::Dodging:
			{
				ani.index = mov.IsFacingLeft ? "dodge-left" : "dodge-right";

				f.Value = 3.0f;
				f.StopThreshold = 0.0f;

				//wait until x time passed
				//then apply force
				if (ani.elapsedTime > 0.2f)
				{
					v.x += mov.dodgeX;
					v.y += mov.dodgeY;

					mov.dodgeX = 0;
					mov.dodgeY = 0;
				}

				//Cant get up till animation is over
				//And player stopped moving (almost)
				if (ani.elapsedTime >= ani.timeUntilNext)
				{
					if (Utility::AlmostZero(v.x, 5) &&
						Utility::AlmostZero(v.y, 5))
					{
						//printf("dodge animation is over and player is almost stopped\n");
						mov.state = PlayerMoveState::GetUp;
						ani.index = mov.IsFacingLeft ? "getup-left" : "getup-right";

						//Clear the playback componenet
						e.set<AnimatedSpriteOnce>({ });
						f32 time = 0.5f;
						if (ani.animations.find("getup-right") != ani.animations.end())
						{
							auto sheet = ani.animations["getup-right"];
							time = sheet->length * sheet->timePerFrame;
							ani.timeUntilNext = time;
							ani.elapsedTime = 0;
						}
					}
				}
			}
			break;
			case PlayerMoveState::GetUp:
			{


				ani.index = mov.IsFacingLeft ? "getup-left" : "getup-right";

				if (ani.elapsedTime >= ani.timeUntilNext)
				{
					//printf("getup animation is over, go to idle\n");
					e.remove<AnimatedSpriteOnce>();
					e.set<AnimatedSprite>({ });
					mov.state = PlayerMoveState::Idle;
					ani.index = mov.IsFacingLeft ? "idle-left" : "idle-right";
				}
			}
			break;
			case PlayerMoveState::Dying:
			{
				if (e.has<AnimatedSprite>())
					e.remove<AnimatedSprite>();

				e.set<AnimatedSpriteOnce>({ });

				f32 time = 1;
				if (ani.animations.find("die-left") != ani.animations.end())
				{
					auto sheet = ani.animations["die-left"];
					time = sheet->length * sheet->timePerFrame;
				}
				ani.timeUntilNext = time;

				ani.index = mov.IsFacingLeft ? "die-left" : "die-right";
				//printf("going to dead state %fs\n", time);

				//set state to dead
				mov.state = PlayerMoveState::Dead;
				return;
			}
			break;
			case PlayerMoveState::Dead:
			{
			}
			break;
			case PlayerMoveState::UnDie:
			{
				e.remove<AnimatedSprite>();
				e.set<AnimatedSpriteOnce>({ });

				f32 time = 1;
				if (ani.animations.find("undie-left") != ani.animations.end())
				{
					auto sheet = ani.animations["undie-left"];
					time = sheet->length * sheet->timePerFrame;
				}
				ani.timeUntilNext = time;

				ani.index = mov.IsFacingLeft ? "undie-left" : "undie-right";
				//printf("going to alive state %fs\n", time);

				mov.state = PlayerMoveState::UnDying;
			}
			break;
			case PlayerMoveState::UnDying:
			{
				if (ani.elapsedTime > ani.timeUntilNext)
				{
					e.remove<AnimatedSpriteOnce>();
					e.set<AnimatedSprite>({ });
					mov.state = PlayerMoveState::Idle;

					ani.index = mov.IsFacingLeft ? "idle-left" : "idle-right";
					return;
				}
			}
			break;
			}
		}

		void PlayerCombatSystem(flecs::entity e, const PlayerInput& input, PlayerMovement& move, Health& h, GunCombat& combat, SimPosition& pos)
		{
			if (input.Fire)
			{
				//printf("FIRE!\n");
			}
			if (input.Grenade)
			{
				//printf("Grenade!\n");
			}
			if (input.UsePowerup)
			{
				//printf("Powerup!\n");
			}

			if (h.Value <= 0)
			{
				if (!(move.state == PlayerMoveState::Dead ||
					move.state == PlayerMoveState::Dying))
				{
					//just became dead
					move.state = PlayerMoveState::Dying;
				}

				//Dead players cant shoot

				//Powerup revive
				if (input.UsePowerup)
				{
					move.state = PlayerMoveState::UnDie;
					h.Value = 1000;
					combat.Bullets = combat.MagBullets;
					combat.gunState = GunState::Idle;
					combat.reloadTimer = 0;
				}

				return;
			}

			switch (combat.gunState)
			{
			case GunState::Idle:
			{
				combat.fireTimer -= _engine->fixedDeltatime;
				if (combat.fireTimer <= 0)
				{
					if (input.Fire &&
						combat.Bullets > 0)
					{
						combat.Bullets--;
						combat.fireTimer = 0.10f;

						auto c = Utility::GetMouseWorldPos(_engine);
						move.IsFacingLeft = c.x < pos.x;

						auto dirxy = Utility::GetLookToDirection({ pos.x, pos.y }, { c.x, c.y });
						f32 gx = pos.x + dirxy.x * -9;
						f32 gy = pos.y + dirxy.y * -9 + 4;

						auto vxy = Utility::GetLookDirection(_engine, { gx, gy });
						f32 ang = Utility::AngleDeg({ gx, gy }, c);

						char* buff = (char*)calloc(120, sizeof(char));
						if (buff == nullptr)
							abort();
						sprintf(buff, "(spawn-bullet %f %f %f %f %f)", gx, gy, vxy.x, vxy.y, ang);
						Scheme::Eval(_engine, buff);
						free(buff);
					}
				}
				if (input.Reload)
				{
					combat.gunState = GunState::Reloading;
					combat.reloadTimer = 1.0f;
					combat.Bullets = 0;
				}
			}
			break;
			case GunState::Reloading:
			{
				combat.reloadTimer -= _engine->fixedDeltatime;
				if (combat.reloadTimer <= 0)
				{
					combat.Bullets = combat.MagBullets;
					combat.gunState = GunState::Idle;
					combat.reloadTimer = 0;
				}
			}
			break;
			default:
				break;
			}
		}

		//NPCs
		void NPCMovementSystem(flecs::entity e, AIState& ai, NPCMovement& move, AnimationSet& ani, SimPosition& lp, Velocity& v, Friction& f)
		{
			switch (move.state)
			{
			default:
			case NPCMoveState::Idle:
			{
				//do nothing

				//When not moving increase friction
				f.Value = 20.0f;
				f.StopThreshold = 1.0f;

				ani.index = move.IsFacingLeft ? "idle-left" : "idle-right";
			}
			break;
			case NPCMoveState::Run:
			{

				move.IsFacingLeft = v.x < 0;

				//When moving lower friction
				f.Value = 6.0f;
				f.StopThreshold = 0.0f;

				//scale back velocity as it reaches our move speed
				f32 distToMaxX = Utility::Clamp01(move.speed - abs(v.x));
				f32 distToMaxY = Utility::Clamp01(move.speed - abs(v.y));

				v.x += move.targetX * 800.0f * (f32)_engine->fixedDeltatime * distToMaxX;
				v.y += move.targetY * 800.0f * (f32)_engine->fixedDeltatime * distToMaxY;

				//move towards ai target
				ani.index = move.IsFacingLeft ? "run-left" : "run-right";
			}
			break;
			case NPCMoveState::Attack:
			{
				//Stuck in place during attack
				ani.index = move.IsFacingLeft ? "attack-left" : "attack-right";

				//When not moving increase friction
				f.Value = 20.0f;
				f.StopThreshold = 1.0f;
			}
			break;
			case NPCMoveState::Dying:
			{
				//no movement
				//display death animation
				//go to dead state
				move.state = NPCMoveState::Dead;
				ani.index = move.IsFacingLeft ? "dying-left" : "dying-right";


				//When not moving increase friction
				f.Value = 20.0f;
				f.StopThreshold = 1.0f;
			}
			break;
			case NPCMoveState::Dead:
			{
				//dead monsters do nothing
				//unless we add reving, which this would have a exit
				ani.index = move.IsFacingLeft ? "dead-left" : "dead-right";


				//When not moving increase friction
				f.Value = 20.0f;
				f.StopThreshold = 1.0f;
			}
			break;
			}
		}

		bool GetFirstPlayerInRangeAroundPoint(flecs::entity self, f32 x, f32 y, f32 range, flecs::entity** found)
		{
			if (Physics::CircleRayIntersect({ x, y }, range))
			{
				u32 count = _engine->Physics->QueryCount;
				//printf("%i overlaps \n", count);
				for (u32 i = 0; i < count; i++)
				{
					auto b = _engine->Physics->LastQuery[i].Body;
					auto ent = GetEntityFromBody(b);

					//ignore self
					if (ent->id() != self.id())
					{
						if (ent->has<PlayerInput>() &&
							ent->has<SimPosition>())
						{
							//printf("Found player entity \n");
							*found = ent;
							return true;
						}
					}
				}
			}
			return false;
		}

		void NPCCombatSystem(flecs::entity e, AIState& ai, NPCMovement& move, AnimationSet& ani, SimPosition& p, NPCCombat& combat, Health& h)
		{
			if (h.Value <= 0)
			{
				char* buff = (char*)calloc(120, sizeof(char));
				if (buff == nullptr)
					abort();
				sprintf(buff, "(spawn-gibs %f %f)", p.x, p.y);
				Scheme::Eval(_engine, buff);
				free(buff);

				e.destruct();
				return;
			}



			//Get path
			//std::vector<sf::Vector2f> path = AI::GeneratePath(_engine, sf::Vector2f(-99, -99), sf::Vector2f(99, 99));

			combat.accumlator += _engine->fixedDeltatime;

			switch (move.state)
			{
			default:
			case NPCMoveState::Idle:
			case NPCMoveState::Run:
			{
				combat.haveTarget = GetFirstPlayerInRangeAroundPoint(e, p.x, p.y, combat.range, &combat.target);
				if (combat.haveTarget)
				{
					auto otherPos = combat.target->get<SimPosition>();
					auto dir = Utility::GetLookToDirection({ p.x, p.y }, { otherPos->x, otherPos->y });
					move.range = Utility::Distance({ p.x, p.y }, { otherPos->x, otherPos->y });
					//printf("range %f \n", move.range);
					if (move.range > combat.attackRange)
					{
						move.targetX = -dir.x;
						move.targetY = -dir.y;
					}
					else
					{
						move.targetX = dir.x;
						move.targetY = dir.y;
					}
					move.state = NPCMoveState::Run;

					if (move.range <= combat.attackRange && combat.accumlator >= combat.attackTime)
					{
						move.state = NPCMoveState::Attack;
						combat.accumlator = 0;

						e.remove<AnimatedSprite>();
						e.set<AnimatedSpriteOnce>({ });
					}
				}
				else
				{
					move.state = NPCMoveState::Idle;
				}
			}
			break;
			case NPCMoveState::Attack:
			{
				//dont look for new target
				//just follow though attack

				if (!combat.didAttack && combat.accumlator >= combat.attackDamageTime)
				{
					//do damage
					//printf("do damage point\n");
					combat.didAttack = true;


					if (Physics::CircleRayIntersect({ p.x, p.y }, combat.range))
					{
						u32 count = _engine->Physics->QueryCount;
						//printf("%i overlaps \n", count);
						for (u32 i = 0; i < count; i++)
						{
							auto b = _engine->Physics->LastQuery[i].Body;
							auto ent = GetEntityFromBody(b);

							//ignore self
							if (ent->id() != e.id())
							{
								if (ent->has<Health>() &&
									ent->has<SimPosition>())
								{
									auto otherPos = ent->get<SimPosition>();
									move.range = Utility::Distance({ p.x, p.y }, { otherPos->x, otherPos->y });
									auto dir = Utility::GetLookToDirection({ p.x, p.y }, { otherPos->x, otherPos->y });
									f32 r = Utility::AngleDeg({ p.x, p.y }, { otherPos->x, otherPos->y });

									if (move.range < combat.attackRange)
									{
										//printf("in range for damage:%f \n", move.range);
										auto otherHealth = ent->get_mut<Health>();
										otherHealth->Value -= combat.damage;

										//spawn splatter
										if (_engine->particlesThisFrame < _engine->maxParticlesPerFrame)
										{
											_engine->particlesThisFrame++;
											char* buff = (char*)calloc(200, sizeof(char));
											if (buff == nullptr)
												abort();
											sprintf(buff, "(spawn-blood-spray %f %f %f %f %f)", otherPos->x, otherPos->y, dir.x, dir.y, r);
											Scheme::Eval(_engine, buff);
											free(buff);
										}
										//spawn hit splash
										if (_engine->particlesThisFrame < _engine->maxParticlesPerFrame)
										{
											_engine->particlesThisFrame++;
											char* buff = (char*)calloc(200, sizeof(char));
											if (buff == nullptr)
												abort();
											sprintf(buff, "(spawn-hit-splash %f %f %f %f %f)", otherPos->x, otherPos->y, dir.x, dir.y, r);
											Scheme::Eval(_engine, buff);
											free(buff);
										}
									}
								}
							}
						}
					}
				}
				

				if (combat.accumlator > combat.attackTime)
				{
					//done attacking, go back to idle
					move.state = NPCMoveState::Idle;
					combat.didAttack = false;
					e.remove<AnimatedSpriteOnce>();
					e.set<AnimatedSprite>({ });
				}
			}
			break;
			case NPCMoveState::Dying:
			{
			}
			break;
			case NPCMoveState::Dead:
			{
			}
			break;
			}
		}

		void AnimationSetSystem(flecs::entity e, AnimationSet& set)
		{
			if (set.lastIndex != set.index)
			{
				set.elapsedTime = 0;

				if (_engine->ShowPlayerDebug)
					printf("Change state from %s to %s\n", set.lastIndex.c_str(), set.index.c_str());

				//If the hash set has the key our index is set to, lets use that
				//otherwise use the default
				if (set.animations.find(set.index) != set.animations.end())
					set.current = set.animations[set.index];
				else
					set.current = set.default;
			}
			set.elapsedTime += _engine->fixedDeltatime;
			set.lastIndex = set.index;
		}

		//Update
		void TickInterpolateSystem(flecs::entity e, Position& p, const SimPosition& lp)
		{
			p.x = Utility::Lerp(lp.last_x, lp.x, _engine->tickPercent);
			p.y = Utility::Lerp(lp.last_y, lp.y, _engine->tickPercent);
		}

		void UpdateAnimatedSpriteSystem(flecs::entity e, SpriteComponent& sc, AnimatedSprite& ani)
		{
			ani.accumlator += _engine->deltatime;
			while (ani.accumlator >= ani.sprites->timePerFrame)
			{
				ani.accumlator -= ani.sprites->timePerFrame;

				if (ani.currentFrame == ani.sprites->length - 2)
				{
					//was second to last frame
					//is now last frame
					if (ani.haveCThunk)
					{
						printf("[Animation] Last frame event, have c thunk:%s \n", ani.haveCThunk ? "true" : "false");

						(*ani.c_thunk)(&e);
					}
					if (ani.haveThunk)
					{
						printf("[AnimationOnce] Last frame event, have scheme thunk:%s \n", ani.haveThunk ? "true" : "false");

						s7_call(_engine->Scheme.s7, ani.thunk,
							s7_cons(_engine->Scheme.s7,
								s7_make_c_pointer(_engine->Scheme.s7, &e),
								s7_nil(_engine->Scheme.s7)));
					}
				}
				ani.currentFrame++;
				if (ani.sprites->length <= 1)
					ani.currentFrame = 0;
				else
					ani.currentFrame %= ani.sprites->length - 1;

			}
			ani.currentFrame = Utility::Clamp(ani.currentFrame, 0, ani.sprites->length - 1);

			sc.sprite = ani.sprites->sprites[ani.currentFrame];

			sc.offsetX = ani.sprites->offsetX;
			sc.offsetY = ani.sprites->offsetY;

			//printf("%lld ani time:%f frame:%i\n", _engine->tick, ani.accumlator, ani.currentFrame);
		}


		void UpdateAnimatedSpriteOnceSystem(flecs::entity e, SpriteComponent& sc, AnimatedSpriteOnce& ani)
		{
			ani.accumlator += _engine->deltatime;
			while (ani.accumlator >= ani.sprites->timePerFrame)
			{
				ani.accumlator -= ani.sprites->timePerFrame;

				//printf("%lld %i\n", _engine->tick, ani.currentFrame);
				if (ani.currentFrame == ani.sprites->length - 2)
				{
					//was second to last frame
					//is now last frame
					if (ani.haveCThunk)
					{
						printf("[Animation] Last frame event, have c thunk:%s \n", ani.haveCThunk ? "true" : "false");

						(*ani.c_thunk)(&e);
					}
					if (ani.haveThunk)
					{
						printf("[AnimationOnce] Last frame event, have scheme thunk:%s \n", ani.haveThunk ? "true" : "false");

						s7_call(_engine->Scheme.s7, ani.thunk,
							s7_cons(_engine->Scheme.s7,
								s7_make_c_pointer(_engine->Scheme.s7, &e),
								s7_nil(_engine->Scheme.s7)));
					}
				}
				ani.currentFrame++;
			}
			ani.currentFrame = Utility::Clamp(ani.currentFrame, 0, ani.sprites->length - 1);

			sc.sprite = ani.sprites->sprites[ani.currentFrame];

			sc.offsetX = ani.sprites->offsetX;
			sc.offsetY = ani.sprites->offsetY;

			//printf("%lld ani time:%f frame:%i\n", _engine->tick, ani.accumlator, ani.currentFrame);
		}

		void UpdateSpriteSystem(flecs::entity e, const Bounds& b, const Position& p, const SpriteComponent& sc)
		{
			sc.sprite->setPosition(p.x + sc.offsetX, p.y + sc.offsetY);
			sc.sprite->setOrigin(b.width / 2, b.height / 2);
		}

		void UpdateSpriteRotationSystem(flecs::entity e, const Rotation& r, const SpriteComponent& sc)
		{
			sc.sprite->setRotation(r.value);
		}

		void UpdateRenderableSystem(flecs::entity e, const Bounds& b, const Position& p, const Renderable& r)
		{
			r.drawable->setPosition(p.x, p.y);
			r.drawable->setOrigin(b.width / 2, b.height / 2);
		}

		//Rendering
		void DrawSystem(flecs::entity e, Renderable& r)
		{
			auto xy = r.drawable->getPosition();
			f32 y = xy.y - _engine->cameraY;
			r.depth = (s32)y - _engine->ScaledWorldY / 2 + r.depthOffset;

			Rendering::AddDrawable(_engine->Rendering, r.drawable, r.depth);
		}

		void DrawSpriteSystem(flecs::entity e, SpriteComponent& sc)
		{
			auto xy = sc.sprite->getPosition();
			f32 x = xy.x - sc.offsetX;
			f32 y = xy.y - _engine->cameraY - sc.offsetY;
			sc.depth = (s32)y - _engine->ScaledWorldY / 2 + sc.depthOffset;

			Rendering::AddDrawable(_engine->Rendering, sc.sprite, sc.depth);
			//printf("cameray:%f screenheight:%f y:%f z:%i\n", _engine->cameraY, _engine->ScaledWorldY, y, sc.depth);
		}

		void CameraFocusSystem(flecs::entity e, const Position& p, const CameraFocus& cf)
		{
			_engine->cameraX = p.x + cf.offsetX - _engine->ScaledWorldX / 2;
			_engine->cameraY = p.y + cf.offsetY - _engine->ScaledWorldY / 2;
		}

		void PlayerInfoSystem(flecs::entity e, const PlayerInput& input, const PlayerMovement& mov, const AnimationSet& ani, const SpriteComponent& sc)
		{
			if (!_engine->ShowPlayerDebug) return;

			char* buff = (char*)calloc(200, sizeof(char));
			if (buff == nullptr)
				abort();
			sprintf(buff, "state: %s", playerMoveStateString(mov.state));
			Text(buff, UI::Rect(500, 0, 200, 30), UI::White, nullptr, _engine->GUIState->Fonts[0]);

			memset(buff, 0, 200);
			sprintf(buff, "ani state: %s %f %f offset:%f, %f", ani.index.c_str(), ani.elapsedTime, ani.timeUntilNext, sc.offsetX, sc.offsetY);
			Text(buff, UI::Rect(500, 0, 400, 30), UI::White, nullptr, _engine->GUIState->Fonts[0]);

			memset(buff, 0, 200);
			auto xy = Utility::GetMouseWorldPos(_engine);
			sprintf(buff, "mouse:%i, %i  world:%f %f", _engine->MouseX, _engine->MouseY, xy.x, xy.y);
			Text(buff, UI::Rect(500, 100, 400, 30), UI::White, nullptr, _engine->GUIState->Fonts[0]);

			free(buff);
		}

		void PlayerHudHealthSystem(flecs::entity e, const PlayerInput& input, const Health& h)
		{
			char* buff = (char*)calloc(120, sizeof(char));
			if (buff == nullptr)
				abort();
			sprintf(buff, "Health:%0.f", floorf(h.Value));
			Text(buff, UI::Rect(0, 0, 200, 30), UI::White, nullptr, _engine->GUIState->Fonts[0]);
			free(buff);
		}

		SpriteSheet* MakeSpriteSheet(std::vector<sf::Sprite*> sprites, f32 timePerFrame, f32 offsetX, f32 offsetY)
		{
			sf::Sprite** ani = (sf::Sprite**)calloc((u32)sprites.size(), sizeof(sf::Sprite*));
			if (ani == nullptr)
				abort();

			for (u32 i = 0; i < sprites.size(); i++)
			{
				ani[i] = sprites[i];
			}

			SpriteSheet* sheet = (SpriteSheet*)calloc(1, sizeof(SpriteSheet));
			if (sheet == nullptr)
				abort();

			*sheet = {
				ani, (u32)sprites.size(), timePerFrame, offsetX, offsetY
			};

			return sheet;
		}

		void LevelEditSystem(flecs::entity e, LevelEdit& edit, Position& p)
		{
			s32 ww = _engine->Window->getSize().x * _engine->UIScale;
			s32 wh = _engine->Window->getSize().y * _engine->UIScale;

			auto world = _engine->ECSWorld.world;

			char* buff = (char*)calloc(200, sizeof(char));
			if (buff == nullptr)
				abort();

			f32 UpDown = 0;
			f32 LeftRight = 0;

			if (_engine->KeyState[sf::Keyboard::W])
				UpDown += -1.0f;

			if (_engine->KeyState[sf::Keyboard::S])
				UpDown += 1.0f;

			if (_engine->KeyState[sf::Keyboard::D])
				LeftRight += 1.0f;

			if (_engine->KeyState[sf::Keyboard::A])
				LeftRight += -1.0f;

			p.x += LeftRight * 80 * _engine->deltatime;
			p.y += UpDown * 80 * _engine->deltatime;

			if (_engine->KeyPressedState[sf::Keyboard::E])
			{
				printf("export\n");

				auto q = world->query_builder<SchemeMarker, Position>()
					.build();

				std::ofstream myfile;
				myfile.open("export.scm");


				q.each([&](flecs::entity e, SchemeMarker& marker, Position& pos)
					{
						memset(buff, 0, 200);
						sprintf(buff, marker.formatString.c_str(), pos.x, pos.y);
						printf(buff);
						printf("\n");

						myfile << buff << "\n";
					});

				myfile.close();
			}


			auto xy = Utility::GetMouseWorldPos(_engine);
			auto txy = Utility::GetMouseTilePos(_engine);
			if (_engine->MouseButtonState[sf::Mouse::Left])
			{
				memset(buff, 0, 200);
				sprintf(buff, "(editor-place %f %f %f %f)", xy.x, xy.y, txy.x, txy.y);
				Scheme::Eval(_engine, buff);
			}
			if (_engine->MouseButtonState[sf::Mouse::Right])
			{
				memset(buff, 0, 200);
				sprintf(buff, "(editor-pick %f %f %f %f)", xy.x, xy.y, txy.x, txy.y);
				Scheme::Eval(_engine, buff);
			}
			if (_engine->KeyPressedState[sf::Keyboard::Z])
			{
				Scheme::Eval(_engine, "(editor-undo)");
			}
			if (_engine->MouseScrollwheel > 0.1f)
			{
				Scheme::Eval(_engine, "(editor-scroll-up)");
			}
			if (_engine->MouseScrollwheel < -0.1f)
			{
				Scheme::Eval(_engine, "(editor-scroll-down)");
			}


			free(buff);
		}


		void Init(Unstable::Engine* engine)
		{
			_engine = engine;
			flecs::world* ecs = new flecs::world();
			engine->ECSWorld.world = ecs;

			//Fixed update
			//Start of fixed update, move position in simulation positions from current to last
			flecs::entity sys = ecs->system<SimPosition>()
				.each(LastPositionSystem);
			engine->ECSWorld.PrePhysicsSystem.push_back(sys);

			//Player
			{
				sys = ecs->system<PlayerInput>()
					.each(PlayerInputSystem);
				engine->ECSWorld.PrePhysicsSystem.push_back(sys);

				sys = ecs->system<const PlayerInput, PlayerMovement, AnimationSet, SimPosition, Velocity, Friction>()
					.each(PlayerMovementSystem);
				engine->ECSWorld.PrePhysicsSystem.push_back(sys);

				sys = ecs->system<const PlayerInput, PlayerMovement, Health, GunCombat, SimPosition>()
					.each(PlayerCombatSystem);
				engine->ECSWorld.PrePhysicsSystem.push_back(sys);


				//gun rotation
				engine->ECSWorld.UpdateSystems.push_back(ecs->system<const Position, Rotation, SpriteComponent, const GunCombat>()
					.each([](const Position& p, Rotation& r, SpriteComponent& cs, const GunCombat& g)
						{
							auto c = Utility::GetMouseWorldPos(_engine);

							auto cc = Utility::GetMouseWorldPos(_engine);
							bool left = cc.x < p.x;
							r.value = Utility::AngleDeg({ p.x, p.y + 6 }, { c.x, c.y });

							if (left)
								r.value += 180;

							cs.sprite->setScale({ left ? -1.0f : 1.0f, 1.0f });
						}));

				engine->ECSWorld.UpdateSystems.push_back(ecs->system<const Position, const Rotation, const GunCombat, SpriteComponent>()
					.each([](const Position& p, const Rotation& i, const GunCombat& g, SpriteComponent& c)
						{
							auto cc = Utility::GetMouseWorldPos(_engine);
							bool left = cc.x < p.x;

							c.offsetX = left ? -2 : 2;
							c.offsetY = 6;
						}));


				sys = ecs->system<const PlayerAttachment, const PlayerMovement>()
					.each([](const PlayerAttachment& att, const PlayerMovement& move)
						{
							switch (move.state)
							{
							case PlayerMoveState::Idle:
							case PlayerMoveState::Run:
							{
								//auto sp = att.ent->get_mut<SpriteComponent>();
							}
							break;
							default:
							{
								auto sp = att.ent->get_mut<SpriteComponent>();
								sp->sprite->setScale({ 0, 0 });
							}
							break;
							}
						});
				engine->ECSWorld.UpdateSystems.push_back(sys);
			}

			//NPCs
			{
				sys = ecs->system<AIState, NPCMovement, AnimationSet, SimPosition, Velocity, Friction>()
					.each(NPCMovementSystem);
				engine->ECSWorld.PrePhysicsSystem.push_back(sys);


				sys = ecs->system<AIState, NPCMovement, AnimationSet, SimPosition, NPCCombat, Health>()
					.each(NPCCombatSystem);
				engine->ECSWorld.PrePhysicsSystem.push_back(sys);

			}

			sys = ecs->system<const PhysicsBody, const SimPosition, const Velocity>()
				.each(CopyToPhysicsSystem);
			engine->ECSWorld.PrePhysicsSystem.push_back(sys);

			//Cache all physics bodies for easier lookup by AI pathfinding
			engine->ECSWorld.PrePhysicsSystem.push_back(ecs->system<const PhysicsBody>()
				.each([](const PhysicsBody& p)
					{
						_engine->allBodies.push_back(p.Body);
					}));


			//Post physics tick
			sys = ecs->system<const PhysicsBody, SimPosition, const Velocity, const SimCopy>()
				.each(CopyFromPhysicsSystem);
			engine->ECSWorld.PostPhysicsSystem.push_back(sys);

			sys = ecs->system<Velocity, const Friction>()
				.each(FrictionSystem);
			engine->ECSWorld.PostPhysicsSystem.push_back(sys);

			sys = ecs->system<SimPosition, const Velocity>()
				.each(VelocitySystem);
			engine->ECSWorld.PostPhysicsSystem.push_back(sys);

			engine->ECSWorld.PostPhysicsSystem.push_back(ecs->system<const SimPosition, const PhysicsBody, const OverlapDamage>()
				.each([](flecs::entity e, const SimPosition& pos, const PhysicsBody& b, const OverlapDamage& over)
					{
						if (Physics::CircleRayIntersect({ pos.x, pos.y }, over.radius))
						{
							u32 count = _engine->Physics->QueryCount;
							//printf("%i overlaps \n", count);
							for (u32 i = 0; i < count; i++)
							{
								auto b = _engine->Physics->LastQuery[i].Body;
								auto ent = GetEntityFromBody(b);

								if (ent->id() != over.ignoreEnt->id())
								{
									//printf("Overlap other entity \n");
									auto bpos = b->GetPosition();
									auto dir = Utility::GetLookToDirection({ pos.x, pos.y }, { bpos.x, bpos.y });
									f32 r = Utility::AngleDeg({ pos.x, pos.y }, { bpos.x, bpos.y });

									//ignore player (hack since we want this to work on player later)
									if (!ent->has<PlayerInput>())
									{
										if (ent->has<Health>())
										{
											//printf("entity has health \n");
											auto h = ent->get_mut<Health>();
											h->Value -= over.damage;

											//spawn splatter
											if (_engine->particlesThisFrame < _engine->maxParticlesPerFrame)
											{
												_engine->particlesThisFrame++;
												char* buff = (char*)calloc(200, sizeof(char));
												if (buff == nullptr)
													abort();
												sprintf(buff, "(spawn-blood-spray %f %f %f %f %f)", pos.x, pos.y, dir.x, dir.y, r);
												Scheme::Eval(_engine, buff);
												free(buff);
											}
											//spawn hit splash
											if (_engine->particlesThisFrame < _engine->maxParticlesPerFrame)
											{
												_engine->particlesThisFrame++;
												char* buff = (char*)calloc(200, sizeof(char));
												if (buff == nullptr)
													abort();
												sprintf(buff, "(spawn-hit-splash %f %f %f %f %f)", pos.x, pos.y, dir.x, dir.y, r);
												Scheme::Eval(_engine, buff);
												free(buff);
											}
											e.destruct();
										}
										else
										{
											//no health, lets flick it, if its dynamic
											printf("impulse %f %f\n", dir.x, dir.y);
											b->ApplyForce({ dir.x * 10000, dir.y * 10000 }, { 0, 0 }, true);

											//spawn hit splash
											if (_engine->particlesThisFrame < _engine->maxParticlesPerFrame)
											{
												_engine->particlesThisFrame++;
												char* buff = (char*)calloc(200, sizeof(char));
												if (buff == nullptr)
													abort();
												sprintf(buff, "(spawn-hit-splash %f %f %f %f %f)", pos.x, pos.y, dir.x, dir.y, r);
												Scheme::Eval(_engine, buff);
												free(buff);
											}
										}
									}
								}
								else
								{
									//printf("Ignore entity\n");
								}
							}
						}
					}));

			//Fixed update
			//Update animation set selection
			//copy sheet into animated sprite components
			{
				sys = ecs->system<AnimationSet>()
					.each(AnimationSetSystem);
				engine->ECSWorld.PostPhysicsSystem.push_back(sys);

				engine->ECSWorld.PostPhysicsSystem.push_back(ecs->system<const AnimationSet, AnimatedSpriteOnce>()
					.each([](const AnimationSet& set, AnimatedSpriteOnce& sprite)
						{
							sprite.sprites = set.current;
						}));

				engine->ECSWorld.PostPhysicsSystem.push_back(ecs->system<const AnimationSet, AnimatedSprite>()
					.each([](const AnimationSet& set, AnimatedSprite& sprite)
						{
							sprite.sprites = set.current;
						}));
			}

			engine->ECSWorld.PostPhysicsSystem.push_back(ecs->system<DestroyTimer>()
				.each([](flecs::entity e, DestroyTimer& destroy)
					{
						destroy.Elapsed += _engine->fixedDeltatime;
						if (destroy.Elapsed > destroy.Value)
							e.destruct();
					}));

			engine->ECSWorld.PostPhysicsSystem.push_back(ecs->system<SpawnTimer, const SimPosition>()
				.each([](flecs::entity e, SpawnTimer& spawn, const SimPosition& pos)
					{
						spawn.Elapsed += _engine->fixedDeltatime;
						if (spawn.Elapsed > spawn.Value)
						{
							spawn.Elapsed = 0;

							//printf("spawning prefab entity at %f, %f\n", pos.x, pos.y);

							char* buff = (char*)calloc(120, sizeof(char));
							if (buff == nullptr)
								abort();
							sprintf(buff, "(%s %f %f)", spawn.spawn.c_str(), pos.x + 4, pos.y - 8);
							Scheme::Eval(_engine, buff);
							free(buff);

							/*auto se = _engine->ECSWorld.world->entity(*spawn.prefab);
							if (se.has<SimPosition>())
								se.set<SimPosition>({ pos.x, pos.y });
							if (se.has<Position>())
								se.set<Position>({ pos.x, pos.y });*/
						}
					}));

			//Begining of update
			sys = ecs->system<Position, const SimPosition>()
				.each(TickInterpolateSystem);
			engine->ECSWorld.UpdateSystems.push_back(sys);

			//Make use of animated sprite components, and put sprite objects into rendering
			//This will crash if the sprite* is invalid
			{
				sys = ecs->system<SpriteComponent, AnimatedSprite>()
					.each(UpdateAnimatedSpriteSystem);
				engine->ECSWorld.UpdateSystems.push_back(sys);

				sys = ecs->system<SpriteComponent, AnimatedSpriteOnce>()
					.each(UpdateAnimatedSpriteOnceSystem);
				engine->ECSWorld.UpdateSystems.push_back(sys);
			}

			sys = ecs->system<const Bounds, const Position, const SpriteComponent>()
				.each(UpdateSpriteSystem);
			engine->ECSWorld.UpdateSystems.push_back(sys);

			sys = ecs->system<const Rotation, const SpriteComponent>()
				.each(UpdateSpriteRotationSystem);
			engine->ECSWorld.UpdateSystems.push_back(sys);

			sys = ecs->system<const Bounds, const Position, const Renderable>()
				.each(UpdateRenderableSystem);
			engine->ECSWorld.UpdateSystems.push_back(sys);

			sys = ecs->system<const Position, const CameraFocus>()
				.each(CameraFocusSystem);
			engine->ECSWorld.UpdateSystems.push_back(sys);

			sys = ecs->system<Renderable>()
				.each(DrawSystem);
			engine->ECSWorld.UpdateSystems.push_back(sys);

			sys = ecs->system<SpriteComponent>()
				.each(DrawSpriteSystem);
			engine->ECSWorld.UpdateSystems.push_back(sys);

			sys = ecs->system<const PlayerInput, const PlayerMovement, const AnimationSet, const SpriteComponent>()
				.each(PlayerInfoSystem);
			engine->ECSWorld.UpdateSystems.push_back(sys);

			sys = ecs->system<const PlayerInput, const Health>()
				.each(PlayerHudHealthSystem);
			engine->ECSWorld.UpdateSystems.push_back(sys);

			sys = ecs->system<LevelEdit, Position>()
				.each(LevelEditSystem);
			engine->ECSWorld.UpdateSystems.push_back(sys);

			engine->ECSWorld.UpdateSystems.push_back(ecs->system<const PlayerInput, const GunCombat>()
				.each([](const PlayerInput& p, const GunCombat& combat)
					{
						char* buff = (char*)calloc(120, sizeof(char));
						if (buff == nullptr)
							abort();

						if (combat.gunState == GunState::Reloading)
						{
							sprintf(buff, "Reloading:%0.f", combat.reloadTimer * 1000.f);
							Text(buff, UI::Rect(0, 60, 200, 30), UI::White, nullptr, _engine->GUIState->Fonts[0]);
						}
						else
						{
							sprintf(buff, "Rounds:%i / %i", combat.Bullets, combat.MagBullets);
							Text(buff, UI::Rect(0, 60, 200, 30), UI::White, nullptr, _engine->GUIState->Fonts[0]);
						}
					}));

			//Update world bounds every frame
			//First clear bounds
			engine->ECSWorld.UpdateSystems.push_back(ecs->system<const Position>()
				.each([](const Position& p)
					{
						_engine->minX = -1;
						_engine->minY = -1;
						_engine->maxX = 1;
						_engine->maxY = 1;
					}));

			//Update all things that have a position (but dont know size)
			engine->ECSWorld.UpdateSystems.push_back(ecs->system<const Position>()
				.each([](const Position& p)
					{
						if (p.x <= _engine->minX) _engine->minX = p.x;
						if (p.y <= _engine->minY) _engine->minY = p.y;
						if (p.x >= _engine->maxX) _engine->maxX = p.x;
						if (p.y >= _engine->maxY) _engine->maxY = p.y;
					}));

			//then expand to things with a bounds, to get larger bounds
			engine->ECSWorld.UpdateSystems.push_back(ecs->system<const Position, const Bounds>()
				.each([](const Position& p, const Bounds& b)
					{
						if (p.x - b.width <= _engine->minX) _engine->minX = p.x - b.width;
						if (p.y - b.height <= _engine->minY) _engine->minY = p.y - b.height;
						if (p.x + b.width >= _engine->maxX) _engine->maxX = p.x + b.width;
						if (p.y + b.height >= _engine->maxY) _engine->maxY = p.y + b.height;
					}));

			//Cleanup sprite objects when the component is removed
			/*ecs->system<SpriteComponent>()
				.kind(flecs::OnRemove)
				.each([](SpriteComponent& sc)
					{
						delete sc.sprite;
					});*/

					//Delete drawable objects when the renderable component is removed
			ecs->system<Renderable>()
				.kind(flecs::OnRemove)
				.each([](Renderable& r)
					{
						delete r.drawable;
					});

			//Cleanup physics handles
			ecs->system<PhysicsBody>()
				.kind(flecs::OnRemove)
				.each([](PhysicsBody& pb)
					{
						//printf("cleaning up physics handle\n");
						pb.Body->GetWorld()->DestroyBody(pb.Body);
					});

			//Create a default sprite sheet to fallback onto
			//This creates a default path, to reduce crashes from lack of animation data
			std::vector<sf::Sprite*> def = {};
			sf::Sprite* s = new sf::Sprite();
			def.push_back(s);
			auto defaultSpriteSheet = MakeSpriteSheet(def, 1, 0, 0);
			//On Add system to populate animation set's with default data
			ecs->system<AnimationSet>()
				.kind(flecs::OnAdd)
				.each([&](AnimationSet& set)
					{
						//printf("[AnimationSet] Adding default animation set\n");
						set.default = defaultSpriteSheet;
					});


			auto world = engine->ECSWorld.world;
			q = world->query_builder<SimPosition, SimPosition>()
				.arg(2).set(flecs::Cascade, flecs::ChildOf)
				.build();
		}

		void UpdateFixedTransformSystem(Unstable::Engine* engine)
		{
			//transform system update
			q.each([](flecs::entity e, SimPosition& p, SimPosition& p_parent)
				{
					//printf("%f %f -> %f %f\n", p.x, p.y, p_parent.x, p_parent.y);
					p.x = p_parent.x;
					p.y = p_parent.y;
				});
		}

		void PrePhysicsUpdate(Unstable::Engine* engine)
		{
			//printf("[PrePhysicsUpdate] %f\n", _engine->fixedDeltatime);
			_engine->allBodies.clear();

			for (u32 i = 0; i < engine->ECSWorld.PrePhysicsSystem.size(); i++)
			{
				ecs_run(engine->ECSWorld.world->c_ptr(), engine->ECSWorld.PrePhysicsSystem[i], engine->fixedDeltatime, 0);
			}
		}

		void PostPhysicsUpdate(Unstable::Engine* engine)
		{
			//printf("[PostPhysicsUpdate] %f\n", _engine->fixedDeltatime);

			for (u32 i = 0; i < engine->ECSWorld.PostPhysicsSystem.size(); i++)
			{
				ecs_run(engine->ECSWorld.world->c_ptr(), engine->ECSWorld.PostPhysicsSystem[i], engine->fixedDeltatime, 0);
			}
			UpdateFixedTransformSystem(engine);
		}

		void EarlyFrameUpdate(Unstable::Engine* engine)
		{
			for (u32 i = 0; i < engine->ECSWorld.EarlyUpdateSystems.size(); i++)
			{
				ecs_run(engine->ECSWorld.world->c_ptr(), engine->ECSWorld.EarlyUpdateSystems[i], engine->deltatime, 0);
			}
		}

		void Update(Unstable::Engine* engine)
		{
			//printf("[Update] %lld %lld %f %f%%\n", _engine->framecount, _engine->tick, _engine->deltatime, _engine->tickPercent);
			for (u32 i = 0; i < engine->ECSWorld.UpdateSystems.size(); i++)
			{
				ecs_run(engine->ECSWorld.world->c_ptr(), engine->ECSWorld.UpdateSystems[i], engine->deltatime, 0);
			}
		}
	}
}