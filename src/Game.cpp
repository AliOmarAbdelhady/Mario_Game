#include "Game.hpp"

#include "core/Config.hpp"
#include "core/InputState.hpp"

#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

namespace {
std::vector<std::string> buildLevel() {
    constexpr int width  = 220;
    constexpr int height = 44;   // doubled — upper half is "level 2" sky area
    std::vector<std::string> rows(height, std::string(width, '.'));

    auto fillRect = [&](int x0, int y0, int x1, int y1) {
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = '#';
                }
            }
        }
    };

    // ── LEVEL 1 (bottom half) ──────────────────────────────────────────────
    const int g = height - 2;   // ground row index

    fillRect(0, g, width - 1, height - 1);  // ground floor

    // low floating platforms
    fillRect(7,  g - 3, 16,  g - 3);
    fillRect(22, g - 3, 29,  g - 3);
    fillRect(33, g - 3, 39,  g - 3);
    fillRect(47, g - 3, 56,  g - 3);
    fillRect(64, g - 4, 71,  g - 4);
    fillRect(79, g - 3, 91,  g - 3);

    // staircase (5 steps)
    for (int i = 0; i < 5; ++i) {
        fillRect(102 + i * 2, g - 1 - i, 103 + i * 2, g);
    }

    // more ground-level platforms
    fillRect(131, g - 3, 151, g - 3);
    fillRect(158, g - 3, 168, g - 3);
    fillRect(179, g - 3, 195, g - 3);
    fillRect(202, g - 4, 215, g - 4);

    // pits (2 tiles wide each)
    fillRect(44, g, 50, height - 1);
    fillRect(113, g, 119, height - 1);
    fillRect(171, g, 176, height - 1);
    for (int x = 46; x <= 47; ++x) {
        rows[static_cast<std::size_t>(g)][static_cast<std::size_t>(x)]          = '.';
        rows[static_cast<std::size_t>(height-1)][static_cast<std::size_t>(x)]   = '.';
    }
    for (int x = 115; x <= 116; ++x) {
        rows[static_cast<std::size_t>(g)][static_cast<std::size_t>(x)]          = '.';
        rows[static_cast<std::size_t>(height-1)][static_cast<std::size_t>(x)]   = '.';
    }
    for (int x = 172; x <= 173; ++x) {
        rows[static_cast<std::size_t>(g)][static_cast<std::size_t>(x)]          = '.';
        rows[static_cast<std::size_t>(height-1)][static_cast<std::size_t>(x)]   = '.';
    }

    // ── LEVEL 2 (upper half) ───────────────────────────────────────────────
    // Solid ceiling/floor of level 2
    const int l2floor = height / 2;          // row 22
    fillRect(0, l2floor, width - 1, l2floor);   // L2 ground row

    // Upper platforms (6–10 tiles above L2 floor)
    fillRect(0,   l2floor - 5,  18,  l2floor - 5);
    fillRect(24,  l2floor - 7,  38,  l2floor - 7);
    fillRect(44,  l2floor - 5,  60,  l2floor - 5);
    fillRect(66,  l2floor - 8,  80,  l2floor - 8);
    fillRect(86,  l2floor - 5, 105,  l2floor - 5);
    fillRect(110, l2floor - 7, 128,  l2floor - 7);
    fillRect(134, l2floor - 5, 152,  l2floor - 5);
    fillRect(158, l2floor - 8, 174,  l2floor - 8);
    fillRect(180, l2floor - 5, 200,  l2floor - 5);
    fillRect(205, l2floor - 6, 219,  l2floor - 6);

    // Small gaps in L2 floor so Mario can drop back to L1
    for (int x = 20; x <= 22; ++x)
        rows[static_cast<std::size_t>(l2floor)][static_cast<std::size_t>(x)] = '.';
    for (int x = 62; x <= 64; ++x)
        rows[static_cast<std::size_t>(l2floor)][static_cast<std::size_t>(x)] = '.';
    for (int x = 130; x <= 132; ++x)
        rows[static_cast<std::size_t>(l2floor)][static_cast<std::size_t>(x)] = '.';
    for (int x = 175; x <= 177; ++x)
        rows[static_cast<std::size_t>(l2floor)][static_cast<std::size_t>(x)] = '.';

    // Spawn
    rows[static_cast<std::size_t>(g - 3)][5] = 'S';
    return rows;
}

