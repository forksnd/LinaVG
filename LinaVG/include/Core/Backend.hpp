/*
This file is a part of: LinaVG
https://github.com/inanevin/LinaVG

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2022-] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
Class: Backend



Timestamp: 3/28/2022 2:55:16 PM
*/

#pragma once

#ifndef LinaVGBackend_HPP
#define LinaVGBackend_HPP

// Headers here.
#include "Common.hpp"

#ifdef LINAVG_BACKEND_GL
#include "Backends/GLBackend.hpp"
#endif

namespace LinaVG::Internal
{
    /// <summary>
    /// Rendering data for various backends.
    /// </summary>
    struct BackendData
    {
        BackendHandle                                                                     m_vbo                    = 0;
        BackendHandle                                                                     m_vao                    = 0;
        BackendHandle                                                                     m_ebo                    = 0;
        BackendHandle                                                                     m_textVBO                = 0;
        BackendHandle                                                                     m_textVAO                = 0;
        BackendHandle                                                                     m_textEBO                = 0;
        BackendHandle                                                                     m_boundVAO               = 0;
        BackendHandle                                                                     m_defaultShaderHandle    = 0;
        BackendHandle                                                                     m_gradientShaderHandle   = 0;
        BackendHandle                                                                     m_texturedShaderHandle   = 0;
        BackendHandle                                                                     m_sdfTextShaderHandle    = 0;
        BackendHandle                                                                     m_simpleTextShaderHandle = 0;
        std::unordered_map<BackendHandle, std::unordered_map<std::string, BackendHandle>> m_shaderUniformMap;
        float                                                                             m_proj[4][4]                = {0};
        char*                                                                             m_defaultVtxShader          = nullptr;
        char*                                                                             m_defaultFragShader         = nullptr;
        char*                                                                             m_roundedGradientFragShader = nullptr;
        char*                                                                             m_texturedFragShader        = nullptr;
        char*                                                                             m_sdfTextFragShader         = nullptr;
        char*                                                                             m_simpleTextFragShader      = nullptr;
        bool                                                                              m_skipDraw                  = false;
    };

    extern LINAVG_API BackendData g_backendData;

} // namespace LinaVG::Internal

#endif
