#pragma once

#include <SFML/Graphics.hpp>

class TileMap;

class Coin {
public:
    /// Create a coin at position (typically the top of a box)
    explicit Coin(sf::Vector2f position);

    void update(float dt, const TileMap& map);
    void draw(sf::RenderTarget& target) const;

    sf::FloatRect bounds() const;
    bool isCollectable() const;   // true once the coin is on the ground
    bool isExpired() const;       // true when coin has timed out (not collected in time)

    void collect();               // mark as collected (plays vanish)
    bool isCollected() const;

private:
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    sf::Vector2f m_size{28.0f, 28.0f};

    float m_popTimer    = 0.0f;   // counts up during the pop-out phase
    float m_groundTimer = 0.0f;   // how long the coin has been on the ground
    float m_spinAngle   = 0.0f;   // visual spin

    bool  m_popping     = true;   // in the initial upward pop animation
    bool  m_onGround    = false;  // landed on the ground
    bool  m_collected   = false;
    bool  m_expired     = false;

    float m_collectFlash = 0.0f;  // brief flash animation on collect

    static constexpr float kPopDuration   = 0.35f;
    static constexpr float kGroundLifetime = 6.0f;  // seconds on ground before disappearing
    static constexpr float kPopSpeed      = 420.0f;
    static constexpr float kGravity       = 1800.0f;
    static constexpr float kMaxFall       = 900.0f;
};
