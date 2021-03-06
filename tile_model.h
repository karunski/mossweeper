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

template<class T>
struct Height
{
  static constexpr std::uint8_t value = 1;
};

template<class TileType, std::uint8_t w, std::uint8_t h>
struct Height<MetaTile<TileType, w, h>>
{
  static constexpr auto value = h;
};

template <class T> struct Width { static constexpr std::uint8_t value = 1; };

template <class TileType, std::uint8_t w, std::uint8_t h>
struct Width<MetaTile<TileType, w, h>> {
  static constexpr auto value = w;
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

template <std::uint8_t... Is> struct index_sequence {};
template <std::uint8_t N, std::uint8_t... Is>
struct generate_index_sequence : generate_index_sequence<N - 1, N - 1, Is...> {
};
template <std::uint8_t... Is>
struct generate_index_sequence<0, Is...> : index_sequence<Is...> {};

template <class tile_type, std::uint8_t len> struct TilePattern {
  tile_type m_data[len] ;
};

template <class tile_type, class transform, std::uint8_t len, std::uint8_t... indexes>
constexpr auto transform_string_impl(const char (&str)[len],
                                     const index_sequence<indexes...> &) {
  return TilePattern<tile_type, len - 1> { transform::call(str[indexes])... };
}

template <class tile_type, class transform, std::uint8_t len>
constexpr auto transform_string(const char (&str)[len]) {
  return transform_string_impl<tile_type, transform, len>(str, generate_index_sequence<len - 1>{});
}

#endif
