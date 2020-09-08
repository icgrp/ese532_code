# 3D Rendering

## Description
This design renders 2D images from 3D models (3D triangle mesh). It is composed of the following stages.
1. Projection: 3D triangle -> 2D triangle
2. Rasterization (1, 2): search pixels in 2D triangle within the bounding box
3. Z-culling: hide or display pixels according to each pixel's "z" value (depth)
4. ColoringFB: coloring framebuffer according to the zbuffer


