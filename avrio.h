#ifndef __AVRIO_H_
#define __AVRIO_H_

// based on http://dostmuhammad.com/blog/easyavr-h-header-file-for-pinport-operations/

#define PORT_(port) PORT ## port
#define DDR_(port)  DDR  ## port
#define PIN_(port)  PIN  ## port

#define PORT(port) PORT_(port)
#define DDR(port)  DDR_(port)
#define PIN(port)  PIN_(port)

#define PIN_ON_AUX(pn, pin) ((PORT(pn)) |= (1 << (pin)))
#define PIN_ON(io) PIN_ON_AUX(io)

#define PIN_OFF_AUX(pn, pin) ((PORT(pn)) &= ~(1 << (pin)))
#define PIN_OFF(io) PIN_OFF_AUX(io)

#define PIN_SET(io, val) ((!!(val)) ? PIN_ON_AUX(io) : PIN_OFF_AUX(io))

#define PORT_AS_OUTPUT(pn) (DDR(pn) = 0xFF)
#define PORT_AS_INPUT(pn) (DDR(pn) = 0x00)

#define PIN_AS_OUTPUT_AUX(pn, pin) (DDR(pn) |= (1 << (pin)))
#define PIN_AS_OUTPUT(io) PIN_AS_OUTPUT_AUX(io)

#define PIN_AS_INPUT_AUX(pn, pin) (DDR(pn) &= ~(1 << (pin)))
#define PIN_AS_INPUT(io) PIN_AS_INPUT_AUX(io)

#define PIN_READ_AUX(pn, pin) ((PIN(pn) & (1 << (pin))) != 0)
#define PIN_READ(io) PIN_READ_AUX(io)

#endif
