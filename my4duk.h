//#my4duk.h
#pragma once

#define DEBUG_PRINTF
#ifdef DEBUG_PRINTF
#define debugPrintf(fmt, ...) {  Serial.printf("%d %s # ", __LINE__, __PRETTY_FUNCTION__); Serial.printf_P( (PGM_P)PSTR(fmt), ## __VA_ARGS__ ); }
#else
#define debugPrintf
#endif

#include "my4duk_gate.h"
#include "my4duk_udp.h"
#include "my4duk_device.h"

//#include "my4duk_parcer.h"


