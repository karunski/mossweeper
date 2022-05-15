#pragma once

#ifndef INPUT_MODEL_H
#define INPUT_MODEL_H

struct key_scan_res {
  bool space : 1; // Or "fire" or  "button A"
  bool w : 1;     // Or "Up"
  bool a : 1;     // Or "Left"
  bool s : 1;     // Or "Down"
  bool d : 1;     // Or "Right"
};

#endif
