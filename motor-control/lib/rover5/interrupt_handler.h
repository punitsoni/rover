#pragma once

typedef void (*interrupt_cb)(void *arg);

const int MAX_INTERRUPT_PINS 4;

class interrupt_handler()
{
public:
    interrupt_handler();
    void attach(uint8_t pin, interrupt_cb cb, uint8_t mode);
private:
    void *args[MAX_INTERRUPT_PINS];
}
