
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
unsigned compute_v4 (unsigned nb_iter);
void_func_t first_touch [] = {
  NULL,
  first_touch_v1,
  first_touch_v2,
  NULL,
  NULL,
};

int_func_t compute [] = {
  compute_v0,
  compute_v1,
  compute_v2,
  compute_v3,
  compute_v4,
};

char *version_name [] = {
  "Séquentielle",
  "OpenMP",
  "OpenMP zone",
  "OpenCL",
  "OpenMP task",
};

unsigned opencl_used [] = {
  0,
  0,
  0,
  1,
  0,
};

///////////////////////////// Fonctions pour la version séquentielle 

// déterminer l'état d'une cellule pour la prochaine itération
unsigned will_live(unsigned x, unsigned y, bool alive){
	int somme = 0;
	int i, j;

		for (j = -1; j < 2 ; j++)
		{
			for (i = -1; i < 2; i++)
				if (cur_img(x+i,y+j) != 0x0 ) // on regarde si la voisine est vivante
				{
					if (i == 0 && j==0)
						;// on ne compte pas la cellule courante
					else
						somme += 1;	
				}
		}
		if (alive == 0) 
		{
			if (somme == 3) // Résurrection
				return COULEUR;
			return 0x0;
		}
		else
		{
			if (somme == 2 || somme == 3) // La cellule vit si la somme de ses voisins est égale à 2 ou 3
				return COULEUR;
			return 0x0;
		}
}


// donner le bon contexte à will live 
void update(unsigned i, unsigned j, unsigned x, unsigned y){

	bool alive = cur_img(x,y) != 0x0;
	
	if (x != i && x != TILEX && y != j && y != TILEY && x < DIM && y < DIM)
		next_img(x,y) = will_live(x,y,alive);
	else
	{
		if (x == i || y == j)
		{
			if (i == 0 || j == 0)
				; // rien
			else
				next_img(x,y) = will_live(x,y,alive);
		}
		else if (x == TILEX)
		{
			if (j == 0 )
				;// rien
			else if (y == TILEY)
				;// rien
			else 
				next_img(x,y) = will_live(x,y,alive);
		}
		else if (y == j)
		{
			if (i == 0 || j == 0)
				;// rien
			else
				next_img(x,y) = will_live(x,y,alive);
		}
		else if (y == TILEY)
		{
			if (i == 0)
				;// rien
			else if (x == TILEX)
				;// rien
			else
				next_img(x,y) = will_live(x,y,alive);
		}
		else if (x == DIM || y == DIM)
			;// rien
		else
			printf("deadbeef  \n");
	}
}
///////////////////////////// versions séquentielles simples
unsigned compute_v0 (unsigned nb_iter)
{
 /* version naive */	
 
   //~ for (unsigned it = 1; it <= nb_iter; it ++)
  //~ {
    //~ for (unsigned i = 0; i < DIM; i++)
      //~ for (unsigned j = 0; j < DIM; j++) 
      //~ {
		//~ if(i != 0 && i != DIM && j != 0 && j != DIM){  
			//~ if (cur_img(i,j) == 0) // si la cellule est morte
					//~ next_img(i,j) = will_live(i,j,0);
			//~ else 
				//~ next_img(i,j) = will_live(i,j,1);
		//~ }
      //~ }
    //~ swap_images ();
  //~ }
 
 
 /* version tuilée */ 
 
 for (unsigned it = 1; it <= nb_iter; it ++)
  {
    for (unsigned i = 1; i < DIM-1; i+=TILEX)
      for (unsigned j = 1; j < DIM-1; j+=TILEY) 
      {
		for (unsigned x = i; x < i+TILEX; x++)
		{
			for (unsigned y = j; y < j+TILEY; y++)
			{
				update(i,j,x,y);
			}
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
/*  #pragma omp parallel for schedule(dynamic, 16)
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
	*/
/* version tuilée*/ 

	for (unsigned it = 1; it <= nb_iter; it ++)
	{				
		#pragma omp parallel for collapse(2) schedule(static,32)
		for (unsigned i = 1; i < DIM-1; i++)
		{
		for (unsigned j = 1; j < DIM-1; j++) 
			{
				if(i != 0 && i != DIM && j != 0 && j != DIM){  
					if (cur_img(i,j) == 0) // si la cellule est morte
						next_img(i,j) = will_live(i,j,0);
					else 
						next_img(i,j) = will_live(i,j,1);
				}
			}
		}
		swap_images ();
	}
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

///////////////////////////// Version OpenMP task
unsigned compute_v4 (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it ++)
  {
    #pragma omp parallel
    #pragma omp single
    {
    for (unsigned i = 1; i < DIM-1; i+=TILEX)
      for (unsigned j = 1; j < DIM-1; j+=TILEY) 
      {
    
		for (unsigned x = i; x < i+TILEX; x++)
		{
		  #pragma omp task firstprivate(x)
			for (unsigned y = j; y < j+TILEY; y++)
			{
				update(i,j,x,y);
			}
		}
      }
    #pragma omp taskwait
    swap_images ();
    } // end parallel
  }

  // retourne le nombre d'étapes nécessaires à la
  // stabilisation du calcul ou bien 0 si le calcul n'est pas
  // stabilisé au bout des nb_iter itérations
  return 0;

}
