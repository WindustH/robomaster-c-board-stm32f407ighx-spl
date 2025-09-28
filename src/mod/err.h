#pragma once
#include "type.h"

typedef struct {
  void (*setup)();
} _ErrMod;

extern const _ErrMod _err;