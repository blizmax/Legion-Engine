#pragma once
#include <core/core.hpp>
#include <audio/audio.hpp>
#include <application/application.hpp>
#include <rendering/rendering.hpp>

using namespace legion;

struct player_move : public app::input_axis<player_move> {};
struct player_strive : public app::input_axis<player_strive> {};
struct player_fly : public app::input_axis<player_fly> {};
struct player_look_x : public app::input_axis<player_look_x> {};
struct player_look_y : public app::input_axis<player_look_y> {};

struct exit_action : public app::input_action<exit_action> {};

struct fullscreen_action : public app::input_action<fullscreen_action> {};
struct escape_cursor_action : public app::input_action<escape_cursor_action> {};
struct vsync_action : public app::input_action<vsync_action> {};

class SimpleCameraController final : public System<SimpleCameraController>
{
public:
    ecs::entity_handle camera;
    ecs::entity_handle skybox;
    ecs::entity_handle groundplane;

    virtual void setup()
    {

#pragma region Input binding
        app::InputSystem::createBinding<player_move>(app::inputmap::method::W, 1.f);
        app::InputSystem::createBinding<player_move>(app::inputmap::method::S, -1.f);
        app::InputSystem::createBinding<player_strive>(app::inputmap::method::D, 1.f);
        app::InputSystem::createBinding<player_strive>(app::inputmap::method::A, -1.f);
        app::InputSystem::createBinding<player_fly>(app::inputmap::method::SPACE, 1.f);
        app::InputSystem::createBinding<player_fly>(app::inputmap::method::LEFT_SHIFT, -1.f);
        app::InputSystem::createBinding<player_look_x>(app::inputmap::method::MOUSE_X, 0.f);
        app::InputSystem::createBinding<player_look_y>(app::inputmap::method::MOUSE_Y, 0.f);

        app::InputSystem::createBinding<exit_action>(app::inputmap::method::ESCAPE);
        app::InputSystem::createBinding<fullscreen_action>(app::inputmap::method::F11);
        app::InputSystem::createBinding<escape_cursor_action>(app::inputmap::method::TAB);
        app::InputSystem::createBinding<vsync_action>(app::inputmap::method::F1);

        bindToEvent<player_move, &SimpleCameraController::onPlayerMove>();
        bindToEvent<player_strive, &SimpleCameraController::onPlayerStrive>();
        bindToEvent<player_fly, &SimpleCameraController::onPlayerFly>();
        bindToEvent<player_look_x, &SimpleCameraController::onPlayerLookX>();
        bindToEvent<player_look_y, &SimpleCameraController::onPlayerLookY>();
        bindToEvent<exit_action, &SimpleCameraController::onExit>();
        bindToEvent<fullscreen_action, &SimpleCameraController::onFullscreen>();
        bindToEvent<escape_cursor_action, &SimpleCameraController::onEscapeCursor>();
        bindToEvent<vsync_action, &SimpleCameraController::onVSYNCSwap>();

#pragma endregion

        app::window window = m_ecs->world.get_component_handle<app::window>().read();
        window.enableCursor(false);
        window.show();

        {
            async::readwrite_guard guard(*window.lock);
            app::ContextHelper::makeContextCurrent(window);
            setupCameraEntity();
            app::ContextHelper::makeContextCurrent(nullptr);
        }
    }

#pragma region input stuff
    void setupCameraEntity()
    {
        groundplane = createEntity();
        auto groundmat = rendering::MaterialCache::create_material("floor", "assets://shaders/groundplane.shs"_view);
        groundmat.set_param("heightMap", rendering::TextureCache::create_texture("heightMap", "assets://textures/mississippi.png"_view));
        groundplane.add_component<rendering::renderable>({ rendering::ModelCache::create_model("floor", "assets://models/groundplane.obj"_view), groundmat });
        groundplane.add_components<transform>();

        skybox = createEntity();
        skybox.add_component<rendering::renderable>({ rendering::ModelCache::create_model("uvsphere", "assets://models/uvsphere.obj"_view), rendering::MaterialCache::create_material("skybox", "assets://shaders/skybox.shs"_view) });
        skybox.add_components<transform>(position(), rotation(), scale(1000.f));

        camera = createEntity();
        camera.add_components<transform>(position(0.f, 3.f, 0.f), rotation::lookat(math::vec3::zero, math::vec3::forward), scale());
        camera.add_component<audio::audio_listener>();

        rendering::camera cam;
        cam.set_projection(90.f, 0.1f, 1000.f);
        camera.add_component<rendering::camera>(cam);
    }