sf::Vector2f findSpawn(const std::vector<std::string>& rows) {
    for (std::size_t y = 0; y < rows.size(); ++y) {
        for (std::size_t x = 0; x < rows[y].size(); ++x) {
            if (rows[y][x] == 'S') {
                return {static_cast<float>(x * config::TileSize + config::TileSize / 2),
                        static_cast<float>(y * config::TileSize + config::TileSize)};
            }
        }
    }
    return {200.0f, 300.0f};
}
}  // namespace

Game::Game()
    : m_window(sf::VideoMode(config::WindowWidth, config::WindowHeight), "Super Mario"),
      m_map(buildLevel(), config::TileSize),
      m_player(findSpawn(m_map.rows())),
      m_camera(sf::Vector2f(static_cast<float>(config::WindowWidth), static_cast<float>(config::WindowHeight))) {
    m_window.setVerticalSyncEnabled(true);
    m_window.setKeyRepeatEnabled(false);
    m_lighting.initialize(m_window.getSize());

    m_spawnPosition = findSpawn(m_map.rows());

    const std::vector<std::string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf"};
    for (const auto& path : fontPaths) {
        if (m_font.loadFromFile(path)) {
            m_fontLoaded = true;
            break;
        }
    }

    const sf::FloatRect worldBounds(0.0f, 0.0f,
        static_cast<float>(m_map.pixelWidth()),
        static_cast<float>(m_map.pixelHeight()));
    m_camera.update(10.0f, m_spawnPosition + sf::Vector2f(170.0f, -150.0f), worldBounds, 1.0f);

    loadHighScore();
}

void Game::run() {
    while (m_window.isOpen()) {
        processEvents();

        float frameSeconds = m_frameClock.restart().asSeconds();
        frameSeconds = std::clamp(frameSeconds, 0.0f, 0.1f);
        m_accumulator += frameSeconds;

        while (m_accumulator >= config::FixedDeltaSeconds) {
            update(config::FixedDeltaSeconds);
            m_accumulator -= config::FixedDeltaSeconds;
        }

        render();
    }
}

void Game::processEvents() {
    sf::Event event{};
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }

        if (event.type == sf::Event::Resized) {
            const sf::Vector2u newSize(event.size.width, event.size.height);
            if (newSize.x > 0 && newSize.y > 0) {
                m_camera.resize(sf::Vector2f(static_cast<float>(newSize.x), static_cast<float>(newSize.y)));
                m_lighting.resize(newSize);
            }
        }

        if (event.type == sf::Event::KeyPressed) {
            if (m_state == GameState::Menu) {
                if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                    resetGame();
                    m_state = GameState::Playing;
                }
            } else if (m_state == GameState::GameOver) {
                if (event.key.code == sf::Keyboard::R || event.key.code == sf::Keyboard::Return) {
                    resetGame();
                    m_state = GameState::Playing;
                }
                if (event.key.code == sf::Keyboard::Escape) {
                    m_state = GameState::Menu;
                }
            } else if (m_state == GameState::Playing) {
                if (event.key.code == sf::Keyboard::Escape) {
                    m_state = GameState::Menu;
                }
                if (event.key.code == sf::Keyboard::R) {
                    m_turboMode = !m_turboMode;
                }
            }
        }
    }
}

