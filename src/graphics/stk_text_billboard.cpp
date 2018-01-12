//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SERVER_ONLY

#include "graphics/stk_text_billboard.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/graphics_restrictions.hpp"

// ----------------------------------------------------------------------------
STKTextBillboard::STKTextBillboard(const video::SColor& color_top,
                                   const video::SColor& color_bottom,
                                   ISceneNode* parent, ISceneManager* mgr,
                                   s32 id,
                                   const core::vector3df& position,
                                   const core::vector3df& size)
                : ISceneNode(parent, mgr, id, position,
                             core::vector3df(0.0f, 0.0f, 0.0f), size)
{
    using namespace SP;
    m_color_top = color_top;
    if (CVS->isDefferedEnabled())
    {
        m_color_top.setRed(srgb255ToLinear(m_color_top.getRed()));
        m_color_top.setGreen(srgb255ToLinear(m_color_top.getGreen()));
        m_color_top.setBlue(srgb255ToLinear(m_color_top.getBlue()));
    }
    m_color_bottom = color_bottom;
    if (CVS->isDefferedEnabled())
    {
        video::SColorf tmp(m_color_bottom);
        m_color_bottom.setRed(srgb255ToLinear(m_color_bottom.getRed()));
        m_color_bottom.setGreen(srgb255ToLinear(m_color_bottom.getGreen()));
        m_color_bottom.setBlue(srgb255ToLinear(m_color_bottom.getBlue()));
    }
    static_assert(sizeof(GLTB) == 20, "Wrong compiler padding");
}   // STKTextBillboard

// ----------------------------------------------------------------------------
void STKTextBillboard::updateAbsolutePosition()
{
    // Make billboard always face the camera
    scene::ICameraSceneNode* curr_cam =
        irr_driver->getSceneManager()->getActiveCamera();
    if (!curr_cam) return;
    core::quaternion q(curr_cam->getViewMatrix());
    q.W = -q.W;

    if (Parent)
    {
        // Override to not use the parent's rotation
        core::vector3df wc = RelativeTranslation;
        Parent->getAbsoluteTransformation().transformVect(wc);
        AbsoluteTransformation.setTranslation(wc);
        q.getMatrix(AbsoluteTransformation, wc);
    }
    else
    {
        q.getMatrix(AbsoluteTransformation, RelativeTranslation);
    }
    core::matrix4 m;
    m.setScale(RelativeScale);
    AbsoluteTransformation *= m;
    m_instanced_data = SP::SPInstancedData(AbsoluteTransformation, 0, 0, 0, 0);

}   // updateAbsolutePosition

