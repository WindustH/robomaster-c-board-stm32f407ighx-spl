#include "led.h"
#include "setup.h"


int main(void) {
    setup_led();
    setup_schedule();
    setup_uart();

    //  main loop
    while (true) {
    }
}
