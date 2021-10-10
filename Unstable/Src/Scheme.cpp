#pragma once

#include "Scheme.h"

#include "Engine.h"
#include "ECS.h"
#include <flecs.h>
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <box2d/box2d.h>
#include "Physics.h"

using flecs::entity;

namespace ecs = Unstable::ECS;
namespace fs = std::filesystem;

namespace Unstable
{
	namespace Scheme
	{
		struct SchemeCallback
		{
			u32 key;
			s7_pointer func;
		};

		Engine* _engine;
		s7_scheme* sc;
		bool closed = false;
		std::vector<SchemeCallback> keyCallbacks;

		s7_pointer s_CreateSoundClip(s7_scheme* sc, s7_pointer args)
		{
			bool havepath = s7_is_string(s7_car(args));
			if (havepath)
			{
				const char* path = s7_string(s7_car(args));

				std::string cwd = fs::current_path().string();

				auto fullpath = cwd + path;

				//TODO audio clips into fmod

				return s7_make_c_pointer(sc, nullptr);
			}
			return s7_nil(sc);
		}

		s7_pointer s_CreateSprite(s7_scheme* sc, s7_pointer args)
		{
			bool havepath = s7_is_string(s7_car(args));
			bool havex = s7_is_real(s7_car(s7_cdr(args)));
			bool havey = s7_is_real(s7_car(s7_cdr(s7_cdr(args))));
			bool havewidth = s7_is_real(s7_car(s7_cdr(s7_cdr(s7_cdr(args)))));
			bool haveheight = s7_is_real(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(args))))));

			if (havepath && havex && havey && havewidth && haveheight)
			{
				const char* path = s7_string(s7_car(args));
				f32 x = s7_real(s7_car(s7_cdr(args)));
				f32 y = s7_real(s7_car(s7_cdr(s7_cdr(args))));
				f32 width = s7_real(s7_car(s7_cdr(s7_cdr(s7_cdr(args)))));
				f32 height = s7_real(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(args))))));

				std::string cwd = fs::current_path().string();
				sf::Texture* tex;
				if (_engine->textureCache.find(path) != _engine->textureCache.end())
					tex = _engine->textureCache[path];
				else
				{
					printf("texture cache miss %s \n", path);
					tex = new sf::Texture();
					tex->loadFromFile(cwd + path);
					_engine->textureCache.try_emplace(path, tex);
				}
				auto sprite = new sf::Sprite(*tex, { (s32)x, (s32)y, (s32)width, (s32)height });

				return s7_make_c_pointer(sc, sprite);
			}
			return s7_nil(sc);
		}

		s7_pointer s_CreateSpriteSheet(s7_scheme* sc, s7_pointer args)
		{
			bool havelist = s7_is_list(sc, s7_car(args));
			bool havetimeperframe = s7_is_real(s7_car(s7_cdr(args)));
			bool haveX = s7_is_real(s7_car(s7_cdr(s7_cdr(args))));
			bool haveY = s7_is_real(s7_car(s7_cdr(s7_cdr(s7_cdr(args)))));

			if (havelist && havetimeperframe && haveX && haveY)
			{
				s7_pointer list = s7_car(args);
				f32 timeperframe = s7_real(s7_car(s7_cdr(args)));

				f32 offsetX = s7_real(s7_car(s7_cdr(s7_cdr(args))));
				f32 offsetY = s7_real(s7_car(s7_cdr(s7_cdr(s7_cdr(args)))));

				std::vector<sf::Sprite*> sprites;
				s32 len = s7_list_length(sc, list);

				s7_pointer elm = list;
				for (s32 i = 0; i < len; i++)
				{
					s7_pointer val = s7_car(elm);
					elm = s7_cdr(elm);

					if (!s7_is_null(sc, val) &&
						!s7_is_c_pointer(val)) return s7_nil(sc);

					sprites.push_back((sf::Sprite*)s7_c_pointer(val));
				}

				auto sheet = ECS::MakeSpriteSheet(sprites, timeperframe, offsetX, offsetY);
				return s7_make_c_pointer(sc, sheet);
			}
			return s7_nil(sc);
		}


		s7_pointer s_ParentEntity(s7_scheme* sc, s7_pointer args)
		{
			bool haveent = s7_is_c_pointer(s7_car(args));
			bool havechildent = s7_is_c_pointer(s7_car(s7_cdr(args)));

			if (haveent && havechildent)
			{
				flecs::entity* ent = (flecs::entity*)s7_c_pointer(s7_car(args));
				flecs::entity* childent = (flecs::entity*)s7_c_pointer(s7_car(s7_cdr(args)));

				printf("parenting entities \n");
				childent->child_of(*ent);

				return s7_make_boolean(sc, true);
			}
			return s7_make_boolean(sc, false);
		}


		s7_pointer s_SetComponent(s7_scheme* sc, s7_pointer args)
		{
			bool cptr = s7_is_c_pointer(s7_car(args));
			bool str = s7_is_symbol(s7_car(s7_cdr(args)));
			bool list = s7_is_list(sc, s7_car(s7_cdr(s7_cdr(args))));

			if (cptr && str)
			{
				entity* ent = (entity*)s7_c_pointer(s7_car(args));
				std::string component(s7_symbol_name(s7_car(s7_cdr(args))));
				s7_pointer list = s7_car(s7_cdr(s7_cdr(args)));

				if (!component.compare(0, component.length(), "DestroyTimer") && list)
				{
					bool haveV = s7_is_real(s7_car(list));
					bool haveE = s7_is_real(s7_car(s7_cdr(list)));
					if (!haveV || !haveE) return s7_make_boolean(sc, false);

					f32 val = (f32)s7_real(s7_car(list));
					f32 elapsed = (f32)s7_real(s7_car(s7_cdr(list)));

					ent->set<ecs::DestroyTimer>({ (f32)val, (f32)elapsed });
				}
				if (!component.compare(0, component.length(), "Spawner") && list)
				{
					bool haveV = s7_is_real(s7_car(list));
					bool haveE = s7_is_real(s7_car(s7_cdr(list)));
					//bool haveS = s7_is_c_pointer(s7_car(s7_cdr(s7_cdr(list))));

					//spawner takes a string for now, that is called in scheme, so make it a function
					bool haveS = s7_is_string(s7_car(s7_cdr(s7_cdr(list))));
					if (!haveV || !haveE || !haveS) return s7_make_boolean(sc, false);

					f32 val = (f32)s7_real(s7_car(list));
					f32 elapsed = (f32)s7_real(s7_car(s7_cdr(list)));
					//flecs::prefab* ent = (flecs::prefab*)s7_c_pointer(s7_car(s7_cdr(s7_cdr(list))));
					const char* str = s7_string(s7_car(s7_cdr(s7_cdr(list))));

					ent->set<ecs::SpawnTimer>({ (f32)val, (f32)elapsed, str });
				}
				if (!component.compare(0, component.length(), "OverlapDamage") && list)
				{
					bool haveDmg = s7_is_number(s7_car(list));
					bool haveRange = s7_is_number(s7_car(s7_cdr(list)));
					bool haveent = s7_is_c_pointer(s7_car(s7_cdr(s7_cdr(list))));
					if (!haveent || !haveDmg || !haveRange) return s7_make_boolean(sc, false);

					f32 dmg = (f32)s7_real(s7_car(list));
					f32 range = (f32)s7_real(s7_car(s7_cdr(list)));
					entity* ent = (entity*)s7_c_pointer(s7_car(s7_cdr(s7_cdr(list))));

					ent->set<ecs::OverlapDamage>({ dmg, range, ent });
				}
				if (!component.compare(0, component.length(), "Sprite") && list)
				{
					bool havesprite = s7_is_c_pointer(s7_car(list));
					bool havedepth = s7_is_integer(s7_car(s7_cdr(list)));
					if (!havesprite || !havedepth) return s7_make_boolean(sc, false);
					sf::Sprite* sprite = (sf::Sprite*)s7_c_pointer(s7_car(list));
					s32 depthOffset = (s32)s7_integer(s7_car(s7_cdr(list)));

					ent->set<ecs::SpriteComponent>({ sprite, depthOffset, 0 });
				}
				if (!component.compare(0, component.length(), "AnimatedSprite") && list)
				{
					bool havesheet = s7_is_c_pointer(s7_car(list));
					bool havethunk = s7_is_procedure(s7_car(s7_cdr(list)));
					if (!havesheet) return s7_make_boolean(sc, false);
					ecs::SpriteSheet* sheet = (ecs::SpriteSheet*)s7_c_pointer(s7_car(list));

					if (havethunk)
					{
						auto func = s7_car(s7_cdr(list));
						ent->set<ecs::AnimatedSprite>({ sheet, 0, 0, false, nullptr, true, func });
					}
					else
					{
						ent->set<ecs::AnimatedSprite>({ sheet, 0, 0, false, nullptr, false, nullptr });
					}
				}
				if (!component.compare(0, component.length(), "AnimatedSpriteOnce") && list)
				{
					bool havesheet = s7_is_c_pointer(s7_car(list));
					bool havethunk = s7_is_procedure(s7_car(s7_cdr(list)));
					if (!havesheet) return s7_make_boolean(sc, false);
					ecs::SpriteSheet* sheet = (ecs::SpriteSheet*)s7_c_pointer(s7_car(list));

					if (havethunk)
					{
						auto func = s7_car(s7_cdr(list));
						ent->set<ecs::AnimatedSpriteOnce>({ sheet, 0, 0, false, nullptr, true, func });
					}
					else
					{
						ent->set<ecs::AnimatedSpriteOnce>({ sheet, 0, 0, false, nullptr, false, nullptr });
					}
				}
				if (!component.compare(0, component.length(), "CameraFocus") && list)
				{
					bool havex = s7_is_real(s7_car(list));
					bool havey = s7_is_real(s7_car(s7_cdr(list)));
					if (!havex || !havey) return s7_make_boolean(sc, false);

					f32 x = (f32)s7_real(s7_car(list));
					f32 y = (f32)s7_real(s7_car(s7_cdr(list)));

					ent->set<ecs::CameraFocus>({ (f32)x, (f32)y });
				}
				if (!component.compare(0, component.length(), "Position") && list)
				{
					bool havex = s7_is_real(s7_car(list));
					bool havey = s7_is_real(s7_car(s7_cdr(list)));
					if (!havex || !havey) return s7_make_boolean(sc, false);

					f32 x = (f32)s7_real(s7_car(list));
					f32 y = (f32)s7_real(s7_car(s7_cdr(list)));

					ent->set<ecs::Position>({ x, y });
				}
				if (!component.compare(0, component.length(), "Rotation") && list)
				{
					bool haveV = s7_is_real(s7_car(list));
					if (!haveV) return s7_make_boolean(sc, false);

					f32 v = (f32)s7_real(s7_car(list));

					ent->set<ecs::Rotation>({ v });
				}
				if (!component.compare(0, component.length(), "SimPosition") && list)
				{
					bool havex = s7_is_real(s7_car(list));
					bool havey = s7_is_real(s7_car(s7_cdr(list)));
					if (!havex || !havey) return s7_make_boolean(sc, false);

					f32 x = (f32)s7_real(s7_car(list));
					f32 y = (f32)s7_real(s7_car(s7_cdr(list)));

					ent->set<ecs::SimPosition>({ x, y, x, y });
				}
				if (!component.compare(0, component.length(), "SimCopy") && list)
				{
					ent->add<ecs::SimCopy>();
				}
				if (!component.compare(0, component.length(), "Velocity") && list)
				{
					bool havex = s7_is_real(s7_car(list));
					bool havey = s7_is_real(s7_car(s7_cdr(list)));
					if (!havex || !havey) return s7_make_boolean(sc, false);

					f32 x = (f32)s7_real(s7_car(list));
					f32 y = (f32)s7_real(s7_car(s7_cdr(list)));

					ent->set<ecs::Velocity>({ x, y });
				}
				if (!component.compare(0, component.length(), "Friction") && list)
				{
					bool haveV = s7_is_real(s7_car(list));
					bool haveStop = s7_is_real(s7_car(s7_cdr(list)));
					if (!haveV || !haveStop) return s7_make_boolean(sc, false);

					f32 v = (f32)s7_real(s7_car(list));
					f32 stop = (f32)s7_real(s7_car(s7_cdr(list)));

					ent->set<ecs::Friction>({ v, stop });
				}
				if (!component.compare(0, component.length(), "Bounds") && list)
				{
					bool haveW = s7_is_real(s7_car(list));
					bool haveH = s7_is_real(s7_car(s7_cdr(list)));
					if (!haveW || !haveH) return s7_make_boolean(sc, false);

					f32 w = (f32)s7_real(s7_car(list));
					f32 h = (f32)s7_real(s7_car(s7_cdr(list)));

					ent->set<ecs::Bounds>({ (f32)w, (f32)h });
				}
				if (!component.compare(0, component.length(), "PhysicsBody") && list)
				{
					bool haveSym = s7_is_symbol(s7_car(list));
					if (!haveSym) return s7_make_boolean(sc, false);

					std::string shapeSym(s7_symbol_name(s7_car(list)));

					if (!shapeSym.compare(0, shapeSym.length(), "Circle"))
					{
						bool haveRadius = s7_is_number(s7_car(s7_cdr(list)));
						bool haveFriction = s7_is_number(s7_car(s7_cdr(s7_cdr(list))));
						bool haveDensity = s7_is_number(s7_car(s7_cdr(s7_cdr(s7_cdr(list)))));
						bool haveoffsetY = s7_is_number(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list))))));
						bool haveType = s7_is_symbol(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list)))))));
						if (!haveRadius || !haveFriction || !haveDensity || !haveoffsetY || !haveType) return s7_make_boolean(sc, false);

						f32 radius = s7_real(s7_car(s7_cdr(list)));
						f32 friction = s7_real(s7_car(s7_cdr(s7_cdr(list))));
						f32 density = s7_real(s7_car(s7_cdr(s7_cdr(s7_cdr(list)))));
						f32 offsetY = s7_real(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list))))));
						std::string type(s7_symbol_name(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list))))))));

						b2BodyDef pdef{};
						if (!type.compare(0, type.length(), "static"))
							pdef.type = b2BodyType::b2_staticBody;
						if (!type.compare(0, type.length(), "kinematic"))
							pdef.type = b2BodyType::b2_kinematicBody;
						if (!type.compare(0, type.length(), "dynamic"))
							pdef.type = b2BodyType::b2_dynamicBody;

						pdef.fixedRotation = pdef.type != b2BodyType::b2_dynamicBody;//if were not dyn dont rotate
						pdef.position = b2Vec2(0, 0);
						pdef.enabled = true;
						pdef.userData.pointer = uintptr_t(ent);

						if (pdef.type == b2BodyType::b2_dynamicBody)
							pdef.allowSleep = false;

						b2CircleShape pbod{};
						pbod.m_p.Set(0, offsetY);
						pbod.m_radius = radius;

						b2FixtureDef pfdef{};
						pfdef.density = density;
						pfdef.shape = &pbod;
						pfdef.friction = friction;

						b2Body* ppbod = _engine->Physics->world->CreateBody(&pdef);
						ppbod->CreateFixture(&pfdef);

						//Get the entity back from a body
						//void* ptr = (void*)ppbod->GetUserData().pointer;

						ent->set<ecs::PhysicsBody>({ ppbod });
					}
					if (!shapeSym.compare(0, shapeSym.length(), "Box"))
					{
						bool haveW = s7_is_number(s7_car(s7_cdr(list)));
						bool haveH = s7_is_number(s7_car(s7_cdr(s7_cdr(list))));
						bool haveFriction = s7_is_number(s7_car(s7_cdr(s7_cdr(s7_cdr(list)))));
						bool haveDensity = s7_is_number(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list))))));
						bool haveoffsetY = s7_is_number(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list)))))));
						bool haveType = s7_is_symbol(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list))))))));
						if (!haveW || !haveH || !haveFriction || !haveDensity || !haveoffsetY || !haveType) return s7_make_boolean(sc, false);

						f32 w = s7_real(s7_car(s7_cdr(list)));
						f32 h = s7_real(s7_car(s7_cdr(s7_cdr(list))));
						f32 friction = s7_real(s7_car(s7_cdr(s7_cdr(s7_cdr(list)))));
						f32 density = s7_real(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list))))));
						f32 offsetY = s7_real(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list)))))));
						std::string type(s7_symbol_name(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list)))))))));

						b2BodyDef pdef{};
						if (!type.compare(0, type.length(), "static"))
							pdef.type = b2BodyType::b2_staticBody;
						if (!type.compare(0, type.length(), "kinematic"))
							pdef.type = b2BodyType::b2_kinematicBody;
						if (!type.compare(0, type.length(), "dynamic"))
							pdef.type = b2BodyType::b2_dynamicBody;

						pdef.fixedRotation = pdef.type != b2BodyType::b2_dynamicBody;//if were not dyn dont rotate
						pdef.position = b2Vec2(0, 0);
						pdef.enabled = true;
						pdef.userData.pointer = uintptr_t(ent);

						if (pdef.type == b2BodyType::b2_dynamicBody)
							pdef.allowSleep = false;

						b2PolygonShape pbod{};
						pbod.SetAsBox(w / 2, h / 2, { 0, offsetY }, 0);

						b2FixtureDef pfdef{};
						pfdef.density = density;
						pfdef.shape = &pbod;
						pfdef.friction = friction;

						b2Body* ppbod = _engine->Physics->world->CreateBody(&pdef);
						ppbod->CreateFixture(&pfdef);

						//Get the entity back from a body
						//void* ptr = (void*)ppbod->GetUserData().pointer;

						ent->set<ecs::PhysicsBody>({ ppbod });
					}
				}
				if (!component.compare(0, component.length(), "AIState"))
				{
					ent->set<ecs::AIState>({ });
				}
				if (!component.compare(0, component.length(), "NPCMovement") && list)
				{
					bool haveS = s7_is_number(s7_car(list));
					if (!haveS) return s7_make_boolean(sc, false);

					f32 speed = s7_real(s7_car(list));

					ent->set<ecs::NPCMovement>({ speed });
				}
				if (!component.compare(0, component.length(), "NPCCombat") && list)
				{
					bool haveR = s7_is_number(s7_car(list));
					bool haveAR = s7_is_number(s7_car(s7_cdr(list)));
					bool haveATT = s7_is_number(s7_car(s7_cdr(s7_cdr(list))));
					bool haveADT = s7_is_number(s7_car(s7_cdr(s7_cdr(s7_cdr(list)))));
					bool haveDmg = s7_is_number(s7_car(s7_cdr(s7_cdr(s7_cdr(list)))));
					if (!haveR || !haveAR || !haveATT || !haveADT || !haveDmg) return s7_make_boolean(sc, false);

					f32 range = s7_real(s7_car(list));
					f32 attackRange = s7_real(s7_car(s7_cdr(list)));
					f32 attackTime = s7_real(s7_car(s7_cdr(s7_cdr(list))));
					f32 attackDamageTime = s7_real(s7_car(s7_cdr(s7_cdr(s7_cdr(list)))));
					f32 damage = s7_real(s7_car(s7_cdr(s7_cdr(s7_cdr(s7_cdr(list))))));

					ent->set<ecs::NPCCombat>({ range, attackRange, attackTime, attackDamageTime, damage });
				}
				if (!component.compare(0, component.length(), "PlayerInput") && list)
				{
					bool haveS = s7_is_real(s7_car(list));
					if (!haveS) return s7_make_boolean(sc, false);

					f32 speed = (f32)s7_real(s7_car(list));
					ent->set<ecs::PlayerInput>({ 0, 0, speed });
				}
				if (!component.compare(0, component.length(), "PlayerMovement"))
				{
					ent->set<ecs::PlayerMovement>({ });
				}
				if (!component.compare(0, component.length(), "Health") && list)
				{
					bool haveS = s7_is_number(s7_car(list));
					if (!haveS) return s7_make_boolean(sc, false);

					f32 health = (f32)s7_real(s7_car(list));
					ent->set<ecs::Health>({ health });
				}
				if (!component.compare(0, component.length(), "GunCombat") && list)
				{
					bool haveS = s7_is_number(s7_car(list));
					bool haveM = s7_is_number(s7_car(s7_cdr(list)));
					if (!haveS || !haveM) return s7_make_boolean(sc, false);

					u32 bullets = (u32)s7_real(s7_car(list));
					u32 magBullets = (u32)s7_real(s7_car(s7_cdr(list)));
					ent->set<ecs::GunCombat>({ bullets, magBullets });
				}
				if (!component.compare(0, component.length(), "AnimationSet") && list)
				{
					s32 len = s7_list_length(sc, list);

					ecs::AnimationSet ani = {};
					s7_pointer elm = list;
					for (s32 i = 0; i < len; i++)
					{
						s7_pointer val = s7_car(elm);
						elm = s7_cdr(elm);

						if (!s7_is_null(sc, val) &&
							!s7_is_list(sc, val)) return s7_nil(sc);


						/*if (!s7_is_null(sc, val) &&
							!s7_is_c_pointer(val)) return s7_nil(sc);*/

						std::string key = std::string(s7_string(s7_car(val)));

						ecs::SpriteSheet* ss = (ecs::SpriteSheet*)s7_c_pointer(s7_car(s7_cdr(val)));

						ani.animations.try_emplace(key, ss);
					}

					ent->set<ecs::AnimationSet>({ ani });
				}
				if (!component.compare(0, component.length(), "PlayerAttachment") && list)
				{
					bool haveent = s7_is_c_pointer(s7_car(list));
					bool haveX = s7_is_number(s7_car(s7_cdr(list)));
					bool haveY = s7_is_number(s7_car(s7_cdr(s7_cdr(list))));
					if (!haveent || !haveX || !haveY) return s7_make_boolean(sc, false);
					entity* ent2 = (entity*)s7_c_pointer(s7_car(list));
					f32 x = (f32)s7_real(s7_car(s7_cdr(list)));
					f32 y = (f32)s7_real(s7_car(s7_cdr(s7_cdr(list))));

					ent->set<ecs::PlayerAttachment>({ ent2, x, y });
				}
				if (!component.compare(0, component.length(), "LevelEdit"))
				{
					ent->set<ecs::LevelEdit>({ });
				}
				if (!component.compare(0, component.length(), "Marker") && list)
				{
					bool haveS = s7_is_string(s7_car(list));
					if (!haveS) return s7_make_boolean(sc, false);

					ent->set<ecs::SchemeMarker>({ s7_string(s7_car(list)) });
				}

				return s7_make_boolean(sc, true);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_RemoveComponent(s7_scheme* sc, s7_pointer args)
		{
			bool cptr = s7_is_c_pointer(s7_car(args));
			bool str = s7_is_symbol(s7_car(s7_cdr(args)));

			if (cptr && str)
			{
				entity* ent = (entity*)s7_c_pointer(s7_car(args));
				std::string component(s7_symbol_name(s7_car(s7_cdr(args))));

				if (!component.compare(0, component.length(), "DestroyTimer"))
				{
					ent->remove<ecs::DestroyTimer>();
				}
				if (!component.compare(0, component.length(), "Spawner"))
				{
					ent->remove<ecs::SpawnTimer>();
				}
				if (!component.compare(0, component.length(), "OverlapDamage"))
				{
					ent->remove<ecs::OverlapDamage>();
				}
				if (!component.compare(0, component.length(), "Sprite"))
				{
					ent->remove<ecs::SpriteComponent>();
				}
				if (!component.compare(0, component.length(), "AnimatedSprite"))
				{
					ent->remove<ecs::AnimatedSprite>();
				}
				if (!component.compare(0, component.length(), "AnimatedSpriteOnce"))
				{
					ent->remove<ecs::AnimatedSpriteOnce>();
				}
				if (!component.compare(0, component.length(), "CameraFocus"))
				{
					ent->remove<ecs::CameraFocus>();
				}
				if (!component.compare(0, component.length(), "Position"))
				{
					ent->remove<ecs::Position>();
				}
				if (!component.compare(0, component.length(), "Rotation"))
				{
					ent->remove<ecs::Rotation>();
				}
				if (!component.compare(0, component.length(), "SimPosition"))
				{
					ent->remove<ecs::SimPosition>();
				}
				if (!component.compare(0, component.length(), "SimCopy"))
				{
					ent->remove<ecs::SimCopy>();
				}
				if (!component.compare(0, component.length(), "Velocity"))
				{
					ent->remove<ecs::Velocity>();
				}
				if (!component.compare(0, component.length(), "Friction"))
				{
					ent->remove<ecs::Friction>();
				}
				if (!component.compare(0, component.length(), "Bounds"))
				{
					ent->remove<ecs::Bounds>();
				}
				if (!component.compare(0, component.length(), "PhysicsBody"))
				{
					ent->remove<ecs::PhysicsBody>();
				}
				if (!component.compare(0, component.length(), "AIState"))
				{
					ent->remove<ecs::AIState>();
				}
				if (!component.compare(0, component.length(), "NPCMovement"))
				{
					ent->remove<ecs::NPCMovement>();
				}
				if (!component.compare(0, component.length(), "NPCCombat"))
				{
					ent->remove<ecs::NPCCombat>();
				}
				if (!component.compare(0, component.length(), "PlayerInput"))
				{
					ent->remove<ecs::PlayerInput>();
				}
				if (!component.compare(0, component.length(), "PlayerMovement"))
				{
					ent->remove<ecs::PlayerMovement>();
				}
				if (!component.compare(0, component.length(), "Health"))
				{
					ent->remove<ecs::Health>();
				}
				if (!component.compare(0, component.length(), "GunCombat"))
				{
					ent->remove<ecs::GunCombat>();
				}
				if (!component.compare(0, component.length(), "AnimationSet"))
				{
					ent->remove<ecs::AnimationSet>();
				}
				if (!component.compare(0, component.length(), "PlayerAttachment"))
				{
					ent->remove<ecs::PlayerAttachment>();
				}

				return s7_make_boolean(sc, true);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_SetPosition(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)) &&
				s7_is_real(s7_car(s7_cdr(args))) &&
				s7_is_real(s7_car(s7_cdr(s7_cdr(args)))))
			{
				entity* ent = (entity*)s7_c_pointer(s7_car(args));
				f32 x = (f32)s7_real(s7_car(s7_cdr(args)));
				f32 y = (f32)s7_real(s7_car(s7_cdr(s7_cdr(args))));

				ent->set<ecs::Position>({ x, y });

				return s7_make_boolean(sc, true);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_DumpType(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)))
			{
				entity* ent = (entity*)s7_c_pointer(s7_car(args));

				printf("Entity: %s \n", ent->type().str().c_str());

				return s7_make_boolean(sc, true);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_CreateEntity(s7_scheme* sc, s7_pointer args)
		{
			entity* ent2 = new entity(_engine->ECSWorld.world->entity());

			return s7_make_c_pointer(sc, ent2);
		}

		s7_pointer s_CreatePrefab(s7_scheme* sc, s7_pointer args)
		{
			entity* ent2 = new entity(_engine->ECSWorld.world->entity());
			ent2->add(EcsPrefab);

			//was creating prefabs for spawners
			//this wont work because physics bodies componets are created from the 
			//scripting function above, so the bodies are created even if its a prefab
			//and then lost when the prefab is spawned

			return s7_make_c_pointer(sc, ent2);
		}

		s7_pointer s_DeleteEntity(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)))
			{
				entity* ent = (entity*)s7_c_pointer(s7_car(args));
				ent->destruct();
				return s7_make_boolean(sc, true);
			}
			return s7_make_boolean(sc, false);
		}

		void Init(Engine* engine)
		{
			_engine = engine;
			sc = s7_init();
			engine->Scheme.s7 = sc;
			closed = false;

			//ECS
			//s7_define_function(sc, "create-prefab", s_CreatePrefab, 0, 0, "", "");

			//nothing
			s7_define_function(sc, "create-entity", s_CreateEntity, 0, 0, "", "");
			//entity*
			s7_define_function(sc, "delete-entity", s_DeleteEntity, 1, 0, "", "");
			//path, x, y, width, height
			s7_define_function(sc, "create-sprite", s_CreateSprite, 5, 0, "", "");
			//list of sprite*, time per frame, offset x, offset y
			s7_define_function(sc, "create-sprite-sheet", s_CreateSpriteSheet, 4, 0, "", "");
			//entity*, 
			s7_define_function(sc, "set-component", s_SetComponent, 2, 1, "", "");
			//entity*, component symbol, list of params
			s7_define_function(sc, "remove-component", s_RemoveComponent, 2, 0, "", "");
			//entity* parent, entity* child
			s7_define_function(sc, "parent-entity", s_ParentEntity, 2, 0, "", "");
			//entity*
			s7_define_function(sc, "entity-type", s_DumpType, 1, 0, "", "");

			//Math
			s7_define_function(sc, "<<", s_Math_LeftShift, 2, 0, "", "");
			s7_define_function(sc, ">>", s_Math_RightShift, 2, 0, "", "");
			s7_define_function(sc, "|", s_Math_Or, 2, 0, "", "");
			s7_define_function(sc, "&", s_Math_And, 2, 0, "", "");
			s7_define_function(sc, "!", s_Math_Not, 1, 0, "", "");

			//Engine
			s7_define_function(sc, "bind-keycode", s_BindKeycode, 2, 0, "", "");
			s7_define_function(sc, "engine/time", s_GetTime, 0, 0, "", "");
			s7_define_function(sc, "rand", s_Random, 0, 0, "", "");

			//Memory
			s7_define_function(sc, "malloc", s_Malloc, 1, 0, "", "");
			s7_define_function(sc, "free", s_Free, 1, 0, "", "");
			s7_define_function(sc, "set/u8", s_MemSetU8, 3, 0, "", "");
			s7_define_function(sc, "set/u16", s_MemSetU16, 3, 0, "", "");
			s7_define_function(sc, "set/u32", s_MemSetU32, 3, 0, "", "");
			s7_define_function(sc, "set/f32", s_MemSetF32, 3, 0, "", "");
			s7_define_function(sc, "get/u8", s_MemGetU8, 2, 0, "", "");
			s7_define_function(sc, "get/u16", s_MemGetU16, 2, 0, "", "");
			s7_define_function(sc, "get/u32", s_MemGetU32, 2, 0, "", "");
			s7_define_function(sc, "get/f32", s_MemGetF32, 2, 0, "", "");


			//Load libs
			//s7_eval_c_string(sc, "(load \"Assets/Scripts/libc.scm\")");
			//s7_eval_c_string(sc, "(load \"Assets/Scripts/r7rs.scm\")");
			s7_eval_c_string(sc, "(load \"Assets/Scripts/srfi-1.scm\")");

			//Built in function
			s7_eval_c_string(sc, "(define (update x y tx ty) #t)");

			//Run game startup script
			s7_eval_c_string(sc, "(load \"Assets/Scripts/init.scm\")");
		}

		void Shutdown(Engine* engine)
		{
			closed = true;
		}

		void Update(Engine* engine)
		{
			auto xy = Utility::GetMouseWorldPos(_engine);
			auto txy = Utility::GetMouseTilePos(_engine);
			char* buff = (char*)calloc(200, sizeof(char));
			if (buff == nullptr)
				abort();
			memset(buff, 0, 200);
			sprintf(buff, "(update %f %f %f %f)", xy.x, xy.y, txy.x, txy.y);
			s7_eval_c_string(sc, buff);



			for (u32 i = 0; i < keyCallbacks.size(); i++)
			{
				if (_engine->KeyPressedState[keyCallbacks[i].key])
				{
					auto mouseXY = Utility::GetMouseWorldPos(_engine);

					s7_call(_engine->Scheme.s7, keyCallbacks[i].func,
						s7_cons(_engine->Scheme.s7,
							s7_make_real(_engine->Scheme.s7, mouseXY.x),
							s7_cons(_engine->Scheme.s7,
								s7_make_real(_engine->Scheme.s7, mouseXY.y),
								s7_nil(_engine->Scheme.s7))));
				}
			}
		}

		void Eval(Engine* engine, char* eval)
		{
			if (!closed)
				s7_eval_c_string(engine->Scheme.s7, eval);
		}

		s7_pointer s_BindKeycode(s7_scheme* sc, s7_pointer args)
		{
			bool haveKey = s7_is_integer(s7_car(args));
			bool havethunk = s7_is_procedure(s7_car(s7_cdr(args)));
			if (!haveKey || !havethunk) return s7_make_boolean(sc, false);
			s32 key = s7_integer(s7_car(args));
			auto func = s7_car(s7_cdr(args));

			SchemeCallback call = {
				key,
				func,
			};

			keyCallbacks.push_back(call);
		}

		//Math
		//Bitwise functions
		s7_pointer s_Math_LeftShift(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_integer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))))
			{
				s64 lhs = s7_integer(s7_car(args));
				s64 rhs = s7_integer(s7_car(s7_cdr(args)));
				return s7_make_integer(sc, lhs << rhs);
			}
			return s7_make_integer(sc, 0);
		}

		s7_pointer s_Math_RightShift(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_integer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))))
			{
				s64 lhs = s7_integer(s7_car(args));
				s64 rhs = s7_integer(s7_car(s7_cdr(args)));
				return s7_make_integer(sc, lhs >> rhs);
			}
			return s7_make_integer(sc, 0);
		}

		s7_pointer s_Math_Or(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_integer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))))
			{
				s64 lhs = s7_integer(s7_car(args));
				s64 rhs = s7_integer(s7_car(s7_cdr(args)));
				return s7_make_integer(sc, lhs | rhs);
			}
			return s7_make_integer(sc, 0);
		}

		s7_pointer s_Math_And(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_integer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))))
			{
				s64 lhs = s7_integer(s7_car(args));
				s64 rhs = s7_integer(s7_car(s7_cdr(args)));
				return s7_make_integer(sc, lhs & rhs);
			}
			return s7_make_integer(sc, 0);
		}

		s7_pointer s_Math_Not(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_integer(s7_car(args)))
			{
				s64 lhs = s7_integer(s7_car(args));
				return s7_make_integer(sc, !lhs);
			}
			return s7_make_integer(sc, 0);
		}
		//Engine
		s7_pointer s_GetTime(s7_scheme* sc, s7_pointer args)
		{
			return s7_make_real(sc, GetTime(_engine));
		}

		s7_pointer s_Random(s7_scheme* sc, s7_pointer args)
		{
			return s7_make_real(sc, Utility::Random01());
		}


		//Memory
		s7_pointer s_Malloc(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_integer(s7_car(args)))
			{
				s64 count = s7_integer(s7_car(args));

				void* buff = malloc(sizeof(u8) * count);
				if (buff != NULL)
				{
					return s7_make_c_pointer(sc, buff);
				}
				return s7_make_boolean(sc, false);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_Free(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)))
			{
				void* buff = s7_c_pointer(s7_car(args));
				free(buff);

				return s7_make_boolean(sc, true);
			}

			return s7_make_boolean(sc, false);
		}

		s7_pointer s_MemSetU8(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))) &&
				s7_is_integer(s7_car(s7_cdr(s7_cdr(args)))))
			{
				u8* buff = (u8*)s7_c_pointer(s7_car(args));
				s64 offset = s7_integer(s7_car(s7_cdr(args)));
				s64 value = s7_integer(s7_car(s7_cdr(s7_cdr(args))));

				u8* p = (buff + offset);
				*p = (u8)value;

				return s7_make_boolean(sc, true);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_MemSetU16(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))) &&
				s7_is_integer(s7_car(s7_cdr(s7_cdr(args)))))
			{
				u8* buff = (u8*)s7_c_pointer(s7_car(args));
				s64 offset = s7_integer(s7_car(s7_cdr(args)));
				s64 value = s7_integer(s7_car(s7_cdr(s7_cdr(args))));

				u16* p = (u16*)(buff + offset);
				*p = (u16)value;

				return s7_make_boolean(sc, true);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_MemSetU32(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))) &&
				s7_is_integer(s7_car(s7_cdr(s7_cdr(args)))))
			{
				u8* buff = (u8*)s7_c_pointer(s7_car(args));
				s64 offset = s7_integer(s7_car(s7_cdr(args)));
				s64 value = s7_integer(s7_car(s7_cdr(s7_cdr(args))));

				u32* p = (u32*)(buff + offset);
				*p = (u32)value;

				return s7_make_boolean(sc, true);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_MemSetF32(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))) &&
				s7_is_real(s7_car(s7_cdr(s7_cdr(args)))))
			{
				u8* buff = (u8*)s7_c_pointer(s7_car(args));
				s64 offset = s7_integer(s7_car(s7_cdr(args)));
				f64 value = s7_real(s7_car(s7_cdr(s7_cdr(args))));

				f32* p = (f32*)(buff + offset);
				*p = (f32)value;

				return s7_make_boolean(sc, true);
			}
			return s7_make_boolean(sc, false);
		}


		s7_pointer s_MemGetU8(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))))
			{
				u8* buff = (u8*)s7_c_pointer(s7_car(args));
				s64 offset = s7_integer(s7_car(s7_cdr(args)));

				u8 value = buff[offset];
				return s7_make_integer(sc, value);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_MemGetU16(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))))
			{
				u8* buff = (u8*)s7_c_pointer(s7_car(args));
				s64 offset = s7_integer(s7_car(s7_cdr(args)));

				u16 value = 0;
				memcpy(&value, (u16*)(buff + offset), sizeof(u16));
				return s7_make_integer(sc, value);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_MemGetU32(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))))
			{
				u8* buff = (u8*)s7_c_pointer(s7_car(args));
				s64 offset = s7_integer(s7_car(s7_cdr(args)));

				u32 value = 0;
				memcpy(&value, (u32*)(buff + offset), sizeof(u32));
				return s7_make_integer(sc, value);
			}
			return s7_make_boolean(sc, false);
		}

		s7_pointer s_MemGetF32(s7_scheme* sc, s7_pointer args)
		{
			if (s7_is_c_pointer(s7_car(args)) &&
				s7_is_integer(s7_car(s7_cdr(args))))
			{
				u8* buff = (u8*)s7_c_pointer(s7_car(args));
				s64 offset = s7_integer(s7_car(s7_cdr(args)));

				f32 value = 0;
				memcpy(&value, (f32*)(buff + offset), sizeof(f32));
				return s7_make_real(sc, (f64)value);
			}
			return s7_make_boolean(sc, false);
		}


	}
}