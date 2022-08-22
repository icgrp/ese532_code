/*===============================================================*/
/*                                                               */
/*                      rendering_sw.cpp                         */
/*                                                               */
/*              Software version for 3D Rendering                */
/*                                                               */
/*===============================================================*/

#include "rendering_sw.h"

/*======================UTILITY FUNCTIONS========================*/
#include <cstdint>
#include <iostream>
#include "stopwatch.h"

// Determine whether three vertices of a trianlgLe
// (x0,y0) (x1,y1) (x2,y2) are in clockwise order by Pineda algorithm
// if so, return a number > 0
// else if three points are in line, return a number == 0
// else in counterclockwise order, return a number < 0
int check_clockwise( Triangle_2D triangle_2d )
{
  int cw;

  cw = (triangle_2d.x2 - triangle_2d.x0) * (triangle_2d.y1 - triangle_2d.y0)
       - (triangle_2d.y2 - triangle_2d.y0) * (triangle_2d.x1 - triangle_2d.x0);

  return cw;

}

// swap (x0, y0) (x1, y1) of a Triangle_2D
void clockwise_vertices( Triangle_2D *triangle_2d )
{

  bit8 tmp_x, tmp_y;

  tmp_x = triangle_2d->x0;
  tmp_y = triangle_2d->y0;

  triangle_2d->x0 = triangle_2d->x1;
  triangle_2d->y0 = triangle_2d->y1;

  triangle_2d->x1 = tmp_x;
  triangle_2d->y1 = tmp_y;

}


// Given a pixel, determine whether it is inside the triangle
// by Pineda algorithm
// if so, return true
// else, return false
bool pixel_in_triangle( bit8 x, bit8 y, Triangle_2D triangle_2d )
{

  int pi0, pi1, pi2;

  pi0 = (x - triangle_2d.x0) * (triangle_2d.y1 - triangle_2d.y0) - (y - triangle_2d.y0) * (triangle_2d.x1 - triangle_2d.x0);
  pi1 = (x - triangle_2d.x1) * (triangle_2d.y2 - triangle_2d.y1) - (y - triangle_2d.y1) * (triangle_2d.x2 - triangle_2d.x1);
  pi2 = (x - triangle_2d.x2) * (triangle_2d.y0 - triangle_2d.y2) - (y - triangle_2d.y2) * (triangle_2d.x0 - triangle_2d.x2);

  return (pi0 >= 0 && pi1 >= 0 && pi2 >= 0);
}

// find the min from 3 integers
bit8 find_min( bit8 in0, bit8 in1, bit8 in2 )
{
  if (in0 < in1)
  {
    if (in0 < in2)
      return in0;
    else 
      return in2;
  }
  else 
  {
    if (in1 < in2) 
      return in1;
    else 
      return in2;
  }
}


// find the max from 3 integers
bit8 find_max( bit8 in0, bit8 in1, bit8 in2 )
{
  if (in0 > in1)
  {
    if (in0 > in2)
      return in0;
    else 
      return in2;
  }
  else 
  {
    if (in1 > in2) 
      return in1;
    else 
      return in2;
  }
}

/*======================PROCESSING STAGES========================*/

// project a 3D triangle to a 2D triangle
void projection ( Triangle_3D triangle_3d, Triangle_2D *triangle_2d, int angle )
{
  // Setting camera to (0,0,-1), the canvas at z=0 plane
  // The 3D model lies in z>0 space
  // The coordinate on canvas is proportional to the corresponding coordinate 
  // on space
  if(angle == 0)
  {
    triangle_2d->x0 = triangle_3d.x0;
    triangle_2d->y0 = triangle_3d.y0;
    triangle_2d->x1 = triangle_3d.x1;
    triangle_2d->y1 = triangle_3d.y1;
    triangle_2d->x2 = triangle_3d.x2;
    triangle_2d->y2 = triangle_3d.y2;
    triangle_2d->z  = triangle_3d.z0 / 3 + triangle_3d.z1 / 3 + triangle_3d.z2 / 3;
  }

  else if(angle == 1)
  {
    triangle_2d->x0 = triangle_3d.x0;
    triangle_2d->y0 = triangle_3d.z0;
    triangle_2d->x1 = triangle_3d.x1;
    triangle_2d->y1 = triangle_3d.z1;
    triangle_2d->x2 = triangle_3d.x2;
    triangle_2d->y2 = triangle_3d.z2;
    triangle_2d->z  = triangle_3d.y0 / 3 + triangle_3d.y1 / 3 + triangle_3d.y2 / 3;
  }
      
  else if(angle == 2)
  {
    triangle_2d->x0 = triangle_3d.z0;
    triangle_2d->y0 = triangle_3d.y0;
    triangle_2d->x1 = triangle_3d.z1;
    triangle_2d->y1 = triangle_3d.y1;
    triangle_2d->x2 = triangle_3d.z2;
    triangle_2d->y2 = triangle_3d.y2;
    triangle_2d->z  = triangle_3d.x0 / 3 + triangle_3d.x1 / 3 + triangle_3d.x2 / 3;
  }
}

