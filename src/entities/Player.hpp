#pragma once

#include <SFML/Graphics.hpp>

#include "core/InputState.hpp"

class ParticleSystem;
class TileMap;

class Player {
public:
    explicit Player(sf::Vector2f spawnPosition);

    void update(float dt, const InputState& input, const TileMap& map, ParticleSystem& particles);
    void draw(sf::RenderTarget& target) const;

    sf::Vector2f position() const;
    sf::FloatRect bounds() const;
    float height() const;
    int facing() const;
    void reset(sf::Vector2f spawnPosition);
    void bounce();   // upward kick when stomping an enemy
    void warpX(float x);  // instant horizontal teleport (world wrap)

private:
    sf::FloatRect boundsAt(sf::Vector2f position) const;
    void resolveHorizontal(const TileMap& map);
    bool resolveVertical(const TileMap& map);
    void emitRunDust(ParticleSystem& particles);

    sf::Vector2f m_position;
    sf::Vector2f m_velocity{0.0f, 0.0f};
    sf::Vector2f m_size{46.0f, 68.0f};

    bool m_onGround = false;
    bool m_wasOnGround = false;
    bool m_jumpCutPending = false;
    bool m_canDoubleJump = false;  // consumed on second jump

    int m_facing = 1;

    float m_coyoteTimer = 0.0f;
    float m_jumpBufferTimer = 0.0f;
    float m_runDustCooldown = 0.0f;
    float m_walkTimer = 0.0f;

    // ── Animation extras ──────────────────────────────────────────────────
    float m_idleBreathTimer = 0.0f;   // drives gentle torso bob when idle
    float m_squashTimer     = 0.0f;   // > 0 → currently squashing after landing
    float m_squashAmount    = 0.0f;   // peak squash magnitude (set on impact)
    float m_stretchAmount   = 0.0f;   // positive while ascending (jump stretch)
    float m_armAngle        = 0.0f;   // current arm-swing angle (degrees)
    float m_tiltAngle       = 0.0f;   // smoothed whole-body lean (degrees)
    float m_eyeBlinkTimer   = 0.0f;   // time until next blink
    float m_eyeBlinkPhase   = 0.0f;   // 0→1 blink animation
};
