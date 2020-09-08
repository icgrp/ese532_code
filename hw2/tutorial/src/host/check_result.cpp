/*===============================================================*/
/*                                                               */
/*                       check_result.cpp                        */
/*                                                               */
/*      Software evaluation of training and test error rate      */
/*                                                               */
/*===============================================================*/

#include <cstdio>
#include "typedefs.h"
#include <fstream>
#include <iostream>

void check_results(bit8 output[MAX_X][MAX_Y])
{
  // print result
  std::ofstream ofile;
  ofile.open("outputs.txt");
  if (ofile.is_open())
  {
    ofile << "Image After Rendering: \n";
    for (int j = MAX_X - 1; j >= 0; j -- )
    {
      for (int i = 0; i < MAX_Y; i ++ )
      {
        int pix;
        pix = output[i][j];
        if (pix)
          ofile << "1";
        else
          ofile << "0";
      }
      ofile << std::endl;
    }
  }
  else
  {
    std::cout << "Failed to create output file!" << std::endl;
  }
}
