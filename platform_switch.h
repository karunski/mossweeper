#pragma once

#ifndef MINESWEEPER_PLATFORM_SWITCH_H
#define MINESWEEPER_PLATFORM_SWITCH_H

#ifdef PLATFORM_C64
#include "target_c64.h"
#endif

#ifdef PLATFORM_NES_NROM_128
#include "target_nes.h"
#endif


#endif
