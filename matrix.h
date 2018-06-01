#ifndef MATRIX_H
#define MATRIX_H

#define HERMITE 0
#define BEZIER 1

struct matrix {
  double **m;
  int rows, cols;
  int lastcol;
} matrix;

//curve routines
struct matrix * make_bezier();
struct matrix * make_hermite();
struct matrix * generate_curve_coefs( double p0, double p1,
				      double p2, double p3, int type );

//transformation routines
struct matrix * make_translate(double x, double y, double z);
struct matrix * make_scale(double x, double y, double z);
struct matrix * make_rotX(double theta);
struct matrix * make_rotY(double theta);
struct matrix * make_rotZ(double theta);

//Basic matrix manipulation routines
struct matrix *new_matrix(int rows, int cols);
void free_matrix(struct matrix *m);
void grow_matrix(struct matrix *m, int newcols);
void copy_matrix(struct matrix *a, struct matrix *b);
void print_matrix(struct matrix *m);
void ident(struct matrix *m);
void scalar_mult(double x, struct matrix *m);
void matrix_mult(struct matrix *a, struct matrix *b);

#endif
