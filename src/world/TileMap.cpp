#include "world/TileMap.hpp"

#include <algorithm>
#include <cmath>

TileMap::TileMap(std::vector<std::string> rows, int tileSize) : m_rows(std::move(rows)), m_tileSize(tileSize) {
    std::size_t maxWidth = 0;
    for (const auto& row : m_rows) {
        maxWidth = std::max(maxWidth, row.size());
    }

    for (auto& row : m_rows) {
        if (row.size() < maxWidth) {
            row.append(maxWidth - row.size(), '.');
        }
    }
}

bool TileMap::isSolidTile(int tileX, int tileY) const {
    if (tileY < 0 || tileY >= heightInTiles() || tileX < 0 || tileX >= widthInTiles()) {
        return false;
    }
    return m_rows[static_cast<std::size_t>(tileY)][static_cast<std::size_t>(tileX)] == '#';
}

std::vector<sf::FloatRect> TileMap::querySolidTiles(const sf::FloatRect& area) const {
    std::vector<sf::FloatRect> tiles;

    const int minX = std::max(0, static_cast<int>(std::floor(area.left / static_cast<float>(m_tileSize))) - 1);
    const int maxX = std::min(widthInTiles() - 1,
                              static_cast<int>(std::floor((area.left + area.width) / static_cast<float>(m_tileSize))) + 1);
    const int minY = std::max(0, static_cast<int>(std::floor(area.top / static_cast<float>(m_tileSize))) - 1);
    const int maxY = std::min(heightInTiles() - 1,
                              static_cast<int>(std::floor((area.top + area.height) / static_cast<float>(m_tileSize))) + 1);

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            if (!isSolidTile(x, y)) {
                continue;
            }
            tiles.emplace_back(static_cast<float>(x * m_tileSize), static_cast<float>(y * m_tileSize),
                               static_cast<float>(m_tileSize), static_cast<float>(m_tileSize));
        }
    }

    return tiles;
}

void TileMap::draw(sf::RenderTarget& target, const sf::View& camera) const {
    const sf::Vector2f center = camera.getCenter();
    const sf::Vector2f size = camera.getSize();
    const sf::Vector2f topLeft = center - (size * 0.5f);

    const int startX = std::max(0, static_cast<int>(std::floor(topLeft.x / static_cast<float>(m_tileSize))) - 1);
    const int endX = std::min(widthInTiles() - 1,
                              static_cast<int>(std::floor((topLeft.x + size.x) / static_cast<float>(m_tileSize))) + 1);
    const int startY = std::max(0, static_cast<int>(std::floor(topLeft.y / static_cast<float>(m_tileSize))) - 1);
    const int endY = std::min(heightInTiles() - 1,
                              static_cast<int>(std::floor((topLeft.y + size.y) / static_cast<float>(m_tileSize))) + 1);

    sf::RectangleShape tileShape({static_cast<float>(m_tileSize), static_cast<float>(m_tileSize)});
    sf::RectangleShape topStrip({static_cast<float>(m_tileSize), 10.0f});
    sf::RectangleShape shadowStrip({8.0f, static_cast<float>(m_tileSize)});

    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            if (!isSolidTile(x, y)) {
                continue;
            }

            const bool topExposed = !isSolidTile(x, y - 1);
            const bool rightExposed = !isSolidTile(x + 1, y);

            tileShape.setPosition(static_cast<float>(x * m_tileSize), static_cast<float>(y * m_tileSize));
            tileShape.setFillColor(topExposed ? m_grassColor : m_dirtColor);
            target.draw(tileShape);

            if (topExposed) {
                topStrip.setPosition(tileShape.getPosition());
                topStrip.setFillColor(sf::Color(159, 214, 91));
                target.draw(topStrip);
            }

            if (rightExposed) {
                shadowStrip.setPosition(tileShape.getPosition().x + static_cast<float>(m_tileSize - 8),
                                        tileShape.getPosition().y);
                shadowStrip.setFillColor(m_shadowColor);
                target.draw(shadowStrip);
            }
        }
    }
}

int TileMap::pixelWidth() const { return widthInTiles() * m_tileSize; }

int TileMap::pixelHeight() const { return heightInTiles() * m_tileSize; }

int TileMap::tileSize() const { return m_tileSize; }

const std::vector<std::string>& TileMap::rows() const { return m_rows; }

int TileMap::widthInTiles() const { return m_rows.empty() ? 0 : static_cast<int>(m_rows.front().size()); }

int TileMap::heightInTiles() const { return static_cast<int>(m_rows.size()); }
