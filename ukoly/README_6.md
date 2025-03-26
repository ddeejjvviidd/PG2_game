# Textures

## Task 1: Explore the source code of the GL demonstration

- implement: loading texture from file
- do __NOT__ use stbi_image library! We already have OpenCV...

## Task 2: Display textured object

- use example to extend functionality of Model+Mesh class
- use shader with texture support
- load OBJ model with texture coordinates
- ...enjoy

## Task 3: generate level

- maze from cubes with different textures
- textured heightmap
- optional: combine both (use heightmap as a start; set some area to fixed level; place the maze on fixed level)

## Task 4 (OPTIONAL): Display texture using ImGUI

- use texture with ImGUI (<https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#example-for-opengl-users>)
  - load as in Task 1

  ``` C++
  GLuint mytex = textureInit("resources/my_favourite_texture.jpg");
  ```

  - display

  ``` C++
  // get texture size (this needs to be done just once, we use immutable format)
  int my_image_width = 0;
  int my_image_height = 0;
  int miplevel = 0;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &my_image_width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &my_image_height); 

  // show texture   
  ImGui::Begin("OpenGL Texture");
  ImGui::Image((ImTextureID)(intptr_t)my_tex, ImVec2(my_image_width, my_image_height));
  ImGui::End();
  ```