void Game::update(float dt) {
    if (m_state == GameState::GameOver) {
        m_particles.update(dt);
        return;
    }
    if (m_state != GameState::Playing) {
        return;
    }

    InputState input{};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        input.moveAxis -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        input.moveAxis += 1.0f;
    }
    input.sprintHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
    input.turboMode  = m_turboMode;

    const bool jumpHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
                          sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
    input.jumpHeld = jumpHeld;
    input.jumpPressed = jumpHeld && !m_prevJumpHeld;
    m_prevJumpHeld = jumpHeld;

    m_player.update(dt, input, m_map, m_particles);
    m_particles.update(dt);

    // ---- survival bonus: +50 every 30 seconds alive ----
    m_timePlayed += dt;
    m_bonusTimer += dt;
    if (m_bonusTimer >= 30.0f) {
        m_bonusTimer -= 30.0f;
        m_score += 50;
    }
    // ---- enemy update & player collision ----
    for (auto& enemy : m_enemies) {
        enemy.update(dt, m_map, m_player.position());
        if (!enemy.isActive()) continue;

        const sf::FloatRect pb = m_player.bounds();
        const sf::FloatRect eb = enemy.bounds();
        sf::FloatRect overlap;
        if (pb.intersects(eb, overlap)) {
            if (m_player.position().y <= eb.top + 18.0f) {
                enemy.stomp(m_particles);
                m_player.bounce();
                m_score += 100;
            } else {
                if (m_score > m_highScore) m_highScore = m_score;
                if (m_coinCount > m_coinHighScore) m_coinHighScore = m_coinCount;
                saveHighScore();
                m_state = GameState::GameOver;
                return;
            }
        }
    }
    // ---- end enemy ----

    // ---- box update & head-hit detection ----
    for (auto& box : m_boxes) {
        box.update(dt);
        if (!box.isActive()) continue;

        const sf::FloatRect pb = m_player.bounds();
        const sf::FloatRect bb = box.bounds();
        sf::FloatRect overlap;
        if (pb.intersects(bb, overlap)) {
            // Mario must be hitting from below: his head (top of pb) is near the bottom of the box
            const float playerTop = pb.top;
            const float boxBottom = bb.top + bb.height;
            if (playerTop <= boxBottom && playerTop >= bb.top + bb.height * 0.5f
                && m_player.position().y > bb.top + bb.height) {
                sf::Vector2f coinPos = box.hit(m_particles);
                m_coins.emplace_back(coinPos);
            }
        }
    }

    // ---- coin update & collection ----
    for (auto& coin : m_coins) {
        coin.update(dt, m_map);
        if (coin.isCollectable() && !coin.isCollected()) {
            const sf::FloatRect pb = m_player.bounds();
            const sf::FloatRect cb = coin.bounds();
            if (pb.intersects(cb)) {
                coin.collect();
                m_coinCount += 1;
                m_score += 200;
            }
        }
    }
    // Remove fully expired coins
    m_coins.erase(
        std::remove_if(m_coins.begin(), m_coins.end(),
                       [](const Coin& c) { return c.isExpired(); }),
        m_coins.end());
    // ---- end coin ----

    const sf::FloatRect worldBounds(0.0f, 0.0f, static_cast<float>(m_map.pixelWidth()),
                                    static_cast<float>(m_map.pixelHeight()));
    const sf::Vector2f cameraFocus =
        m_player.position() + sf::Vector2f(static_cast<float>(m_player.facing()) * 170.0f, -150.0f);

    // Dynamic zoom-out: as Mario rises, zoom out so the ground stays visible.
    // groundY = bottom of the world in pixels
    const float groundY  = static_cast<float>(m_map.pixelHeight());
    const float playerY  = m_player.position().y;
    // heightAboveGround is how far up Mario is (negative = above ground row)
    const float heightAboveGround = groundY - playerY;
    // base view height = 900.  When Mario is >= 1200 px above ground zoom = 2x
    const float baseH    = static_cast<float>(config::WindowHeight);
    const float zoomNeeded = 1.0f + std::max(0.0f, (heightAboveGround - baseH * 0.55f) / (baseH * 0.9f));
    const float targetZoom = std::clamp(zoomNeeded, 1.0f, 2.4f);

    m_camera.update(dt, cameraFocus, worldBounds, targetZoom);

    // ---- world wrap: when Mario reaches either edge, loop him back ----
    {
        const float mapW     = static_cast<float>(m_map.pixelWidth());
        const float wrapIn   = static_cast<float>(m_map.tileSize()) * 3.0f; // land 3 tiles in
        const float wrapEdge = static_cast<float>(m_map.tileSize()) * 1.0f; // trigger 1 tile from edge
        if (m_player.position().x > mapW - wrapEdge) {
            m_player.warpX(wrapIn);
            m_camera.snapX(wrapIn);
        } else if (m_player.position().x < wrapEdge) {
            m_player.warpX(mapW - wrapIn);
            m_camera.snapX(mapW - wrapIn);
        }
    }

    if (m_player.position().y > groundY + 200.0f) {
        if (m_score > m_highScore) m_highScore = m_score;
        if (m_coinCount > m_coinHighScore) m_coinHighScore = m_coinCount;
        saveHighScore();
        m_state = GameState::GameOver;
    }
}

