/*===============================================================*/
/*                                                               */
/*                       3d_rendering.cpp                        */
/*                                                               */
/*      Main host function for the 3D Rendering application.     */
/*                                                               */
/*===============================================================*/

// standard C/C++ headers
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <time.h>
#include <sys/time.h>

# include "../sw/rendering_sw.h"

// other headers
#include "utils.h"
#include "typedefs.h"
#include "check_result.h"

// data
#include "input_data.h"


int main(int argc, char ** argv) 
{
  printf("3D Rendering Application\n");

  // output
  bit8 output[MAX_X][MAX_Y];
  rendering_sw(triangle_3ds, output);
 
  // check results
  printf("Writing output...\n");
  check_results(output);
  printf("Check output.txt for a bunny!\n");

  return EXIT_SUCCESS;

}
