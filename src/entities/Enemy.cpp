#include "entities/Enemy.hpp"

#include "entities/ParticleSystem.hpp"
#include "world/TileMap.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {
constexpr float kEnemySpeed   = 90.0f;
constexpr float kGravity      = 2400.0f;
constexpr float kMaxFall      = 1450.0f;
constexpr float kRespawnDelay = 2.5f;    // seconds after dying before reappearing
constexpr float kFallLimit    = 3200.0f; // y beyond which enemy counts as fallen
}  // namespace

Enemy::Enemy(sf::Vector2f pos)
    : m_spawnPos(pos), m_position(pos), m_velocity(-kEnemySpeed, 0.0f) {}

void Enemy::respawn() {
    m_position     = m_spawnPos;
    m_velocity     = {-kEnemySpeed, 0.0f};
    m_killed       = false;
    m_deadTimer    = 0.45f;
    m_respawning   = false;
    m_respawnTimer = 0.0f;
}

void Enemy::update(float dt, const TileMap& map, sf::Vector2f playerPos) {
    // --- waiting to respawn ---
    if (m_respawning) {
        m_respawnTimer -= dt;
        if (m_respawnTimer <= 0.0f) {
            pickRandomSpawn(map, playerPos);
            respawn();
        }
        return;
    }

    // --- squish display after stomp ---
    if (m_killed) {
        m_deadTimer -= dt;
        if (m_deadTimer <= 0.0f) {
            m_respawning   = true;
            m_respawnTimer = kRespawnDelay;
        }
        return;
    }

    // --- normal movement: chase player horizontally ---
    const float dx = playerPos.x - m_position.x;
    m_velocity.x   = (dx > 0.0f ? 1.0f : -1.0f) * kEnemySpeed;

    // Keep enemies on their current platform/floor: if there is no support
    // one step ahead, turn around instead of walking off the edge.
    {
        const float dir = (m_velocity.x >= 0.0f) ? 1.0f : -1.0f;
        const float ts  = static_cast<float>(map.tileSize());

        sf::FloatRect footProbe(
            m_position.x - m_size.x * 0.35f,
            m_position.y + 1.0f,
            m_size.x * 0.70f,
            3.0f);
        const bool onGround = !map.querySolidTiles(footProbe).empty();

        if (onGround) {
            sf::FloatRect aheadProbe(
                m_position.x + dir * (m_size.x * 0.5f + ts * 0.35f) - ts * 0.20f,
                m_position.y + 2.0f,
                ts * 0.40f,
                ts * 0.35f);
            const bool hasSupportAhead = !map.querySolidTiles(aheadProbe).empty();
            if (!hasSupportAhead) {
                m_velocity.x = -m_velocity.x;
            }
        }
    }

    m_velocity.y = std::min(m_velocity.y + kGravity * dt, kMaxFall);

    m_position.x += m_velocity.x * dt;
    resolveHorizontal(map);

    m_position.y += m_velocity.y * dt;
    resolveVertical(map);

    // --- fell off the world: enter respawn cooldown ---
    if (m_position.y > kFallLimit) {
        m_respawning   = true;
        m_respawnTimer = kRespawnDelay;
    }
}

void Enemy::resolveHorizontal(const TileMap& map) {
    sf::FloatRect body = bounds();
    for (const auto& tile : map.querySolidTiles(body)) {
        sf::FloatRect overlap;
        if (!body.intersects(tile, overlap)) continue;
        if (m_velocity.x > 0.0f)
            m_position.x -= overlap.width;
        else
            m_position.x += overlap.width;
        m_velocity.x = 0.0f;
        body = bounds();
    }
}

void Enemy::resolveVertical(const TileMap& map) {
    sf::FloatRect body = bounds();
    for (const auto& tile : map.querySolidTiles(body)) {
        sf::FloatRect overlap;
        if (!body.intersects(tile, overlap)) continue;
        if (m_velocity.y > 0.0f) {
            m_position.y -= overlap.height;
        } else {
            m_position.y += overlap.height;
        }
        m_velocity.y = 0.0f;
        body = bounds();
    }
}

void Enemy::stomp(ParticleSystem& particles) {
    if (m_killed || m_respawning) return;
    m_killed   = true;
    m_deadTimer = 0.45f;
    m_velocity = {0.0f, 0.0f};
    particles.emit(
        m_position - sf::Vector2f(0.0f, m_size.y * 0.5f),
        sf::Vector2f(0.0f, -200.0f), 12, 3.14f, 0.4f,
        sf::Color(140, 80, 30, 220), 2.0f, 5.0f);
}

