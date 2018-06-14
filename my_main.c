/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser,
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  push: push a new origin matrix onto the origin stack
  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix
  based on the provided values, then
  multiply the current top of the
  origins stack by it.

  box/sphere/torus: create a solid object based on the
  provided values. Store that in a
  temporary matrix, multiply it by the
  current top of the origins stack, then
  call draw_polygons.

  line: create a line based on the provided values. Stores
  that in a temporary matrix, multiply it by the
  current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the image live
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "gmath.h"
#include "hash_table.h"
#include "mesh.h"


/*======== void find_light() ==========
  Inputs:
  Returns: 1 if found, 0 if not

  Checks the op array for a light command

  If light is found, use the specified light(s) and not the default light.
  ====================*/
int find_light() {
    int i;
    int num_lights = 0;

    for (i=0;i<lastop;i++) {
        switch (op[i].opcode) {
        case LIGHT:
            num_lights++;
        }
    }
    return num_lights;
}

/*======== void first_pass() ==========
  Inputs:
  Returns:

  Checks the op array for any animation commands
  (frames, basename, vary)

  Should set num_frames and basename if the frames
  or basename commands are present

  If vary is found, but frames is not, the entire
  program should exit.

  If frames is found, but basename is not, set name
  to some default value, and print out a message
  with the name being used.
  ====================*/
void first_pass() {
    //in order to use name and num_frames throughout
    //they must be extern variables
    extern int num_frames;
    extern char name[128];

    int vary_found = 0;
    int frames_found = 0;
    int name_found = 0;

    int i;
    for (i=0;i<lastop;i++) {
        switch (op[i].opcode) {
        case FRAMES:
            printf("Num frames: %4.0f",op[i].op.frames.num_frames);
            num_frames = op[i].op.frames.num_frames;
            frames_found = 1;
            break;
        case BASENAME:
            printf("Basename: %s",op[i].op.basename.p->name);
            strncpy(name, op[i].op.basename.p->name, sizeof(name));
            name_found = 1;
            break;
        case VARY:
            printf("Vary: %4.0f %4.0f, %4.0f %4.0f",
                   op[i].op.vary.start_frame,
                   op[i].op.vary.end_frame,
                   op[i].op.vary.start_val,
                   op[i].op.vary.end_val);
            vary_found = 1;
            break;
        }
    }

    if (vary_found && !frames_found) {
        printf("Frames command not found.\n");
        exit(1);
    }

    if (frames_found && !name_found) {
        char basename[10] = "default";
        printf("Default basename used: %s.\n", basename);
        strncpy(name, basename, sizeof(name));
    }

    if (!frames_found) {
        num_frames = 1;
    }

}

/*======== struct vary_node ** second_pass() ==========
  Inputs:
  Returns: An array of vary_node linked lists

  In order to set the knobs for animation, we need to keep
  a seaprate value for each knob for each frame. We can do
  this by using an array of linked lists. Each array index
  will correspond to a frame (eg. knobs[0] would be the first
  frame, knobs[2] would be the 3rd frame and so on).

  Each index should contain a linked list of vary_nodes, each
  node contains a knob name, a value, and a pointer to the
  next node.

  Go through the opcode array, and when you find vary, go
  from knobs[0] to knobs[frames-1] and add (or modify) the
  vary_node corresponding to the given knob with the
  appropirate value.
  ====================*/
struct vary_node ** second_pass() {

    struct vary_node **knobs = calloc(num_frames, sizeof(struct vary_node));

