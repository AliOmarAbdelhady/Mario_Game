#pragma once

#include <SFML/Graphics.hpp>

#include <random>
#include <vector>

class ParticleSystem {
public:
    ParticleSystem();

    void emit(const sf::Vector2f& position, const sf::Vector2f& baseVelocity, int count, float spreadRadians, float life,
              const sf::Color& color, float minSize, float maxSize);
    void update(float dt);
    void draw(sf::RenderTarget& target) const;

private:
    struct Particle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float life = 0.0f;
        float maxLife = 0.0f;
        float size = 0.0f;
        sf::Color color = sf::Color::White;
    };

    std::vector<Particle> m_particles;
    std::mt19937 m_rng;
};