void Game::render() {
    m_window.clear(sf::Color::Black);

    sf::RenderTarget& scene = m_lighting.beginScene();
    scene.setView(m_camera.view());

    const sf::FloatRect worldBounds(0.0f, 0.0f, static_cast<float>(m_map.pixelWidth()),
                                    static_cast<float>(m_map.pixelHeight()));
    m_background.draw(scene, m_camera.view(), worldBounds);
    m_map.draw(scene, m_camera.view());
    m_player.draw(scene);
    for (const auto& box : m_boxes) {
        box.draw(scene);
    }
    for (const auto& coin : m_coins) {
        coin.draw(scene);
    }
    for (const auto& enemy : m_enemies) {
        enemy.draw(scene);
    }
    m_particles.draw(scene);

    const sf::Vector2f lightPosition = m_player.position() + sf::Vector2f(0.0f, -m_player.height() * 0.7f);
    m_lighting.endScene(m_window, lightPosition, m_camera.view());

    if (m_state == GameState::Menu) {
        renderMenu();
    } else if (m_state == GameState::GameOver) {
        renderGameOver();
    } else if (m_state == GameState::Playing) {
        drawHUD();
    }

    m_window.display();
}

void Game::renderMenu() {
    const sf::Vector2u winSize = m_window.getSize();
    const float W = static_cast<float>(winSize.x);
    const float H = static_cast<float>(winSize.y);

    m_window.setView(m_window.getDefaultView());

    sf::RectangleShape overlay({W, H});
    overlay.setFillColor(sf::Color(0, 0, 0, 165));
    m_window.draw(overlay);

    if (!m_fontLoaded) return;

    sf::RectangleShape titleBg({W * 0.68f, 130.f});
    titleBg.setOrigin(titleBg.getSize().x * 0.5f, titleBg.getSize().y * 0.5f);
    titleBg.setPosition(W * 0.5f, H * 0.30f);
    titleBg.setFillColor(sf::Color(180, 30, 30, 210));
    m_window.draw(titleBg);

    sf::Text title("SUPER MARIO", m_font, 96);
    title.setFillColor(sf::Color(255, 228, 0));
    title.setOutlineColor(sf::Color(120, 0, 0));
    title.setOutlineThickness(4.f);
    const auto tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width * 0.5f, tb.top + tb.height * 0.5f);
    title.setPosition(W * 0.5f, H * 0.30f);
    m_window.draw(title);

    sf::Text prompt("Press  ENTER  to  Start", m_font, 46);
    prompt.setFillColor(sf::Color(255, 255, 255));
    prompt.setOutlineColor(sf::Color(0, 0, 0));
    prompt.setOutlineThickness(2.f);
    const auto pb = prompt.getLocalBounds();
    prompt.setOrigin(pb.left + pb.width * 0.5f, pb.top + pb.height * 0.5f);
    prompt.setPosition(W * 0.5f, H * 0.52f);
    m_window.draw(prompt);

    sf::Text controls("A / D : Move     Space : Jump     Shift : Sprint     ESC : Menu", m_font, 28);
    controls.setFillColor(sf::Color(190, 190, 190));
    controls.setOutlineColor(sf::Color(0, 0, 0));
    controls.setOutlineThickness(1.f);
    const auto cb = controls.getLocalBounds();
    controls.setOrigin(cb.left + cb.width * 0.5f, cb.top + cb.height * 0.5f);
    controls.setPosition(W * 0.5f, H * 0.75f);
    m_window.draw(controls);

    if (m_highScore > 0) {
        sf::Text hs("Best:  " + std::to_string(m_highScore), m_font, 38);
        hs.setFillColor(sf::Color(255, 215, 0));
        hs.setOutlineColor(sf::Color(80, 50, 0));
        hs.setOutlineThickness(2.f);
        const auto hb = hs.getLocalBounds();
        hs.setOrigin(hb.left + hb.width * 0.5f, hb.top + hb.height * 0.5f);
        hs.setPosition(W * 0.5f, H * 0.63f);
        m_window.draw(hs);
    }
}

