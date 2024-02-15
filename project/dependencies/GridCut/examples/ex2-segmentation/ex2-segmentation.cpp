/*
  ex2-segmentation
  ================

  This example shows a minimal image segmentation code based on GridCut.
*/

#include <cstdio>
#include <cmath>

#include <Image.h>
#include <GridGraph_2D_4C.h>

#define K_PARAM 4000
#define SOFT_PARAMETER 16.0f
#define K(soft) (short)( K_PARAM * (1 - soft) + K_PARAM / SOFT_PARAMETER * soft)

#define SIGMA2 0.012f
//#define WEIGHT(A) (short)(1+K*std::exp((-(A)*(A)/SIGMA2)))
#define WEIGHT(A, B) (short)(1+ K_PARAM* std::pow(std::min(A, B), 4))

#define RED  RGB(1,0,0)
#define BLUE RGB(0,0,1)
#define WHITE RGB(1,1,1)

int main(int argc,char** argv)
{
  typedef GridGraph_2D_4C<short,short,int> Grid;

  const Image<float> image = imread<float>("es.png");
  const Image<RGB> scribbles = imread<RGB>("scribbled.png");
  
  const int width  = image.width();
  const int height = image.height();

  Grid* grid = new Grid(width,height);
  
  for (int y=0;y<height;y++)
  {
    for (int x=0;x<width;x++)
    {
      grid->set_terminal_cap(grid->node_id(x,y),
                             scribbles(x,y)==WHITE ? K(0) : 0,
                             scribbles(x,y)==BLUE  ? K(0) : 0);

      if (x<width-1)
      {
        const short cap = WEIGHT(image(x,y),image(x+1,y));

        grid->set_neighbor_cap(grid->node_id(x  ,y),+1,0,cap);
        grid->set_neighbor_cap(grid->node_id(x+1,y),-1,0,cap);
      }

      if (y<height-1)
      {
        const short cap = WEIGHT(image(x,y),image(x,y+1));

        grid->set_neighbor_cap(grid->node_id(x,y  ),0,+1,cap);
        grid->set_neighbor_cap(grid->node_id(x,y+1),0,-1,cap);
      }
    }
  }


  grid->compute_maxflow();


  Image<RGB> output(width,height);

  for (int y=0;y<height;y++)
  {
    for (int x=0;x<width;x++)
    {
      output(x,y) = image(x, y) * ((grid->get_segment(grid->node_id(x, y)) ? BLUE : WHITE));
    }
  }

  delete grid;

  imwrite(output,"output.png");  

  printf("The result was written to \"output.png\".\n");
  
  return 0;
}
