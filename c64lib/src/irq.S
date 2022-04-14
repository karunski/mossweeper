.global irq_signal
irq_signal:
          .byte 0
.global vicii_interrupt_signal
vicii_interrupt_signal:
          .byte 0

.global my_irq
my_irq:   cld
          pha

          lda $d019        
          sta $d019        ; acknowledge IRQ
          sta vicii_interrupt_signal
          inc irq_signal

          pla

          jmp $ea81        ; return to kernel interrupt routine

.global char_data_ram
char_data_ram = 0xC800
.global sprite_data_ram
sprite_data_ram = 0xD000

;.org 0x2000
;char_data_ram:
;          .rept 0x0800
;          .byte 0
;          .endr
;.global sprite_data_ram
;sprite_data_ram:
;          .rept 0x0800
;          .byte 0
;          .endr
;.global sprite_data_end
;sprite_data_end:

.global pla
pla = 0x0000
