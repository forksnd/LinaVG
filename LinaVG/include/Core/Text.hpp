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
Class: Text

Timestamp: 12/29/2018 10:43:46 PM
*/

#pragma once
#ifndef LinaVGText_HPP
#define LinaVGText_HPP

#include "Common.hpp"
#include <unordered_map>
#include <functional>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace LinaVG
{

    struct TextCharacter
    {
        Vec2 m_uv      = Vec2(0.0f, 0.0f);
        Vec2 m_size    = Vec2(0.0f, 0.0f);
        Vec2 m_bearing = Vec2(0.0f, 0.0f);
        Vec2 m_advance = Vec2(0.0f, 0.0f);
    };

    class LinaVGFont
    {
    public:
        BackendHandle                           m_texture     = 0;
        int                                     m_size        = 0;
        bool                                    m_isSDF       = false;
        Vec2                                    m_textureSize = Vec2(0.0f, 0.0f);
        std::unordered_map<char, TextCharacter> m_characterGlyphs;
    };

    /// <summary>
    /// Management for text rendering.
    /// </summary>
    struct TextData
    {
        FT_Library m_ftlib       = nullptr;
        int        m_defaultFont = 0;

        /// <summary>
        /// !OFFSETTED BY 1! always access m_loadedFonts[myFontHandle - 1];
        /// </summary>
        Array<LinaVGFont*> m_loadedFonts;
    };

    namespace Internal
    {
        extern LINAVG_API TextData g_textData;

        /// <summary>
        /// !Internal! Do not modify.
        /// </summary>
        extern LINAVG_API FontHandle g_fontCounter;
    } // namespace Internal

    namespace Text
    {
        LINAVG_API bool Initialize();
        LINAVG_API void Terminate();
    } // namespace Text

    /// <summary>
    /// Loads the given font & generates textures based on given size.
    /// You can load the same font with different sizes to achieve varying text scales.
    /// Alternatively, you can use the scale modifier in TextOptions but it's not recommended to upscale.
    /// Best quality would be achieved by loading fonts with bigger sizes and scaling them down using TextOptions.
    /// </summary>
    /// <returns>Font handle, store this handle if you like to use multiple fonts. You can pass the handle inside TextOptions to draw with a specific font. </returns>
    LINAVG_API FontHandle LoadFont(const std::string& file, bool loadAsSDF, int size = 48);

    /// <summary>
    /// While drawing texts, the system will try to use the font passed inside TextOptions.
    /// If its 0 or doesn't exists, it will fall-back to the default font.
    /// Set the default font handle using this method.
    /// !NOTE!: When you load a font, it's always set as Default font. So only use this method after you are done loading all your fonts.
    /// </summary>
    /// <returns></returns>
    LINAVG_API void SetDefaultFont(FontHandle font);
}; // namespace LinaVG

#endif