// ----------------------------------------------------------------------------
void STKTextBillboard::init(core::stringw text, FontWithFace* face)
{
    m_chars = new std::vector<STKTextBillboardChar>();
    core::dimension2du size = face->getDimension(text.c_str());
    face->render(text, core::rect<s32>(0, 0, size.Width, size.Height),
        video::SColor(255,255,255,255), false, false, NULL, NULL, this);

    const float scale = 0.02f;
    float max_x = 0;
    float min_y = 0;
    float max_y = 0;
    for (unsigned int i = 0; i < m_chars->size(); i++)
    {
        float char_x = (*m_chars)[i].m_dest_rect.LowerRightCorner.X;
        if (char_x > max_x)
        {
            max_x = char_x;
        }

        float char_min_y = (*m_chars)[i].m_dest_rect.UpperLeftCorner.Y;
        float char_max_y = (*m_chars)[i].m_dest_rect.LowerRightCorner.Y;
        if (char_min_y < min_y)
        {
            min_y = char_min_y;
        }
        if (char_max_y > min_y)
        {
            max_y = char_max_y;
        }
    }
    float scaled_center_x = (max_x / 2.0f) * scale;
    float scaled_y = (max_y / 2.0f) * scale;

    for (unsigned int i = 0; i < m_chars->size(); i++)
    {
        core::vector3df char_pos((*m_chars)[i].m_dest_rect.UpperLeftCorner.X,
            (*m_chars)[i].m_dest_rect.UpperLeftCorner.Y, 0);
        char_pos *= scale;

        core::vector3df char_pos2((*m_chars)[i].m_dest_rect.LowerRightCorner.X,
            (*m_chars)[i].m_dest_rect.LowerRightCorner.Y, 0);
        char_pos2 *= scale;

        float tex_width = (float)(*m_chars)[i].m_texture->getSize().Width;
        float tex_height = (float)(*m_chars)[i].m_texture->getSize().Height;
        using namespace MiniGLM;
        std::array<GLTB, 4> triangle_strip =
        {{
            {
                core::vector3df
                    (char_pos.X - scaled_center_x, char_pos.Y - scaled_y, 0),
                m_color_bottom,
                {
                    toFloat16((*m_chars)
                        [i].m_source_rect.UpperLeftCorner.X / tex_width),
                    toFloat16((*m_chars)
                        [i].m_source_rect.LowerRightCorner.Y / tex_height)
                }
            },

            {
                core::vector3df
                    (char_pos.X - scaled_center_x, char_pos2.Y - scaled_y, 0),
                m_color_top,
                {
                    toFloat16((*m_chars)
                        [i].m_source_rect.UpperLeftCorner.X / tex_width),
                    toFloat16((*m_chars)
                        [i].m_source_rect.UpperLeftCorner.Y / tex_height)
                }
            },

            {
                core::vector3df
                    (char_pos2.X - scaled_center_x, char_pos.Y - scaled_y, 0),
                m_color_bottom,
                {
                    toFloat16((*m_chars)
                        [i].m_source_rect.LowerRightCorner.X / tex_width),
                    toFloat16((*m_chars)
                        [i].m_source_rect.LowerRightCorner.Y / tex_height)
                }
            },

            {
                core::vector3df
                    (char_pos2.X - scaled_center_x, char_pos2.Y - scaled_y, 0),
                m_color_top,
                {
                    toFloat16((*m_chars)
                        [i].m_source_rect.LowerRightCorner.X / tex_width),
                    toFloat16((*m_chars)
                        [i].m_source_rect.UpperLeftCorner.Y / tex_height)
                }
            }
        }};
        m_gl_tbs[(*m_chars)[i].m_texture].push_back(triangle_strip);
    }

    glGenBuffers(1, &m_instanced_array);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanced_array);
    glBufferData(GL_ARRAY_BUFFER,
        12 /*position*/ + 4/*quaternion*/ + 8 /*scale*/, NULL,
        GL_DYNAMIC_DRAW);
    for (auto& p : m_gl_tbs)
    {
        glGenVertexArrays(1, &m_vao_vbos[p.first].first);
        glGenBuffers(1, &m_vao_vbos[p.first].second);
        glBindBuffer(GL_ARRAY_BUFFER, m_vao_vbos.at(p.first).second);
        glBufferData(GL_ARRAY_BUFFER, m_gl_tbs.at(p.first).size() * 4 * 20,
            m_gl_tbs.at(p.first).data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(m_vao_vbos.at(p.first).first);
        glBindBuffer(GL_ARRAY_BUFFER, m_vao_vbos.at(p.first).second);

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (void*)0);

        // Vertex color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 20,
            (void*)12);

        // 1st texture coordinates
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_HALF_FLOAT, GL_FALSE, 20, (void*)16);

        glBindBuffer(GL_ARRAY_BUFFER, m_instanced_array);
        // Origin
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
        glVertexAttribDivisorARB(8, 1);

        // Rotation (quaternion .xyz)
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 4, GL_INT_2_10_10_10_REV,
            GraphicsRestrictions::isDisabled
            (GraphicsRestrictions::GR_CORRECT_10BIT_NORMALIZATION) ?
            GL_FALSE : GL_TRUE, 24, (void*)12);
        glVertexAttribDivisorARB(9, 1);

        // Scale (3 half floats and .w for quaternion .w)
        glEnableVertexAttribArray(10);
        glVertexAttribPointer(10, 4, GL_HALF_FLOAT, GL_FALSE, 24, (void*)16);
        glVertexAttribDivisorARB(10, 1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    Vec3 min = Vec3( 999999.9f);
    Vec3 max = Vec3(-999999.9f);
    for (auto& p : m_gl_tbs)
    {
        for (auto& q : p.second)
        {
            for (auto& r : q)
            {
                Vec3 c(r.m_position.X, r.m_position.Y, r.m_position.Z);
                min.min(c);
                max.max(c);
            }
        }
    }
    m_bbox.MinEdge = min.toIrrVector();
    m_bbox.MaxEdge = max.toIrrVector();

    delete m_chars;
    updateAbsolutePosition();
}   // init

// ----------------------------------------------------------------------------
void STKTextBillboard::collectChar(video::ITexture* texture,
                                   const core::rect<float>& dest_rect,
                                   const core::rect<s32>& source_rect,
                                   const video::SColor* const colors)
{
    assert(m_chars != NULL);
    m_chars->push_back(STKTextBillboardChar(texture, dest_rect, source_rect,
        colors));
}   // collectChar

#endif   // !SERVER_ONLY

