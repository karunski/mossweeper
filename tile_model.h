#pragma once 

#ifndef TILE_MODEL_H
#define TILE_MODEL_H

#include <cstdint>

template <class TileType, std::uint8_t Width, std::uint8_t Height>
struct MetaTile {
  static constexpr std::uint8_t width = Width;
  static constexpr std::uint8_t height = Height;

  using RowType = TileType[Width];

  const RowType tiles[Height];
};

struct TilePoint {

  TilePoint left(std::uint8_t distance) const {
    return TilePoint{static_cast<std::uint8_t>(X - distance), Y};
  }

  TilePoint left() const {
    return TilePoint{static_cast<std::uint8_t>(X - 1), Y};
  }

  TilePoint right(std::uint8_t distance) const {
    return TilePoint{static_cast<std::uint8_t>(X + distance), Y};
  }

  TilePoint right() const {
    return TilePoint{static_cast<std::uint8_t>(X + 1), Y};
  }

  TilePoint up(std::uint8_t distance) const {
    return TilePoint{X, static_cast<std::uint8_t>(Y - distance)};
  }

  TilePoint up() const {
    return TilePoint{X, static_cast<std::uint8_t>(Y - 1)};
  }

  TilePoint down(std::uint8_t distance) const {
    return TilePoint{X, static_cast<std::uint8_t>(Y + distance)};
  }

  TilePoint down() const {
    return TilePoint{X, static_cast<std::uint8_t>(Y + 1)};
  }

  bool operator==(const TilePoint &rhs) const {
    return X == rhs.X && Y == rhs.Y;
  }

  std::uint8_t X;
  std::uint8_t Y;
};

#endif