void Game::renderGameOver() {
    const sf::Vector2u winSize = m_window.getSize();
    const float W = static_cast<float>(winSize.x);
    const float H = static_cast<float>(winSize.y);

    m_window.setView(m_window.getDefaultView());

    sf::RectangleShape overlay({W, H});
    overlay.setFillColor(sf::Color(0, 0, 0, 190));
    m_window.draw(overlay);

    if (!m_fontLoaded) return;

    sf::Text title("GAME  OVER", m_font, 110);
    title.setFillColor(sf::Color(220, 30, 30));
    title.setOutlineColor(sf::Color(70, 0, 0));
    title.setOutlineThickness(5.f);
    const auto tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width * 0.5f, tb.top + tb.height * 0.5f);
    title.setPosition(W * 0.5f, H * 0.30f);
    m_window.draw(title);

    sf::Text scoreTxt("Score:  " + std::to_string(m_score), m_font, 58);
    scoreTxt.setFillColor(sf::Color(255, 228, 0));
    scoreTxt.setOutlineColor(sf::Color(80, 50, 0));
    scoreTxt.setOutlineThickness(3.f);
    const auto st = scoreTxt.getLocalBounds();
    scoreTxt.setOrigin(st.left + st.width * 0.5f, st.top + st.height * 0.5f);
    scoreTxt.setPosition(W * 0.5f, H * 0.46f);
    m_window.draw(scoreTxt);

    // high score line
    const bool isNewRecord = (m_score > 0 && m_score >= m_highScore);
    std::string hsStr = isNewRecord ? "NEW RECORD!  " + std::to_string(m_highScore)
                                    : "Best:  " + std::to_string(m_highScore);
    sf::Text hsTxt(hsStr, m_font, 38);
    hsTxt.setFillColor(isNewRecord ? sf::Color(80, 255, 120) : sf::Color(200, 200, 200));
    hsTxt.setOutlineColor(sf::Color(0, 0, 0));
    hsTxt.setOutlineThickness(2.f);
    const auto ht = hsTxt.getLocalBounds();
    hsTxt.setOrigin(ht.left + ht.width * 0.5f, ht.top + ht.height * 0.5f);
    hsTxt.setPosition(W * 0.5f, H * 0.575f);
    m_window.draw(hsTxt);

    // Coin stats line
    const bool isNewCoinRecord = (m_coinCount > 0 && m_coinCount >= m_coinHighScore);
    const std::string coinStr = "Coins:  " + std::to_string(m_coinCount)
        + (isNewCoinRecord ? "  \u2605 BEST!" : "   Best:  " + std::to_string(m_coinHighScore));
    sf::Text coinTxt(coinStr, m_font, 34);
    coinTxt.setFillColor(isNewCoinRecord ? sf::Color(255, 230, 50) : sf::Color(210, 210, 120));
    coinTxt.setOutlineColor(sf::Color(0, 0, 0));
    coinTxt.setOutlineThickness(2.f);
    const auto ct = coinTxt.getLocalBounds();
    coinTxt.setOrigin(ct.left + ct.width * 0.5f, ct.top + ct.height * 0.5f);
    coinTxt.setPosition(W * 0.5f, H * 0.645f);
    m_window.draw(coinTxt);

    sf::Text restart("Press  R  or  ENTER  to  Restart", m_font, 40);
    restart.setFillColor(sf::Color(255, 255, 255));
    restart.setOutlineColor(sf::Color(0, 0, 0));
    restart.setOutlineThickness(2.f);
    const auto rb = restart.getLocalBounds();
    restart.setOrigin(rb.left + rb.width * 0.5f, rb.top + rb.height * 0.5f);
    restart.setPosition(W * 0.5f, H * 0.69f);
    m_window.draw(restart);

    sf::Text menuHint("Press  ESC  for  Menu", m_font, 34);
    menuHint.setFillColor(sf::Color(170, 170, 170));
    menuHint.setOutlineColor(sf::Color(0, 0, 0));
    menuHint.setOutlineThickness(1.f);
    const auto mh = menuHint.getLocalBounds();
    menuHint.setOrigin(mh.left + mh.width * 0.5f, mh.top + mh.height * 0.5f);
    menuHint.setPosition(W * 0.5f, H * 0.81f);
    m_window.draw(menuHint);
}

