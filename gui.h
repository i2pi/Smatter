#ifndef __GUI_H__
#define __GUI_H__

#ifndef _APPLE
	#include <GL/glut.h>    
	#include <GL/gl.h>	
	#include <GL/glu.h>	
#else
	#include <GLUT/glut.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#endif
#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <openssl/rand.h>

#include "data.h"

#define GUI_MSG_LEN	256


typedef struct gui_stateT
{
	// Config

	char	jitter;
	char	hide_axes;

	float	plot_padding;
	float	axis_padding;

	struct {
		GLfloat	fg[4];
		GLfloat	bg[4];
		GLfloat	scrim[4];
		GLfloat	axis[4];
		GLfloat	faint[4];
		GLfloat	point[4];
		GLfloat	fit[4];
		GLfloat	marked[4];

		GLfloat	gradient_xmin[4];
		GLfloat	gradient_xmax[4];
		GLfloat	gradient_ymax[4];
		GLfloat	gradient_ymin[4];
	} color;

	unsigned long	max_points;

	int	stipple;

	// Internal state

	char	zoom;
	char	gradient;
	char	transform;
	char	help;

	char	need_redraw;

	double	box_size;
	double 	status_bar_height;

	float	scale;

	int		first_display_row;

	char	message[GUI_MSG_LEN];

	frameT	*frame;
	int window; 
	int	w, h;
	int	cur_i, cur_j;
	int	selected_i, selected_j;

	int		pointer_i, pointer_j;
	double	pointer_x, pointer_y;	// These are all in local coordinates for pointer (i,j)
	int		region_i, region_j;
	double	region_x, region_y;
	double	region_w, region_h;

	struct {
		double x, y;
		int	button, state;
	} mouse;
} gui_stateT;

extern gui_stateT	gui_state;

void init_gui(int *argc, char **argv, int Width, int Height, frameT *frame);

#endif

