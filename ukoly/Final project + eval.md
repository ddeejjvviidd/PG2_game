# Create a real-time 3D application

The application should meet following requirements. The final score sets the exam grade. Maximum of two people per project.

START = 100 points

Send full project and installation procedures in advance (use gitlab.tul.cz, github, gitlab, etc.)

## ESSENTIALS: Each missing (non-functional) Essential = -25 points (partial functionality => partial decrement)

- [ ]  3D GL Core profile + shaders version 4.6, GL debug enabled, JSON config file
- [x]  high performance => at least 60 FPS (display FPS)
- [x]  allow VSync control, antialiasing, fullscreen vs. windowed switching (restore window position & size)
- [x]  event processing (camera, object, app behaviour...): mouse (both axes, wheel), keyboard
- [ ]  multiple different independently moving 3D models, at leats two loaded from file
- [x]  at least three different textures (or subtextures from texture atlas etc.)
- [x]  lighting model, all basic lights types (1x ambient, min. 1x directional, min. 2x point, min. 1x reflector; at least two are moving)
- [x]  correct full alpha scale transparency (at least two transparent objects; NOT if(alpha<0.1) {discard;} )
- [ ]  correct collisions

## EXTRAS: Each working Extra = +10 points

- [ ]  height map textured by height + proper player height coords
- [ ]  audio (better than just background)
- [ ]  particle effects
- [ ]  scripting (useful)
- [ ]  some other nice complicated effect...

## INSTAFAIL (reject)

Obsolete functionality used: GLUT, GL compatible profile, no DSA (direct state access)

**NOTE:** Hardware limitation might apply (eg. no mouse wheel on notebook, MAC ~ GL 4.1 etc.), in that case the subtask can be ignored.

|   Grade   |   Point range ||
|-----------|:-----:|:-----:|
| A = "1"   |   91  |   100 |
| B = "1-"  |   81  |   90  |
| C = "2"   |   71  |   80  |
| D = "2-"  |   61  |   70  |
| E = "3"   |   51  |   60  |
| F = "4"   |   0   |   50  |
