#pragma once
#include "object.h"


object_t* create_quad(gl_funcs_t* gl, shader_t* shader)
{
    vertex_t quad[] =
	 {
	 	{.position = {1.0f,  1.0, 0.0f}, .texcoord = {1.0f, 1.0f}, .normal={1.0,1.0,1.0}},
	 	{.position = {1.0f, -1.0f, 0.0f}, .texcoord = {1.0f, 0.0f}, .normal={1.0,1.0,1.0}},
	 	{.position = {-1.0f, -1.0f, 0.0f}, .texcoord = {0.0f, 0.0f}, .normal={1.0,1.0,1.0}},
	 	{.position = {-1.0f,  1.0f, 0.0f}, .texcoord = {0.0f, 1.0f}, .normal={1.0,1.0,1.0}}
	 };
	unsigned int q_incides[6] = {0,1,3,1,2,3};
    object_t* quad_obj = create_object(gl, shader, false, quad, sizeof(quad), q_incides, sizeof(q_incides));
   return quad_obj;
}