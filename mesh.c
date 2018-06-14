#include "mesh.h"

struct matrix *parse_mesh(char *file) {
    struct matrix *polygons = new_matrix(4, 1000);
    struct matrix *verticies = new_matrix(4, 1000);

    FILE *f;
    f = fopen(file, "r");

    if (!f) {
        printf("error: file could not be opened\n");
        exit(1);
    }

    char line[1024];
    double x, y, z;
    int v1, v2, v3, v4;

    add_point(verticies, 0, 0, 0); // placeholder

    while (fgets(line, sizeof(line), f)) {

        if (strncmp(line, "v ", 2) == 0) {
            sscanf(line, "v %lf %lf %lf", &x, &y, &z);
            add_point(verticies, x, y, z);
        }

        else if (strncmp(line, "f", 1) == 0) {
            sscanf(line, "f %d %d %d %d", &v1, &v2, &v3, &v4);
            add_polygon(polygons,
                        verticies->m[0][v1], verticies->m[1][v1], verticies->m[2][v1],
                        verticies->m[0][v2], verticies->m[1][v2], verticies->m[2][v2],
                        verticies->m[0][v3], verticies->m[1][v3], verticies->m[2][v3]);
        }
    }

    fclose(f);

    return polygons;
}
