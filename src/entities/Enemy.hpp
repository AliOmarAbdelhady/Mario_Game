#pragma once

#include <SFML/Graphics.hpp>

class TileMap;
class ParticleSystem;

class Enemy {
public:
    explicit Enemy(sf::Vector2f position);

    void update(float dt, const TileMap& map, sf::Vector2f playerPos);
    void draw(sf::RenderTarget& target) const;

    sf::FloatRect bounds() const;
    sf::Vector2f  position() const;
    bool isActive() const;   // alive and visible (not in respawn cooldown)

    void stomp(ParticleSystem& particles);
    /// Pick a new random surface tile for respawn (avoids playerPos)
    void pickRandomSpawn(const TileMap& map, sf::Vector2f playerPos);

private:
    void resolveHorizontal(const TileMap& map);
    void resolveVertical(const TileMap& map);
    void respawn();

    sf::Vector2f m_spawnPos;       // original spawn — used when respawning
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    sf::Vector2f m_size{52.0f, 50.0f};

    bool  m_killed        = false;
    float m_deadTimer     = 0.45f;  // squish display time
    bool  m_respawning    = false;
    float m_respawnTimer  = 0.0f;
};
