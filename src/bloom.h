#pragma once
#include "gl/gl.h"
#include "log.h"
#include "mesh_factory.h"
#include "fbo.h"

typedef struct 
{
    fbo_t* bloom_theshold;
    fbo_t* upsample_tex[7];
    fbo_t* downsample_tex[7];
    object_t* bloom_quad;
    shader_t* downsample;
    shader_t* upsample;
    shader_t* threshold;

}bloom_t;

static bloom_t bloom;

//game_t* game, float x_res_frac, float y_res_frac
// fbo = res / screen rez;

void load_bloom(game_t* game)
{
    bloom.downsample = create_shader(&game->gl,"bloom_downsample");
  //  bloom.upsample = create_shader(&game->gl,"bloom_upsample");
    bloom.threshold = create_shader(&game->gl,"bloom_threshold");
    bloom.bloom_quad = create_quad(&game->gl,bloom.threshold);
    for (size_t i = 0; i < 5; i++)
	{
        float exp = pow(0.5, i);
		float width = 960 * exp;
		float height = 540 * exp;
		bloom.downsample_tex[i] = new_fbo(game,width/game->win->x_res,height/game->win->y_res);
		bloom.upsample_tex[i] = new_fbo(game,width/game->win->x_res,height/game->win->y_res);
	}
    bloom.bloom_theshold = new_fbo(game,1.0,1.0);
}

void render_bloom(gl_funcs_t* gl, bloom_t bloom, GLuint color_texture)
{
    //Theshold  :-)
    use_fbo(bloom.bloom_theshold);

    //set threshold shader

    set_shader(bloom.bloom_quad,bloom.threshold);
    gl->ActiveTexture(GL_TEXTURE0);
	gl->BindTexture(GL_TEXTURE_2D, color_texture);

    shader_uniform(bloom.threshold, "framebuffer", (uint32_t) 0);
    render_object(gl, bloom.bloom_quad);

    
     //Downsampling ! 
     //Change viewport ? 
    set_shader(bloom.bloom_quad,bloom.downsample);

    render_object(gl, bloom.bloom_quad);

    for (size_t i = 0; i < 5; i++)
	{

		float width = 1920.0F * pow(0.5, i);
		float height = 1080.0F * pow(0.5, i);
		float texel[2] ={1.0F / width, 1.0F / height};
        
        use_fbo(bloom.downsample_tex[i]);

		if (i == 0)
		{
            gl->BindTexture(GL_TEXTURE_2D, bloom.bloom_theshold->colour_texture);
            shader_uniform(bloom.bloom_quad->shader,"bloom_texture",(uint32_t)0);
		}
		else
		{
            gl->BindTexture(GL_TEXTURE_2D, bloom.downsample_tex[i-1]->colour_texture);
            shader_uniform(bloom.bloom_quad->shader,"bloom_texture",(uint32_t)0);
		}
        shader_uniform(bloom.bloom_quad->shader,"TexelSize",texel);
        render_object(gl, bloom.bloom_quad);

        gl->BindFramebuffer(GL_FRAMEBUFFER,0);
	    gl->ClearColor(0.7, 0.1, 0.1, 1.0);
        gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	}

    //Upsample ?



}