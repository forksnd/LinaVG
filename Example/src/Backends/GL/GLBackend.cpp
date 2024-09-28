/*
This file is a part of: LinaVG
https://github.com/inanevin/LinaVG

Author: Inan Evin
http://www.inanevin.com

The 2-Clause BSD License

Copyright (c) [2022-] Inan Evin

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
	  list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "glad/glad.h"
#include "Backends/GL/GLBackend.hpp"
#include "LinaVG/Core/BufferStore.hpp"
#include "LinaVG/Core/Drawer.hpp"
#include "LinaVG/Core/Math.hpp"
#include <iostream>
#include <stdio.h>


#define STB_IMAGE_IMPLEMENTATION
#include "Utility/stb_image.h"

namespace LinaVG::Examples
{

	unsigned int GLBackend::s_displayPosX	= 0;
	unsigned int GLBackend::s_displayPosY	= 0;
	unsigned int GLBackend::s_displayWidth	= 0;
	unsigned int GLBackend::s_displayHeight = 0;

#define FONT_ATLAS_WIDTH 2048
#define FONT_ATLAS_HEIGHT 2048

	GLBackend::GLBackend()
	{
		m_backendData.m_defaultVtxShader = "#version 330 core\n"
										   "layout (location = 0) in vec2 pos;\n"
										   "layout (location = 1) in vec2 uv;\n"
										   "layout (location = 2) in vec4 col;\n"
										   "uniform mat4 proj; \n"
										   "out vec4 fCol;\n"
										   "out vec2 fUV;\n"
										   "void main()\n"
										   "{\n"
										   "   fCol = col;\n"
										   "   fUV = uv;\n"
										   "   gl_Position = proj * vec4(pos.x, pos.y, 0.0f, 1.0);\n"
										   "}\0";

		m_backendData.m_defaultFragShader = "#version 330 core\n"
											"out vec4 fragColor;\n"
											"in vec4 fCol;\n"
											"void main()\n"
											"{\n"
											"   fragColor = fCol;\n"
											"}\0";

		m_backendData.m_texturedFragShader = "#version 330 core\n"
											 "out vec4 fragColor;\n"
											 "in vec2 fUV;\n"
											 "in vec4 fCol;\n"
											 "uniform sampler2D diffuse;\n"
											 "uniform vec2 tiling;\n"
											 "uniform vec2 offset;\n"
											 "uniform vec4 tint;\n"
											 "uniform int isAABuffer;\n"
											 "void main()\n"
											 "{\n"
											 "   vec4 col = texture(diffuse, fUV * tiling + offset);\n"
											 "   fragColor = vec4(col.rgb, isAABuffer == 1 ? fCol.a : col.a) * tint; \n"
											 "}\0";

		m_backendData.m_simpleTextFragShader = "#version 330 core\n"
											   "out vec4 fragColor;\n"
											   "in vec2 fUV;\n"
											   "in vec4 fCol;\n"
											   "uniform sampler2D diffuse;\n"
											   "void main()\n"
											   "{\n"
											   "float a = texture(diffuse, fUV).r; \n"
											   "fragColor = vec4(fCol.rgb, a * fCol.a); \n"
											   "}\0";

		m_backendData.m_sdfTextFragShader = "#version 330 core\n"
											"out vec4 fragColor;\n"
											"in vec2 fUV;\n"
											"in vec4 fCol;\n"
											"uniform sampler2D diffuse;\n"
											"uniform float softness; \n"
											"uniform float thickness; \n"
											"uniform int outlineEnabled; \n"
											"uniform int useOutlineOffset; \n"
											"uniform vec2 outlineOffset; \n"
											"uniform float outlineThickness; \n"
											"uniform vec4 outlineColor; \n"
											"uniform int flipAlpha; \n"
											"void main()\n"
											"{\n"
											"float distance = texture(diffuse, fUV).r;\n"
											"float alpha = smoothstep(thickness - softness, thickness + softness, distance);\n"
											"vec3 baseColor = fCol.rgb;\n"
											"if(outlineEnabled == 1){\n"
											" float border = smoothstep(thickness + outlineThickness - softness, thickness + outlineThickness + softness, distance);\n"
											" baseColor = mix(outlineColor, fCol, border).rgb;\n"
											"} \n"
											"fragColor = vec4(baseColor, flipAlpha == 1 ? 1.0f - alpha : alpha);\n"
											"}\0";

		m_backendData.m_roundedGradientFragShader = "#version 330 core\n"
													"out vec4 fragColor;\n"
													"in vec2 fUV;\n"
													"in vec4 fCol;\n"
													"uniform vec4 startColor;\n"
													"uniform vec4 endColor;\n"
													"uniform int  gradientType;\n"
													"uniform float radialSize;\n"
													"uniform int isAABuffer;\n"
													"void main()\n"
													"{\n"
													"   if(gradientType == 0) {\n"
													"     vec4 col = mix(startColor, endColor, fUV.x);\n"
													"     fragColor = vec4(col.rgb, isAABuffer == 1 ? fCol.a : col.a); \n"
													"}\n"
													"   else if(gradientType == 1){\n"
													"     vec4 col = mix(startColor, endColor, fUV.y);\n"
													"     fragColor = vec4(col.rgb, isAABuffer == 1 ? fCol.a : col.a); \n"
													"}\n"
													"   else if(gradientType == 2) \n"
													"   {\n"
													"       vec2 uv = fUV - vec2(0.5, 0.5);\n"
													"       float dist = length(uv * radialSize);\n"
													"       vec4 col = mix(startColor, endColor, dist);\n"
													"       fragColor = vec4(col.rgb, isAABuffer == 1 ? fCol.a : col.a); \n"
													"    }\n"
													"   else if(gradientType == 3) \n"
													"   {\n"
													"       float dist = length(fUV * radialSize);\n"
													"       vec4 col = mix(startColor, endColor, dist);\n"
													"       fragColor = vec4(col.rgb, isAABuffer == 1 ? fCol.a : col.a); \n"
													"    }\n"
													"}\n\0";

		try
		{
			CreateShader(m_backendData.m_defaultShaderData, m_backendData.m_defaultVtxShader, m_backendData.m_defaultFragShader);
			CreateShader(m_backendData.m_gradientShaderData, m_backendData.m_defaultVtxShader, m_backendData.m_roundedGradientFragShader);
			CreateShader(m_backendData.m_texturedShaderData, m_backendData.m_defaultVtxShader, m_backendData.m_texturedFragShader);
			CreateShader(m_backendData.m_simpleTextShaderData, m_backendData.m_defaultVtxShader, m_backendData.m_simpleTextFragShader);
			CreateShader(m_backendData.m_sdfTextShaderData, m_backendData.m_defaultVtxShader, m_backendData.m_sdfTextFragShader);
		}
		catch (const std::runtime_error& err)
		{
			if (Config.errorCallback)
			{
				Config.errorCallback("LinaVG: Backend shader creation failed!");
				Config.errorCallback(err.what());
			}
			return;
		}

		glGenVertexArrays(1, &m_backendData.m_vao);
		glGenBuffers(1, &m_backendData.m_vbo);
		glGenBuffers(1, &m_backendData.m_ebo);

		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray(m_backendData.m_vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_backendData.m_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_backendData.m_ebo);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glGenVertexArrays(1, &m_backendData.m_vao);
		glGenBuffers(1, &m_backendData.m_vbo);
		glGenBuffers(1, &m_backendData.m_ebo);

		glBindVertexArray(m_backendData.m_vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_backendData.m_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_backendData.m_ebo);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
		// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
		glBindVertexArray(0);
	}

	void GLBackend::StartFrame()
	{
		Config.debugCurrentDrawCalls	 = 0;
		Config.debugCurrentTriangleCount = 0;
		Config.debugCurrentVertexCount	 = 0;

		// Save GL state
		SaveAPIState();

		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glEnable(GL_SCISSOR_TEST);

		if (Config.debugWireframeEnabled)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glViewport(0, 0, (GLsizei)s_displayWidth, (GLsizei)s_displayHeight);

		// Ortho projection matrix.
		int fb_width  = (int)(s_displayWidth);
		int fb_height = (int)(s_displayHeight);
		if (fb_width <= 0 || fb_height <= 0)
		{
			m_backendData.m_skipDraw = true;
			return;
		}

		m_backendData.m_skipDraw = false;

		float		L	 = static_cast<float>(s_displayPosX);
		float		R	 = static_cast<float>(s_displayPosX + s_displayWidth);
		float		T	 = static_cast<float>(s_displayPosY);
		float		B	 = static_cast<float>(s_displayPosY + s_displayHeight);
		const float zoom = Config.debugOrthoProjectionZoom;

		L *= zoom;
		R *= zoom;
		T *= zoom;
		B *= zoom;

		L += Config.debugOrthoOffset.x;
		R += Config.debugOrthoOffset.x;
		T += Config.debugOrthoOffset.y;
		B += Config.debugOrthoOffset.y;

		m_backendData.m_proj[0][0] = 2.0f / (R - L);
		m_backendData.m_proj[0][1] = 0.0f;
		m_backendData.m_proj[0][2] = 0.0f;
		m_backendData.m_proj[0][3] = 0.0f;

		m_backendData.m_proj[1][0] = 0.0f;
		m_backendData.m_proj[1][1] = 2.0f / (T - B);
		m_backendData.m_proj[1][2] = 0.0f;
		m_backendData.m_proj[1][3] = 0.0f;

		m_backendData.m_proj[2][0] = 0.0f;
		m_backendData.m_proj[2][1] = 0.0f;
		m_backendData.m_proj[2][2] = -1.0f;
		m_backendData.m_proj[2][3] = 0.0f;

		m_backendData.m_proj[3][0] = (R + L) / (L - R);
		m_backendData.m_proj[3][1] = (T + B) / (B - T);
		m_backendData.m_proj[3][2] = 0.0f;
		m_backendData.m_proj[3][3] = 1.0f;

		glBindVertexArray(m_backendData.m_vao);
	}

	void GLBackend::DrawGradient(GradientDrawBuffer* buf)
	{
		if (m_backendData.m_skipDraw)
			return;

		SetScissors(buf->clipPosX, buf->clipPosY, buf->clipSizeX, buf->clipSizeY);

		ShaderData& data = m_backendData.m_gradientShaderData;
		glUseProgram(data.m_handle);

		glUniformMatrix4fv(data.m_uniformMap["proj"], 1, GL_FALSE, &m_backendData.m_proj[0][0]);
		glUniform4f(data.m_uniformMap["startColor"], (GLfloat)buf->m_color.start.x, (GLfloat)buf->m_color.start.y, (GLfloat)buf->m_color.start.z, (GLfloat)buf->m_color.start.w);
		glUniform4f(data.m_uniformMap["endColor"], (GLfloat)buf->m_color.end.x, (GLfloat)buf->m_color.end.y, (GLfloat)buf->m_color.end.z, (GLfloat)buf->m_color.end.w);
		glUniform1i(data.m_uniformMap["gradientType"], (GLint)((int)buf->m_color.gradientType));
		glUniform1f(data.m_uniformMap["radialSize"], (GLfloat)buf->m_color.radialSize);
		glUniform1i(data.m_uniformMap["isAABuffer"], (GLint)((int)buf->m_isAABuffer));

		glBindBuffer(GL_ARRAY_BUFFER, m_backendData.m_vbo);
		glBufferData(GL_ARRAY_BUFFER, buf->m_vertexBuffer.m_size * sizeof(Vertex), (const GLvoid*)buf->m_vertexBuffer.begin(), GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_backendData.m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf->m_indexBuffer.m_size * sizeof(Index), (const GLvoid*)buf->m_indexBuffer.begin(), GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElements(GL_TRIANGLES, (GLsizei)buf->m_indexBuffer.m_size, sizeof(Index) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 0);
		Config.debugCurrentDrawCalls++;
		Config.debugCurrentTriangleCount += int((float)buf->m_indexBuffer.m_size / 3.0f);
		Config.debugCurrentVertexCount += buf->m_vertexBuffer.m_size;
	}

	void GLBackend::DrawTextured(TextureDrawBuffer* buf)
	{
		if (m_backendData.m_skipDraw)
			return;

		SetScissors(buf->clipPosX, buf->clipPosY, buf->clipSizeX, buf->clipSizeY);

		const Vec2	uv	 = buf->m_textureUVTiling;
		ShaderData& data = m_backendData.m_texturedShaderData;
		glUseProgram(data.m_handle);
		glUniformMatrix4fv(data.m_uniformMap["proj"], 1, GL_FALSE, &m_backendData.m_proj[0][0]);

		glUniform1i(data.m_uniformMap["diffuse"], 0);
		glUniform2f(data.m_uniformMap["tiling"], (GLfloat)uv.x, (GLfloat)uv.y);
		glUniform2f(data.m_uniformMap["offset"], (GLfloat)buf->m_textureUVOffset.x, (GLfloat)buf->m_textureUVOffset.y);
		glUniform4f(data.m_uniformMap["tint"], (GLfloat)buf->m_tint.x, (GLfloat)buf->m_tint.y, (GLfloat)buf->m_tint.z, (GLfloat)buf->m_tint.w);
		glUniform1i(data.m_uniformMap["isAABuffer"], (GLint)((int)buf->m_isAABuffer));
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, static_cast<Texture*>(buf->m_textureHandle)->handle);

		glBindBuffer(GL_ARRAY_BUFFER, m_backendData.m_vbo);
		glBufferData(GL_ARRAY_BUFFER, buf->m_vertexBuffer.m_size * sizeof(Vertex), (const GLvoid*)buf->m_vertexBuffer.begin(), GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_backendData.m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf->m_indexBuffer.m_size * sizeof(Index), (const GLvoid*)buf->m_indexBuffer.begin(), GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElements(GL_TRIANGLES, (GLsizei)buf->m_indexBuffer.m_size, sizeof(Index) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 0);
		Config.debugCurrentDrawCalls++;
		Config.debugCurrentTriangleCount += int((float)buf->m_indexBuffer.m_size / 3.0f);
		Config.debugCurrentVertexCount += buf->m_vertexBuffer.m_size;
	}

	void GLBackend::DrawDefault(DrawBuffer* buf)
	{
		if (m_backendData.m_skipDraw)
			return;

		SetScissors(buf->clipPosX, buf->clipPosY, buf->clipSizeX, buf->clipSizeY);
		ShaderData& data = m_backendData.m_defaultShaderData;

		glUseProgram(data.m_handle);
		glUniformMatrix4fv(data.m_uniformMap["proj"], 1, GL_FALSE, &m_backendData.m_proj[0][0]);

		glBindBuffer(GL_ARRAY_BUFFER, m_backendData.m_vbo);
		glBufferData(GL_ARRAY_BUFFER, buf->m_vertexBuffer.m_size * sizeof(Vertex), (const GLvoid*)buf->m_vertexBuffer.begin(), GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_backendData.m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf->m_indexBuffer.m_size * sizeof(Index), (const GLvoid*)buf->m_indexBuffer.begin(), GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElements(GL_TRIANGLES, (GLsizei)buf->m_indexBuffer.m_size, sizeof(Index) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 0);
		Config.debugCurrentDrawCalls++;
		Config.debugCurrentTriangleCount += int((float)buf->m_indexBuffer.m_size / 3.0f);
		Config.debugCurrentVertexCount += buf->m_vertexBuffer.m_size;
	}

	void GLBackend::DrawSimpleText(SimpleTextDrawBuffer* buf)
	{
		if (m_backendData.m_skipDraw)
			return;

		SetScissors(buf->clipPosX, buf->clipPosY, buf->clipSizeX, buf->clipSizeY);
		ShaderData& data = m_backendData.m_simpleTextShaderData;
		glUseProgram(data.m_handle);

		glUniformMatrix4fv(data.m_uniformMap["proj"], 1, GL_FALSE, &m_backendData.m_proj[0][0]);

		glUniform1i(data.m_uniformMap["diffuse"], 0);
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_fontTexture);

		glBindBuffer(GL_ARRAY_BUFFER, m_backendData.m_vbo);
		glBufferData(GL_ARRAY_BUFFER, buf->m_vertexBuffer.m_size * sizeof(Vertex), (const GLvoid*)buf->m_vertexBuffer.begin(), GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_backendData.m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf->m_indexBuffer.m_size * sizeof(Index), (const GLvoid*)buf->m_indexBuffer.begin(), GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElements(GL_TRIANGLES, (GLsizei)buf->m_indexBuffer.m_size, sizeof(Index) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 0);
		Config.debugCurrentDrawCalls++;
		Config.debugCurrentTriangleCount += int((float)buf->m_indexBuffer.m_size / 3.0f);
		Config.debugCurrentVertexCount += buf->m_vertexBuffer.m_size;
	}

	void GLBackend::DrawSDFText(SDFTextDrawBuffer* buf)
	{
		if (m_backendData.m_skipDraw)
			return;

		SetScissors(buf->clipPosX, buf->clipPosY, buf->clipSizeX, buf->clipSizeY);
		ShaderData& data = m_backendData.m_sdfTextShaderData;
		glUseProgram(data.m_handle);
		glUniformMatrix4fv(data.m_uniformMap["proj"], 1, GL_FALSE, &m_backendData.m_proj[0][0]);

		const float thickness		 = 1.0f - Math::Clamp(buf->m_thickness, 0.0f, 1.0f);
		const float softness		 = Math::Clamp(buf->m_softness, 0.0f, 10.0f) * 0.1f;
		const float outlineThickness = Math::Clamp(buf->m_outlineThickness, 0.0f, 1.0f);
		glUniform1i(data.m_uniformMap["diffuse"], 0);
		glUniform1f(data.m_uniformMap["thickness"], thickness);
		glUniform1f(data.m_uniformMap["softness"], softness);
		glUniform1i(data.m_uniformMap["outlineEnabled"], outlineThickness != 0.0f ? 1 : 0);
		glUniform1i(data.m_uniformMap["flipAlpha"], buf->m_flipAlpha ? 1 : 0);
		glUniform1f(data.m_uniformMap["outlineThickness"], outlineThickness);
		glUniform4f(data.m_uniformMap["outlineColor"], buf->m_outlineColor.x, buf->m_outlineColor.y, buf->m_outlineColor.z, buf->m_outlineColor.w);
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_fontTexture);

		glBindBuffer(GL_ARRAY_BUFFER, m_backendData.m_vbo);
		glBufferData(GL_ARRAY_BUFFER, buf->m_vertexBuffer.m_size * sizeof(Vertex), (const GLvoid*)buf->m_vertexBuffer.begin(), GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_backendData.m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf->m_indexBuffer.m_size * sizeof(Index), (const GLvoid*)buf->m_indexBuffer.begin(), GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElements(GL_TRIANGLES, (GLsizei)buf->m_indexBuffer.m_size, sizeof(Index) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 0);
		Config.debugCurrentDrawCalls++;
		Config.debugCurrentTriangleCount += int((float)buf->m_indexBuffer.m_size / 3.0f);
		Config.debugCurrentVertexCount += buf->m_vertexBuffer.m_size;
	}

	void GLBackend::SetScissors(BackendHandle x, BackendHandle y, BackendHandle width, BackendHandle height)
	{
		if (width == 0 || height == 0)
		{
			x	   = static_cast<BackendHandle>(s_displayPosX);
			y	   = static_cast<BackendHandle>(s_displayPosY);
			width  = static_cast<BackendHandle>(s_displayWidth);
			height = static_cast<BackendHandle>(s_displayHeight);
		}

		glScissor(x, static_cast<GLint>(s_displayHeight - (y + height)), static_cast<GLint>(width), static_cast<GLint>(height));
	}

	void GLBackend::SaveAPIState()
	{
		GLboolean blendEnabled;
		GLboolean cullFaceEnabled;
		GLboolean stencilTestEnabled;
		GLboolean depthTestEnabled;
		GLboolean scissorTestEnabled;
		GLboolean depthMaskEnabled;
		GLint	  blendEq;
		GLint	  blendSrcAlpha;
		GLint	  blendSrcRGB;
		GLint	  blendDestAlpha;
		GLint	  blendDestRGB;
		GLint	  unpackAlignment;
		glGetIntegerv(GL_BLEND_EQUATION, &blendEq);
		glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcAlpha);
		glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrcRGB);
		glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDestAlpha);
		glGetIntegerv(GL_BLEND_DST_RGB, &blendDestRGB);
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment);
		glGetBooleanv(GL_BLEND, &blendEnabled);
		glGetBooleanv(GL_CULL_FACE, &cullFaceEnabled);
		glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
		glGetBooleanv(GL_STENCIL_TEST, &stencilTestEnabled);
		glGetBooleanv(GL_SCISSOR_TEST, &scissorTestEnabled);
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMaskEnabled);
		m_glState.m_blendDestAlpha	   = static_cast<int>(blendDestAlpha);
		m_glState.m_blendDestRGB	   = static_cast<int>(blendDestRGB);
		m_glState.m_blendEq			   = static_cast<int>(blendEq);
		m_glState.m_blendSrcAlpha	   = static_cast<int>(blendSrcAlpha);
		m_glState.m_blendSrcRGB		   = static_cast<int>(blendSrcRGB);
		m_glState.m_unpackAlignment	   = static_cast<int>(unpackAlignment);
		m_glState.m_blendEnabled	   = static_cast<bool>(blendEnabled);
		m_glState.m_cullFaceEnabled	   = static_cast<bool>(cullFaceEnabled);
		m_glState.m_depthTestEnabled   = static_cast<bool>(depthTestEnabled);
		m_glState.m_scissorTestEnabled = static_cast<bool>(scissorTestEnabled);
		m_glState.m_stencilTestEnabled = static_cast<bool>(stencilTestEnabled);
		m_glState.m_depthMaskEnabled   = static_cast<bool>(depthMaskEnabled);
	}

	void GLBackend::RestoreAPIState()
	{
		if (m_glState.m_blendEnabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);

		if (m_glState.m_depthTestEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		if (m_glState.m_cullFaceEnabled)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

		if (m_glState.m_stencilTestEnabled)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);

		if (m_glState.m_scissorTestEnabled)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);

		if (m_glState.m_depthMaskEnabled)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);

		glBlendEquation(static_cast<GLenum>(m_glState.m_blendEq));
		glBlendFuncSeparate(static_cast<GLenum>(m_glState.m_blendSrcRGB), static_cast<GLenum>(m_glState.m_blendDestRGB), static_cast<GLenum>(m_glState.m_blendSrcAlpha), static_cast<GLenum>(m_glState.m_blendDestAlpha));
		glPixelStorei(GL_UNPACK_ALIGNMENT, m_glState.m_unpackAlignment);
	}

	void GLBackend::EndFrame()
	{
		// Restore state.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		// Reset GL state
		RestoreAPIState();
	}

	void GLBackend::CreateShader(ShaderData& data, const char* vert, const char* frag)
	{
		unsigned int vertex, fragment;
		int			 success;
		char		 infoLog[512];

		// VTX
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vert, NULL);
		glCompileShader(vertex);
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);

			if (Config.errorCallback)
			{
				Config.errorCallback("LinaVG: Backend Error -> Shader vertex compilation failed!");
				Config.errorCallback(infoLog);
			}

			throw std::runtime_error("");
		}

		// FRAG
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &frag, NULL);
		glCompileShader(fragment);
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);

			if (Config.errorCallback)
			{
				Config.errorCallback("LinaVG: Backend Error -> Shader fragment compilation failed!");
				Config.errorCallback(infoLog);
			}

			throw std::runtime_error("");
		}

		GLuint handle = glCreateProgram();
		glAttachShader(handle, vertex);
		glAttachShader(handle, fragment);
		glLinkProgram(handle);

		glGetProgramiv(handle, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(handle, 512, NULL, infoLog);
			Config.errorCallback("LinaVG: Backend Error -> Could not link shader program!");
			Config.errorCallback(infoLog);
			throw std::runtime_error("");
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		data.m_handle = (BackendHandle)handle;
		AddShaderUniforms(data);
	}

	void GLBackend::AddShaderUniforms(ShaderData& data)
	{
		// Load uniforms.
		GLint numUniforms = 0;
		glGetProgramiv(data.m_handle, GL_ACTIVE_UNIFORMS, &numUniforms);

		// Iterate through uniforms.
		GLchar chars[256];
		for (GLint uniform = 0; uniform < numUniforms; ++uniform)
		{
			GLint	arraySize	 = 0;
			GLenum	type		 = 0;
			GLsizei actualLength = 0;

			// Get sampler uniform data & store it on our sampler map.
			glGetActiveUniform(data.m_handle, uniform, (GLsizei)256, &actualLength, &arraySize, &type, &chars[0]);

			GLint loc					 = glGetUniformLocation(data.m_handle, (char*)&chars[0]);
			data.m_uniformMap[&chars[0]] = loc;
		}
	}

    void GLBackend::CreateFontTexture(unsigned int width, unsigned int height)
    {
        GLuint tex;
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        m_fontTexture = static_cast<uint32_t>(tex);
        m_fontTextureCreated = true;
    }
    
    void GLBackend::OnAtlasUpdate(Atlas *atlas)
    {
        SaveAPIState();
        if(!m_fontTextureCreated)
            CreateFontTexture(atlas->GetSize().x, atlas->GetSize().y);
        
        glBindTexture(GL_TEXTURE_2D, m_fontTexture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas->GetSize().x, atlas->GetSize().y, GL_RED, GL_UNSIGNED_BYTE, atlas->GetData());
        RestoreAPIState();
    }

    Texture* GLBackend::LoadTexture(const char *file)
    {
        Texture* txt = new Texture();
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load and generate the texture
        int               width, height, nrChannels;
        unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 4);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        txt->handle = texture;
        return txt;
    }

} // namespace LinaVG::Examples
