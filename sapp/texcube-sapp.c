//------------------------------------------------------------------------------
//  texcube-sapp.c
//  Texture creation, rendering with texture, packed vertex components.
//------------------------------------------------------------------------------
#include "sokol_app.h"
#include "sokol_gfx.h"
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#include "dbgui/dbgui.h"
#include "texcube-sapp.glsl.h"

static const int SAMPLE_COUNT = 4;
static float rx, ry;
static sg_pass_action pass_action = {
    .colors[0] = { .action = SG_ACTION_CLEAR, .val = { 0.25f, 0.5f, 0.75f, 1.0f } }
};
static sg_pipeline pip;
static sg_bindings bind;

typedef struct {
    float x, y, z;
    uint32_t color;
    int16_t u, v;
} vertex_t;

void init(void) {
    sg_setup(&(sg_desc){
        .gl_force_gles2 = sapp_gles2(),
        .mtl_device = sapp_metal_get_device(),
        .mtl_renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor,
        .mtl_drawable_cb = sapp_metal_get_drawable,
        .d3d11_device = sapp_d3d11_get_device(),
        .d3d11_device_context = sapp_d3d11_get_device_context(),
        .d3d11_render_target_view_cb = sapp_d3d11_get_render_target_view,
        .d3d11_depth_stencil_view_cb = sapp_d3d11_get_depth_stencil_view
    });
    __dbgui_setup(SAMPLE_COUNT);

    /*
        Cube vertex buffer with packed vertex formats for color and texture coords.
        Note that a vertex format which must be portable across all 
        backends must only use the normalized integer formats
        (BYTE4N, UBYTE4N, SHORT2N, SHORT4N), which can be converted
        to floating point formats in the vertex shader inputs.

        The reason is that D3D11 cannot convert from non-normalized
        formats to floating point inputs (only to integer inputs),
        and WebGL2 / GLES2 don't support integer vertex shader inputs.
    */
    vertex_t vertices[] = {
        /* pos                  color       uvs */
        { -1.0f, -1.0f, -1.0f,  0xFF0000FF,     0,     0 },
        {  1.0f, -1.0f, -1.0f,  0xFF0000FF, 32767,     0 },
        {  1.0f,  1.0f, -1.0f,  0xFF0000FF, 32767, 32767 },
        { -1.0f,  1.0f, -1.0f,  0xFF0000FF,     0, 32767 },

        { -1.0f, -1.0f,  1.0f,  0xFF00FF00,     0,     0 }, 
        {  1.0f, -1.0f,  1.0f,  0xFF00FF00, 32767,     0 },
        {  1.0f,  1.0f,  1.0f,  0xFF00FF00, 32767, 32767 },
        { -1.0f,  1.0f,  1.0f,  0xFF00FF00,     0, 32767 },

        { -1.0f, -1.0f, -1.0f,  0xFFFF0000,     0,     0 },
        { -1.0f,  1.0f, -1.0f,  0xFFFF0000, 32767,     0 },
        { -1.0f,  1.0f,  1.0f,  0xFFFF0000, 32767, 32767 },
        { -1.0f, -1.0f,  1.0f,  0xFFFF0000,     0, 32767 },

        {  1.0f, -1.0f, -1.0f,  0xFFFF007F,     0,     0 },
        {  1.0f,  1.0f, -1.0f,  0xFFFF007F, 32767,     0 },
        {  1.0f,  1.0f,  1.0f,  0xFFFF007F, 32767, 32767 },
        {  1.0f, -1.0f,  1.0f,  0xFFFF007F,     0, 32767 },

        { -1.0f, -1.0f, -1.0f,  0xFFFF7F00,     0,     0 },
        { -1.0f, -1.0f,  1.0f,  0xFFFF7F00, 32767,     0 },
        {  1.0f, -1.0f,  1.0f,  0xFFFF7F00, 32767, 32767 },
        {  1.0f, -1.0f, -1.0f,  0xFFFF7F00,     0, 32767 },

        { -1.0f,  1.0f, -1.0f,  0xFF007FFF,     0,     0 },
        { -1.0f,  1.0f,  1.0f,  0xFF007FFF, 32767,     0 },
        {  1.0f,  1.0f,  1.0f,  0xFF007FFF, 32767, 32767 },
        {  1.0f,  1.0f, -1.0f,  0xFF007FFF,     0, 32767 },
    };
    bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(vertices),
        .content = vertices,
        .label = "cube-vertices"
    });

    /* create an index buffer for the cube */
    uint16_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };
    bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(indices),
        .content = indices,
        .label = "cube-indices"
    });

    /* create a checkerboard texture */
    uint32_t pixels[4*4] = {
        0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
        0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
        0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
    };
    /* NOTE: tex_slot is provided by shader code generation */
    bind.fs_images[SLOT_tex] = sg_make_image(&(sg_image_desc){
        .width = 4,
        .height = 4,
        .content.subimage[0][0] = {
            .ptr = pixels,
            .size = sizeof(pixels)
        },
        .label = "cube-texture"
    });

    /* a shader */
    sg_shader shd = sg_make_shader(texcube_shader_desc());

    /* a pipeline state object */
    pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .attrs = {
                [ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_color0].format = SG_VERTEXFORMAT_UBYTE4N,
                [ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_SHORT2N
            }
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .depth_stencil = {
            .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true
        },
        .rasterizer = {
            .cull_mode = SG_CULLMODE_BACK,
            .sample_count = SAMPLE_COUNT
        },
        .label = "cube-pipeline"
    });
}

void frame(void) {
    /* compute model-view-projection matrix for vertex shader */
    hmm_mat4 proj = HMM_Perspective(60.0f, (float)sapp_width()/(float)sapp_height(), 0.01f, 10.0f);
    hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
    vs_params_t vs_params;
    rx += 1.0f; ry += 2.0f;
    hmm_mat4 rxm = HMM_Rotate(rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
    hmm_mat4 rym = HMM_Rotate(ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
    vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(pip);
    sg_apply_bindings(&bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    sg_draw(0, 36, 1);
    __dbgui_draw();
    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    __dbgui_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = __dbgui_event,
        .width = 800,
        .height = 600,
        .sample_count = SAMPLE_COUNT,
        .gl_force_gles2 = true,
        .window_title = "Textured Cube (sokol-app)",
    };
}
