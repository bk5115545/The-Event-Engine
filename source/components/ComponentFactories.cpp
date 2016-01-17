// Copyright 2015 Casey Megginson and Blaise Koch
// #include <stdlib.h>
// #include <string>
// #include <vector>
//
// #include "core/PugiXML/pugixml.hpp"
// #include "SDL.h"
//
// #include "components/ComponentFactories.h"
// #include "util/box2d_utils.h"
//
// // Raw ComponentFactory stuff
// ComponentFactory::ComponentFactory() {}
//
// ComponentFactory::~ComponentFactory() {}
//
//
// // CarrierFactory stuff
// CarrierFactory::CarrierFactory() : ComponentFactory() {}
//
// CarrierFactory::~CarrierFactory() {}
//
// Carrier* CarrierFactory::Create(Actor* owner, pugi::xml_node node) {
//     UNUSED(node);
//     Carrier* carrier = new Carrier(owner);
//     return carrier;
// }
//
//
// // InfantryFactory stuff
// InfantryFactory::InfantryFactory() : ComponentFactory() {}
//
// InfantryFactory::~InfantryFactory() {}
//
// Infantry* InfantryFactory::Create(Actor* owner, pugi::xml_node node) {
//     UNUSED(node);
//     Infantry* infantry = new Infantry(owner);
//     return infantry;
// }
//
//
// // PlayerFactory stuff
// PlayerFactory::PlayerFactory(InputDevice* input_device) : ComponentFactory() {
//     input_device_ = input_device;
// }
//
// PlayerFactory::~PlayerFactory() {}
//
// Player* PlayerFactory::Create(Actor* owner, pugi::xml_node node) {
//     UNUSED(node);
//     Player* player = new Player(owner);
//     player->set_input_device(input_device_);
//     return player;
// }
//
// /*
// // RigidbodyFactory stuff
// RigidCircleFactory::RigidCircleFactory(b2World* world) : ComponentFactory() {
//     world_ = world;
// }
//
// RigidCircleFactory::~RigidCircleFactory() {}
//
// RigidCircle* RigidCircleFactory::Create(Actor* owner, pugi::xml_node node) {
//     RigidCircle* rigid_circle = new RigidCircle(owner);
//     Vector2 position = owner->position();
//     b2BodyDef body_definition;
//     b2CircleShape circle_shape;
//     b2FixtureDef shape_fixture_definition;
//
//     if (std::stof(node.attribute("dynamic").value())) {
//         body_definition.type = b2_dynamicBody;
//     } else {
//         body_definition.type = b2_staticBody;
//     }
//
//     // body_definition.bullet = true;
//     body_definition.position.Set(RW2PW(position.x), RW2PW(position.y));
//     body_definition.angle = RW2PWAngle(owner->angle());
//     body_definition.angularDamping = std::stof(node.attribute("angular_damping").value());
//     body_definition.linearDamping = std::stof(node.attribute("linear_damping").value());
//
//     circle_shape.m_radius = RW2PW(std::stof(node.attribute("radius").value()));
//     shape_fixture_definition.shape = &circle_shape;
//     shape_fixture_definition.density = std::stof(node.attribute("density").value());
//     shape_fixture_definition.friction = std::stof(node.attribute("friction").value());
//     shape_fixture_definition.restitution = std::stof(node.attribute("restitution").value());
//
//     bool movable, turnable;
//     // movable = std::stof(node.attribute("movable").value());
//     // turnable = std::stof(node.attribute("turnable").value());
//
//     rigid_circle->Initialize(world_, body_definition, shape_fixture_definition);
//
//     return rigid_circle;
// }
// */
//
// // RigidbodyFactory stuff
// RigidRectangleFactory::RigidRectangleFactory(b2World* world) : ComponentFactory() {
//     world_ = world;
// }
//
// RigidRectangleFactory::~RigidRectangleFactory() {}
//
// RigidRectangle* RigidRectangleFactory::Create(Actor* owner,
//                                               pugi::xml_node node) {
//     RigidRectangle* rigid_rectangle = new RigidRectangle(owner);
//     Vector2 position = owner->position();
//     b2BodyDef body_definition;
//     b2PolygonShape polygon_shape;
//     b2FixtureDef shape_fixture_definition;
//
//     if (std::stof(node.attribute("dynamic").value())) {
//         body_definition.type = b2_dynamicBody;
//     } else {
//         body_definition.type = b2_staticBody;
//     }
//
//     // body_definition.bullet = true;
//     body_definition.position.Set(RW2PW(position.x), RW2PW(position.y));
//     body_definition.angle = RW2PWAngle(owner->angle());
//     body_definition.angularDamping = std::stof(node.attribute("angular_damping").value());
//     body_definition.linearDamping = std::stof(node.attribute("linear_damping").value());
//     body_definition.fixedRotation = std::stoi(node.attribute("fixed_rotation").value());
//
//     Vector2 dimensions;
//     dimensions.x = std::stof(node.attribute("width").value());
//     dimensions.y = std::stof(node.attribute("height").value());
//     polygon_shape.SetAsBox(RW2PW(dimensions.x), RW2PW(dimensions.y));
//     shape_fixture_definition.shape = &polygon_shape;
//     shape_fixture_definition.density = std::stof(node.attribute("density").value());
//     shape_fixture_definition.friction = std::stof(node.attribute("friction").value());
//     shape_fixture_definition.restitution = std::stof(node.attribute("restitution").value());
//
//     // bool movable, turnable;
//     // movable = std::stof(node.attribute("movable").value());
//     // turnable = std::stof(node.attribute("turnable").value());
//
//     rigid_rectangle->Initialize(world_, body_definition, shape_fixture_definition);
//
//     return rigid_rectangle;
// }
//
//
// // SpriteFactory stuff
// SpriteFactory::SpriteFactory(Renderer* renderer,
//                              ArtAssetLibrary* art_library) :
//                              ComponentFactory() {
//     renderer_ = renderer;
//     art_library_ = art_library;
// }
//
// SpriteFactory::~SpriteFactory() {}
//
// Sprite* SpriteFactory::Create(Actor* owner,
//                               pugi::xml_node node) {
//     Sprite* new_sprite = new Sprite(owner);
//     std::string texture = node.attribute("texture").value();
//     new_sprite->Initialize(renderer_, art_library_->Search(texture));
//     return new_sprite;
// }
//
//
// // AnimationFactory stuff
// AnimationFactory::AnimationFactory() : ComponentFactory() {}
//
// AnimationFactory::~AnimationFactory() {}
//
// Animation* AnimationFactory::Create(Actor* owner,
//                                     pugi::xml_node node) {
//     Animation* new_animation = new Animation(owner);
//
//     std::string name;
//     SDL_Rect frame;
//     std::vector<SDL_Rect>* frame_set;
//
//     // Loop through Animation XML nodes
//     for (pugi::xml_node animation_node : node.children("Animation")) {
//         name = animation_node.attribute("name").value();
//         frame_set = new std::vector<SDL_Rect>;
//         for (pugi::xml_node frame_node : animation_node.children("Frame")) {
//             frame.x = std::stoi(frame_node.attribute("x").value());
//             frame.y = std::stoi(frame_node.attribute("y").value());
//             frame.w = std::stoi(frame_node.attribute("w").value());
//             frame.h = std::stoi(frame_node.attribute("h").value());
//             frame_set->push_back(frame);
//         }
//         new_animation->AddAnimation(name, frame_set);
//     }
//     new_animation->Initialize();
//     return new_animation;
// }

// GlDrawableFactory stuff

// GlDrawableFactory::GlDrawableFactory() : ComponentFactory() {}
//
// GlDrawableFactory::~GlDrawableFactory() {}
//
// GlDrawable* GlDrawableFactory::Create(Actor* owner,
//                                     pugi::xml_node node) {
//     GlDrawable* new_animation = new GlDrawable(owner);
//
//     std::string name;
//     SDL_Rect frame;
//     std::vector<SDL_Rect>* frame_set;
//
//     // Loop through Animation XML nodes
//     for (pugi::xml_node animation_node : node.children("GlDrawable")) {
//         name = animation_node.attribute("name").value();
//         frame_set = new std::vector<SDL_Rect>;
//         for (pugi::xml_node frame_node : animation_node.children("Frame")) {
//             frame.x = std::stoi(frame_node.attribute("x").value());
//             frame.y = std::stoi(frame_node.attribute("y").value());
//             frame.w = std::stoi(frame_node.attribute("w").value());
//             frame.h = std::stoi(frame_node.attribute("h").value());
//             frame_set->push_back(frame);
//         }
//         new_animation->AddAnimation(name, frame_set);
//     }
//     new_animation->Initialize();
//     return new_animation;
// }
