#include "entities/Player.hpp"

#include "entities/ParticleSystem.hpp"
#include "world/TileMap.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {
constexpr float kWalkSpeed = 260.0f;
constexpr float kSprintSpeed = 370.0f;
constexpr float kGroundAcceleration = 2600.0f;
constexpr float kAirAcceleration = 1500.0f;
constexpr float kGroundFriction = 2300.0f;
constexpr float kAirFriction = 420.0f;
constexpr float kGravity = 2400.0f;
constexpr float kJumpVelocity       = 950.0f;
constexpr float kDoubleJumpVelocity = 880.0f;
constexpr float kMaxFallSpeed = 1450.0f;
constexpr float kCoyoteTime = 0.1f;
constexpr float kJumpBufferTime = 0.12f;
constexpr float kJumpCutMultiplier = 0.5f;

float approach(float current, float target, float delta) {
    if (current < target) {
        return std::min(current + delta, target);
    }
    return std::max(current - delta, target);
}
}  // namespace

Player::Player(sf::Vector2f spawnPosition)
    : m_position(spawnPosition)
    , m_eyeBlinkTimer(3.5f)
{}

void Player::reset(sf::Vector2f spawnPosition) {
    m_position = spawnPosition;
    m_velocity = {0.0f, 0.0f};
    m_onGround = false;
    m_wasOnGround = false;
    m_jumpCutPending = false;
    m_canDoubleJump = false;
    m_facing = 1;
    m_coyoteTimer = 0.0f;
    m_jumpBufferTimer = 0.0f;
    m_runDustCooldown = 0.0f;
    m_walkTimer = 0.0f;
    m_idleBreathTimer = 0.0f;
    m_squashTimer     = 0.0f;
    m_squashAmount    = 0.0f;
    m_stretchAmount   = 0.0f;
    m_armAngle        = 0.0f;
    m_tiltAngle       = 0.0f;
    m_eyeBlinkTimer   = 3.5f;
    m_eyeBlinkPhase   = 0.0f;
}

void Player::bounce() {
    m_velocity.y     = -720.0f;
    m_onGround       = false;
    m_coyoteTimer    = 0.0f;
    m_jumpCutPending = false;
    m_canDoubleJump  = true;  // stomp-bounce gives a free double-jump
}

void Player::warpX(float x) {
    m_position.x = x;
    m_velocity.x = 0.0f;
}

