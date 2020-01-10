#pragma once
#include <SFML/Graphics.hpp>
#include <array>

namespace NESemu
{
    struct Screen : public sf::Drawable
    {
        void create (unsigned int width, unsigned int height, float pixel_size, sf::Color color);
        void setPixel (std::size_t x, std::size_t y, uint8_t palette_color);
        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        sf::Vector2u m_screenSize;
        float m_pixelSize; //virtual pixel size in real pixels
        sf::VertexArray m_vertices;

        uint8_t m_screen_matrix[256*240];
    };
};
