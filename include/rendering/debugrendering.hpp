#pragma once

#include <core/core.hpp>

namespace legion::debug
{
    struct debug_line final : public events::event<debug_line>
    {
        math::vec3 start;
        math::vec3 end;
        math::color color = math::colors::white;
        float width = 1.f;
        float time = 0;
        mutable float timeBuffer = 0;
        bool ignoreDepth = false;

        debug_line(math::vec3 start, math::vec3 end, math::color color = math::colors::white, float width = 1.f, float time = 0, bool ignoreDepth = false) : start(start), end(end), color(color), width(width), time(time), ignoreDepth(ignoreDepth) {}
        debug_line() = default;
        debug_line(const debug_line&) = default;
        debug_line(debug_line&&) = default;

        debug_line& operator=(const debug_line&) = default;
        debug_line& operator=(debug_line&&) = default;

        bool operator==(const debug_line& other) const
        {
            return start == other.start && end == other.end && color == other.color && width == other.width && ignoreDepth == other.ignoreDepth;
        }

        virtual bool unique() override { return false; }
    };

#if defined(LEGION_ENTRY)
    namespace detail
    {
        events::EventBus* eventBus = nullptr;
    }

    void drawLine(math::vec3 start, math::vec3 end, math::color color = math::colors::white, float width = 1.f, float time = 0, bool ignoreDepth = false)
    {
        if (!detail::eventBus)
            return;

        detail::eventBus->raiseEvent<debug_line>(start, end, color, width, time, ignoreDepth);
    }

    void setEventBus(events::EventBus* eventBus)
    {
        detail::eventBus = eventBus;
    }
#else
    extern void drawLine(math::vec3 start, math::vec3 end, math::color color = math::colors::white, float width = 1.f, float time = 0, bool ignoreDepth = false);

    extern void setEventBus(events::EventBus* eventBus);
#endif
}

#if !defined(DOXY_EXCLUDE)
namespace std
{
    template<>
    struct hash<legion::debug::debug_line>
    {
        std::size_t operator()(legion::debug::debug_line const& line) const noexcept
        {
            std::hash<legion::core::math::vec3> vecHasher;
            std::hash<legion::core::math::color> colHasher;
            std::hash<float> fltHasher;
            std::hash<bool> boolHasher;

            size_t seed = 0;
            legion::core::math::detail::hash_combine(seed, vecHasher(line.start));
            legion::core::math::detail::hash_combine(seed, vecHasher(line.end));
            legion::core::math::detail::hash_combine(seed, colHasher(line.color));
            legion::core::math::detail::hash_combine(seed, fltHasher(line.width));
            legion::core::math::detail::hash_combine(seed, boolHasher(line.ignoreDepth));
            return seed;
        }
    };
}
#endif
