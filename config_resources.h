#pragma once

#ifndef CONFIG_RESOURCES_H
#define CONFIG_RESOURCES_H

#include <c64.h>



struct Sprites
{
  c64::ColorCode SpriteColor;
  c64::ColorCode MultiColor1;
  c64::ColorCode MultiColor2;
  c64::Sprite    sprites[16];
};

extern const unsigned char sprites_spr[];
extern const unsigned int sprites_spr_len;

inline const Sprites & address_sprites = *reinterpret_cast<const Sprites *>(sprites_spr);
inline const std::uint8_t sprite_count = (sprites_spr_len - 3) / sizeof(c64::Sprite);
#endif