void Game::resetGame() {
    m_score        = 0;
    m_coinCount    = 0;
    m_turboMode    = false;
    m_timePlayed   = 0.0f;
    m_bonusTimer   = 0.0f;
    m_respawnTimer = 2.0f;
    m_player.reset(m_spawnPosition);
    m_prevJumpHeld = false;
    m_accumulator  = 0.0f;
    m_coins.clear();
    const sf::FloatRect worldBounds(0.0f, 0.0f,
        static_cast<float>(m_map.pixelWidth()),
        static_cast<float>(m_map.pixelHeight()));
    m_camera.update(10.0f, m_spawnPosition + sf::Vector2f(170.0f, -150.0f), worldBounds, 1.0f);
    spawnEnemies();
    spawnBoxes();
}

void Game::spawnEnemies() {
    m_enemies.clear();

    // Seed the random number generator
    static bool seeded = false;
    if (!seeded) { std::srand(static_cast<unsigned>(std::time(nullptr))); seeded = true; }

    const auto& rows = m_map.rows();
    const int   h    = static_cast<int>(rows.size());
    const int   w    = static_cast<int>(rows[0].size());
    const float ts   = static_cast<float>(m_map.tileSize());
    const int   groundRow = h - 2;  // the main L1 ground row

    struct SpawnSlot { float x; float y; };
    std::vector<SpawnSlot> groundSlots;   // on the main ground floor
    std::vector<SpawnSlot> elevatedSlots; // on platforms / upper levels

    for (int y = 0; y < h - 1; ++y) {
        for (int x = 0; x < w; ++x) {
            const char cell  = rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            const char below = rows[static_cast<std::size_t>(y + 1)][static_cast<std::size_t>(x)];
            if (cell != '#' && cell != 'S' && below == '#') {
                const float px = static_cast<float>(x) * ts + ts * 0.5f;
                const float py = static_cast<float>(y + 1) * ts;
                if (y + 1 == groundRow) {
                    groundSlots.push_back({px, py});
                } else {
                    elevatedSlots.push_back({px, py});
                }
            }
        }
    }

    // Shuffle both pools
    auto shuffle = [](std::vector<SpawnSlot>& v) {
        if (v.size() < 2) return;
        for (std::size_t i = v.size() - 1; i > 0; --i) {
            const std::size_t j = static_cast<std::size_t>(std::rand()) % (i + 1);
            std::swap(v[i], v[j]);
        }
    };
    shuffle(groundSlots);
    shuffle(elevatedSlots);

    // Fixed counts: 8 on ground, 8 on elevated — always exactly 50/50
    const int halfCount    = 8;
    const float safeRadius = ts * 8.0f;

    auto spawnFrom = [&](std::vector<SpawnSlot>& pool, int count) {
        int spawned = 0;
        for (const auto& slot : pool) {
            if (spawned >= count) break;
            const float dx = slot.x - m_spawnPosition.x;
            const float dy = slot.y - m_spawnPosition.y;
            if (dx * dx + dy * dy < safeRadius * safeRadius) continue;
            m_enemies.emplace_back(sf::Vector2f(slot.x, slot.y));
            ++spawned;
        }
    };

    spawnFrom(groundSlots,   halfCount);
    spawnFrom(elevatedSlots, halfCount);
}

