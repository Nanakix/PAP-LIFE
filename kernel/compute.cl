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

__kernel void test (__global unsigned *in, __global unsigned *out)
{
  
  int x = get_global_id (0);
  int y = get_global_id (1);
  unsigned tmp_couleur = 0;
  if (x != 0 && x < DIM-1 && y != 0 && y < DIM-1)
  {
    
    int somme = 0;
    for (int i = -1; i < 2; i++)
    {
      
      for (int j = -1; j < 2; j++)
      {
      /*
        if(in[(y) * DIM + (x )]!=0){
        printf("%u",in[(y) * DIM + (x )]);
        printf("%2.2v4hlf\n",color_scatter(in[(y ) * DIM + (x )]));
        float4 tmp = 0;
        //printf("%2.2v4hlf\n",tmp);
        //float4 tmp2 = all(color_scatter(in[(y ) * DIM + (x )]));
        //printf("%2.2v4hlf\n",tmp2);
        }
        */
        float4 tmp = 0;
        //printf("%2.2v4hlf\n",color_scatter(in[(y +i) * DIM + (x +j)]));
        if (all(color_scatter(in[(y + i) * DIM + (x + j)]) == tmp))
        {
          //ne fait rien
        }
        else{
          //printf("%2.2v4hlf\n",color_scatter(in[(y +i) * DIM + (x +j)]));
          if (i == 0 && j == 0)
          {
          // on ne compte pas la cellule courante
          
          }
          else
          {
            //printf("h");
            somme += 1;
            tmp_couleur = in[(y + i) * DIM + (x + j)];
            //printf("tmp_couleur %u",tmp_couleur);
          }
        }
      }
    }
    //printf("somme : %d",somme);
    // A finir
    int result = 0;
    if (all(color_scatter(in[y * DIM + x ]) == 0))
    {
      //printf("%2.2v4hlf\n",color_scatter(in[(y ) * DIM + (x )]));
      if (somme == 3)
        result = tmp_couleur;
      else
        result = 0;
    }
    else
    {
      //printf("%2.2v4hlf\n",color_scatter(in[(y ) * DIM + (x )]));
      if (somme == 2 || somme == 3)
        result = tmp_couleur;
      else
        result = 0;
    }
    //printf("%u ",result);
    out[y * DIM + x] = result;
    //printf("%d", somme);
    
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
