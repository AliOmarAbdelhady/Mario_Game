#include "render/CameraController.hpp"

#include "core/Config.hpp"

#include <algorithm>
#include <cmath>

CameraController::CameraController(sf::Vector2f viewportSize)
    : m_baseSize(viewportSize) {
    m_view.setSize(viewportSize);
    m_view.setCenter(viewportSize * 0.5f);
}

void CameraController::update(float dt, const sf::Vector2f& focus,
                               const sf::FloatRect& worldBounds, float targetZoom) {
    // Smooth zoom lerp
    const float zoomBlend = 1.0f - std::exp(-1.8f * dt);
    m_currentZoom += (targetZoom - m_currentZoom) * zoomBlend;

    // Apply zoom to view size
    const sf::Vector2f zoomedSize = m_baseSize * m_currentZoom;
    m_view.setSize(zoomedSize);

    // Smooth position lerp
    sf::Vector2f center = m_view.getCenter();
    const float blend = 1.0f - std::exp(-config::CameraLerp * dt);
    center += (focus - center) * blend;

    const sf::Vector2f half = zoomedSize * 0.5f;

    if (worldBounds.width <= zoomedSize.x) {
        center.x = worldBounds.left + worldBounds.width * 0.5f;
    } else {
        center.x = std::clamp(center.x, worldBounds.left + half.x,
                               worldBounds.left + worldBounds.width - half.x);
    }

    if (worldBounds.height <= zoomedSize.y) {
        center.y = worldBounds.top + worldBounds.height * 0.5f;
    } else {
        center.y = std::clamp(center.y, worldBounds.top + half.y,
                               worldBounds.top + worldBounds.height - half.y);
    }

    m_view.setCenter(center);
}

void CameraController::resize(const sf::Vector2f& viewportSize) {
    m_baseSize = viewportSize;
    m_view.setSize(viewportSize * m_currentZoom);
}

void CameraController::snapX(float x) {
    sf::Vector2f c = m_view.getCenter();
    c.x = x;
    m_view.setCenter(c);
}

const sf::View& CameraController::view() const { return m_view; }
