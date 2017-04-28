
#include "compute.h"
#include "graphics.h"
#include "debug.h"
#include "ocl.h"

#include <stdbool.h>

#define COULEUR 0xFF00FFFF
#define TILEX 32
#define TILEY 32

#define TAILLETABLEAU DEFAULT_DIM/TILEX
//int tailleTableau = DEFAULT_DIM/TILEX;

// Tableau qui contiendra le nombre de cellule non morte d'une tuile
int tabTuile[TAILLETABLEAU][TAILLETABLEAU];
int realIt = 0;
unsigned version = 0;

void first_touch_v1 (void);
void first_touch_v2 (void);

unsigned compute_v0 (unsigned nb_iter);
unsigned compute_v1 (unsigned nb_iter);
unsigned compute_v2 (unsigned nb_iter);
unsigned compute_v3 (unsigned nb_iter);
unsigned compute_v4 (unsigned nb_iter);
unsigned compute_v5 (unsigned nb_iter);
unsigned compute_v6 (unsigned nb_iter);
unsigned compute_v7 (unsigned nb_iter);
unsigned compute_v8 (unsigned nb_iter);
unsigned compute_v9 (unsigned nb_iter);
unsigned compute_v10 (unsigned nb_iter);


void_func_t first_touch [] = {
  NULL,
  first_touch_v1,
  first_touch_v2,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

int_func_t compute [] = {
  compute_v0,
  compute_v1,
  compute_v2,
  compute_v3,
  compute_v4,
  compute_v5,
  compute_v6,
  compute_v7,
  compute_v8,
  compute_v9,
  compute_v10,
};

char *version_name [] = {
  "Séquentielle Naïve",
  "OpenMP Naïve",
  "OpenMP zone",
  "OpenCL",
  "OpenMP task",
  "Séquentielle tuilée",
  "OpenMP tuilée(collapse)",
  "Séquentielle optimisée",
  "OpenCL optimisée",
  "OpenMP task optimisée",
  "Mixte CPU/GPU",
};

unsigned opencl_used [] = {
  0,
  0,
  0,
  1,
  0,
  0,
  0,
  0,
  1,
  0,
  0,
};

/* * * * * * * * * * * * * * * * * * * * * * 
 * Fonctions pour la version séquentielle  *
 * * * * * * * * * * * * * * * * * * * * * */
 
/* déterminer l'état d'une cellule pour la prochaine itération */
unsigned will_live(unsigned x, unsigned y, bool alive){
	int somme = 0;
	int i, j;

		for (j = -1; j < 2 ; j++)
		{
			for (i = -1; i < 2; i++)
			{
				if (cur_img(x+i,y+j) != 0x0 ) // on regarde si la voisine est vivante
				{
					//if (i == 0 && j==0)
						//;// on ne compte pas la cellule courante
					//else
						somme += 1;	
				}
			}
		}
		if(cur_img(x,y) != 0x0)
			somme -=1;
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


/* donner le bon contexte à will_live  */
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
			printf("deadbeef\n");
	}
}


/* * * * * * * * * * * * * * * * * * *
 * Fonctions pour version optimisée  *
 * * * * * * * * * * * * * * * * * * */  

// Permet de calculer et d'ajouter le contenu du tableau de tuile
void calculTableauTuile(unsigned i, unsigned j, int indiceI, int indiceJ)
{
	int nbCaseCouleur = 0;
	for (unsigned x = i; x < i+TILEX; x++)
	{
		for (unsigned y = j; y < j+TILEY; y++)
		{
			update(i,j,x,y);
			// Regarde si la case n'est pas vide est imcrémente le compteur.
			if(next_img(x,y) != 0x0)
				nbCaseCouleur++;
		}
	}
	//printf("nbCaseCouleur = %d ",nbCaseCouleur);
	
	tabTuile[indiceI][indiceJ] = nbCaseCouleur;
	//printf("tabTuile : indiceI : %d, indiceJ : %d, nbCaseCouleur = %d\n", indiceI, indiceJ, nbCaseCouleur);
}

