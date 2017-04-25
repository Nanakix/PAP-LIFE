
#include "compute.h"
#include "graphics.h"
#include "debug.h"
#include "ocl.h"

#include <stdbool.h>

#define COULEUR 0xFF00FFFF
#define TILEX 32
#define TILEY 32

unsigned version = 0;

void first_touch_v1 (void);
void first_touch_v2 (void);

unsigned compute_v0 (unsigned nb_iter);
unsigned compute_v1 (unsigned nb_iter);
unsigned compute_v2 (unsigned nb_iter);
unsigned compute_v3 (unsigned nb_iter);

void_func_t first_touch [] = {
  NULL,
  first_touch_v1,
  first_touch_v2,
  NULL,
};

int_func_t compute [] = {
  compute_v0,
  compute_v1,
  compute_v2,
  compute_v3,
};

char *version_name [] = {
  "Séquentielle",
  "OpenMP",
  "OpenMP zone",
  "OpenCL",
};

unsigned opencl_used [] = {
  0,
  0,
  0,
  1,
};

///////////////////////////// Version séquentielle simple

/* TODO :  eviter duplication de code avec version tuilée */
unsigned will_live(unsigned x, unsigned y, bool alive){
	int somme = 0;
	int i, j;

		for (j = -1; j < 2 ; j++)
		{
			for (i = -1; i < 2; i++)
				if (cur_img(x+i,y+j) != 0 ) // on regarde si la voisine est vivante
				{
					if (i == 0 && j==0)
					{
						// on ne compte pas la cellule courante
					}
					else
						somme += 1;	
				}
		}
		if (alive == 0) 
		{
			if (somme == 3) // Resurrection
				return COULEUR;
			return 0;
		}
		else
		{
			if (somme == 2 || somme == 3) // La cellule vit si la somme de ses voisins est égale à 2 ou 3
				return COULEUR;
			return 0;
		}
}

// met à jour les cellules locales à la tuile
unsigned will_live_tile(unsigned x, unsigned y, bool alive, unsigned** tile)
{
	unsigned i,j;
	unsigned somme = 0;
	
	for (i = 0; i <= 2 ; i++)
	{
		
		printf("boucle 1 \n");
		for (j = 0; j <= 2 ; j++)
		{
		printf("boucle 2 \n");
			if(tile[x+i-1][y+j-1] != 0) // on regarde si la voisine est vivante
			{
				printf("if \n");
				if (i == 1 && j == 1)
				{
					// rien
				}
				else
				{
					somme += 1;
				}
			}
		}
	}
	if (alive == 0) 
	{
		if (somme == 3) // Resurrection
			return COULEUR;
		return 0;
	}
	else
	{
		if (somme == 2 || somme == 3) // La cellule vit si la somme de ses voisins est égale à 2 ou 3
			return COULEUR;
		return 0;
	}
	
	
}

unsigned compute_v0 (unsigned nb_iter)
{
 /* version naive */	
/* 
 *  for (unsigned it = 1; it <= nb_iter; it ++)
  {
    for (unsigned i = 0; i < DIM; i++)
      for (unsigned j = 0; j < DIM; j++) 
      {
		if(i != 0 && i != DIM && j != 0 && j != DIM){  
			if (cur_img(i,j) == 0) // si la cellule est morte
					next_img(i,j) = will_live(i,j,0);
			else 
				next_img(i,j) = will_live(i,j,1);
		}
      }
    swap_images ();
  }
 */
 /* version tuilée */ 
 
  //~ unsigned TILEX,TILEY = 32;
  unsigned tile[TILEX+2][TILEY+2];
/*  unsigned **tile = malloc((TILEX+2)*sizeof(unsigned));
  if (tile)
  {
	  for (int i = 0; i < TILEX+2; i++)
	  {
		  tile[i] = malloc(sizeof *tile[i] * (TILEY+2));
	  }
  }
*/	  
  
  int x,y;
  int nbTileX = 0, nbTileY = 0;
  for (unsigned it = 1; it <= nb_iter; it++)
  {
	  for (unsigned i = 1; i < DIM-1 ; i += TILEX) // zone dangereuse
	  {
		  for (unsigned j = 1; j < DIM-1; j+= TILEY)
		  {
			  
		  
		  
		  for (y = -1; y < TILEY+1; y++)
		  {
			  for (x = -1; x < TILEX+1; x++)
			  {
				  //remplissage de la tuile
				  tile[x+1][y+1] = cur_img((y+1)+(nbTileY*TILEY),((x+1)+(nbTileX * TILEX)));
			  }
			  
		  }
		  // tuile remplie, on regarde l'état de chacune de ses cellules
			printf("tuile remplie \n");
			
		  for (x = -1; x < TILEX+1 ; x++)
	      {
			for (y = -1; y < TILEY+1; y++)
			{
				/* si la cellule suivante est dans la tuile */
				if (x != -1 || x != TILEX || y != -1 || y != TILEY)
				{
					if (tile[x][y] == 0)
					{
						printf("mort \n");
						next_img(x+(nbTileX*TILEX),(y+(nbTileY * TILEY))) = will_live_tile(x,y,0,tile);
					}
					else
					{
						printf("vie \n");
						next_img(x+(nbTileX*TILEX),(y+(nbTileY * TILEY))) = will_live_tile(x,y,1,tile); 
					}
				}
				else
				{
					// rien
				}
				
				
			}
							
		  }
		  nbTileY++;
	      }		  
	      nbTileY = 0;
		  nbTileX++;
	  }
	  swap_images();
  }
  
 
 
 
  // retourne le nombre d'étapes nécessaires à la
  // stabilisation du calcul ou bien 0 si le calcul n'est pas
  // stabilisé au bout des nb_iter itérations
  return 0;
}


///////////////////////////// Version OpenMP de base

void first_touch_v1 ()
{
  int i,j ;

#pragma omp parallel for
  for(i=0; i<DIM ; i++) {
    for(j=0; j < DIM ; j++)
      next_img (i, j) = cur_img (i, j) = 0 ;
  }
}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v1(unsigned nb_iter)
{
/* version naïve */	
  #pragma omp parallel for schedule(dynamic, 16)
  for (unsigned it = 1; it <= nb_iter; it ++)
  {
    for (unsigned i = 0; i < DIM; i++)
      for (unsigned j = 0; j < DIM; j++) 
      {
		if(i != 0 && i != DIM && j != 0 && j != DIM){  
			if (cur_img(i,j) == 0) // si la cellule est morte
				next_img(i,j) = will_live(i,j,0);
			else 
				next_img(i,j) = will_live(i,j,1);
		}
      }
    swap_images ();
  }
	
/* version tuilée*/ 








	
  return 0;
}







///////////////////////////// Version OpenMP optimisée

void first_touch_v2 ()
{

}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v2(unsigned nb_iter)
{
  return 0; // on ne s'arrête jamais
}


///////////////////////////// Version OpenCL

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v3 (unsigned nb_iter)
{
  return ocl_compute (nb_iter);
}