    int i, frame;
    for (i=0;i<lastop;i++) {
        if (op[i].opcode == VARY) {
            printf("Vary: %4.0f %4.0f, %4.0f %4.0f",
                   op[i].op.vary.start_frame,
                   op[i].op.vary.end_frame,
                   op[i].op.vary.start_val,
                   op[i].op.vary.end_val);

            double start_frame = op[i].op.vary.start_frame;
            double end_frame = op[i].op.vary.end_frame;
            double start_val = op[i].op.vary.start_val;
            double end_val = op[i].op.vary.end_val;

            double d = (end_val - start_val) / (end_frame - start_frame);

            for (frame=start_frame; frame<=end_frame; frame++) {
                struct vary_node * new_knob = (struct vary_node*)malloc(sizeof(struct vary_node));
                strncpy(new_knob->name, op[i].op.vary.p->name, sizeof(new_knob->name));
                new_knob->value = start_val + (frame - start_frame) * d;
                new_knob->next = knobs[frame];
                knobs[frame] = new_knob;
            }
        }
    }

    return knobs;
}

/*======== void print_knobs() ==========
  Inputs:
  Returns:

  Goes through symtab and display all the knobs and their
  currnt values
  ====================*/
void print_knobs() {
    int i;

    printf( "ID\tNAME\t\tTYPE\t\tVALUE\n" );
    for ( i=0; i < lastsym; i++ ) {

        if ( symtab[i].type == SYM_VALUE ) {
            printf( "%d\t%s\t\t", i, symtab[i].name );

            printf( "SYM_VALUE\t");
            printf( "%6.2f\n", symtab[i].s.value);
        }
    }
}

/*======== void set_constants() ==========
  Inputs:
  Returns:

  Sets the reflection arrays to the values specified by the constant
  ====================*/
void set_constants(struct constants *c, double *a, double *d, double *s) {
    a[0] = c->r[0];
    a[1] = c->g[0];
    a[2] = c->b[0];
    d[0] = c->r[1];
    d[1] = c->g[1];
    d[2] = c->b[1];
    s[0] = c->r[2];
    s[1] = c->g[2];
    s[2] = c->b[2];
}

/*======== void reset_constants() ==========
  Inputs:
  Returns:

  Resets the reflection arrays to the default values
  ====================*/
void reset_constants(double *a, double *d, double *s, double *a_default, double *d_default, double *s_default) {
    a[0] = a_default[RED];
    a[1] = a_default[GREEN];
    a[2] = a_default[BLUE];
    d[0] = d_default[RED];
    d[1] = d_default[GREEN];
    d[2] = d_default[BLUE];
    s[0] = s_default[RED];
    s[1] = s_default[GREEN];
    s[2] = s_default[BLUE];
}


/*======== void my_main() ==========
  Inputs:
  Returns:

  This is the main engine of the interpreter, it should
  handle most of the commadns in mdl.

  If frames is not present in the source (and therefore
  num_frames is 1, then process_knobs should be called.

  If frames is present, the enitre op array must be
  applied frames time. At the end of each frame iteration
  save the current screen to a file named the
  provided basename plus a numeric string such that the
  files will be listed in order, then clear the screen and
  reset any other data structures that need it.

  Important note: you cannot just name your files in
  regular sequence, like pic0, pic1, pic2, pic3... if that
  is done, then pic1, pic10, pic11... will come before pic2
  and so on. In order to keep things clear, add leading 0s
  to the numeric portion of the name. If you use sprintf,
  you can use "%0xd" for this purpose. It will add at most
  x 0s in front of a number, if needed, so if used correctly,
  and x = 4, you would get numbers like 0001, 0002, 0011,
  0487
  ====================*/