int check_neighbors_opti2(int indiceI, int indiceJ)
{
	//int tmp = 0;
	int sommeDesVoisins = 0;
	for (int j = -1; j < 2 ; j++)
	{
		for (int i = -1; i < 2; i++)
		{
			if((indiceI+i) == -1 || 
				 (indiceI+i) == TAILLETABLEAU ||
				(indiceJ+j) == -1 ||  
				(indiceJ+j) == TAILLETABLEAU)
			{
				// rien
				
			}
			else
			{
				sommeDesVoisins += tabTuile[indiceI+i][indiceJ+j];
				//tmp++;
			}
		}
	}
	if(tabTuile[indiceI][indiceJ] != 0)
		sommeDesVoisins -= tabTuile[indiceI][indiceJ];
	//printf("nbvoisin : %d \n", tmp);
	return sommeDesVoisins;
}

int check_neighbors_opti (int indiceI, int indiceJ)
{
	int sommeDesVoisins = 0;
	int indiceMaxTableau = TAILLETABLEAU - 1;
	if(indiceI == 0 && indiceJ == 0) // coin nord-ouest
	{
		// 3 voisins à voir s, se, e
		sommeDesVoisins += tabTuile[indiceI][indiceJ+1]
		+ tabTuile[indiceI+1][indiceJ]
		+ tabTuile[indiceI+1][indiceJ+1];
	}
	else if(indiceI == 0)
	{
		if(indiceJ == indiceMaxTableau) // coin sud-ouest
		{
			// 3 voisins à voir n, ne, e
			sommeDesVoisins += tabTuile[indiceI][indiceMaxTableau-1]
			+ tabTuile[indiceI+1][indiceMaxTableau]
			+ tabTuile[indiceI+1][indiceMaxTableau-1];
		}
		else // bord ouest hors coin
		{
			// 5 voisins à voir n, ne, e , se, s // 
			sommeDesVoisins += tabTuile[indiceI][indiceJ-1]
			+ tabTuile[indiceI+1][indiceJ-1]
			+ tabTuile[indiceI+1][indiceJ]
			+ tabTuile[indiceI][indiceJ+1]
			+ tabTuile[indiceI+1][indiceJ+1];
		}
	}
	else if(indiceJ == 0)
	{
		if(indiceI == indiceMaxTableau) // coin nord-est
		{
			// 3 voisins à voir o, so, s
			sommeDesVoisins += tabTuile[indiceMaxTableau-1][indiceJ]
			+ tabTuile[indiceMaxTableau][indiceJ+1]
			+ tabTuile[indiceMaxTableau-1][indiceJ+1];
		}
		else // bord nord hors coin
		{
			// 5 voisins à voir o, so , s , se, e
			sommeDesVoisins += tabTuile[indiceI-1][indiceJ]
			+ tabTuile[indiceI-1][indiceJ+1]
			+ tabTuile[indiceI][indiceJ+1]
			+ tabTuile[indiceI+1][indiceJ+1]
			+ tabTuile[indiceI+1][indiceJ];
		}
	}
	else if(indiceI == indiceMaxTableau && indiceJ == indiceMaxTableau) // coin sud-est
	{
		// 3 voisins à voir o, no , n
		sommeDesVoisins += tabTuile[indiceMaxTableau-1][indiceMaxTableau]
		 + tabTuile[indiceMaxTableau-1][indiceMaxTableau-1]
		 + tabTuile[indiceMaxTableau][indiceMaxTableau-1];
	}
	else if(indiceI == indiceMaxTableau) // bord est hors coin
	{
		// 5 voisins à voir n, no, o, so, s
		sommeDesVoisins += tabTuile[indiceMaxTableau][indiceJ-1]
		+ tabTuile[indiceMaxTableau-1][indiceJ-1]
		+ tabTuile[indiceMaxTableau-1][indiceJ]
		+ tabTuile[indiceMaxTableau-1][indiceJ+1]
		+ tabTuile[indiceMaxTableau][indiceJ+1];
	}
	else if(indiceJ == indiceMaxTableau) // bord sud hors coin
	{
		// 5 voisins à voir o, no, n, ne, e
		sommeDesVoisins += tabTuile[indiceI-1][indiceMaxTableau]
		+ tabTuile[indiceI-1][indiceMaxTableau-1]
		+ tabTuile[indiceI][indiceMaxTableau-1]
		+ tabTuile[indiceI+1][indiceMaxTableau-1]
		+ tabTuile[indiceI+1][indiceMaxTableau];
	}
	else // cellule hors bord
	{
		// 8 voisins à voir : toutes directions sauf soi-même
		sommeDesVoisins += tabTuile[indiceI-1][indiceJ-1]
		+ tabTuile[indiceI][indiceJ-1]
		+ tabTuile[indiceI+1][indiceJ-1]
		+ tabTuile[indiceI-1][indiceJ]
		+ tabTuile[indiceI+1][indiceJ]
		+ tabTuile[indiceI-1][indiceJ+1]
		+ tabTuile[indiceI][indiceJ+1]
		+ tabTuile[indiceI+1][indiceJ+1];
	}
	return sommeDesVoisins;
}



