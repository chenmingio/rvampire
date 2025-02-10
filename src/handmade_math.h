#include "handmade_platform.h"

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

inline rectangle2 operator+(rectangle2 A, v2 B) {
  return rectangle2{A.Min + B, A.Max + B};
}