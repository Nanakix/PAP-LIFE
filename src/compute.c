
#include "compute.h"
#include "graphics.h"
#include "debug.h"
#include "ocl.h"

#include <stdbool.h>

#define COULEUR 0xFF00FFFF
#define TILEX 32
#define TILEY 32

// global
int nbx =0, nby = 0;

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
/* // met à jour les cellules locales à la tuile
unsigned will_live_tile(unsigned x, unsigned y, bool alive, unsigned** tile)
{
	int i,j;
	unsigned somme = 0;
	// voisinage cellule
	for (i = -1; i < 2 ; i++)
	{
		
		printf("boucle 1 \n");
		for (j = -1; j < 2 ; j++)
		{
		printf("boucle 2 \n");
			if(tile[x+i][y+j] != 0) // on regarde si la voisine est vivante
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
*/

// donner le bon contexte à will live 
void update(unsigned i, unsigned j, unsigned x, unsigned y){

	bool alive = cur_img(x,y) != 0;
	if (x != i && x != TILEX && y != j && y != TILEY)
	{
		next_img(x,y) = will_live(x,y,alive);
	}
	else
	{
		if (x == i || y == j)
		{
			//~ printf("x == i \n");
			if (i == 0 || j == 0)
			{
				// rien
			}
			else
			{
				next_img(x,y) = will_live(x,y,alive);
			}
		}
		else if (x == TILEX)
		{
			if (j == 0 )
			{
				// rien
			}
			else if (y == TILEY)
			{
				//
			}
			else 
			{
				next_img(x,y) = will_live(x,y,alive);
			}
			
			//~ printf("x == TILEX \n");
		}
		else if (y == j)
		{
			if (i == 0 || j == 0)
			{
				// rien
			}
			else
			{
				next_img(x,y) = will_live(x,y,alive);
			}
			
			//~ printf("y == j \n");
		}
		else if (y == TILEY)
		{
			if (i == 0)
			{
				// rien
			}
			else if (x == TILEX)
			{
				//
			}
			else
			{
				next_img(x,y) = will_live(x,y,alive);
			}
			
			//~ printf("y == TILEY \n");
		}
		else
			printf("deadend \n");
		
	}
	
	
	
	
}

unsigned compute_v0 (unsigned nb_iter)
{
 /* version naive */	
 /*
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
 /* version tuilée */ 
 
 for (unsigned it = 1; it <= nb_iter; it ++)
  {
    for (unsigned i = 0; i < DIM; i+=TILEX)
      for (unsigned j = 0; j < DIM; j+=TILEY) 
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






























  
  //~ unsigned tile[TILEX][TILEY];  
  //~ unsigned x,y,k,l;
  //~ for (unsigned it = 1; it <= nb_iter; it++)
  //~ {
	  //~ for (unsigned i = 1; i < DIM-1 ; i += TILEX) // parcours en ligne de tuiles de DIM
	  //~ {
		  //~ for (unsigned j = 1; j < DIM-1; j+= TILEY) // parcours en col de tuiles dim //
		  //~ { 
			 //~ for (x = i; x < TILEX; x++) // parcours en ligne d'une tuile
			 //~ {
				//~ for (y = j; y < TILEY; y++)   // parcours en colonne d'une tuile
			    //~ {
				   //~ //remplissage de la tuile
					//~ tile[y%TILEY][x%TILEX] = cur_img(y,x);	
					  						
					
					//~ if ( tile[y][x] != 0)
					//~ {
						//~ printf("%u\n",tile[y][x]);	  
					//~ }
				//~ }
			  
			//~ }
		  //~ // tuile remplie, on regarde l'état de chacune de ses cellules
			//~ printf("tuile remplie \n");
			
		  //~ for (x = 0; x < TILEX ; x++)
	      //~ {
			//~ for (y = 0; y < TILEY; y++)
			//~ {
				//~ /* si la cellule suivante est dans la tuile */
				//~ if (x != 0 && x != TILEX && y != 0 && y != TILEY)
				//~ {
					//~ if (tile[y][x] == 0)
					//~ {
						//~ printf("mort \n");
					//////	next_img(x+(nbx*TILEX),(y+(nby * TILEY))) = will_live_tile(x,y,0,tile);
						//~ for (k = x; k < x+TILEX; k++)
						//~ {
							//~ for (l = y; l < y +TILEY ; l++)
							//~ {
								//~ //next_img(k,l) = will_live(x,y,0);
							//~ }
						//~ }
						
					//~ }
					//~ else
					//~ {
						//~ printf("vie \n");
						/////next_img(x+(nbx*TILEX),(y+(nby * TILEY))) = will_live_tile(x,y,1,tile); 
						//~ for (k = x; k < x+TILEX; k++)
						//~ {
							//~ for (l = y; l < y +TILEY ; l++)
							//~ {
								//~ next_img(k,l) = will_live(x,y,1);
							//~ }
						//~ }
					//~ }
				//~ }
				//~ else
				//~ {
					//~ // rien
				//~ }
				
				
			//~ }
							
		  //~ }
		  //~ nby++;
	      //~ }		  
	      //~ nby = 0;
		  //~ nbx++;
	  //~ }
	  //~ swap_images();
  //~ }
  
 
 
 
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