/* * * * * * * * * * * * * * * * 
 * Version séquentielle naïve  *
 * * * * * * * * * * * * * * * */
 
unsigned compute_v0 (unsigned nb_iter)
{
	for (unsigned it = 1; it <= nb_iter; it ++)
	{	
		//printf("%u \n",it);
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
  
  // retourne le nombre d'étapes nécessaires à la
  // stabilisation du calcul ou bien 0 si le calcul n'est pas
  // stabilisé au bout des nb_iter itérations
  return 0;
}


/* * * * * * * * * * * * * *
 * Version OpenMP de base  *
 * * * * * * * * * * * * * */

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







/* * * * * * * * * * * * * * * * 
 *  Version OpenMP optimisée   *
 * * * * * * * * * * * * * * * */
void first_touch_v2 ()
{

}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v2(unsigned nb_iter)
{
	int indiceI = 0;
	int indiceJ = 0;
	for (unsigned it = 1; it <= nb_iter; it ++)
	{
		
		#pragma omp parallel for shared(realIt) collapse(2) schedule(static,TAILLETABLEAU/TILEX) shared(indiceI, indiceJ)
			for (unsigned i = 1; i < DIM-1; i++)
			{
				for (unsigned j = 1; j < DIM-1; j++) 
				{
					//printf("avant les indices \n");
					indiceI = (i-(i%TILEX)) / TILEX;
					indiceJ = (j-(j%TILEY)) / TILEY;
					if(realIt == 1)
						calculTableauTuile(i,j,indiceI,indiceJ);
					else
					{
						if(tabTuile[indiceI][indiceJ] == 0)
						{
							printf("i : %d j : %d\n", i,j);
							if(check_neighbors_opti(indiceI, indiceJ) != 0)
								calculTableauTuile(i,j,indiceI,indiceJ);
						}
						else
							calculTableauTuile(i,j,indiceI,indiceJ);
					}
				}
			}
		realIt++;
		swap_images ();
	}
	return 0; // on ne s'arrête jamais
  return 0; // on ne s'arrête jamais
}



/* * * * * * * * * * 
 * Version OpenCL  *
 * * * * * * * * * */


// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v3 (unsigned nb_iter)
{
  return ocl_compute (nb_iter);
}



/* * * * * * * * * * * * 
 * Version OpenMP task *
 * * * * * * * * * * * */

unsigned compute_v4 (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it ++)
  {
    
	#pragma omp parallel for collapse(2)
	
	for (unsigned i = 1; i < DIM-1; i+=TILEX)
	{
		for (unsigned j = 1; j < DIM-1; j+=TILEY) 
		{
			#pragma omp task
			{
				for (unsigned x = i; x < i+TILEX; x++)
				{
					for (unsigned y = j; y < j+TILEY; y++)
					{
						update(i,j,x,y);
					}
				}
			}
		}
	} // end parallel
	swap_images ();
  }
  // retourne le nombre d'étapes nécessaires à la
  // stabilisation du calcul ou bien 0 si le calcul n'est pas
  // stabilisé au bout des nb_iter itérations
  return 0;

}

