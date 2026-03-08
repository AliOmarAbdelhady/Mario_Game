#include "entities/Coin.hpp"

#include "world/TileMap.hpp"

#include <algorithm>
#include <cmath>

Coin::Coin(sf::Vector2f position)
    : m_position(position)
    , m_velocity(0.0f, -kPopSpeed)
{}

void Coin::update(float dt, const TileMap& map) {
    // Already collected — play flash then done
    if (m_collected) {
        m_collectFlash += dt * 6.0f;
        return;
    }
    if (m_expired) return;

    // Spin animation
    m_spinAngle += dt * 360.0f;
    if (m_spinAngle > 360.0f) m_spinAngle -= 360.0f;

    if (m_onGround) {
        // Coin is sitting on the ground — count down lifetime
        m_groundTimer += dt;
        if (m_groundTimer >= kGroundLifetime) {
            m_expired = true;
        }
    } else {
        // Apply gravity
        m_velocity.y = std::min(m_velocity.y + kGravity * dt, kMaxFall);

        // Move
        m_position += m_velocity * dt;

        // Pop phase tracking
        if (m_popping) {
            m_popTimer += dt;
            if (m_popTimer >= kPopDuration) {
                m_popping = false;
            }
        }

        // Resolve vertical collision with ground tiles
        const sf::FloatRect coinRect = bounds();
        const auto colliders = map.querySolidTiles(coinRect);
        for (const auto& tile : colliders) {
            sf::FloatRect overlap;
            if (!coinRect.intersects(tile, overlap)) continue;
            if (m_velocity.y > 0.0f) {
                // Landing on top of a tile
                m_position.y -= overlap.height;
                m_velocity = {0.0f, 0.0f};
                m_onGround = true;
                m_popping  = false;
                break;
            } else if (m_velocity.y < 0.0f) {
                // Hit ceiling
                m_position.y += overlap.height;
                m_velocity.y = 0.0f;
            }
        }

        // Fall off the world
        if (m_position.y > 4000.0f) {
            m_expired = true;
        }
    }
}

void Coin::draw(sf::RenderTarget& target) const {
    if (m_expired && !m_collected) return;

    // Collected flash effect
    if (m_collected) {
        if (m_collectFlash >= 1.0f) return;  // fully faded
        const sf::Uint8 alpha = static_cast<sf::Uint8>((1.0f - m_collectFlash) * 255.0f);
        const float scale = 1.0f + m_collectFlash * 1.5f;
        sf::CircleShape flash(18.0f * scale, 16);
        flash.setOrigin(flash.getRadius(), flash.getRadius());
        flash.setPosition(m_position);
        flash.setFillColor(sf::Color(255, 230, 50, alpha));
        target.draw(flash);
        return;
    }

    // Blink effect near end of life
    if (m_onGround && m_groundTimer > kGroundLifetime - 2.5f) {
        if (static_cast<int>(m_groundTimer * 6.0f) % 2 == 0) return;  // blink off
    }

    // Coin "spin" — scale X to simulate 3D rotation
    const float scaleX = std::abs(std::cos(m_spinAngle * 3.14159f / 180.0f));
    const float coinW = m_size.x * std::max(scaleX, 0.15f);
    const float coinH = m_size.y;

    // Outer gold
    sf::RectangleShape body({coinW, coinH});
    body.setOrigin(coinW * 0.5f, coinH * 0.5f);
    body.setPosition(m_position);
    body.setFillColor(sf::Color(255, 210, 40));
    target.draw(body);

    // Inner shine
    if (scaleX > 0.3f) {
        const float innerW = coinW * 0.55f;
        const float innerH = coinH * 0.55f;
        sf::RectangleShape inner({innerW, innerH});
        inner.setOrigin(innerW * 0.5f, innerH * 0.5f);
        inner.setPosition(m_position);
        inner.setFillColor(sf::Color(255, 240, 140));
        target.draw(inner);
    }

    // Sparkle highlight
    sf::CircleShape sparkle(3.0f, 6);
    sparkle.setOrigin(sparkle.getRadius(), sparkle.getRadius());
    sparkle.setPosition(m_position.x + coinW * 0.2f, m_position.y - coinH * 0.2f);
    sparkle.setFillColor(sf::Color(255, 255, 255, 180));
    target.draw(sparkle);
}

sf::FloatRect Coin::bounds() const {
    return {m_position.x - m_size.x * 0.5f, m_position.y - m_size.y * 0.5f,
            m_size.x, m_size.y};
}

bool Coin::isCollectable() const { return !m_popping && !m_collected && !m_expired; }

bool Coin::isExpired() const { return m_expired || (m_collected && m_collectFlash >= 1.0f); }

void Coin::collect() { m_collected = true; m_collectFlash = 0.0f; }

bool Coin::isCollected() const { return m_collected; }
