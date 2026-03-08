#include "render/ParallaxBackground.hpp"

#include <algorithm>
#include <cmath>

void ParallaxBackground::draw(sf::RenderTarget& target, const sf::View& worldView, const sf::FloatRect& worldBounds) const {
    const sf::Vector2f center = worldView.getCenter();
    const sf::Vector2f size = worldView.getSize();
    const float left = center.x - size.x * 0.5f;
    const float right = center.x + size.x * 0.5f;
    const float top = center.y - size.y * 0.5f;
    const float bottom = center.y + size.y * 0.5f;

    sf::VertexArray sky(sf::Quads, 4);
    sky[0].position = {left, top};
    sky[1].position = {right, top};
    sky[2].position = {right, bottom};
    sky[3].position = {left, bottom};
    sky[0].color = sf::Color(89, 188, 255);
    sky[1].color = sf::Color(89, 188, 255);
    sky[2].color = sf::Color(206, 241, 255);
    sky[3].color = sf::Color(206, 241, 255);
    target.draw(sky);

    sf::CircleShape sun(72.0f, 40);
    sun.setOrigin(sun.getRadius(), sun.getRadius());
    sun.setFillColor(sf::Color(255, 246, 187, 220));
    sun.setPosition(left + size.x * 0.74f + center.x * 0.03f, top + size.y * 0.18f);
    target.draw(sun);

    auto drawCloudLayer = [&](float parallax, float y, const sf::Color& color) {
        const float spacing = 360.0f;
        const float offset = center.x * parallax;
        const int start = static_cast<int>(std::floor((left + offset - spacing) / spacing));

        for (int i = 0; i < 14; ++i) {
            const float baseX = (start + i) * spacing - offset;

            sf::CircleShape puffA(30.0f, 20);
            sf::CircleShape puffB(38.0f, 20);
            sf::CircleShape puffC(25.0f, 20);

            puffA.setFillColor(color);
            puffB.setFillColor(color);
            puffC.setFillColor(color);

            puffA.setPosition(baseX, y);
            puffB.setPosition(baseX + 34.0f, y - 14.0f);
            puffC.setPosition(baseX + 70.0f, y + 4.0f);

            target.draw(puffA);
            target.draw(puffB);
            target.draw(puffC);
        }
    };

    drawCloudLayer(0.12f, top + size.y * 0.22f, sf::Color(255, 255, 255, 180));
    drawCloudLayer(0.2f, top + size.y * 0.32f, sf::Color(255, 255, 255, 145));

    auto drawHillLayer = [&](float parallax, float baseline, float radius, const sf::Color& color) {
        const float spacing = radius * 1.7f;
        const float offset = center.x * parallax;
        const int start = static_cast<int>(std::floor((left + offset - spacing) / spacing));

        sf::CircleShape hill(radius, 48);
        hill.setOrigin(radius, radius);
        hill.setFillColor(color);

        for (int i = 0; i < 16; ++i) {
            const float x = (start + i) * spacing - offset;
            hill.setPosition(x, baseline);
            target.draw(hill);
        }
    };

    const float horizon = std::max(worldBounds.top + worldBounds.height * 0.55f, top + size.y * 0.58f);
    drawHillLayer(0.24f, horizon + 80.0f, 190.0f, sf::Color(116, 189, 157));
    drawHillLayer(0.38f, horizon + 130.0f, 160.0f, sf::Color(95, 165, 126));

    sf::RectangleShape farBand({size.x + 500.0f, bottom - horizon + 260.0f});
    farBand.setPosition(left - 250.0f, horizon + 120.0f);
    farBand.setFillColor(sf::Color(77, 145, 103));
    target.draw(farBand);
}