/* * * * * * * * * * * * * * * * * * *
 *  Version séquentielle tuilée     *
 * * * * * * * * * * * * * * * * * * */
unsigned compute_v5 (unsigned nb_iter)
{
 for (unsigned it = 1; it <= nb_iter; it ++)
  {
    for (unsigned i = 1; i < DIM-1; i+=TILEX)
    {
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
    }
    swap_images ();
  }

  // retourne le nombre d'étapes nécessaires à la
  // stabilisation du calcul ou bien 0 si le calcul n'est pas
  // stabilisé au bout des nb_iter itérations
  return 0;
}

/* * * * * * * * * * * * * * * * 
 *  Version OpenMP tuilée      *
 * * * * * * * * * * * * * * * */


unsigned compute_v6(unsigned nb_iter)
{
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
		} // end parallel for
		swap_images ();
	}
  return 0;
}

/* * * * * * * * * * * * * * * * * *
 * Version séquentielle optimisée  *
 * * * * * * * * * * * * * * * * * */
unsigned compute_v7(unsigned nb_iter)
{
	for (unsigned it = 1; it <= nb_iter; it ++)
	{
		realIt++;
		for (unsigned i = 1; i < DIM-1; i+=TILEX)
		{
			int indiceI = (i-(1%TILEX)) / TILEX;
			for (unsigned j = 1; j < DIM-1; j+=TILEY) 
			{
				int indiceJ = (j-(1%TILEY)) / TILEY;
				if(realIt == 1)
					calculTableauTuile(i,j,indiceI,indiceJ);
				else
				{
					if(tabTuile[indiceI][indiceJ] == 0)
					{
						if(check_neighbors_opti(indiceI, indiceJ) != 0)
							calculTableauTuile(i,j,indiceI,indiceJ);
					}
					else
						calculTableauTuile(i,j,indiceI,indiceJ);
				}
			}
		}
		swap_images ();
	}
	return 0; // on ne s'arrête jamais
}

/* * * * * * * * * * * * * * * 
 * Version OpenCL optimisée  *
 * * * * * * * * * * * * * * */
unsigned compute_v8(unsigned nb_iter)
{
  return 0; // on ne s'arrête jamais
}

/* * * * * * * * * * * * * * * * * *
 * Version OpenMP task optimisée   *
 * * * * * * * * * * * * * * * * * */
unsigned compute_v9(unsigned nb_iter)
{
			for (unsigned it = 1; it <= nb_iter; it ++)
	{
		realIt++;
		#pragma omp parallel for collapse(2)
			for (unsigned i = 1; i < DIM-1; i+=TILEX)
			{
				for (unsigned j = 1; j < DIM-1; j+=TILEY) 
				{
					#pragma omp task
					{
						int indiceI = (i-(1%TILEX)) / TILEX;
						int indiceJ = (j-(1%TILEY)) / TILEY;
						if(realIt == 1)
							calculTableauTuile(i,j,indiceI,indiceJ);
						else
						{
							if(tabTuile[indiceI][indiceJ] == 0)
							{
								if(check_neighbors_opti(indiceI, indiceJ) != 0)
									calculTableauTuile(i,j,indiceI,indiceJ);
							}
							else
								calculTableauTuile(i,j,indiceI,indiceJ);
						}
					}
				}
				
			}
			//end pragma omp parallel for
		swap_images ();
	}
	return 0; // on ne s'arrête jamais
	//return ocl_compute (nb_iter);
}

/* * * * * * * * * * * * * * *
 * Version mixte optimisée   *
 * * * * * * * * * * * * * * */
unsigned compute_v10(unsigned nb_iter)
{
  return 0; // on ne s'arrête jamais
}
