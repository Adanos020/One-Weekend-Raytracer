#include <scene_definitions_for_vulkan/render_plan.hpp>


render_plan render_plan::hello_ball(const extent_2d<uint32_t>& image_size)
{
    const camera cam = camera_create_info{
        position{ 3.f, 3.f, 2.f },
        position{ 0.f, 0.f, 0.f },
        y_axis,
        30.f,
        image_size.aspect(),
        0.05f,
        { 0.f, 1.f }
    };

    scene world;
    world.sky = world.add_texture(checker_texture{ 20.f, color{ 0.5f, 0.7f, 1.f }, color{ 1.f, 0.7f, 0.5f } });
    world.add_shape(
        sphere_shape{ sphere{ position{ 0.f, 0.f, 0.f }, 1.f } },
        world.add_material(lambertian_material{
            world.add_texture(constant_texture{ color{ 1.f, 0.f, 0.f } }) }));

    return render_plan{ image_size, cam, std::move(world) };
}