void Game::spawnBoxes() {
    m_boxes.clear();

    const auto& rows = m_map.rows();
    const int   h    = static_cast<int>(rows.size());
    const int   w    = h > 0 ? static_cast<int>(rows[0].size()) : 0;
    const float ts   = static_cast<float>(m_map.tileSize());

    // Valid box position: empty tile with 1 empty tile above (coin pops out)
    // and exactly 2 empty tiles below (Mario head room), then solid ground
    // within 3 tiles below that (reachable by a normal jump).
    struct BoxSlot { float x; float y; };
    std::vector<BoxSlot> validSlots;

    for (int y = 2; y < h - 3; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            const char cell  = rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            if (cell != '.') continue;  // must be empty

            // Need 1 empty tile above (so coin can pop out)
            const char above = rows[static_cast<std::size_t>(y - 1)][static_cast<std::size_t>(x)];  
            if (above == '#') continue;

            // Need 2 empty tiles below (Mario's body needs room to jump)
            if (y + 2 >= h) continue;
            const char below1 = rows[static_cast<std::size_t>(y + 1)][static_cast<std::size_t>(x)];
            const char below2 = rows[static_cast<std::size_t>(y + 2)][static_cast<std::size_t>(x)];
            if (below1 == '#' || below2 == '#') continue;

            // Must have a solid ground tile exactly 3 tiles below (jump-reachable)
            // This keeps boxes at a consistent, reachable height
            bool hasGround = false;
            for (int dy = 3; dy <= 4 && y + dy < h; ++dy) {
                if (rows[static_cast<std::size_t>(y + dy)][static_cast<std::size_t>(x)] == '#') {
                    hasGround = true;
                    break;
                }
            }
            if (!hasGround) continue;

            const float px = static_cast<float>(x) * ts + ts * 0.5f;
            const float py = static_cast<float>(y) * ts + ts * 0.5f;
            validSlots.push_back({px, py});
        }
    }

    if (validSlots.empty()) return;

    // Shuffle
    for (std::size_t i = validSlots.size() - 1; i > 0; --i) {
        const std::size_t j = static_cast<std::size_t>(std::rand()) % (i + 1);
        std::swap(validSlots[i], validSlots[j]);
    }

    // Spawn 25-35 boxes spread across the map
    const int desiredCount = std::min(35, std::max(25, static_cast<int>(validSlots.size()) / 12));
    const float minDist    = ts * 4.0f;  // boxes must be spaced at least 4 tiles apart

    for (const auto& slot : validSlots) {
        if (static_cast<int>(m_boxes.size()) >= desiredCount) break;

        // Ensure minimum spacing between boxes
        bool tooClose = false;
        for (const auto& existing : m_boxes) {
            const float dx = slot.x - existing.position().x;
            const float dy = slot.y - existing.position().y;
            if (dx * dx + dy * dy < minDist * minDist) {
                tooClose = true;
                break;
            }
        }
        if (tooClose) continue;

        m_boxes.emplace_back(sf::Vector2f(slot.x, slot.y), ts);
    }
}

