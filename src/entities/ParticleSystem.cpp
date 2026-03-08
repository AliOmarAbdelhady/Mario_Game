#include "entities/ParticleSystem.hpp"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kParticleGravity = 1400.0f;
}  // namespace

ParticleSystem::ParticleSystem() : m_rng(std::random_device{}()) { m_particles.reserve(4096); }

void ParticleSystem::emit(const sf::Vector2f& position, const sf::Vector2f& baseVelocity, int count, float spreadRadians,
                          float life, const sf::Color& color, float minSize, float maxSize) {
    std::uniform_real_distribution<float> angleDist(-spreadRadians, spreadRadians);
    std::uniform_real_distribution<float> speedScaleDist(0.55f, 1.25f);
    std::uniform_real_distribution<float> lifeDist(life * 0.7f, life * 1.2f);
    std::uniform_real_distribution<float> sizeDist(minSize, maxSize);

    const float emissionSpeed = 180.0f;
    for (int i = 0; i < count; ++i) {
        const float angle = angleDist(m_rng);
        const float speedScale = speedScaleDist(m_rng);

        Particle p;
        p.position = position;
        p.velocity.x = baseVelocity.x + std::cos(angle) * emissionSpeed * speedScale;
        p.velocity.y = baseVelocity.y + std::sin(angle) * emissionSpeed * speedScale;
        p.maxLife = lifeDist(m_rng);
        p.life = p.maxLife;
        p.size = sizeDist(m_rng);
        p.color = color;
        m_particles.push_back(p);
    }
}

void ParticleSystem::update(float dt) {
    for (auto& p : m_particles) {
        p.life -= dt;
        p.velocity.y += kParticleGravity * dt;
        p.position += p.velocity * dt;
    }

    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
                                     [](const Particle& p) {
                                         return p.life <= 0.0f;
                                     }),
                      m_particles.end());
}

void ParticleSystem::draw(sf::RenderTarget& target) const {
    sf::CircleShape particleShape;
    particleShape.setPointCount(12);

    for (const auto& p : m_particles) {
        const float lifeRatio = std::clamp(p.life / p.maxLife, 0.0f, 1.0f);
        const float radius = p.size * lifeRatio;
        if (radius <= 0.01f) {
            continue;
        }

        particleShape.setRadius(radius);
        particleShape.setOrigin(radius, radius);
        particleShape.setPosition(p.position);

        sf::Color color = p.color;
        color.a = static_cast<sf::Uint8>(255.0f * lifeRatio);
        particleShape.setFillColor(color);
        target.draw(particleShape);
    }
}
