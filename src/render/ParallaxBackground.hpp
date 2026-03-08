#pragma once

#include <SFML/Graphics.hpp>

class ParallaxBackground {
public:
    void draw(sf::RenderTarget& target, const sf::View& worldView, const sf::FloatRect& worldBounds) const;
};
