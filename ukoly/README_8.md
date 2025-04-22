# Lighting

## Task 1: Implement most simple light source - directional light (sun)

- implement Phong lighting model for directional light
  - start with shaders: create static light source with hardcoded settings - create light model parameters as __uniforms with default value__. This will later allow you to override them from CPU side (C++).
  - if it works for default values, go to C++ and create data structures for light parameters. Modify params in time (moving sun, sun changing color, etc.) and update uniforms dynamically.

## Task 2: Implement at least 3 different point lights

Create at least 3 point lights with different parameters.

## Task 3: Implement at least one spot light

Implement spot light = reflector light source, for example as a light attached to the camera (headlight).