void Player::update(float dt, const InputState& input, const TileMap& map, ParticleSystem& particles) {
    m_wasOnGround = m_onGround;

    m_jumpBufferTimer = std::max(0.0f, m_jumpBufferTimer - dt);
    m_coyoteTimer = m_onGround ? kCoyoteTime : std::max(0.0f, m_coyoteTimer - dt);
    if (input.jumpPressed) {
        m_jumpBufferTimer = kJumpBufferTime;
    }
    if (!input.jumpHeld && m_velocity.y < 0.0f) {
        m_jumpCutPending = true;
    }

    const float maxSpeed = input.turboMode   ? kSprintSpeed * 3.0f
                         : input.sprintHeld  ? kSprintSpeed
                                             : kWalkSpeed;
    const bool hasMovementInput = std::abs(input.moveAxis) > 0.01f;
    const float targetSpeed = input.moveAxis * maxSpeed;

    if (hasMovementInput) {
        const float accel = m_onGround ? kGroundAcceleration : kAirAcceleration;
        m_velocity.x = approach(m_velocity.x, targetSpeed, accel * dt);
        m_facing = input.moveAxis > 0.0f ? 1 : -1;
    } else {
        const float friction = m_onGround ? kGroundFriction : kAirFriction;
        m_velocity.x = approach(m_velocity.x, 0.0f, friction * dt);
    }

    if (m_jumpBufferTimer > 0.0f && (m_onGround || m_coyoteTimer > 0.0f)) {
        m_velocity.y = -kJumpVelocity;
        m_jumpBufferTimer = 0.0f;
        m_coyoteTimer = 0.0f;
        m_onGround = false;
        m_canDoubleJump = true;   // first jump grants double-jump token

        particles.emit(m_position - sf::Vector2f(0.0f, 8.0f), sf::Vector2f(0.0f, -150.0f), 16, 2.7f, 0.35f,
                       sf::Color(225, 211, 171, 220), 2.0f, 4.5f);
    } else if (m_jumpBufferTimer > 0.0f && m_canDoubleJump) {
        // double jump — slightly weaker, bright particle burst
        m_velocity.y      = -kDoubleJumpVelocity;
        m_jumpBufferTimer = 0.0f;
        m_canDoubleJump   = false;
        m_jumpCutPending  = false;

        particles.emit(m_position - sf::Vector2f(0.0f, m_size.y * 0.5f),
                       sf::Vector2f(0.0f, -180.0f), 22, 3.14f, 0.45f,
                       sf::Color(255, 220, 80, 230), 2.5f, 5.0f);
    }

    m_velocity.y = std::min(m_velocity.y + kGravity * dt, kMaxFallSpeed);

    m_position.x += m_velocity.x * dt;
    resolveHorizontal(map);

    const float impactSpeed = m_velocity.y;
    m_position.y += m_velocity.y * dt;
    const bool landedThisFrame = resolveVertical(map);

    if (m_jumpCutPending) {
        m_velocity.y *= kJumpCutMultiplier;
        m_jumpCutPending = false;
    }

    if (landedThisFrame && !m_wasOnGround) {
        m_canDoubleJump = false;  // clear token on landing (re-granted by jumping)
        const float impact = std::clamp(std::abs(impactSpeed), 280.0f, 1000.0f);
        const int burstCount = static_cast<int>(impact / 90.0f);
        particles.emit(m_position - sf::Vector2f(0.0f, 4.0f), sf::Vector2f(0.0f, -120.0f), burstCount, 2.9f, 0.4f,
                       sf::Color(221, 193, 148, 220), 2.3f, 5.5f);
        // Squash on landing — bigger squash the harder the fall
        m_squashAmount = std::clamp(std::abs(impactSpeed) / kMaxFallSpeed, 0.08f, 0.36f);
        m_squashTimer  = 0.18f;
    }

    // ── Jump / fall stretch ───────────────────────────────────────────────
    if (!m_onGround) {
        const float t = std::clamp(-m_velocity.y / kJumpVelocity, 0.0f, 1.0f);
        m_stretchAmount = t * 0.18f;  // max 18 % taller at apex of jump
    } else {
        m_stretchAmount = 0.0f;
    }

    // ── Squash decay ──────────────────────────────────────────────────────
    if (m_squashTimer > 0.0f) {
        m_squashTimer = std::max(0.0f, m_squashTimer - dt);
    } else {
        m_squashAmount = approach(m_squashAmount, 0.0f, 2.0f * dt);
    }

    // ── Idle breath ───────────────────────────────────────────────────────
    const bool isIdle = m_onGround && std::abs(m_velocity.x) < 10.0f;
    if (isIdle) {
        m_idleBreathTimer += dt * 1.8f;  // ~1.8 Hz breathing
    }

    // ── Body tilt (smooth lean toward velocity) ───────────────────────────
    const float targetTilt = std::clamp(m_velocity.x / kSprintSpeed, -1.0f, 1.0f) * 10.0f;
    m_tiltAngle = approach(m_tiltAngle, targetTilt, 180.0f * dt);

    // ── Arm swing (mirrors legs but opposite phase) ───────────────────────
    m_armAngle = m_onGround ? std::sin(m_walkTimer + 3.14159f) * 30.0f * static_cast<float>(m_facing)
                            : (m_velocity.y < 0.0f ? -20.0f : 15.0f) * static_cast<float>(m_facing);

    // ── Eye blink ─────────────────────────────────────────────────────────
    m_eyeBlinkTimer -= dt;
    if (m_eyeBlinkPhase > 0.0f) {
        m_eyeBlinkPhase += dt * 8.0f;  // blink animation progresses
        if (m_eyeBlinkPhase >= 1.0f) {
            m_eyeBlinkPhase = 0.0f;
            m_eyeBlinkTimer = 2.5f + static_cast<float>(rand() % 300) / 100.0f;
        }
    } else if (m_eyeBlinkTimer <= 0.0f) {
        m_eyeBlinkPhase = 0.001f;  // start blink
    }

    // Walk animation timer
    if (m_onGround && std::abs(m_velocity.x) > 10.0f) {
        m_walkTimer += dt * std::abs(m_velocity.x) / kWalkSpeed * 12.0f;
    } else if (m_onGround) {
        m_walkTimer = 0.0f;  // standing still – reset to straight legs
    }
    // (timer is frozen mid-air to hold the last leg pose)

    if (m_onGround && std::abs(m_velocity.x) > 100.0f) {
        m_runDustCooldown -= dt;
        if (m_runDustCooldown <= 0.0f) {
            emitRunDust(particles);
            m_runDustCooldown = 0.045f;
        }
    } else {
        m_runDustCooldown = 0.0f;
    }
}

