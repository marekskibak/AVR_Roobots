/*
 * makra.h
 *
 *  Created on: 2011-11-20
 *      Author: Skiba Krzysztof
 */

// Makra upraszczaj�ce dost�p do port�w
// *** PORT
#define PORT(x) XPORT(x)
#define XPORT(x) (PORT##x)
// *** PIN
#define PIN(x) XPIN(x)
#define XPIN(x) (PIN##x)
// *** DDR
#define DDR(x) XDDR(x)
#define XDDR(x) (DDR##x)
