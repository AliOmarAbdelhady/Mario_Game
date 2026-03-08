#include "entities/Box.hpp"

#include "entities/ParticleSystem.hpp"

#include <cmath>

Box::Box(sf::Vector2f position, float tileSize)
    : m_position(position)
    , m_size(tileSize)
{}

void Box::update(float dt) {
    // Shimmer animation (question mark glow)
    if (!m_hasBeenHit) {
        m_shimmerTimer += dt * 2.5f;
    }

    // Bump animation
    if (m_bumpTimer > 0.0f) {
        m_bumpTimer = std::max(0.0f, m_bumpTimer - dt);
        const float t = m_bumpTimer / kBumpDuration;  // 1→0
        // Quick up, ease back down
        m_bumpOffset = -kBumpHeight * std::sin(t * 3.14159f);
    } else {
        m_bumpOffset = 0.0f;
    }
}

void Box::draw(sf::RenderTarget& target) const {
    const float half = m_size * 0.5f;
    const float drawY = m_position.y + m_bumpOffset;

    // Main box body
    sf::RectangleShape box({m_size * 0.92f, m_size * 0.92f});
    box.setOrigin(box.getSize().x * 0.5f, box.getSize().y * 0.5f);
    box.setPosition(m_position.x, drawY);

    if (m_hasBeenHit) {
        // Empty box — dark brown
        box.setFillColor(sf::Color(110, 75, 45));
        box.setOutlineColor(sf::Color(70, 45, 25));
        box.setOutlineThickness(2.0f);
        target.draw(box);
        return;
    }

    // Active question-mark box — golden yellow
    const float shimmer = std::sin(m_shimmerTimer) * 0.12f + 0.88f;
    const sf::Uint8 r = static_cast<sf::Uint8>(std::min(255.0f, 220.0f * shimmer + 35.0f));
    const sf::Uint8 g = static_cast<sf::Uint8>(std::min(255.0f, 170.0f * shimmer + 30.0f));
    box.setFillColor(sf::Color(r, g, 30));
    box.setOutlineColor(sf::Color(160, 100, 20));
    box.setOutlineThickness(2.5f);
    target.draw(box);

    // Inner darker square (border effect)
    const float innerSz = m_size * 0.70f;
    sf::RectangleShape inner({innerSz, innerSz});
    inner.setOrigin(innerSz * 0.5f, innerSz * 0.5f);
    inner.setPosition(m_position.x, drawY);
    inner.setFillColor(sf::Color(200, 140, 20));
    target.draw(inner);

    // Question mark — drawn with simple shapes
    // Vertical bar of '?'
    const float qW = m_size * 0.12f;
    const float qH = m_size * 0.28f;
    sf::RectangleShape qBar({qW, qH});
    qBar.setOrigin(qW * 0.5f, qH * 0.5f);
    qBar.setPosition(m_position.x, drawY - m_size * 0.08f);
    qBar.setFillColor(sf::Color(245, 245, 245));
    target.draw(qBar);

    // Top curve of '?'
    sf::RectangleShape qTop({qW * 2.2f, qW});
    qTop.setOrigin(qTop.getSize().x * 0.5f, qTop.getSize().y * 0.5f);
    qTop.setPosition(m_position.x + qW * 0.5f, drawY - m_size * 0.22f);
    qTop.setFillColor(sf::Color(245, 245, 245));
    target.draw(qTop);

    // Right side of top curve
    sf::RectangleShape qRight({qW, qW * 1.5f});
    qRight.setOrigin(qRight.getSize().x * 0.5f, qRight.getSize().y * 0.5f);
    qRight.setPosition(m_position.x + qW * 1.1f, drawY - m_size * 0.15f);
    qRight.setFillColor(sf::Color(245, 245, 245));
    target.draw(qRight);

    // Dot of '?'
    sf::CircleShape qDot(qW * 0.7f, 8);
    qDot.setOrigin(qDot.getRadius(), qDot.getRadius());
    qDot.setPosition(m_position.x, drawY + m_size * 0.18f);
    qDot.setFillColor(sf::Color(245, 245, 245));
    target.draw(qDot);

    // Corner rivets
    for (int sx = -1; sx <= 1; sx += 2) {
        for (int sy = -1; sy <= 1; sy += 2) {
            sf::CircleShape rivet(m_size * 0.04f, 6);
            rivet.setOrigin(rivet.getRadius(), rivet.getRadius());
            rivet.setPosition(m_position.x + sx * half * 0.72f,
                              drawY + sy * half * 0.72f);
            rivet.setFillColor(sf::Color(170, 110, 30));
            target.draw(rivet);
        }
    }
}

sf::FloatRect Box::bounds() const {
    return {m_position.x - m_size * 0.5f, m_position.y - m_size * 0.5f,
            m_size, m_size};
}

bool Box::isActive() const { return !m_hasBeenHit; }

sf::Vector2f Box::position() const { return m_position; }

bool Box::isHit() const { return m_hasBeenHit; }

sf::Vector2f Box::hit(ParticleSystem& particles) {
    if (m_hasBeenHit) return m_position;
    m_hasBeenHit = true;
    m_bumpTimer  = kBumpDuration;

    // Burst particles from the box
    particles.emit(
        sf::Vector2f(m_position.x, m_position.y - m_size * 0.5f),
        sf::Vector2f(0.0f, -180.0f), 10, 2.5f, 0.35f,
        sf::Color(255, 220, 50, 230), 2.0f, 4.0f);

    // Return the coin spawn position (just above the box)
    return {m_position.x, m_position.y - m_size * 0.6f};
}
