//global unsigned realIt = 0;
#define COULEUR 0xFF00FFFF
__kernel void transpose_naif (__global unsigned *in, __global unsigned *out)
{
  int x = get_global_id (0);
  int y = get_global_id (1);

  out [x * DIM + y] = in [y * DIM + x];
}



__kernel void transpose (__global unsigned *in, __global unsigned *out)
{
  __local unsigned tile [TILEX][TILEY+1];
  int x = get_global_id (0);
  int y = get_global_id (1);
  int xloc = get_local_id (0);
  int yloc = get_local_id (1);

  tile [xloc][yloc] = in [y * DIM + x];

  barrier (CLK_LOCAL_MEM_FENCE);

  out [(x - xloc + yloc) * DIM + y - yloc + xloc] = tile [yloc][xloc];
}

static float4 color_scatter (unsigned c);

int test2(__global unsigned *in, __global unsigned *out, int x, int y)
{
  int somme = 0;
  if (x != 0 && x < DIM-1 && y != 0 && y < DIM-1)
  {
    int somme = (in[(y - 1) * DIM + (x - 1)] != 0)
              + (in[(y - 1) * DIM + x] != 0)
              + (in[(y - 1) * DIM + (x + 1)] != 0)
              + (in[y * DIM + (x - 1)] != 0)
              + (in[y * DIM + (x + 1)] != 0)
              + (in[(y + 1) * DIM + (x - 1)] != 0)
              + (in[(y + 1) * DIM + x] != 0)
              + (in[(y + 1) * DIM + (x + 1)] != 0);

    int result = 0;
    if (in[y * DIM + x ] == 0)
    {
      if (somme == 3)
        result = COULEUR;
      else
        result = 0;
    }
    else
    {
      if (somme == 2 || somme == 3)
        result = COULEUR;
      else
        result = 0;
    }
    out[y * DIM + x] = result;
  }
}

__kernel void test (__global unsigned *in, __global unsigned *out)
{
  int x = get_global_id (0);
  int y = get_global_id (1);
    test2(in, out, x, y);
}

int calculeTuile(unsigned tile[DIM / TILEX][DIM / TILEX], int x, int y, int tailleLocalMax)
{
  int sommeDesVoisins = 0;
  //int tailleLocalMax = get_local_size(0);
  for (int j = -1; j < 2 ; j++)
  {
    for (int i = -1; i < 2; i++)
    {
      if(x+j == -1 ||
        x+j == tailleLocalMax ||
        y+i == -1 ||
        y+i == tailleLocalMax)
      {
        //printf("hors-cadre ");
      }
      else
      {
        //printf("somme ");
        sommeDesVoisins += tile[x+i][y+j];
      }
    }
  }
  if(tile[x][y] != 0)
    sommeDesVoisins -= tile[x][y];
  return sommeDesVoisins;
}


void remplissageTuileVoisin(__local int tab[TILEX][TILEX])
{
  int xloc = get_local_id (0);
  int yloc = get_local_id (1);
  
  
  //int tailleTableau = DIM / TILEX;

  unsigned nbvoisins = 0;
  if(tab[xloc][yloc] != 0);
  else
    nbvoisins ++;
  //barrier(CLK_LOCAL_MEM_FENCE); // fin du comptage des voisins
  //tab[xloc][yloc] = nbvoisins;
  printf(" %u ", nbvoisins);
  //barrier(CLK_LOCAL_MEM_FENCE); // fin écriture des voisins
  
}

__kernel void opti (__global unsigned *in, __global unsigned *out, __global int tab[DIM / TILEX][DIM / TILEX])
{
  //printf("tab : %d", tab[0][0]);
  //realIt++;
  int x = get_global_id (0);
  int y = get_global_id (1);
  
  __local unsigned tile[TILEX][TILEY];
  int xloc = get_local_id (0);
  int yloc = get_local_id (1);
  int tailleLocalMax = get_local_size(0);
  
  tile[xloc][yloc] = in[y * DIM +x];
  /*
  
  int tailleTableau = DIM / TILEX;

  unsigned nbvoisins = 0;
  float4 zero = 0;
  if(all(color_scatter(in[y * DIM + x]) != zero))
    nbvoisins ++;
  barrier(CLK_LOCAL_MEM_FENCE); // fin du comptage des voisins
  tile[xloc][yloc] = nbvoisins;
  barrier(CLK_LOCAL_MEM_FENCE); // fin écriture des voisins
  */
  // A faire que par un thread
  //printf("x : %d, y : %d ",x, y);
  double partieEntier;
  double resteDivision;
  resteDivision = modf((double) x/TILEX, &partieEntier) +
                  modf((double) y/TILEY, &partieEntier);
  //printf("reste :%ld ", resteDivision);
  
  if(resteDivision == (double) 0)
  {
    if(tab[0][0] == -1)
    {
      //printf(" %d ", tab[xloc][yloc]);
      //calculTableauTuile
      remplissageTuileVoisin(tile);
      printf(" tile %u ", tile[xloc][yloc]);
    }
    /*
    else
    {
      if(tab[xloc][yloc] == 0)
      {
        //printf(" %d ", tab[xloc][yloc]);
        if(calculeTuile(tab, xloc, yloc, tailleLocalMax) != 0)
        {
          //calculTableauTuile
          test2(in, out, x, y);
          remplissageTuileVoisin(in, tab, x, y);
        }
      }
      else
      {
        printf(" %d ", tab[xloc][yloc]);
        //calculTableauTuile
        test2(in, out, x, y);
        remplissageTuileVoisin(in, tab, x, y);
      }
    }
    */
  }
  

}



// NE PAS MODIFIER
static float4 color_scatter (unsigned c)
{
  uchar4 ci;

  ci.s0123 = (*((uchar4 *) &c)).s3210;
  return convert_float4 (ci) / (float4) 255;
}

// NE PAS MODIFIER: ce noyau est appelé lorsqu'une mise à jour de la
// texture de l'image affichée est requise
__kernel void update_texture (__global unsigned *cur, __write_only image2d_t tex)
{
  int y = get_global_id (1);
  int x = get_global_id (0);
  int2 pos = (int2)(x, y);
  unsigned c;

  c = cur [y * DIM + x];

  write_imagef (tex, pos, color_scatter (c));
}