void Player::draw(sf::RenderTarget& target) const {
    const sf::FloatRect body = bounds();
    const float fDir = static_cast<float>(m_facing);

    // ── Squash / stretch scale factors ────────────────────────────────────
    // squashTimer > 0 means we apply squash, otherwise we may be stretching
    const float squashProg = (m_squashTimer > 0.0f)
        ? (m_squashTimer / 0.18f)  // 1→0 as timer drains
        : 0.0f;
    const float scaleY = 1.0f - m_squashAmount * squashProg + m_stretchAmount;
    const float scaleX = 1.0f / std::max(scaleY, 0.6f);   // volume-preserve X

    // Breath: subtle vertical bob when idle
    const float breathOff = std::sin(m_idleBreathTimer) * 2.2f;

    // Effective character dimensions after squash/stretch
    const float W  = body.width  * scaleX;
    const float H  = body.height * scaleY;
    const float cx = m_position.x;            // horizontal centre
    const float fy = m_position.y + breathOff; // feet Y (bottom of character)

    // ── Ground shadow ─────────────────────────────────────────────────────
    sf::CircleShape shadow(body.width * 0.44f, 28);
    shadow.setOrigin(shadow.getRadius(), shadow.getRadius() * 0.55f);
    shadow.setScale(1.0f, 0.45f);
    shadow.setPosition(cx, m_position.y + 5.0f);
    shadow.setFillColor(sf::Color(0, 0, 0, 55));
    target.draw(shadow);

    // ── Proportional heights (fraction of total H) ────────────────────────
    const float legH      = H * 0.30f;
    const float shoeH     = H * 0.09f;
    const float torsoH    = H * 0.34f;
    const float headR     = W * 0.30f;  // head radius
    const float armLen    = torsoH * 0.80f;
    const float armW      = W * 0.18f;

    const float hipY      = fy - legH - shoeH;          // where legs attach
    const float torsoTop  = hipY - torsoH;               // top of red torso
    const float headCy    = torsoTop - headR * 0.6f;    // head centre Y

    const float legW      = W * 0.24f;
    const float shoeW     = legW * 1.60f;
    const float hipOff    = W * 0.04f;  // small offset — side-view legs overlap

    const bool  walking   = m_onGround && std::abs(m_velocity.x) > 10.0f;
    const float maxAng    = 32.0f;
    const float phases[2] = { m_walkTimer,  m_walkTimer + 3.14159f };
    const float hipXs[2]  = { cx - fDir * hipOff,  cx + fDir * hipOff };

    // ── Arms (drawn behind torso when arm is behind) ──────────────────────
    // Arm behind torso: opposite side to facing direction
    const float armSwing  = m_armAngle;
    // Back arm (behind torso, rendered first)
    {
        const float backSide  = -fDir;  // back arm is on opposite side
        const float ang       = -armSwing * 0.8f;  // OPPOSITE swing to front arm
        const float rad       = ang * 3.14159f / 180.0f;
        const float shoulderX = cx + backSide * W * 0.08f;  // close to body — side view
        const float shoulderY = torsoTop + torsoH * 0.12f;

        sf::RectangleShape arm({armW, armLen});
        arm.setOrigin(armW * 0.5f, 0.0f);
        arm.setPosition(shoulderX, shoulderY);
        arm.setRotation(ang);
        arm.setFillColor(sf::Color(196, 41, 49));  // red sleeve
        target.draw(arm);

        // Gloved hand
        const float hx = shoulderX + std::sin(rad) * armLen;
        const float hy = shoulderY + std::cos(rad) * armLen;
        sf::CircleShape glove(armW * 0.75f, 12);
        glove.setOrigin(glove.getRadius(), glove.getRadius());
        glove.setPosition(hx, hy);
        glove.setFillColor(sf::Color(245, 245, 245));
        target.draw(glove);
    }

    // ── Legs & shoes ─────────────────────────────────────────────────────
    // Draw back leg first (i=0), then front leg (i=1) for correct layering
    for (int i = 0; i < 2; ++i) {
        const float swingDeg = walking ? std::sin(phases[i]) * maxAng * fDir : 0.0f;
        const float swingRad = swingDeg * 3.14159f / 180.0f;
        const float hx = hipXs[i];
        const bool isBackLeg = (i == 0);  // back leg is darker for depth

        // Upper leg (dark blue jeans — back leg is darker for depth cue)
        sf::RectangleShape leg({legW, legH * 0.65f});
        leg.setOrigin(legW * 0.5f, 0.0f);
        leg.setPosition(hx, hipY);
        leg.setRotation(swingDeg);
        leg.setFillColor(isBackLeg ? sf::Color(22, 55, 140) : sf::Color(31, 77, 181));
        target.draw(leg);

        // Lower leg
        const float kneeLegH = legH * 0.42f;
        const float kneeX = hx + std::sin(swingRad) * legH * 0.65f;
        const float kneeY = hipY + std::cos(swingRad) * legH * 0.65f;
        sf::RectangleShape lowerLeg({legW * 0.88f, kneeLegH});
        lowerLeg.setOrigin(legW * 0.44f, 0.0f);
        lowerLeg.setPosition(kneeX, kneeY);
        lowerLeg.setRotation(swingDeg * 0.6f);  // slight knee bend
        lowerLeg.setFillColor(isBackLeg ? sf::Color(28, 68, 160) : sf::Color(40, 90, 200));
        target.draw(lowerLeg);

        // Shoe at foot tip — stays flat on the ground (no rotation)
        const float footSwingRad = swingDeg * 0.6f * 3.14159f / 180.0f;
        const float fx = kneeX + std::sin(footSwingRad) * kneeLegH;
        const float fy2 = kneeY + std::cos(footSwingRad) * kneeLegH;
        sf::RectangleShape shoe({shoeW, shoeH});
        shoe.setOrigin(shoeW * 0.5f, 0.0f);
        shoe.setPosition(fx, fy2);
        shoe.setRotation(0.0f);  // shoe stays flat
        shoe.setFillColor(isBackLeg ? sf::Color(30, 12, 3) : sf::Color(40, 18, 5));
        target.draw(shoe);
        // Shoe highlight
        sf::RectangleShape shoeHL({shoeW * 0.5f, shoeH * 0.35f});
        shoeHL.setOrigin(shoeW * 0.25f, 0.0f);
        shoeHL.setPosition(fx - shoeW * 0.1f, fy2);
        shoeHL.setRotation(0.0f);  // flat highlight
        shoeHL.setFillColor(sf::Color(80, 48, 20, 160));
        target.draw(shoeHL);
    }

    // ── Torso (red shirt with overalls bib) ───────────────────────────────
    sf::RectangleShape torso({W * 0.44f, torsoH});
    torso.setOrigin(torso.getSize().x * 0.5f, torso.getSize().y);
    torso.setPosition(cx + fDir * W * 0.06f, hipY);
    torso.setRotation(m_tiltAngle * 0.5f);
    torso.setFillColor(sf::Color(196, 41, 49));
    target.draw(torso);

    // Overalls bib (blue rectangle on chest)
    sf::RectangleShape bib({W * 0.26f, torsoH * 0.55f});
    bib.setOrigin(bib.getSize().x * 0.5f, bib.getSize().y);
    bib.setPosition(cx + fDir * W * 0.08f, hipY - torsoH * 0.05f);
    bib.setRotation(m_tiltAngle * 0.5f);
    bib.setFillColor(sf::Color(31, 77, 181));
    target.draw(bib);

    // Overall strap left
    sf::RectangleShape strapL({W * 0.07f, torsoH * 0.45f});
    strapL.setOrigin(strapL.getSize().x * 0.5f, strapL.getSize().y);
    strapL.setPosition(cx + fDir * W * 0.02f, torsoTop + torsoH * 0.18f);
    strapL.setFillColor(sf::Color(31, 77, 181));
    target.draw(strapL);

    // Overall strap right
    sf::RectangleShape strapR({W * 0.07f, torsoH * 0.45f});
    strapR.setOrigin(strapR.getSize().x * 0.5f, strapR.getSize().y);
    strapR.setPosition(cx + fDir * W * 0.14f, torsoTop + torsoH * 0.18f);
    strapR.setFillColor(sf::Color(31, 77, 181));
    target.draw(strapR);

    // Overall buttons (two small yellow circles)
    for (int b = -1; b <= 1; b += 2) {
        sf::CircleShape btn(W * 0.045f, 8);
        btn.setOrigin(btn.getRadius(), btn.getRadius());
        btn.setPosition(cx + fDir * W * 0.08f + b * W * 0.05f, hipY - torsoH * 0.52f);
        btn.setFillColor(sf::Color(255, 210, 60));
        target.draw(btn);
    }

    // ── Front arm (in front of torso) ─────────────────────────────────────
    {
        const float frontSide = fDir;
        const float ang       = armSwing;
        const float rad       = ang * 3.14159f / 180.0f;
        const float shoulderX = cx + frontSide * W * 0.12f;  // close to body — side view
        const float shoulderY = torsoTop + torsoH * 0.12f;

        sf::RectangleShape arm({armW, armLen});
        arm.setOrigin(armW * 0.5f, 0.0f);
        arm.setPosition(shoulderX, shoulderY);
        arm.setRotation(ang);
        arm.setFillColor(sf::Color(196, 41, 49));
        target.draw(arm);

        const float hx = shoulderX + std::sin(rad) * armLen;
        const float hy = shoulderY + std::cos(rad) * armLen;
        sf::CircleShape glove(armW * 0.75f, 12);
        glove.setOrigin(glove.getRadius(), glove.getRadius());
        glove.setPosition(hx, hy);
        glove.setFillColor(sf::Color(245, 245, 245));
        target.draw(glove);
    }

    // ── Neck ──────────────────────────────────────────────────────────────
    sf::RectangleShape neck({W * 0.22f, H * 0.06f});
    neck.setOrigin(neck.getSize().x * 0.5f, neck.getSize().y);
    neck.setPosition(cx + fDir * W * 0.06f, torsoTop + H * 0.02f);
    neck.setFillColor(sf::Color(246, 206, 165));
    target.draw(neck);

    // ── Head (skin circle) ────────────────────────────────────────────────
    sf::CircleShape head(headR, 28);
    head.setOrigin(headR, headR);
    head.setPosition(cx, headCy);
    head.setFillColor(sf::Color(246, 206, 165));
    target.draw(head);

    // Ear (on the non-facing side)
    sf::CircleShape ear(headR * 0.28f, 14);
    ear.setOrigin(ear.getRadius(), ear.getRadius());
    ear.setPosition(cx - fDir * headR * 0.90f, headCy + headR * 0.10f);
    ear.setFillColor(sf::Color(235, 188, 148));
    target.draw(ear);

    // ── Hat (red cap) ─────────────────────────────────────────────────────
    const float hatTiltDeg = m_tiltAngle * 0.4f;

    // Hat dome
    sf::RectangleShape hatDome({W * 0.88f, H * 0.13f});
    hatDome.setOrigin(hatDome.getSize().x * 0.5f, hatDome.getSize().y);
    hatDome.setPosition(cx + fDir * W * 0.04f, headCy - headR * 0.62f);
    hatDome.setRotation(hatTiltDeg);
    hatDome.setFillColor(sf::Color(215, 40, 48));
    target.draw(hatDome);

    // Hat brim
    sf::RectangleShape hatBrim({W * 0.56f, H * 0.055f});
    hatBrim.setOrigin(hatBrim.getSize().x * 0.45f, hatBrim.getSize().y * 0.5f);
    hatBrim.setPosition(cx + fDir * W * 0.10f, headCy - headR * 0.62f + H * 0.005f);
    hatBrim.setRotation(hatTiltDeg);
    hatBrim.setFillColor(sf::Color(165, 25, 30));
    target.draw(hatBrim);

    // Hat 'M' badge (small white rectangle)
    sf::RectangleShape badge({W * 0.18f, H * 0.07f});
    badge.setOrigin(badge.getSize().x * 0.5f, badge.getSize().y * 0.5f);
    badge.setPosition(cx + fDir * W * 0.04f, headCy - headR * 0.62f - H * 0.06f);
    badge.setRotation(hatTiltDeg);
    badge.setFillColor(sf::Color(245, 245, 245));
    target.draw(badge);

    // ── Eyebrow (arched rect, rises when jumping) ─────────────────────────
    const float browLift = m_onGround ? 0.0f : -headR * 0.15f;
    sf::RectangleShape brow({headR * 0.50f, headR * 0.12f});
    brow.setOrigin(brow.getSize().x * 0.5f, brow.getSize().y * 0.5f);
    brow.setPosition(cx + fDir * headR * 0.35f, headCy - headR * 0.32f + browLift);
    brow.setRotation(fDir * -8.0f);  // slight inward angle
    brow.setFillColor(sf::Color(60, 30, 10));
    target.draw(brow);

    // ── Eyes ─────────────────────────────────────────────────────────────
    // Blink: scaleY of eye socket goes 1→0→1
    const float blinkScale = (m_eyeBlinkPhase > 0.0f)
        ? std::abs(std::cos(m_eyeBlinkPhase * 3.14159f))
        : 1.0f;

    // Only one visible eye on the side we are facing (Mario is 2.5D side-view)
    // Draw white eye socket
    sf::CircleShape eyeSocket(headR * 0.18f, 14);
    eyeSocket.setOrigin(eyeSocket.getRadius(), eyeSocket.getRadius());
    eyeSocket.setPosition(cx + fDir * headR * 0.35f, headCy - headR * 0.08f);
    eyeSocket.setScale(1.0f, blinkScale);
    eyeSocket.setFillColor(sf::Color(245, 245, 245));
    target.draw(eyeSocket);

    // Pupil
    if (blinkScale > 0.1f) {
        sf::CircleShape pupil(headR * 0.10f, 10);
        pupil.setOrigin(pupil.getRadius(), pupil.getRadius());
        // Pupil looks in facing direction, slightly upward when jumping
        const float pupilOffX = fDir * headR * 0.08f;
        const float pupilOffY = m_onGround ? 0.0f : -headR * 0.06f;
        pupil.setPosition(eyeSocket.getPosition().x + pupilOffX,
                          eyeSocket.getPosition().y + pupilOffY);
        pupil.setScale(1.0f, blinkScale);
        pupil.setFillColor(sf::Color(28, 28, 36));
        target.draw(pupil);

        // Pupil highlight
        sf::CircleShape highlight(headR * 0.04f, 6);
        highlight.setOrigin(highlight.getRadius(), highlight.getRadius());
        highlight.setPosition(pupil.getPosition().x + headR * 0.05f,
                              pupil.getPosition().y - headR * 0.05f);
        highlight.setScale(1.0f, blinkScale);
        highlight.setFillColor(sf::Color(255, 255, 255, 200));
        target.draw(highlight);
    }

    // ── Nose ──────────────────────────────────────────────────────────────
    sf::CircleShape nose(headR * 0.20f, 12);
    nose.setOrigin(nose.getRadius(), nose.getRadius());
    nose.setScale(1.1f, 0.85f);
    nose.setPosition(cx + fDir * headR * 0.55f, headCy + headR * 0.08f);
    nose.setFillColor(sf::Color(220, 155, 110));
    target.draw(nose);

    // ── Moustache (two curved bushy shapes) ───────────────────────────────
    const float moustacheY = headCy + headR * 0.36f;
    // Centre moustache piece
    sf::CircleShape mCentre(headR * 0.18f, 10);
    mCentre.setOrigin(mCentre.getRadius(), mCentre.getRadius());
    mCentre.setScale(1.0f, 0.70f);
    mCentre.setPosition(cx + fDir * headR * 0.28f, moustacheY);
    mCentre.setFillColor(sf::Color(62, 32, 12));
    target.draw(mCentre);

    // Side moustache piece
    sf::CircleShape mSide(headR * 0.15f, 10);
    mSide.setOrigin(mSide.getRadius(), mSide.getRadius());
    mSide.setScale(1.0f, 0.60f);
    mSide.setPosition(cx + fDir * headR * 0.55f, moustacheY + headR * 0.06f);
    mSide.setFillColor(sf::Color(62, 32, 12));
    target.draw(mSide);

    // Inner moustache piece (toward facing dir — gives full bushy look)
    sf::CircleShape mInner(headR * 0.13f, 10);
    mInner.setOrigin(mInner.getRadius(), mInner.getRadius());
    mInner.setScale(1.0f, 0.60f);
    mInner.setPosition(cx + fDir * headR * 0.04f, moustacheY + headR * 0.02f);
    mInner.setFillColor(sf::Color(62, 32, 12));
    target.draw(mInner);

    // ── Mouth / expression (smile when on ground, open O when jumping) ────
    if (!m_onGround && m_velocity.y < -100.0f) {
        // Open-mouth "Woo!" during jump ascent
        sf::CircleShape mouth(headR * 0.13f, 10);
        mouth.setOrigin(mouth.getRadius(), mouth.getRadius());
        mouth.setScale(1.0f, 0.8f);
        mouth.setPosition(cx + fDir * headR * 0.28f, headCy + headR * 0.55f);
        mouth.setFillColor(sf::Color(100, 20, 5));
        target.draw(mouth);
    } else {
        // Smile — two small rects angled outward
        for (int s = -1; s <= 1; s += 2) {
            sf::RectangleShape smileSeg({headR * 0.18f, headR * 0.07f});
            smileSeg.setOrigin(smileSeg.getSize().x * 0.5f, smileSeg.getSize().y * 0.5f);
            smileSeg.setPosition(cx + fDir * (headR * 0.18f + s * headR * 0.15f),
                                 headCy + headR * 0.55f + std::abs(s) * headR * 0.08f);
            smileSeg.setRotation(s * fDir * 20.0f);
            smileSeg.setFillColor(sf::Color(110, 30, 10));
            target.draw(smileSeg);
        }
    }
}

