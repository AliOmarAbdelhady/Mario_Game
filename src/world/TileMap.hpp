#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

class TileMap {
public:
    explicit TileMap(std::vector<std::string> rows, int tileSize);

    bool isSolidTile(int tileX, int tileY) const;
    std::vector<sf::FloatRect> querySolidTiles(const sf::FloatRect& area) const;
    void draw(sf::RenderTarget& target, const sf::View& camera) const;

    int pixelWidth() const;
    int pixelHeight() const;
    int tileSize() const;
    const std::vector<std::string>& rows() const;

private:
    int widthInTiles() const;
    int heightInTiles() const;

    std::vector<std::string> m_rows;
    int m_tileSize = 64;
    sf::Color m_dirtColor = sf::Color(128, 80, 52);
    sf::Color m_grassColor = sf::Color(112, 170, 74);
    sf::Color m_shadowColor = sf::Color(82, 48, 30);
};
