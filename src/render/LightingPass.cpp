#include "render/LightingPass.hpp"

#include <algorithm>

bool LightingPass::initialize(const sf::Vector2u& size) {
    if (!m_sceneTexture.create(size.x, size.y)) {
        return false;
    }

    m_shaderLoaded = m_shader.loadFromFile("assets/shaders/lighting.frag", sf::Shader::Fragment);
    return true;
}

void LightingPass::resize(const sf::Vector2u& size) {
    if (size.x == 0 || size.y == 0) {
        return;
    }
    m_sceneTexture.create(size.x, size.y);
}

sf::RenderTarget& LightingPass::beginScene() {
    m_sceneTexture.clear(sf::Color(32, 45, 74));
    return m_sceneTexture;
}

void LightingPass::endScene(sf::RenderWindow& window, const sf::Vector2f& lightWorldPosition, const sf::View& worldView) {
    m_sceneTexture.display();
    m_time += 1.0f / 60.0f;

    const sf::Vector2f topLeft = worldView.getCenter() - (worldView.getSize() * 0.5f);
    sf::Vector2f lightUV((lightWorldPosition.x - topLeft.x) / worldView.getSize().x,
                         (lightWorldPosition.y - topLeft.y) / worldView.getSize().y);
    lightUV.x = std::clamp(lightUV.x, 0.0f, 1.0f);
    lightUV.y = std::clamp(lightUV.y, 0.0f, 1.0f);

    sf::Sprite sceneSprite(m_sceneTexture.getTexture());

    const sf::View previousView = window.getView();
    window.setView(window.getDefaultView());

    if (m_shaderLoaded) {
        m_shader.setUniform("u_lightPos", lightUV);
        m_shader.setUniform("u_time", m_time);
        window.draw(sceneSprite, &m_shader);
    } else {
        window.draw(sceneSprite);
    }

    window.setView(previousView);
}