// calculate bounding box for a 2D triangle
bool rasterization1 ( Triangle_2D triangle_2d, bit8 max_min[], int max_index[])
{
  // clockwise the vertices of input 2d triangle
  if ( check_clockwise( triangle_2d ) == 0 )
    return 1;
  if ( check_clockwise( triangle_2d ) < 0 )
    clockwise_vertices( &triangle_2d );

  // find the rectangle bounds of 2D triangles
  max_min[0] = find_min( triangle_2d.x0, triangle_2d.x1, triangle_2d.x2 );
  max_min[1] = find_max( triangle_2d.x0, triangle_2d.x1, triangle_2d.x2 );
  max_min[2] = find_min( triangle_2d.y0, triangle_2d.y1, triangle_2d.y2 );
  max_min[3] = find_max( triangle_2d.y0, triangle_2d.y1, triangle_2d.y2 );
  max_min[4] = max_min[1] - max_min[0];

  // calculate index for searching pixels
  max_index[0] = (max_min[1] - max_min[0]) * (max_min[3] - max_min[2]);

  return 0;
}

// find pixels in the triangles from the bounding box
int rasterization2 ( bool flag, bit8 max_min[], int max_index[], Triangle_2D triangle_2d, CandidatePixel fragment[] )
{
  // clockwise the vertices of input 2d triangle
  if ( flag )
  {
    return 0;
  }

  bit8 color = 100;
  int i = 0;

  RAST2: for ( int k = 0; k < max_index[0]; k ++ )
  {
    bit8 x = max_min[0] + k % max_min[4];
    bit8 y = max_min[2] + k / max_min[4];

    if( pixel_in_triangle( x, y, triangle_2d ) )
    {
      fragment[i].x = x;
      fragment[i].y = y;
      fragment[i].z = triangle_2d.z;
      fragment[i].color = color;
      i++;
    }
  }

  return i;
}

// filter hidden pixels
int zculling ( int counter, CandidatePixel fragments[], int size, Pixel pixels[])
{

  // initilize the z-buffer in rendering first triangle for an image
  static bit8 z_buffer[MAX_X][MAX_Y];
  if (counter == 0)
  {
    ZCULLING_INIT_ROW: for ( int i = 0; i < MAX_X; i ++ )
    {
      ZCULLING_INIT_COL: for ( int j = 0; j < MAX_Y; j ++ )
      {
        z_buffer[i][j] = 255;
      }
    }
  }

  // pixel counter
  int pixel_cntr = 0;
  
  // update z-buffer and pixels
  ZCULLING: for ( int n = 0; n < size; n ++ ) 
  {
    if( fragments[n].z < z_buffer[fragments[n].y][fragments[n].x] )
    {
      pixels[pixel_cntr].x     = fragments[n].x;
      pixels[pixel_cntr].y     = fragments[n].y;
      pixels[pixel_cntr].color = fragments[n].color;
      pixel_cntr++;
      z_buffer[fragments[n].y][fragments[n].x] = fragments[n].z;
    }
  }

  return pixel_cntr;
}

// color the frame buffer
void coloringFB(int counter, int size_pixels, Pixel pixels[], bit8 frame_buffer[MAX_X][MAX_Y])
{

  if ( counter == 0 )
  {
    // initilize the framebuffer for a new image
    COLORING_FB_INIT_ROW: for ( int i = 0; i < MAX_X; i ++ )
    {
      COLORING_FB_INIT_COL: for ( int j = 0; j < MAX_Y; j ++ )
        frame_buffer[i][j] = 0;
    }
  }

  // update the framebuffer
  COLORING_FB: for ( int i = 0; i < size_pixels; i ++ )
    frame_buffer[ pixels[i].x ][ pixels[i].y ] = pixels[i].color;

}