void my_main() {

    struct matrix *tmp;
    struct stack *systems;
    screen t;
    zbuffer zb;
    color g;
    g.red = 0;
    g.green = 0;
    g.blue = 0;
    double step_3d = 20;
    double theta;
    double knob_value, xval, yval, zval;

    //Lighting values here for easy access
    struct constants *c;
    struct light *lights[MAX_LIGHTS];
    int num_lights;
    int current_light;
    color ambient;
    double view[3];
    double areflect[3];  // default
    double dreflect[3];  // default
    double sreflect[3];  // default
    double a[3], d[3], s[3];

    ambient.red = 50;
    ambient.green = 50;
    ambient.blue = 50;

    view[0] = 0;
    view[1] = 0;
    view[2] = 1;

    areflect[RED] = 0.1;
    areflect[GREEN] = 0.1;
    areflect[BLUE] = 0.1;

    dreflect[RED] = 0.5;
    dreflect[GREEN] = 0.5;
    dreflect[BLUE] = 0.5;

    sreflect[RED] = 0.5;
    sreflect[GREEN] = 0.5;
    sreflect[BLUE] = 0.5;

    c = (struct constants *)malloc(sizeof(struct constants));

    num_lights = find_light();
    current_light = 0;

    if (num_lights == 0) {
        lights[0] = (struct light *)malloc(sizeof(struct light));

        lights[0]->l[0] = 0.5;
        lights[0]->l[1] = 0.75;
        lights[0]->l[2] = 1;

        lights[0]->c[0] = 0;
        lights[0]->c[1] = 255;
        lights[0]->c[2] = 255;

        num_lights = 1;
    }

    systems = new_stack();
    tmp = new_matrix(4, 1000);
    clear_screen( t );
    clear_zbuffer(zb);

    first_pass();
    struct vary_node **knobs = second_pass();

    int frame;
    for (frame = 0; frame < num_frames; frame++) {
        printf("Frame: %d\n", frame);

        if (num_frames > 1) {
            struct vary_node * node = knobs[frame];

            while(node){
                set_value(lookup_symbol(node->name), node->value);
                node = node->next;
            }
        }

        int i;
        for (i=0;i<lastop;i++) {
            //printf("%d: ",i);

            switch (op[i].opcode)
                {
                case SPHERE:
                    /* printf("Sphere: %6.2f %6.2f %6.2f r=%6.2f", */
                    /* 	 op[i].op.sphere.d[0],op[i].op.sphere.d[1], */
                    /* 	 op[i].op.sphere.d[2], */
                    /* 	 op[i].op.sphere.r); */
                    reset_constants(a, d, s, areflect, dreflect, sreflect);
                    if (op[i].op.sphere.constants != NULL)
                        {
                            //printf("\tconstants: %s",op[i].op.sphere.constants->name);
                            c = lookup_symbol(op[i].op.sphere.constants->name)->s.c;
                            set_constants(c, a, d, s);
                        }
                    if (op[i].op.sphere.cs != NULL)
                        {
                            //printf("\tcs: %s",op[i].op.sphere.cs->name);
                        }
                    add_sphere(tmp, op[i].op.sphere.d[0],
                               op[i].op.sphere.d[1],
                               op[i].op.sphere.d[2],
                               op[i].op.sphere.r, step_3d);
                    matrix_mult( peek(systems), tmp );
                    draw_polygons(tmp, t, zb, view, lights, num_lights, ambient,
                                  a, d, s);
                    tmp->lastcol = 0;
                    break;
                case TORUS:
                    /* printf("Torus: %6.2f %6.2f %6.2f r0=%6.2f r1=%6.2f", */
                    /* 	 op[i].op.torus.d[0],op[i].op.torus.d[1], */
                    /* 	 op[i].op.torus.d[2], */
                    /* 	 op[i].op.torus.r0,op[i].op.torus.r1); */
                    reset_constants(a, d, s, areflect, dreflect, sreflect);
                    if (op[i].op.torus.constants != NULL)
                        {
                            //printf("\tconstants: %s",op[i].op.torus.constants->name);
                            c = lookup_symbol(op[i].op.torus.constants->name)->s.c;
                            set_constants(c, a, d, s);
                        }
                    if (op[i].op.torus.cs != NULL)
                        {
                            //printf("\tcs: %s",op[i].op.torus.cs->name);
                        }
                    add_torus(tmp,
                              op[i].op.torus.d[0],
                              op[i].op.torus.d[1],
                              op[i].op.torus.d[2],
                              op[i].op.torus.r0,op[i].op.torus.r1, step_3d);
                    matrix_mult( peek(systems), tmp );
                    draw_polygons(tmp, t, zb, view, lights, num_lights, ambient,
                                  a, d, s);
                    tmp->lastcol = 0;
                    break;
                case BOX:
                    /* printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f", */
                    /* 	 op[i].op.box.d0[0],op[i].op.box.d0[1], */
                    /* 	 op[i].op.box.d0[2], */
                    /* 	 op[i].op.box.d1[0],op[i].op.box.d1[1], */
                    /* 	 op[i].op.box.d1[2]); */
                    reset_constants(a, d, s, areflect, dreflect, sreflect);
                    if (op[i].op.box.constants != NULL)
                        {
                            //printf("\tconstants: %s",op[i].op.box.constants->name);
                            c = lookup_symbol(op[i].op.box.constants->name)->s.c;
                            set_constants(c, a, d, s);
                        }
                    if (op[i].op.box.cs != NULL)
                        {
                            //printf("\tcs: %s",op[i].op.box.cs->name);
                        }
                    add_box(tmp,
                            op[i].op.box.d0[0],op[i].op.box.d0[1],
                            op[i].op.box.d0[2],
                            op[i].op.box.d1[0],op[i].op.box.d1[1],
                            op[i].op.box.d1[2]);
                    matrix_mult( peek(systems), tmp );
                    draw_polygons(tmp, t, zb, view, lights, num_lights, ambient,
                                  a, d, s);
                    tmp->lastcol = 0;
                    break;
                case LINE:
                    /* printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",*/
                    /* 	 op[i].op.line.p0[0],op[i].op.line.p0[1], */
                    /* 	 op[i].op.line.p0[1], */
                    /* 	 op[i].op.line.p1[0],op[i].op.line.p1[1], */
                    /* 	 op[i].op.line.p1[1]); */
                    reset_constants(a, d, s, areflect, dreflect, sreflect);
                    if (op[i].op.line.constants != NULL)
                        {
                            //printf("\n\tConstants: %s",op[i].op.line.constants->name);
                            c = lookup_symbol(op[i].op.line.constants->name)->s.c;
                            set_constants(c, a, d, s);
                        }
                    if (op[i].op.line.cs0 != NULL)
                        {
                            //printf("\n\tCS0: %s",op[i].op.line.cs0->name);
                        }
                    if (op[i].op.line.cs1 != NULL)
                        {
                            //printf("\n\tCS1: %s",op[i].op.line.cs1->name);
                        }
                    add_edge(tmp,
                             op[i].op.line.p0[0],op[i].op.line.p0[1],
                             op[i].op.line.p0[2],
                             op[i].op.line.p1[0],op[i].op.line.p1[1],
                             op[i].op.line.p1[2]);
                    matrix_mult( peek(systems), tmp );
                    draw_lines(tmp, t, zb, g);
                    tmp->lastcol = 0;
                    break;
                case MOVE:
                    xval = op[i].op.move.d[0];
                    yval = op[i].op.move.d[1];
                    zval = op[i].op.move.d[2];

                    if (op[i].op.move.p != NULL)
                        {
                            printf("\tknob: %s",op[i].op.move.p->name);
                            if (num_frames > 1) {
                                struct vary_node *knob = knobs[frame];
                                while(strcmp(knob->name, op[i].op.move.p->name) && knob){
                                    knob = knob->next;
                                }
                                knob_value = knob->value;
                                xval *= knob_value;
                                yval *= knob_value;
                                zval *= knob_value;
                            }
                        }
                    printf("Move: %6.2f %6.2f %6.2f",
                           xval, yval, zval);

                    tmp = make_translate( xval, yval, zval );
                    matrix_mult(peek(systems), tmp);
                    copy_matrix(tmp, peek(systems));
                    tmp->lastcol = 0;
                    break;
                case SCALE:
                    xval = op[i].op.scale.d[0];
                    yval = op[i].op.scale.d[1];
                    zval = op[i].op.scale.d[2];

                    if (op[i].op.scale.p != NULL)
                        {
                            printf("\tknob: %s",op[i].op.scale.p->name);
                            if (num_frames > 1) {
                                struct vary_node *knob = knobs[frame];
                                while(strcmp(knob->name, op[i].op.scale.p->name) && knob){
                                    knob = knob->next;
                                }

                                knob_value = knob->value;
                                xval *= knob_value;
                                yval *= knob_value;
                                zval *= knob_value;
                            }
                        }
                    printf("Scale: %6.2f %6.2f %6.2f",
                           xval, yval, zval);

                    tmp = make_scale( xval, yval, zval );
                    matrix_mult(peek(systems), tmp);
                    copy_matrix(tmp, peek(systems));
                    tmp->lastcol = 0;
                    break;
                case ROTATE:
                    xval = op[i].op.rotate.axis;
                    theta = op[i].op.rotate.degrees;

                    if (op[i].op.rotate.p != NULL)
                        {
                            printf("\tknob: %s",op[i].op.rotate.p->name);
                            if (num_frames > 1) {
                                struct vary_node *knob = knobs[frame];
                                while(strcmp(knob->name, op[i].op.rotate.p->name) && knob){
                                    knob = knob->next;
                                }

                                knob_value = knob->value;
                                theta *= knob_value;
                            }
                        }
                    printf("Rotate: axis: %6.2f degrees: %6.2f",
                           xval, theta);

                    theta*= (M_PI / 180);
                    if (op[i].op.rotate.axis == 0 )
                        tmp = make_rotX( theta );
                    else if (op[i].op.rotate.axis == 1 )
                        tmp = make_rotY( theta );
                    else
                        tmp = make_rotZ( theta );

                    matrix_mult(peek(systems), tmp);
                    copy_matrix(tmp, peek(systems));
                    tmp->lastcol = 0;
                    break;
                case PUSH:
                    //printf("Push");
                    push(systems);
                    break;
                case POP:
                    //printf("Pop");
                    pop(systems);
                    break;
                case AMBIENT:
                    ambient.red = op[i].op.ambient.c[0];
                    ambient.green = op[i].op.ambient.c[1];
                    ambient.blue = op[i].op.ambient.c[2];
                    break;
                case LIGHT:
                    if (current_light < num_lights) {
                        struct light *l = (struct light *)malloc(sizeof(struct light));
                        l = lookup_symbol(op[i].op.light.p->name)->s.l;
                        lights[current_light] = l;
                        current_light++;
                    }
                    break;
                case CONSTANTS:
                    break;
                case MESH:
                    reset_constants(a, d, s, areflect, dreflect, sreflect);
                    if (op[i].op.mesh.constants != NULL)
                        {
                            //printf("\n\tConstants: %s",op[i].op.line.constants->name);
                            c = lookup_symbol(op[i].op.mesh.constants->name)->s.c;
                            set_constants(c, a, d, s);
                        }
                    tmp = parse_mesh(op[i].op.mesh.name);
                    matrix_mult(peek(systems), tmp);
                    draw_polygons(tmp, t, zb, view, lights, num_lights, ambient,
                                  a, d, s);
                    tmp->lastcol = 0;
                    break;
                case SAVE:
                    //printf("Save: %s",op[i].op.save.p->name);
                    save_extension(t, op[i].op.save.p->name);
                    break;
                case DISPLAY:
                    //printf("Display");
                    display(t);
                    break;
                } //end opcode switch
            printf("\n");
        }//end operation loop

        if (num_frames > 1) {
            char pic_name[128];
            sprintf(pic_name, "anim/%s%03d.png", name, frame);
            save_extension(t, pic_name);
            systems = new_stack();
            tmp = new_matrix(4, 1000);
            clear_screen(t);
            clear_zbuffer(zb);
        }

    }

    if (num_frames > 1) {
        make_animation(name);
    }

    free(knobs);
}
