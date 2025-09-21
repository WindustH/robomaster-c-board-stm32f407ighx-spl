#include "modu.h"
u32 modu_distance(const u32 a, const u32 b, const u32 m) {
  if (a == b)
    return 0;
  if (a < b)
    return b - a;
  return m + b - a;
}
u32v4 modu_span(const u32 a, const u32 b, const u32 m) {
  u32v4 span;
  if (a == b) {
    span.x = 0;
    span.y = 0;
    span.u = 0;
    span.v = 0;
    return span;
  }
  if (a < b) {
    span.x = a;
    span.y = b;
    span.u = 0;
    span.v = 0;
    return span;
  }
  span.x = a;
  span.y = m;
  span.u = 0;
  span.v = b;
  return span;
}