    void onExit(exit_action* action)
    {
        if (action->released())
            raiseEvent<events::exit>();
    }

    void onFullscreen(fullscreen_action* action)
    {
        if (action->released())
        {
            app::WindowSystem::requestFullscreenToggle(world_entity_id, math::ivec2(100, 100), math::ivec2(1360, 768));
        }
    }

    void onEscapeCursor(escape_cursor_action* action)
    {
        if (action->released())
        {
            static bool enabled = false;
            app::window window = m_ecs->world.get_component_handle<app::window>().read();
            enabled = !enabled;
            window.enableCursor(enabled);
            window.show();
        }
    }

    void onVSYNCSwap(vsync_action* action)
    {
        if (action->released())
        {
            auto handle = m_ecs->world.get_component_handle<app::window>();
            app::window window = handle.read();
            window.setSwapInterval(window.swapInterval() ? 0 : 1);
            log::debug("set swap interval to {}", window.swapInterval());
            handle.write(window);
        }
    }

    void onPlayerMove(player_move* action)
    {
        auto posH = camera.get_component_handle<position>();
        auto rot = camera.get_component_handle<rotation>().read();
        math::vec3 move = math::toMat3(rot) * math::vec3(0.f, 0.f, 1.f);
        move = math::normalize(move * math::vec3(1, 0, 1)) * action->value * action->input_delta * 10.f;
        posH.fetch_add(move);

        auto pos = posH.read();
        skybox.write_component(pos);
        //groundplane.write_component(position(pos.x, 0, pos.z));
    }

    void onPlayerStrive(player_strive* action)
    {
        auto posH = camera.get_component_handle<position>();
        auto rot = camera.get_component_handle<rotation>().read();
        math::vec3 move = math::toMat3(rot) * math::vec3(1.f, 0.f, 0.f);
        move = math::normalize(move * math::vec3(1, 0, 1)) * action->value * action->input_delta * 10.f;
        posH.fetch_add(move);

        auto pos = posH.read();
        skybox.write_component(pos);
        //groundplane.write_component(position(pos.x, 0, pos.z));
    }

    void onPlayerFly(player_fly* action)
    {
        auto posH = camera.get_component_handle<position>();
        posH.fetch_add(math::vec3(0.f, action->value * action->input_delta * 10.f, 0.f));

        auto pos = posH.read();
        skybox.write_component(pos);
        //groundplane.write_component(position(pos.x, 0, pos.z));
    }

    void onPlayerLookX(player_look_x* action)
    {
        auto rotH = camera.get_component_handle<rotation>();
        rotH.fetch_multiply(math::angleAxis(action->value, math::vec3(0, 1, 0)));
        rotH.read_modify_write(rotation(), [](const rotation& src, rotation&& dummy)
            {
                (void)dummy;
                math::vec3 fwd = math::toMat3(src) * math::vec3(0.f, 0.f, 1.f);
                if (fwd.y < -0.95f)
                    fwd.y = -0.95f;
                else if (fwd.y > 0.95f)
                    fwd.y = 0.95f;
                fwd = math::normalize(fwd);
                math::vec3 right = math::cross(fwd, math::vec3(0.f, 1.f, 0.f));
                return (rotation)math::conjugate(math::toQuat(math::lookAt(math::vec3(0.f, 0.f, 0.f), fwd, math::cross(right, fwd))));
            });
    }

    void onPlayerLookY(player_look_y* action)
    {
        auto rotH = camera.get_component_handle<rotation>();
        rotH.fetch_multiply(math::angleAxis(action->value, math::vec3(1, 0, 0)));
        rotH.read_modify_write(rotation(), [](const rotation& src, rotation&& dummy)
            {
                (void)dummy;
                math::vec3 fwd = math::toMat3(src) * math::vec3(0.f, 0.f, 1.f);
                if (fwd.y < -0.95f)
                    fwd.y = -0.95f;
                else if (fwd.y > 0.95f)
                    fwd.y = 0.95f;
                fwd = math::normalize(fwd);
                math::vec3 right = math::cross(fwd, math::vec3(0.f, 1.f, 0.f));
                return (rotation)math::conjugate(math::toQuat(math::lookAt(math::vec3(0.f, 0.f, 0.f), fwd, math::cross(right, fwd))));
            });
    }
#pragma endregion

};
