#pragma once
#include <application/window/window.hpp>
#include <application/context/contexthelper.hpp>
#include <application/events/windowevents.hpp>
#include "application/events/windowinputevents.hpp"

/**@file windowsystem.hpp
*/

namespace legion::application
{
    /**@class WindowSystem
     * @brief The system that's responsible for raising and polling all window events,
     *        swapping the buffers of the windows and creation and destruction of windows.
     */
    class WindowSystem final : public System<WindowSystem>
    {
    private:
        struct window_request
        {
            id_type entityId;
            math::ivec2 size;
            std::string name;
            image_handle icon;
            GLFWmonitor* monitor;
            GLFWwindow* share;
            int swapInterval;
            std::vector<std::pair<int, int>> hints;

            window_request(id_type entityId, math::ivec2 size, const std::string& name, image_handle icon, GLFWmonitor* monitor, GLFWwindow* share, int swapInterval, const std::vector<std::pair<int, int>>& hints)
                : entityId(entityId), size(size), name(name), icon(icon), monitor(monitor), share(share), swapInterval(swapInterval), hints(hints)
            {}
            window_request(id_type entityId, math::ivec2 size = { 400, 400 }, const std::string& name = "LEGION Engine", image_handle icon = invalid_image_handle, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr, int swapInterval = 0)
                : entityId(entityId), size(size), name(name), icon(icon), monitor(monitor), share(share), swapInterval(swapInterval)
            {}

            window_request(id_type entityId, math::ivec2 size, const std::string& name, const std::string& iconName, GLFWmonitor* monitor, GLFWwindow* share, int swapInterval, const std::vector<std::pair<int, int>>& hints)
                : entityId(entityId), size(size), name(name), icon(ImageCache::get_handle(iconName)), monitor(monitor), share(share), swapInterval(swapInterval), hints(hints)
            {}
            window_request(id_type entityId, math::ivec2 size, const std::string& name, const std::string& iconName, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr, int swapInterval = 0)
                : entityId(entityId), size(size), name(name), icon(ImageCache::get_handle(iconName)), monitor(monitor), share(share), swapInterval(swapInterval)
            {}
        };

        struct fullscreen_toggle_request
        {
            id_type entityId;
            math::ivec2 position;
            math::ivec2 size;

            fullscreen_toggle_request(id_type entityId, math::ivec2 position = { 100 ,100 }, math::ivec2 size = { 400, 400 }) : entityId(entityId), position(position), size(size) {}
        };

        struct icon_request
        {
            id_type entityId;
            image_handle icon;

            icon_request(id_type entityId, image_handle icon) : entityId(entityId), icon(icon) {}
            icon_request(id_type entityId, const std::string& iconName) : entityId(entityId), icon(ImageCache::get_handle(iconName)) {}
        };

        static sparse_map<GLFWwindow*, ecs::component_handle<window>> m_windowComponents;
        static async::readonly_rw_spinlock m_creationLock;

        ecs::EntityQuery m_windowQuery{}; // Query with all the windows to update.
        bool m_exit = false; // Keep track of whether the exit event has been raised.
                             // If any window requests happen after this boolean has been set then they will be denied.

        static async::readonly_rw_spinlock m_creationRequestLock; // Lock to keep the creation request list thread-safe.
        static std::vector<window_request> m_creationRequests; // List of requests since the last creation loop.

        static async::readonly_rw_spinlock m_fullscreenRequestLock; // Lock to keep the fullscreen request list thread-safe.
        static std::vector<fullscreen_toggle_request> m_fullscreenRequests; // List of requests since the last fullscreen update loop.

        static async::readonly_rw_spinlock m_iconRequestLock; // Lock to keep the icon request list thread-safe.
        static std::vector<icon_request> m_iconRequests; // List of requests since the last icon update loop.

        // Internal function for closing a window safely.
        static void closeWindow(GLFWwindow* window);

        image_handle m_defaultIcon;

#pragma region Callbacks
        static void onWindowMoved(GLFWwindow* window, int x, int y);

        static void onWindowResize(GLFWwindow* window, int width, int height);

        static void onWindowRefresh(GLFWwindow* window);

        static void onWindowFocus(GLFWwindow* window, int focused);

        static void onWindowIconify(GLFWwindow* window, int iconified);

        static void onWindowMaximize(GLFWwindow* window, int maximized);

        static void onWindowFrameBufferResize(GLFWwindow* window, int width, int height);

        static void onWindowContentRescale(GLFWwindow* window, float xscale, float yscale);

        static void onItemDroppedInWindow(GLFWwindow* window, int count, const char** paths);

        static void onMouseEnterWindow(GLFWwindow* window, int entered);

        static void onKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods);

        static void onCharInput(GLFWwindow* window, uint codepoint);

        static void onMouseMoved(GLFWwindow* window, double xpos, double ypos);

        static void onMouseButton(GLFWwindow* window, int button, int action, int mods);

        static void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset);

        void onExit(events::exit* event);
#pragma endregion

    public:
        static void requestIconChange(id_type entityId, image_handle icon);
        static void requestIconChange(id_type entityId, const std::string& iconName);

        static void requestFullscreenToggle(id_type entityId, math::ivec2 position = { 100 ,100 }, math::ivec2 size = { 400, 400 });

        static void requestWindow(id_type entityId, math::ivec2 size, const std::string& name, image_handle icon, GLFWmonitor* monitor, GLFWwindow* share, int swapInterval, const std::vector<std::pair<int, int>>& hints);
        static void requestWindow(id_type entityId, math::ivec2 size = { 400, 400 }, const std::string& name = "LEGION Engine", image_handle icon = invalid_image_handle, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr, int swapInterval = 0);
        static void requestWindow(id_type entityId, math::ivec2 size, const std::string& name, const std::string& iconName, GLFWmonitor* monitor, GLFWwindow* share, int swapInterval, const std::vector<std::pair<int, int>>& hints);
        static void requestWindow(id_type entityId, math::ivec2 size, const std::string& name, const std::string& iconName, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr, int swapInterval = 0);

        void showMainWindow()
        {
            ContextHelper::showWindow(m_ecs->world.read_component<window>());
        }

        void exit()
        {
            raiseEvent<events::exit>();
        }

        virtual void setup();

        void createWindows();

        void fullscreenWindows();

        void updateWindowIcons();

        void refreshWindows(time::time_span<fast_time> deltaTime);

        void handleWindowEvents(time::time_span<fast_time> deltaTime);
    };
}
