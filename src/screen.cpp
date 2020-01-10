#include "screen.hpp"

namespace NESemu
{
    const std::uint32_t colormap[] = {
        0x666666ff, 0x002a88ff, 0x1412a7ff, 0x3b00a4ff, 0x5c007eff, 0x6e0040ff, 0x6c0600ff, 0x561d00ff,
        0x333500ff, 0x0b4800ff, 0x005200ff, 0x004f08ff, 0x00404dff, 0x000000ff, 0x000000ff, 0x000000ff,
        
        0xadadadff, 0x155fd9ff, 0x4240ffff, 0x7527feff, 0xa01accff, 0xb71e7bff, 0xb53120ff, 0x994e00ff,
        0x6b6d00ff, 0x388700ff, 0x0c9300ff, 0x008f32ff, 0x007c8dff, 0x000000ff, 0x000000ff, 0x000000ff,
        
        0xfffeffff, 0x64b0ffff, 0x9290ffff, 0xc676ffff, 0xf36affff, 0xfe6eccff, 0xfe8170ff, 0xea9e22ff,
        0xbcbe00ff, 0x88d800ff, 0x5ce430ff, 0x45e082ff, 0x48cddeff, 0x4f4f4fff, 0x000000ff, 0x000000ff,
        
        0xfffeffff, 0xc0dfffff, 0xd3d2ffff, 0xe8c8ffff, 0xfbc2ffff, 0xfec4eaff, 0xfeccc5ff, 0xf7d8a5ff,
        0xe4e594ff, 0xcfef96ff, 0xbdf4abff, 0xb3f3ccff, 0xb5ebf2ff, 0xb8b8b8ff, 0x000000ff, 0x000000ff,
    };
    void Screen::create(unsigned int w, unsigned int h, float pixel_size, sf::Color color)
    {
        m_vertices.resize(w * h * 6);
        m_screenSize = {w, h};
        m_vertices.setPrimitiveType(sf::Triangles);
        m_pixelSize = pixel_size;
        for (std::size_t x = 0; x < w; ++x)
        {
            for (std::size_t y = 0; y < h; ++y)
            {
                auto index = (x * m_screenSize.y + y) * 6;
                sf::Vector2f coord2d (x * m_pixelSize, y * m_pixelSize);

                //Triangle-1
                //top-left
                m_vertices[index].position = coord2d;
                m_vertices[index].color    = color;

                //top-right
                m_vertices[index + 1].position = coord2d + sf::Vector2f{m_pixelSize, 0};
                m_vertices[index + 1].color = color;

                //bottom-right
                m_vertices[index + 2].position = coord2d + sf::Vector2f{m_pixelSize, m_pixelSize};
                m_vertices[index + 2].color = color;

                //Triangle-2
                //bottom-right
                m_vertices[index + 3].position = coord2d + sf::Vector2f{m_pixelSize, m_pixelSize};
                m_vertices[index + 3].color = color;

                //bottom-left
                m_vertices[index + 4].position = coord2d + sf::Vector2f{0, m_pixelSize};
                m_vertices[index + 4].color = color;

                //top-left
                m_vertices[index + 5].position = coord2d;
                m_vertices[index + 5].color    = color;
            }
        }
    }

    void Screen::setPixel(std::size_t x, std::size_t y, uint8_t palette_color)
    {
        m_screen_matrix[x*240 + y] = palette_color;
        auto color = static_cast<sf::Color>(colormap[palette_color]);
        auto index = (x * m_screenSize.y + y) * 6;
        if (index >= m_vertices.getVertexCount())
            return;

        sf::Vector2f coord2d (x * m_pixelSize, y * m_pixelSize);

        //Triangle-1
        //top-left
        m_vertices[index].color    = color;

        //top-right
        m_vertices[index + 1].color = color;

        //bottom-right
        m_vertices[index + 2].color = color;

        //Triangle-2
        //bottom-right
        m_vertices[index + 3].color = color;

        //bottom-left
        m_vertices[index + 4].color = color;

        //top-left
        m_vertices[index + 5].color = color;
    }

    void Screen::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_vertices, states);
    }
}