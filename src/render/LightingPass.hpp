#pragma once

#include <SFML/Graphics.hpp>

class LightingPass {
public:
    bool initialize(const sf::Vector2u& size);
    void resize(const sf::Vector2u& size);

    sf::RenderTarget& beginScene();
    void endScene(sf::RenderWindow& window, const sf::Vector2f& lightWorldPosition, const sf::View& worldView);

private:
    sf::RenderTexture m_sceneTexture;
    sf::Shader m_shader;
    bool m_shaderLoaded = false;
    float m_time = 0.0f;
};
