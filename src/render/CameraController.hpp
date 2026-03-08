#pragma once

#include <SFML/Graphics.hpp>

class CameraController {
public:
    explicit CameraController(sf::Vector2f viewportSize);

    // dt=large snaps immediately; targetZoom=1 is normal, >1 zooms out
    void update(float dt, const sf::Vector2f& focus, const sf::FloatRect& worldBounds,
                float targetZoom = 1.0f);
    void resize(const sf::Vector2f& viewportSize);
    void snapX(float x);  // instant horizontal camera reposition (world wrap)

    const sf::View& view() const;

private:
    sf::View  m_view;
    sf::Vector2f m_baseSize;   // original viewport size
    float     m_currentZoom = 1.0f;
};
