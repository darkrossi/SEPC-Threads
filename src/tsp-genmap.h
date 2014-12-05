#ifndef TSP_GENMAP_H
#define TSP_GENMAP_H

/* nombre de villes */
extern int nb_towns;
typedef struct {
  int x, y ;
} coor_t ;

typedef coor_t *coortab_t;
coortab_t towns;

extern int *cutprefix; /* cutprefix[i] = plus petite distance entre la ville i est une autre ville */

/* initialisation du tableau des distances */
void genmap ();
#endif
