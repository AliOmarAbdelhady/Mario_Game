#pragma once

#include <SFML/Graphics.hpp>

class ParticleSystem;

class Box {
public:
    explicit Box(sf::Vector2f position, float tileSize);

    void update(float dt);
    void draw(sf::RenderTarget& target) const;

    sf::FloatRect bounds() const;
    sf::Vector2f  position() const;
    bool isActive() const;    // still has a coin inside
    bool isHit() const;       // has been hit already (empty)

    /// Hit from below — returns the position where the coin should spawn
    sf::Vector2f hit(ParticleSystem& particles);

private:
    sf::Vector2f m_position;      // centre of the box
    float        m_size;          // width & height (square, tile-sized)

    bool  m_hasBeenHit   = false;
    float m_bumpTimer    = 0.0f;  // visual bump animation when hit
    float m_bumpOffset   = 0.0f;  // current Y offset from bump
    float m_shimmerTimer = 0.0f;  // drives the '?' shimmer

    static constexpr float kBumpDuration = 0.18f;
    static constexpr float kBumpHeight   = 12.0f;
};
