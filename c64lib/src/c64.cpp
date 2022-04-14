#include "c64.h"
#include "cia.h"

extern "C" volatile std::uint8_t irq_signal;
extern "C" volatile c64::VicIIInterruptStatus vicii_interrupt_signal;

namespace c64
{
  volatile std::byte dead_store;

  void VsyncWaitFunc::operator()() const
  {
    irq_signal = 0;

    while (true) {
      if (irq_signal) {
        irq_signal = 0;

        if (vicii_interrupt_signal.raster()) {
          return;
        }
      }
    }
  }

  VsyncWaitFunc get_vsync_wait() {
    ScopedInterruptDisable sei;
    cia1.interrupt_control(CIA::InterruptSet{CIA::InterruptSet::CLEAR, true,
                                             true, true, true, true});
    cia2.interrupt_control(CIA::InterruptSet{CIA::InterruptSet::CLEAR, true,
                                             true, true, true, true});
    auto status = cia1.interrupt_status();
    status = cia2.interrupt_status();

    vic_ii.enable_interrupts(VicII::InterruptSet{false, false, false, true});

    vic_ii.set_raster(0);

    typedef void (*isr_func)();
    auto &isr_vec = *reinterpret_cast<isr_func *>(0x314);
    isr_vec = my_irq;

    return VsyncWaitFunc{};
  }
}
