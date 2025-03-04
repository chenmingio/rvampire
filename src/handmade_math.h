#pragma once
#include "handmade_platform.h"
#include <math.h>

// union v2 {
//   struct {
//     r32 x, y;
//   };
//   r32 E[2];
// };

typedef struct v2 {
  r32 x, y;
} v2;

struct rectangle2 {
  v2 Min;
  v2 Max;
};

inline v2 V2i(i32 X, i32 Y) { return (v2){(r32)X, (r32)Y}; }

inline v2 V2i(u32 X, u32 Y) { return v2{(r32)X, (r32)Y}; }

inline v2 V2(r32 X, r32 Y) { return v2{X, Y}; }

inline v2 operator*(r32 A, v2 B) { return V2(A * B.x, A * B.y); }
inline v2 operator*(v2 B, r32 A) { return A * B; }

inline v2 operator+(v2 A, v2 B) { return V2(A.x + B.x, A.y + B.y); }
inline v2 operator-(v2 A, v2 B) { return V2(A.x - B.x, A.y - B.y); }

inline rectangle2 operator+(rectangle2 a, v2 b) {
  return rectangle2{a.Min + b, a.Max + b};
}

inline rectangle2 operator-(rectangle2 a, v2 b) {
  return rectangle2{a.Min - b, a.Max - b};
}

inline rectangle2 operator*(rectangle2 a, r32 s) {
  return rectangle2{a.Min * s, a.Max * s};
}

inline r32 squareRoot(r32 x) { return sqrtf(x); }

inline r32 square(r32 x) { return x * x; }

inline r32 scala(v2 v) { return squareRoot(v.x * v.x + v.y * v.y); }

inline v2 unitVector(v2 v) {
  r32 scalar = scala(v);
  if (scalar == 0) {
    return v2{0, 0};
  } else {
    return v * (1 / scalar);
  }
}