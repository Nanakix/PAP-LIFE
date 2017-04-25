
#include "compute.h"
#include "graphics.h"
#include "debug.h"
#include "ocl.h"

#include <stdbool.h>

#define COULEUR 0xFF00FFFF

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


unsigned will_live(unsigned x, unsigned y, bool alive){
	int somme = 0;
	int i, j;

		for (j = -1; j < 2 ; j++)
		{
			for (i = -1; i < 2; i++)
				if (cur_img(x+i,y+j) != 0 ) 
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


unsigned compute_v0 (unsigned nb_iter)
{
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