void Game::drawHUD() {
    if (!m_fontLoaded) return;

    m_window.setView(m_window.getDefaultView());

    const float pad = 28.f;

    // Score pill background
    sf::RectangleShape pill({270.f, 56.f});
    pill.setPosition(pad, pad);
    pill.setFillColor(sf::Color(0, 0, 0, 150));
    m_window.draw(pill);

    // "SCORE" label
    sf::Text label("SCORE", m_font, 22);
    label.setFillColor(sf::Color(255, 228, 0));
    label.setPosition(pad + 12.f, pad + 4.f);
    m_window.draw(label);

    // Score value
    sf::Text value(std::to_string(m_score), m_font, 30);
    value.setFillColor(sf::Color::White);
    value.setOutlineColor(sf::Color(0, 0, 0, 180));
    value.setOutlineThickness(2.f);
    value.setPosition(pad + 12.f, pad + 22.f);
    m_window.draw(value);

    // Best score (inline on same pill, right-aligned)
    if (m_highScore > 0) {
        sf::Text best("BEST  " + std::to_string(m_highScore), m_font, 18);
        best.setFillColor(sf::Color(255, 215, 0, 180));
        best.setPosition(pad + 148.f, pad + 8.f);
        m_window.draw(best);
    }

    // Enemy count pill background
    sf::RectangleShape pill2({270.f, 56.f});
    pill2.setPosition(pad, pad + 66.f);
    pill2.setFillColor(sf::Color(0, 0, 0, 150));
    m_window.draw(pill2);

    // Enemy count
    const int alive = static_cast<int>(
        std::count_if(m_enemies.begin(), m_enemies.end(),
                      [](const Enemy& e){ return e.isActive(); }));
    sf::Text enemyLabel("ENEMIES", m_font, 22);
    enemyLabel.setFillColor(sf::Color(255, 120, 100));
    enemyLabel.setPosition(pad + 12.f, pad + 70.f);
    m_window.draw(enemyLabel);

    sf::Text enemyCount(std::to_string(alive), m_font, 30);
    enemyCount.setFillColor(sf::Color::White);
    enemyCount.setOutlineColor(sf::Color(0, 0, 0, 180));
    enemyCount.setOutlineThickness(2.f);
    enemyCount.setPosition(pad + 12.f, pad + 88.f);
    m_window.draw(enemyCount);

    // Coin count pill background
    sf::RectangleShape pill3({270.f, 56.f});
    pill3.setPosition(pad, pad + 132.f);
    pill3.setFillColor(sf::Color(0, 0, 0, 150));
    m_window.draw(pill3);

    // Coin icon (small yellow circle)
    sf::CircleShape coinIcon(10.f, 12);
    coinIcon.setOrigin(coinIcon.getRadius(), coinIcon.getRadius());
    coinIcon.setPosition(pad + 22.f, pad + 150.f);
    coinIcon.setFillColor(sf::Color(255, 210, 40));
    m_window.draw(coinIcon);

    sf::Text coinLabel("COINS", m_font, 22);
    coinLabel.setFillColor(sf::Color(255, 210, 40));
    coinLabel.setPosition(pad + 38.f, pad + 136.f);
    m_window.draw(coinLabel);

    sf::Text coinCountTxt(std::to_string(m_coinCount), m_font, 30);
    coinCountTxt.setFillColor(sf::Color::White);
    coinCountTxt.setOutlineColor(sf::Color(0, 0, 0, 180));
    coinCountTxt.setOutlineThickness(2.f);
    coinCountTxt.setPosition(pad + 38.f, pad + 154.f);
    m_window.draw(coinCountTxt);

    // Turbo mode indicator
    if (m_turboMode) {
        sf::RectangleShape turboPill({200.f, 44.f});
        turboPill.setPosition(pad, pad + 198.f);
        turboPill.setFillColor(sf::Color(200, 30, 30, 200));
        m_window.draw(turboPill);
        sf::Text turboTxt(">> TURBO <<", m_font, 26);
        turboTxt.setFillColor(sf::Color(255, 240, 60));
        turboTxt.setOutlineColor(sf::Color(80, 0, 0));
        turboTxt.setOutlineThickness(2.f);
        turboTxt.setPosition(pad + 10.f, pad + 204.f);
        m_window.draw(turboTxt);
    }
    sf::Text coinBest("BEST " + std::to_string(m_coinHighScore), m_font, 18);
    coinBest.setFillColor(sf::Color(255, 215, 0, 180));
    coinBest.setPosition(pad + 148.f, pad + 140.f);
    m_window.draw(coinBest);
}
// ── Persistent high score ──────────────────────────────────────────────────

static std::string highScorePath() {
    const char* home = std::getenv("HOME");
    return std::string(home ? home : ".") + "/.mario_highscore";
}

void Game::loadHighScore() {
    std::ifstream f(highScorePath());
    if (f.is_open()) {
        f >> m_highScore;
        if (!(f >> m_coinHighScore)) m_coinHighScore = 0;
    }
    if (m_highScore < 0) m_highScore = 0;
    if (m_coinHighScore < 0) m_coinHighScore = 0;
}

void Game::saveHighScore() {
    std::ofstream f(highScorePath());
    if (f.is_open()) f << m_highScore << '\n' << m_coinHighScore;
}