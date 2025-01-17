//------------------------------------------------------------------------------
//  offscreen-sapp.c
//  Render to an offscreen rendertarget texture, and use this texture
//  for rendering to the display.
//------------------------------------------------------------------------------
#include "sokol_app.h"
#include "sokol_gfx.h"
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#include "dbgui/dbgui.h"
#include "offscreen-sapp.glsl.h"

static const int MSAA_SAMPLES = 4;

static sg_pass offscreen_pass;
static sg_pipeline offscreen_pip;
static sg_bindings offscreen_bind;
static sg_pipeline default_pip;
static sg_bindings default_bind;

/* offscreen: clear to black */
static sg_pass_action offscreen_pass_action = {
    .colors[0] = { .action = SG_ACTION_CLEAR, .val = { 0.0f, 0.0f, 0.0f, 1.0f } }
};

/* display: clear to blue-ish */
static sg_pass_action default_pass_action = {
    .colors[0] = { .action = SG_ACTION_CLEAR, .val = { 0.0f, 0.25f, 1.0f, 1.0f } }
};

/* rotation angles */
static float rx, ry;

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
    __dbgui_setup(MSAA_SAMPLES);

    /* a render pass with one color- and one depth-attachment image */
    sg_image_desc img_desc = {
        .render_target = true,
        .width = 256,
        .height = 256,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .sample_count = MSAA_SAMPLES,
        .label = "color-image"
    };
    sg_image color_img = sg_make_image(&img_desc);
    img_desc.pixel_format = SG_PIXELFORMAT_DEPTH;
    img_desc.label = "depth-image";
    sg_image depth_img = sg_make_image(&img_desc);
    offscreen_pass = sg_make_pass(&(sg_pass_desc){
        .color_attachments[0].image = color_img,
        .depth_stencil_attachment.image = depth_img,
        .label = "offscreen-pass"
    });

    /* cube vertex buffer with positions, colors and tex coords */
    float vertices[] = {
        /* pos                  color                       uvs */
        -1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     0.0f, 1.0f,

        -1.0f, -1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     0.0f, 0.0f, 
         1.0f, -1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     0.0f, 1.0f,

        -1.0f, -1.0f, -1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     1.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     0.0f, 1.0f,

         1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 1.0f,

        -1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 1.0f,

        -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 1.0f,
         1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 1.0f
    };
    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(vertices),
        .content = vertices,
        .label = "cube-vertices"
    });

    /* an index buffer for the cube */
    uint16_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };
    sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(indices),
        .content = indices,
        .label = "cube-indices"
    });

    /* pipeline-state-object for offscreen-rendered cube, don't need texture coord here */
    offscreen_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            /* need to provide stride, because the buffer's texcoord is skipped */
            .buffers[0].stride = 36,
            /* but don't need to provide attr offsets, because pos and color are continuous */
            .attrs = {
                [ATTR_vs_offscreen_pos].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_offscreen_color0].format = SG_VERTEXFORMAT_FLOAT4
            }
        },
        .shader = sg_make_shader(offscreen_shader_desc()),
        .index_type = SG_INDEXTYPE_UINT16,
        .depth_stencil = {
            .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true,
        },
        .blend = {
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH
        },
        .rasterizer = {
            .cull_mode = SG_CULLMODE_BACK,
            .sample_count = MSAA_SAMPLES
        },
        .label = "offscreen-pipeline"
    });

    /* and another pipeline-state-object for the default pass */
    default_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            /* don't need to provide buffer stride or attr offsets, no gaps here */
            .attrs = {
                [ATTR_vs_default_pos].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_default_color0].format = SG_VERTEXFORMAT_FLOAT4,
                [ATTR_vs_default_uv0].format = SG_VERTEXFORMAT_FLOAT2
            }
        },
        .shader = sg_make_shader(default_shader_desc()),
        .index_type = SG_INDEXTYPE_UINT16,
        .depth_stencil = {
            .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true
        },
        .rasterizer = {
            .cull_mode = SG_CULLMODE_BACK,
            .sample_count = MSAA_SAMPLES
        },
        .label = "default-pipeline"
    });

    /* the resource bindings for rendering a non-textured cube into offscreen render target */
    offscreen_bind = (sg_bindings){
        .vertex_buffers[0] = vbuf,
        .index_buffer = ibuf
    };

    /* resource bindings to render a textured cube, using the offscreen render target as texture */
    default_bind = (sg_bindings){
        .vertex_buffers[0] = vbuf,
        .index_buffer = ibuf,
        .fs_images[SLOT_tex] = color_img
    };
}

void frame(void) {
    /* compute model-view-projection matrix for vertex shader, this will be
       used both for the offscreen-pass, and the display-pass */
    hmm_mat4 proj = HMM_Perspective(60.0f, (float)sapp_width()/(float)sapp_height(), 0.01f, 10.0f);
    hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
    vs_params_t vs_params;
    rx += 1.0f; ry += 2.0f;
    hmm_mat4 rxm = HMM_Rotate(rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
    hmm_mat4 rym = HMM_Rotate(ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
    vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

    /* the offscreen pass, rendering an rotating, untextured cube into a render target image */
    sg_begin_pass(offscreen_pass, &offscreen_pass_action);
    sg_apply_pipeline(offscreen_pip);
    sg_apply_bindings(&offscreen_bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    sg_draw(0, 36, 1);
    sg_end_pass();

    /* and the display-pass, rendering a rotating, textured cube, using the
       previously rendered offscreen render-target as texture */
    sg_begin_default_pass(&default_pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(default_pip);
    sg_apply_bindings(&default_bind);
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
        .sample_count = MSAA_SAMPLES,
        .window_title = "Offscreen Rendering (sokol-app)",
    };
}
