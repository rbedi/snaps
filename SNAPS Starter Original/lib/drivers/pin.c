#include <stm32f4xx.h>
#include <pin.h>

// Atomic set pin high
void Pin_SetHigh(Pin* pin)
{
	pin->port->BSRRL |= pin->pin;
}

// Atomic set pin low
void Pin_SetLow(Pin* pin)
{
    pin->port->BSRRH |= pin->pin;
}

// Non-atomic toggle pin
void Pin_Toggle(Pin* pin)
{
    if(pin->port->ODR &= pin->pin)
        Pin_SetLow(pin);
    else
        Pin_SetHigh(pin);
}