#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "../externals/stb_image.h"

#include "gl/gl.h"
#include "log.h"
#include <malloc.h>

GLuint loadTexture2D(gl_funcs_t *gl, const char *texture_src)
{
	GLuint texture;
	int width, height, channels;
	unsigned char *data = stbi_load(texture_src, &width, &height, &channels, STBI_rgb_alpha);
	if(!data)
		LOG("Error while loading image: %s",texture_src)
	gl->GenTextures(1, &texture);
	gl->BindTexture(GL_TEXTURE_2D, texture);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (channels > 3)
	{
		gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	gl->GenerateMipmap(GL_TEXTURE_2D);

	gl->BindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
	return texture;
}

GLuint loadcubemap(gl_funcs_t*gl, char* directory)
{

    int width, height, channels;
    unsigned char* data;
    GLuint texture;
    const char* positions[6] = {"right.jpg","left.jpg","top.jpg","bottom.jpg","front.jpg","back.jpg"};
    gl->GenTextures(1,&texture);
    gl->BindTexture(GL_TEXTURE_CUBE_MAP,texture);
    for(unsigned int i = 0 ; i < 6 ; i++)
    {
        size_t len = snprintf(NULL, 0, directory, positions[i]);
	    char* path = malloc(len + 1);
	    snprintf(path, len + 1, directory, positions[i]);
        data = stbi_load(path,&width,&height,&channels,STBI_rgb);
        if(!data)
            LOG("Error while loading : %s",path);
        gl->TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
    }
    //set texture parameters:
    gl->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    gl->BindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return texture;
}

void create_albedo_tex_array(gl_funcs_t*gl)
{
	GLuint texture;
	gl->GenTextures(1,&texture);
	gl->BindTexture(GL_TEXTURE_2D_ARRAY,texture);

}