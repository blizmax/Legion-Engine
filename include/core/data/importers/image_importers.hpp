#pragma once
#include <core/filesystem/assetimporter.hpp>
#include <core/data/image.hpp>

namespace legion::core
{
    struct stb_image_loader : public filesystem::resource_converter<image, image_import_settings>
    {
        constexpr static cstring extensions[] = { "", ".png", ".jpg", ".jpeg", ".jpe", ".jfif", ".jfi", ".jif", ".bmp", ".dib", ".raw", ".psd", ".psb", ".tga", ".icb", ".vda", ".vst", ".hdr", ".ppm", ".pgm" };

        virtual common::result_decay_more<image, fs_error> load(const filesystem::basic_resource& resource, image_import_settings&& settings) override;
    };
}