sf::Vector2f Player::position() const { return m_position; }

sf::FloatRect Player::bounds() const { return boundsAt(m_position); }

float Player::height() const { return m_size.y; }

int Player::facing() const { return m_facing; }

sf::FloatRect Player::boundsAt(sf::Vector2f position) const {
    return {position.x - m_size.x * 0.5f, position.y - m_size.y, m_size.x, m_size.y};
}

void Player::resolveHorizontal(const TileMap& map) {
    sf::FloatRect body = bounds();
    const auto colliders = map.querySolidTiles(body);

    for (const auto& tile : colliders) {
        sf::FloatRect overlap;
        if (!body.intersects(tile, overlap)) {
            continue;
        }
        if (m_velocity.x > 0.0f) {
            m_position.x -= overlap.width;
        } else if (m_velocity.x < 0.0f) {
            m_position.x += overlap.width;
        }
        m_velocity.x = 0.0f;
        body = bounds();
    }
}

bool Player::resolveVertical(const TileMap& map) {
    bool landed = false;
    m_onGround = false;

    sf::FloatRect body = bounds();
    const auto colliders = map.querySolidTiles(body);

    for (const auto& tile : colliders) {
        sf::FloatRect overlap;
        if (!body.intersects(tile, overlap)) {
            continue;
        }

        if (m_velocity.y > 0.0f) {
            m_position.y -= overlap.height;
            m_onGround = true;
            landed = true;
        } else if (m_velocity.y < 0.0f) {
            m_position.y += overlap.height;
        }

        m_velocity.y = 0.0f;
        body = bounds();
    }

    return landed;
}

void Player::emitRunDust(ParticleSystem& particles) {
    particles.emit(m_position - sf::Vector2f(static_cast<float>(m_facing) * 10.0f, 4.0f),
                   sf::Vector2f(-m_velocity.x * 0.3f, -20.0f), 3, 0.75f, 0.22f, sf::Color(235, 205, 162, 210), 1.5f,
                   3.5f);
}
