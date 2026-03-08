#pragma once

#include <SFML/Graphics.hpp>

#include "entities/Enemy.hpp"
#include "entities/ParticleSystem.hpp"
#include "entities/Player.hpp"
#include "entities/Box.hpp"
#include "entities/Coin.hpp"
#include "render/CameraController.hpp"
#include "render/LightingPass.hpp"
#include "render/ParallaxBackground.hpp"
#include "world/TileMap.hpp"

#include <vector>

enum class GameState { Menu, Playing, GameOver };

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void renderMenu();
    void renderGameOver();
    void resetGame();
    void spawnEnemies();
    void spawnBoxes();
    void drawHUD();
    void loadHighScore();
    void saveHighScore();

    sf::RenderWindow m_window;
    TileMap m_map;
    Player m_player;
    ParticleSystem m_particles;
    CameraController m_camera;
    ParallaxBackground m_background;
    LightingPass m_lighting;
    std::vector<Enemy> m_enemies;
    std::vector<Box>   m_boxes;
    std::vector<Coin>  m_coins;

    GameState m_state = GameState::Menu;
    sf::Font m_font;
    bool m_fontLoaded = false;
    sf::Vector2f m_spawnPosition;

    int   m_score          = 0;
    int   m_highScore      = 0;
    int   m_coinCount      = 0;
    int   m_coinHighScore  = 0;
    float m_timePlayed     = 0.0f;  // seconds alive this run — grants bonus points
    float m_bonusTimer     = 0.0f;  // counts toward next survival bonus
    float m_respawnTimer   = 0.0f;
    bool  m_prevJumpHeld   = false;
    bool  m_turboMode      = false;
    float m_accumulator    = 0.0f;
    sf::Clock m_frameClock;
};