/*========================TOP FUNCTION===========================*/
void rendering_sw_with_timer( Triangle_3D triangle_3ds[NUM_3D_TRI], bit8 output[MAX_X][MAX_Y])
{
  // local variables

  // 2D triangle
  Triangle_2D triangle_2ds;
  // projection angle
  int angle = 0;

  // max-min index arrays
  bit8 max_min[5];
  int max_index[1];

  // fragments
  CandidatePixel fragment[500];

  // pixel buffer
  Pixel pixels[500];

  // processing NUM_3D_TRI 3D triangles
  stopwatch time_projection;
  stopwatch time_rasterization1;
  stopwatch time_rasterization2;
  stopwatch time_zculling;
  stopwatch time_coloringFB;
  stopwatch total_time;

  // processing NUM_3D_TRI 3D triangles
  TRIANGLES: for (int i = 0; i < NUM_3D_TRI; i ++ )
  {
    total_time.start();

    // five stages for processing each 3D triangle
    time_projection.start();
    projection( triangle_3ds[i], &triangle_2ds, angle );
    time_projection.stop();

    time_rasterization1.start();
    bool flag = rasterization1(triangle_2ds, max_min, max_index);
    time_rasterization1.stop();

    time_rasterization2.start();
    int size_fragment = rasterization2( flag, max_min, max_index, triangle_2ds, fragment );
    time_rasterization2.stop();

    time_zculling.start();
    int size_pixels = zculling( i, fragment, size_fragment, pixels);
    time_zculling.stop();

    time_coloringFB.start();
    coloringFB ( i, size_pixels, pixels, output);
    time_coloringFB.stop();

    total_time.stop();
  }
  std::cout << "Total latency of projection is: " << time_projection.latency() << " ns." << std::endl;
  std::cout << "Total latency of rasterization1 is: " << time_rasterization1.latency() << " ns." << std::endl;
  std::cout << "Total latency of rasterization2 is: " << time_rasterization2.latency() << " ns." << std::endl;
  std::cout << "Total latency of zculling is: " << time_zculling.latency() << " ns." << std::endl;
  std::cout << "Total latency of coloringFB is: " << time_coloringFB.latency() << " ns." << std::endl;
  std::cout << "Total time taken by the loop is: " << total_time.latency() << " ns." << std::endl;
  std::cout << "---------------------------------------------------------------" << std::endl;
  std::cout << "Average latency of projection per loop iteration is: " << time_projection.avg_latency() << " ns." << std::endl;
  std::cout << "Average latency of rasterization1 per loop iteration is: " << time_rasterization1.avg_latency() << " ns." << std::endl;
  std::cout << "Average latency of rasterization2 per loop iteration is: " << time_rasterization2.avg_latency() << " ns." << std::endl;
  std::cout << "Average latency of zculling per loop iteration is: " << time_zculling.avg_latency() << " ns." << std::endl;
  std::cout << "Average latency of coloringFB per loop iteration is: " << time_coloringFB.avg_latency() << " ns." << std::endl;
  std::cout << "Average latency of each loop iteration is: " << total_time.avg_latency() << " ns." << std::endl;

}

void rendering_sw( Triangle_3D triangle_3ds[NUM_3D_TRI], bit8 output[MAX_X][MAX_Y])
{
  // local variables

  // 2D triangle
  Triangle_2D triangle_2ds;
  // projection angle
  int angle = 0;

  // max-min index arrays
  bit8 max_min[5];
  int max_index[1];

  // fragments
  CandidatePixel fragment[500];

  // pixel buffer
  Pixel pixels[500];
  bool flag;
  int size_fragment;
  int size_pixels;

  // processing NUM_3D_TRI 3D triangles
  TRIANGLES: for (int i = 0; i < NUM_3D_TRI; i ++ )
  {
    // five stages for processing each 3D triangle
    for(int i = 0; i < 100; i++) {
      projection( triangle_3ds[i], &triangle_2ds, angle );
    }

    for(int i = 0; i < 100; i++) {
      flag = rasterization1(triangle_2ds, max_min, max_index);
    }

    for(int i = 0; i < 100; i++) {
      size_fragment = rasterization2( flag, max_min, max_index, triangle_2ds, fragment );
    }

    for(int i = 0; i < 100; i++) {
      size_pixels = zculling( i, fragment, size_fragment, pixels);
    }

    for(int i = 0; i < 100; i++) {
      coloringFB ( i, size_pixels, pixels, output);
    }
  }
}
