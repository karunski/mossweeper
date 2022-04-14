.section .rodata
.global minesweeper_gfx
minesweeper_gfx:
.include "minesweeper-charset.s"

;.global minesweeper_map
;minesweeper_map:
;.include "minesweeper-map.S"

.global minesweeper_color
minesweeper_color:
.include "minesweeper-colors.S"

.global minesweeper_cursor
minesweeper_cursor:
.include "cursor.S"

.global minesweeper_bg_sprites
minesweeper_bg_sprites:
.include "bg-sprites.S"