void Enemy::draw(sf::RenderTarget& target) const {
    // invisible while in respawn cooldown
    if (m_respawning) return;

    if (m_killed) {
        // flat squish sprite
        sf::RectangleShape squish({m_size.x * 1.15f, m_size.y * 0.22f});
        squish.setOrigin(squish.getSize().x * 0.5f, squish.getSize().y);
        squish.setPosition(m_position.x, m_position.y);
        squish.setFillColor(sf::Color(100, 55, 20));
        target.draw(squish);
        return;
    }

    // ground shadow
    sf::CircleShape shadow(m_size.x * 0.38f, 20);
    shadow.setOrigin(shadow.getRadius(), shadow.getRadius() * 0.5f);
    shadow.setScale(1.0f, 0.42f);
    shadow.setPosition(m_position.x, m_position.y + 3.0f);
    shadow.setFillColor(sf::Color(0, 0, 0, 55));
    target.draw(shadow);

    // mushroom cap / body
    sf::CircleShape cap(m_size.x * 0.49f, 22);
    cap.setOrigin(cap.getRadius(), cap.getRadius());
    cap.setPosition(m_position.x, m_position.y - m_size.y * 0.54f);
    cap.setFillColor(sf::Color(140, 80, 30));
    target.draw(cap);

    // lighter face area
    sf::CircleShape face(m_size.x * 0.30f, 18);
    face.setOrigin(face.getRadius(), face.getRadius());
    face.setPosition(m_position.x, m_position.y - m_size.y * 0.42f);
    face.setFillColor(sf::Color(200, 130, 60));
    target.draw(face);

    // feet
    for (int side : {-1, 1}) {
        sf::RectangleShape foot({m_size.x * 0.38f, m_size.y * 0.28f});
        foot.setOrigin(foot.getSize().x * 0.5f, foot.getSize().y);
        foot.setPosition(m_position.x + static_cast<float>(side) * m_size.x * 0.20f,
                         m_position.y);
        foot.setFillColor(sf::Color(55, 28, 8));
        target.draw(foot);

        // white of eye
        sf::CircleShape eyeW(m_size.x * 0.09f, 10);
        eyeW.setOrigin(eyeW.getRadius(), eyeW.getRadius());
        eyeW.setPosition(m_position.x + static_cast<float>(side) * m_size.x * 0.20f,
                         m_position.y - m_size.y * 0.56f);
        eyeW.setFillColor(sf::Color::White);
        target.draw(eyeW);

        // pupil (angled inward)
        sf::CircleShape pupil(m_size.x * 0.05f, 10);
        pupil.setOrigin(pupil.getRadius(), pupil.getRadius());
        pupil.setPosition(m_position.x + static_cast<float>(side) * m_size.x * 0.22f,
                          m_position.y - m_size.y * 0.54f);
        pupil.setFillColor(sf::Color(15, 15, 15));
        target.draw(pupil);

        // angry brow
        sf::RectangleShape brow({m_size.x * 0.26f, m_size.y * 0.055f});
        brow.setOrigin(brow.getSize().x * 0.5f, brow.getSize().y * 0.5f);
        brow.setPosition(m_position.x + static_cast<float>(side) * m_size.x * 0.20f,
                         m_position.y - m_size.y * 0.72f);
        brow.setRotation(static_cast<float>(-side) * 22.0f);
        brow.setFillColor(sf::Color(25, 10, 0));
        target.draw(brow);
    }
}

sf::FloatRect Enemy::bounds() const {
    return {m_position.x - m_size.x * 0.5f, m_position.y - m_size.y,
            m_size.x, m_size.y};
}

sf::Vector2f Enemy::position() const { return m_position; }

bool Enemy::isActive() const { return !m_killed && !m_respawning; }

void Enemy::pickRandomSpawn(const TileMap& map, sf::Vector2f playerPos) {
    const auto& rows = map.rows();
    const int   h    = static_cast<int>(rows.size());
    const int   w    = h > 0 ? static_cast<int>(rows[0].size()) : 0;
    const float ts   = static_cast<float>(map.tileSize());
    const float safeR = ts * 8.0f;  // min distance from player
    const int groundRow = h - 2;
    const int currentRow = static_cast<int>(std::lround(m_spawnPos.y / ts));
    const bool wantGround = (currentRow == groundRow);

    // Try up to 200 random positions to find a valid surface tile
    for (int attempt = 0; attempt < 200; ++attempt) {
        const int rx = std::rand() % w;
        const int ry = std::rand() % (h - 1);
        const char cell  = rows[static_cast<std::size_t>(ry)][static_cast<std::size_t>(rx)];
        const char below = rows[static_cast<std::size_t>(ry + 1)][static_cast<std::size_t>(rx)];
        if (cell == '#' || cell == 'S' || below != '#') continue;

        const bool candidateGround = (ry + 1 == groundRow);
        if (candidateGround != wantGround) continue;

        const float px = static_cast<float>(rx) * ts + ts * 0.5f;
        const float py = static_cast<float>(ry + 1) * ts;

        const float dx = px - playerPos.x;
        const float dy = py - playerPos.y;
        if (dx * dx + dy * dy < safeR * safeR) continue;

        m_spawnPos = {px, py};
        return;
    }
    // Fallback: keep current spawnPos if no valid spot found
}
