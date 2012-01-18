
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
#include <ctype.h>
#include <time.h>
#include <limits.h>

#include "data.h"
#include "gui.h"

#define ESCAPE 27

#undef _TIMING		// Use for calculating FPS

#ifdef _TIMING
#include <sys/time.h>
#endif

#define REDRAW_MSEC	30

gui_stateT	gui_state;

void resize_gui(int Width, int Height)
{
  if (Height==0)				
    Height=1;

  glViewport(0, 0, Width, Height);		
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);
  glMatrixMode(GL_MODELVIEW);

	gui_state.w = Width;
	gui_state.h = Height;
}

void	print (GLfloat x, GLfloat y, char *text)
{
	char	*p;

	glColor4fv(gui_state.color.fg);
	glRasterPos2d(x,y);
	for (p=text; *p; p++)
	{
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);
	}
}

void	print_color (GLfloat c[], GLfloat x, GLfloat y, char *text)
{
	char	*p;

	glColor4fv(c);
	glRasterPos2d(x,y);
	for (p=text; *p; p++)
	{
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);
	}
}



void	print_right (GLfloat x, GLfloat y, char *text)
{
	char	*p;
	double fw = 8 / (double) gui_state.w;

	int		len = strlen (text);

	x = x - len * fw;

	glColor4fv(gui_state.color.fg);
	glRasterPos2d(x,y);
	for (p=text; *p; p++)
	{
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);
	}
}

void	print_in_box (GLfloat x, GLfloat y, GLfloat w, GLfloat h, char *text)
{
	// Font dimensions in screen units, not pixels
	double fw = 8 / (double) gui_state.w;
	double fh = 13 / (double) gui_state.h;

	double 	tw, th;	// Text width
	int		i, pi, j, len;
	int		lines;
	
	len = strlen (text);


	lines = 1;
	for (i=0; i<len; i++) if (!isalnum(text[i])) lines++;

	th = (h - (lines - 1) * fh) / 2;

	pi = 0;
	for (i=0; i<len+1; i++)
	{
		if (!isalnum(text[i]))
		{
			tw = (i - pi) * fw;
			glColor4fv(gui_state.color.fg);
			glRasterPos2d(x + (w-tw)/2, y + th);
			for (j=pi; j<i; j++)
			{
				glutBitmapCharacter(GLUT_BITMAP_8_BY_13, text[j]);
			}
			pi = i+1;
			th += fh * (1.01);
		}
	}	
}


double normalize_data_stats (statsT *s, double x)
{
	return (x - s->min) / (s->max -  s->min);
}

double normalize_data (columnT *c, double x)
{
	statsT	*s = get_stats(c);
	return (x - s->min) / (s->max -  s->min);
}

double denormalize_data (columnT *c, double x)
{
	statsT	*s = get_stats(c);
	return x * (s->max -  s->min) + s->min; 
}

double	get_norm_row (columnT *c, unsigned long row)
{
	return normalize_data (c, get_data(c)[row]);
}

double	limit (double x)
{
	return (x < 0) ? 0 : ((x > 1) ? 1 : x);
}

void	gl_box (GLfloat *c, double X, double Y, double W, double H, char is_filled)
{
	if (!is_filled)
	{
		glBegin(GL_LINE_LOOP);
	} else
	{
		glBegin(GL_POLYGON);
	}
	glColor4fv(c);
	glVertex2d(X,Y);
	glVertex2d(X+W,Y);
	glVertex2d(X+W,Y+H);
	glVertex2d(X,Y+H);
	glEnd();
}

void	gl_stipple_box (GLfloat *c, double x, double y, double w, double h, char is_filled)
{
	if (is_filled) gl_box (c,x,y,w,h,1);
	glLineStipple(1,gui_state.stipple);
	gl_box (c,x,y,w,h,0);
	glLineStipple(1,0xFFFF);
}

void	gl_line (GLfloat *c, double X, double Y, double X1, double Y1)
{
	glBegin(GL_LINES);
	glColor4fv(c);
	glVertex2d(X,Y);
	glVertex2d(X1,Y1);
	glEnd();
}

void	gl_stipple_line (GLfloat *c, double X, double Y, double X1, double Y1)
{
	glLineStipple (1, gui_state.stipple);
	glBegin(GL_LINES);
	glColor4fv(c);
	glVertex2d(X,Y);
	glVertex2d(X1,Y1);
	glEnd();
	glLineStipple (1, 0xFFFF);
}

void	gl_h_line (GLfloat *c, double X, double Y, double X1)
{
	gl_line (c, X,Y,  X1,Y);
}

void	gl_v_line (GLfloat *c, double X, double Y, double Y1)
{
	gl_line (c, X,Y,  X,Y1);
}

void	gl_h_stipple_line (GLfloat *c, double X, double Y, double X1)
{
	gl_stipple_line (c, X,Y,  X1,Y);
}

void	gl_v_stipple_line (GLfloat *c, double X, double Y, double Y1)
{
	gl_stipple_line (c, X,Y,  X,Y1);
}

double	reference_cdf (double x)
{
	// TODO: Placeholder function - later to be replaced with a reference cdf :)

	return x;
}

void	print_transforms (frameT *f, int i, double x, double y, double w, double h)
{
		// TODO, make sure this doesn't overflow
	
		double fw = 8 / (double) gui_state.w;
		double fh = 13 / (double) gui_state.h;
		double	start_x = x;

		transformT	*t = f->column[i]->transform;

		while (t)
		{
			print (x,y, t->name);
			x += strlen (t->name)*fw + w*0.01;
			print_color (gui_state.color.marked, x,y, ">");
			x += fw + w*0.01;
			if (x >= w*0.7) 
			{
				x = start_x;		
				y = y + fh * 1.25;
			}

			t = t->prev;
		}
		print (x,y, "original");
}

void	plot_stats (frameT *f, int i, double x, double y, double w, double h)
{
	histogramT	*g;
	statsT		*s;
	float		u,v,t, pX, X,Y,W,H, maxH, minY;
	int			b;
	double		max_break, min_break;
	double		prev_sum, cum_sum;
	double		prc, rc;
	GLfloat		fill_color[4];
	
	memcpy(&fill_color, gui_state.color.point, sizeof (GLfloat) * 4);
	fill_color[3] = 0.1;

	minY = y + h*0.9;
	maxH = h * 0.8;
 
	s = get_stats(f->column[i]);
	g = &s->histogram;	

	max_break = g->breaks[g->bins-1];
	min_break = g->breaks[0];

	cum_sum = 0;
	rc = reference_cdf (1 / (double) g->bins);
	for (b=0; b<g->bins; b++)
	{
		
		u = (b+0.5) / (g->bins + 1.0);
		v = (b+1.5) / (g->bins + 1.0);
		t = g->counts[b] / (double) g->max_count;

		prev_sum = cum_sum;
		cum_sum += (g->counts[b] / (double) f->rows);

		pX = X + W/2.0;
		X = x + u*w;
		W = (v-u) * w * (1 - 100.0/(double)gui_state.w);
		Y = minY;
		H = maxH * t;

		gl_box (gui_state.color.point, X,Y, W,-H, 0);
		gl_box (fill_color, X,Y, W,-H, 1);

		if (b > 0)
		{		
			// CDF
			prc = rc;
			rc = reference_cdf ((b+1) / (double) g->bins);
			gl_line (gui_state.color.axis, pX, Y - prev_sum * h * 0.8, X + W/2.0, Y - cum_sum * h * 0.8);
			gl_stipple_line (gui_state.color.fit, pX, Y - prc *h * 0.8, X + W/2.0, Y - rc*h*0.8);
		}
	}
	// Median
	gl_h_line (gui_state.color.axis, x,minY-maxH/2,x+w);
	gl_h_stipple_line (gui_state.color.axis, x, minY - 3*maxH/4,  x+w);
	gl_h_stipple_line (gui_state.color.axis, x, minY - maxH/4,  x+w);

	// Mean
	gl_v_stipple_line (gui_state.color.axis, x + w*(s->mean - s->min)/(s->max - s->min), y, y+h);
	gl_v_stipple_line (gui_state.color.axis, x + w*(s->mean - s->min - s->stddev)/(s->max - s->min), y, y+h);
	gl_v_stipple_line (gui_state.color.axis, x + w*(s->mean - s->min + s->stddev)/(s->max - s->min), y, y+h);

	print_right (x+w*0.99, y+h*0.99, gui_state.frame->column[i]->name);

	print_transforms (f,i, x+w*0.01, y+h*0.05, w*0.99, h*0.95);
}

void	set_mixed_color (GLfloat a[], GLfloat b[], float mix)
{
	GLfloat	c[4];
	int		i;

	for (i=0; i<4; i++)	c[i] = a[i] * mix + b[i] * (1.0 - mix);
	glColor4fv (c);
}

void	set_mixed_4color (GLfloat xa[], GLfloat xb[], GLfloat ya[], GLfloat yb[], float x, float y)
{
	GLfloat	c[4];
	int		i;

	for (i=0; i<4; i++)	c[i] = ((xa[i] * x + xb[i] * (1.0 - x)) + (ya[i] * y + yb[i] * (1.0 - y))) / 2.0;

	glColor4fv (c);
}

double	plot_to_screen_x (double px)
{
	// convert plot coordinates which have (0,0) in the bottom left to
	// screen coordinates, which has (0,0) on the top left, and indented by
	// the axis padding.

	float	a = gui_state.axis_padding;
	return limit(px) * (1 - a*2) + a;
}

double	plot_to_screen_y (double py)
{
	float	a = gui_state.axis_padding;
	return limit(1 - py) * (1 - a*2) + a;
}

double	bsearch_near (double v, double *data, unsigned int n)
{
	// Like a regular bsearch, but will return the nearest value if not found.

	unsigned int	i,l,u;

	l = 0;
	u = n;
	while (l < u)
	{
		i = (l + u) / 2;
		if (v < data[i])
			u = i;
		else if (v > data[i])
			l = i+1;
		else return data[i]; 
	}

	if (i >= n) i = n-1;

	return data[i];
}

double	get_cdf_by_value (statsT *s, double v)
{
	histogramT	*h = &s->histogram;
	return	bsearch_near (v, h->cdf,  h->bins);
}

double	gradient (int col, int row)
{
	double	g;
	frameT	*f = gui_state.frame;
	statsT	*s = get_stats (f->column[col]);
	// TODO: I'm not 100% happy with the coloring algo. Need to think about it.

	if (s->histogram.bins == DEFAULT_HIST_BINS)
	{
		g = get_cdf_by_value (s, get_data(f->column[col])[row]);
	} else
	{
		// Low cardinality, therefore linear coloring
		g = normalize_data (f->column[col], get_data(f->column[col])[row]);
	}

	return g;
}

void	plot (frameT *f, int i, int j, double x, double y, double w, double h, double pct)
{
	int		k, max_k;
	double	x1,y1, x2,y2;
	double	u,v;
	double	a;		// % of width or height to be used for axes
	statsT	*s;
	double	q[3];
	double	mean, sp, sm;
	double	jit;		// Jitter %

	jit = (gui_state.jitter * gui_state.jitter) * 0.015;

	a = gui_state.axis_padding;

	if (!gui_state.hide_axes)
	{
		// Y-Axis
		s = get_stats (f->column[j]);
		for (k=0; k<3; k++) q[k] = normalize_data(f->column[j], s->quartile[k]);
		mean = normalize_data(f->column[j], s->mean);
		sp = normalize_data(f->column[j], s->mean + s->stddev);
		sm = normalize_data(f->column[j], s->mean - s->stddev);

		// IQR
		y1 = y + plot_to_screen_y (q[0]) * h;
		y2 = y + plot_to_screen_y (q[2]) * h;
		gl_v_line (gui_state.color.axis, x+w*a/2,  y1, y2);
		y1 = y + plot_to_screen_y (q[1]) * h;
		gl_h_line (gui_state.color.axis, x, y1, x+w*a/2);
		// Mean	
		x1 = x + w*a/2;
		x2 = x + w*(1-a/2);
		y1 = y + plot_to_screen_y(mean) * h;
		gl_h_line (gui_state.color.faint, x1,y1, x2);
		y1 = y + plot_to_screen_y(sp) * h;
		gl_h_stipple_line (gui_state.color.faint, x1, y1, x2);
		y1 = y + plot_to_screen_y(sm) * h;
		gl_h_stipple_line (gui_state.color.faint, x1, y1, x2);
	
		// X-Axis
		s = get_stats (f->column[i]);
		for (k=0; k<3; k++) q[k] = normalize_data(f->column[i], s->quartile[k]);
		mean = normalize_data(f->column[i], s->mean);
		sp = normalize_data(f->column[i], s->mean + s->stddev);
		sm = normalize_data(f->column[i], s->mean - s->stddev);

		// IQR
		x1 = x + plot_to_screen_x (q[0]) * w;
		x2 = x + plot_to_screen_x (q[2]) * w;
		gl_h_line (gui_state.color.axis, x1, y+h*(1-a/2), x2);
		x1 = x + plot_to_screen_x (q[1]) * w;
		gl_v_line (gui_state.color.axis, x1, y+h, y+h*(1-a/2));
		// Mean	
		y1 = y + h*a/2;
		y2 = y + h*(1-a/2);
		x1 = x + plot_to_screen_x(mean) * w;
		gl_v_line (gui_state.color.faint, x1,y1, y2);
		x1 = x + plot_to_screen_x(sm) * w;
		gl_v_stipple_line (gui_state.color.faint, x1,y1, y2);
		x1 = x + plot_to_screen_x(sp) * w;
		gl_v_stipple_line (gui_state.color.faint, x1,y1, y2);
	}

	max_k = (f->rows < gui_state.max_points ? f->rows : gui_state.max_points) * pct;

	glBegin(GL_POINTS);
	for (k=0; k<max_k; k++)
	{
		unsigned int	l;
		
		l = (k + gui_state.first_display_row) % f->rows;

		switch (gui_state.gradient)
		{
			case 1: // Nearest neighbour
				set_mixed_color (gui_state.color.gradient_xmin, gui_state.color.gradient_xmax, gui_state.frame->nn_distance[l]);
				break;
			case 2:	// X gradient
				u = gradient (gui_state.selected_i, l);
				set_mixed_color (gui_state.color.gradient_xmin, gui_state.color.gradient_xmax, u);
				break;
			case 3:	// Y gradient
				u = gradient (gui_state.selected_j, l);
				set_mixed_color (gui_state.color.gradient_ymin, gui_state.color.gradient_ymax, u);
				break;
			case 4:	// X-Y gradient
				u = gradient (gui_state.selected_i, l);
				v = gradient (gui_state.selected_j, l);
				set_mixed_4color (gui_state.color.gradient_xmin, gui_state.color.gradient_xmax,
				                gui_state.color.gradient_ymin, gui_state.color.gradient_ymax, u, v);
				break;
	
			default: glColor4fv (gui_state.color.point);
		};

		// TODO: not sure why valgrind complains about this.
		if (f->region_rows && f->region_rows[l]) glColor4fv(gui_state.color.marked);
		
		u = get_norm_row(f->column[i], l) + (jit * ((random() / (double) RAND_MAX)-0.5));
		v = get_norm_row(f->column[j], l) + (jit * ((random() / (double)  RAND_MAX)-0.5));
		u = x + plot_to_screen_x (u) * w;
		v = y + plot_to_screen_y (v) * h;
		glVertex2d(u,v);	
	}

	glEnd();
	if ((gui_state.region_i == i) && (gui_state.region_j == j))
	{
		
		float	uu, vv;
		u = x + gui_state.region_x * w;
		v = y + gui_state.region_y * h;

		uu = gui_state.region_w * w;
		vv = gui_state.region_h * h;

		gl_box (gui_state.color.marked, u,v, uu, vv, 0);
	}
}

#define HELP_STRING_LEN 32

void	draw_help_window ()
{
	float	x,y, box;
	int		i;
	char	help[][HELP_STRING_LEN] = 
		{"*General", "",
		   	"ESC", "Quit",
		  	"h", "Help",
			"e", "Export",
		"*Navigation", "",
			"HOME", "Jump left",
			"END",	"Jump right",
			"PAGE UP", "Jump up",
			"PAGE DOWN", "Jump down",
			"Cursors", "Move around",
			"f", "Flip X<>Y",
			"x", "Jump to X",
			"y", "Jump to Y",
			"ENTER", "Zoom", 
		"*Transforms", "",
			"t", "Transform",
			"u", "Undo",
		"*Display","",
			"a", "Hide/Show axes",
			"g", "Gradient modes",
			"j", "Jitter",
		"*Data","",
			"-", "Draw fewer points",
			"+", "Draw more points",
			"l", "Load more data",
			"r", "Reselect points"
		};
	
		 
	int		help_items = sizeof(help) / HELP_STRING_LEN;
	float	Y, X;
	double fw = 8 / (double) gui_state.w;
	double fh = 13 / (double) gui_state.h;
	int		maxw;

	x = 0.1;
	y = x;
	box = 1 - 2*x;

	gl_box (gui_state.color.scrim, 0,0, 1,1, 1);
	gl_box (gui_state.color.bg, x,y, box,box, 1);
	gl_box (gui_state.color.fg, x,y, box,box, 0);

	Y = y + box*0.05;
	X = x + box*0.05;

	maxw = 0;

	for (i=0; i<help_items; i+=2)
	{
		int	len;
		if (help[i][0] != '*')
		{
			print_color (gui_state.color.marked, X, Y, help[i]);
			print (X + (strlen(help[i])+1)*fw, Y, help[i+1]);
			len = 1 + strlen(help[i]) + strlen(help[i+1]);
		} else
		{
			if (Y > y + box*0.6) 
			{
				Y = y + box*0.05;
				X += fw * (maxw + 3);
				maxw = 0;
			}


			if (Y != y+box*0.05) Y += fh;
			print_color (gui_state.color.axis, X, Y, &help[i][1]);
			len = strlen(help[i]);
		}
		if (len >= maxw) maxw = len;

		Y += fh*1.5;
		if (Y > y + box*0.95) 
		{
			Y = y + box*0.05;
			X += fw * (maxw + 3);
			maxw = 0;
		}
	}
}



void	draw_transform_window ()
{
	float	x,y, box;
	int		i;
	char	key[2];
	float	Y;
	double fw = 8 / (double) gui_state.w;
	double fh = 13 / (double) gui_state.h;

	x = 0.2;
	y = x;
	box = 1 - 2*x;

	gl_box (gui_state.color.scrim, 0,0, 1,1, 1);
	
	gl_box (gui_state.color.fg, x,y, box,box, 0);

	key[1] = '\0';
	for (i=0; i<transforms; i++)
	{
		key[0] = transform[i].key;

		Y = y + box*0.05 + i*fh*1.5;

		print_color (gui_state.color.marked, x + box*0.05, Y, key);
		print (x + box*0.05 + 2*fw, Y, transform[i].name);
	}
	Y += fh*2.5;
	key[0] = 'u';
	print_color (gui_state.color.marked, x + box*0.05, Y, key);
	print (x + box*0.05 + 2*fw, Y, "undo last transform");
}

void	draw_zoomed ()
{
	float	x,y, box;

	x = 0.2;
	y = x;
	box = 1 - 2*x;

	gl_box (gui_state.color.scrim, 0,0, 1,1, 1);
	
	gl_box (gui_state.color.bg, x,y, box,box, 1);
	gl_box (gui_state.color.fg, x,y, box,box, 0);

	if (gui_state.cur_i != gui_state.cur_j)
	{
		plot (gui_state.frame, gui_state.cur_i, gui_state.cur_j, x,y, box, box, 1.0);
		print_right (x+box*0.99, y+box*0.99, gui_state.frame->column[gui_state.cur_i]->name);
		print (x+box*0.01, y+box*0.05, gui_state.frame->column[gui_state.cur_j]->name);
	} else
	{
		plot_stats (gui_state.frame, gui_state.cur_i, x,y, box, box);
	}

}

void	prog_bar (double x, double y, double w, double h, double a)
{	
	gl_box (gui_state.color.point, x,y, w*a,h, 1);
	gl_box (gui_state.color.axis, x,y, w,h, 0);
}

void status_bar ()
{
	double	x,y, w,h;
	double	a,b;
	frameT	*f = gui_state.frame;

	// Show the % of estimated rows that have been loaded

	x = 0.9;
	y = 1.00;
	w = 0.08;
	h = gui_state.status_bar_height;
	a = gui_state.frame->rows / (double) gui_state.frame->csv->est_rows;

	prog_bar (x,y, w,2*h/5.0, a);

	// And of these rows, what % are we displaying ?

	b = (f->rows < gui_state.max_points ? f->rows : gui_state.max_points) / (double) gui_state.frame->rows;

	prog_bar (x,y+3*h/5.0, w,2*h/5.0, b);

	if ((gui_state.message[0] == '@') || (gui_state.message[0] == '\0'))
	{
		columnT	*c;
		char	*i_name, *j_name;
		char	i_val[256], j_val[256];
		double	px, py;

		c = gui_state.frame->column[gui_state.pointer_i];
		i_name = c->name;
		px = denormalize_data(c, gui_state.pointer_x);
		c->type.to_string (&c->type, &px, i_val);

		c = gui_state.frame->column[gui_state.pointer_j];
		j_name = c->name;
		py = denormalize_data(c, gui_state.pointer_y);
		c->type.to_string (&c->type, &py, j_val);


		snprintf (gui_state.message, GUI_MSG_LEN, "@ %s: %s, %s: %s",
				i_name, i_val,
				j_name, j_val);
	}
	print (0.05, 1.0 + gui_state.status_bar_height/2, gui_state.message);
	
}

void draw_gui()
{
	int		ncol = gui_state.frame->columns;
	frameT	*f = gui_state.frame;
	double	pad = gui_state.plot_padding;
	double	box, p;
	int		i, j;
	double	x,y;
	

#ifdef _TIMING
	static int		frames=0;
	static double	time_taken=0;
	double	start_time;
	double	end_time;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	start_time = (double)tv.tv_sec + (double)tv.tv_usec/1.0e6;
#endif


	gui_state.box_size = 1.0 / ((double)ncol * (1.0 + 2.0*pad) );
	box = gui_state.box_size;
	p = box * pad;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				
	gluOrtho2D(0, 1, -0.05, 1);


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0,1,0);	// Make (0,0) in top left
	glScalef(gui_state.scale,-gui_state.scale,1);

	if (gui_state.gradient == 1) // Nearest neighbor
	{
		double px, py;
		px = denormalize_data(f->column[gui_state.pointer_i], gui_state.pointer_x);
		py = denormalize_data(f->column[gui_state.pointer_j], 1.0 - (gui_state.pointer_y));

		nearest_neighbor (gui_state.frame, gui_state.pointer_i, gui_state.pointer_j, px, py);
	}

	gl_box (gui_state.color.bg, 0,0, 1,1+gui_state.status_bar_height, 1);

	for (j=0; j<ncol; j++)
	for (i=0; i<ncol; i++)
	{
		x = i * (box + 2.0*p) + p;
		y = j * (box + 2.0*p) + p ;
	
		if ((i == gui_state.cur_i) && (j == gui_state.cur_j))
		{
			gl_box(gui_state.color.axis, x,y,  box, box, 0);
		} 

		if ((i == gui_state.region_i) && (j == gui_state.region_j))
		{
			gl_stipple_box (gui_state.color.marked, x,y, box,box, 0);
		} 
	
		if (i == j)
		{	
			print_in_box (x,y, box, box, f->column[i]->name);
		} else
		{
			// TODO, should use a bitmap for the (j,i) after drawing (i,j)
			plot (f, i, j, x, y, box, box, gui_state.zoom ? 0.1 : 1.0);
		}
	}

	if (gui_state.zoom)	 draw_zoomed();
	if (gui_state.transform) draw_transform_window (); 
	if (gui_state.help) draw_help_window (); 


	status_bar();


	glPopMatrix();
	glutSwapBuffers();

	gui_state.need_redraw = 0;

#ifdef _TIMING
	gettimeofday(&tv, NULL);
	end_time = (double)tv.tv_sec + (double)tv.tv_usec/1.0e6;
	time_taken += (end_time - start_time);
	frames++;
	if (time_taken > 5)
	{
		sprintf (gui_state.message, "%7.4f",frames / time_taken);
		frames = 0;
		time_taken = 0;
	}
#endif
}

void	draw_gui_timer (int v)
{
	if (gui_state.need_redraw)	draw_gui();
	glutTimerFunc (REDRAW_MSEC, draw_gui_timer, 0);
}

void	finish ()
{
	glutDestroyWindow(gui_state.window); 
	exit(0);                   
}

void keyPressed(unsigned char key, int x, int y) 
{
	usleep(100);

	gui_state.message[0] = '\0';

	key = tolower (key);

	if (gui_state.transform)
	{
		if (key == 27) finish();
		if (key == 'u')
		{
			column_pop_transform (gui_state.frame, gui_state.cur_i);
			gui_state.transform = 0;
		} else
		if (key == 't') gui_state.transform = 0; else
		{
			int	i;
			for (i=0; i<transforms; i++) if (transform[i].key == key) break;
			if (i < transforms)
			{
				transformT *t = clone_transform (&transform[i]);
				column_add_transform (gui_state.frame->column[gui_state.cur_i], t);
				column_apply_transforms (gui_state.frame, gui_state.cur_i);
				gui_state.transform = 0;
			} else
			{
				snprintf (gui_state.message, GUI_MSG_LEN, "nope. thats not a valid tranform key. press 't' to leave this mode.");
			}
		}	
	} else
	switch (key)
	{
		case 27: finish(); break;

		case 'u':
		if (gui_state.cur_i == gui_state.cur_j)
			column_pop_transform (gui_state.frame, gui_state.cur_i);
		else
			snprintf (gui_state.message, GUI_MSG_LEN, "can only undo transforms from a diagonal");	
		break;


		case 'f':
		{
			int	t;
			t = gui_state.cur_i;
			gui_state.cur_i = gui_state.cur_j;
			gui_state.cur_j = t;
		}; break; 


		case 't': 
		if (gui_state.cur_i == gui_state.cur_j) gui_state.transform = 1; else
		snprintf (gui_state.message, GUI_MSG_LEN, "need to be on a diagonal to invoke transforms");
		break;

		case 'h': gui_state.help = (gui_state.help == 0) ? 1 : 0; break;

		case 13: gui_state.zoom = gui_state.zoom ? 0 : 1; break;		

		case 'j': gui_state.jitter = (gui_state.jitter+1) % 3; break;

		case 'g': 
			gui_state.gradient = (gui_state.gradient+1) % 5; 
			gui_state.selected_i = gui_state.cur_i;
			gui_state.selected_j = gui_state.cur_j;
			break;

		case 'r': gui_state.first_display_row = (gui_state.first_display_row += gui_state.max_points / 3.0) < gui_state.frame->rows ? gui_state.first_display_row  : 0; break;

		case '-': 
		case '_': 
			gui_state.max_points -= 1000; 
			if (gui_state.max_points < 1000) gui_state.max_points = 1000; 
			break;

		case '=': 
		case '+': 
			gui_state.max_points += 1000; break;
			if (gui_state.max_points > gui_state.frame->rows) gui_state.max_points = gui_state.frame->rows;
			break;

		case 'a': gui_state.hide_axes = gui_state.hide_axes ? 0 : 1; break;

		case 'x': gui_state.cur_j = gui_state.cur_i; break;
		case 'y': gui_state.cur_i = gui_state.cur_j; break;

		case '<':
		case ',':
			gui_state.scale += 0.1;
			if (gui_state.scale > 10) gui_state.scale = 10;
			break;
		case '>':
		case '.':
			gui_state.scale -= 0.1;
			if (gui_state.scale <1.0) gui_state.scale = 1.0; 
			break;
		case '/':
		case '?':
			gui_state.scale = 1.0;
			break;

		case 'l': 
		{
			load_random_rows (gui_state.frame, 0.5);
		}; break;

		case 'e': 
		{
			char	name[4096];
			if (!export_frame(gui_state.frame, name))
			{
				snprintf (gui_state.message, GUI_MSG_LEN, "exported %s OK", name);
			} else
			{
				snprintf (gui_state.message, GUI_MSG_LEN, "export failed :(");
			}
		}; break;
	}

	gui_state.need_redraw = 1;
}

void specialPressed(int key, int x, int y) 
{
	usleep (100);

	gui_state.message[0] = '\0';

	// transforms have to be applied while cur_i = cur_j, so don't
	// let them move about while transform mode enabled.
	if (gui_state.transform) return;

	switch (key)
	{
		case GLUT_KEY_HOME: gui_state.cur_i = 0; break;
		case GLUT_KEY_END: gui_state.cur_i = gui_state.frame->columns-1; break;
		case GLUT_KEY_PAGE_UP: gui_state.cur_j = 0; break;
		case GLUT_KEY_PAGE_DOWN: gui_state.cur_j = gui_state.frame->columns-1; break;
		case GLUT_KEY_LEFT: 
			gui_state.cur_i = (--gui_state.cur_i < 0) ? 0 : gui_state.cur_i; 
			break;
		case GLUT_KEY_RIGHT: 
			gui_state.cur_i = (++gui_state.cur_i >= gui_state.frame->columns) ? gui_state.frame->columns-1 : gui_state.cur_i; 
			break;
		case GLUT_KEY_UP:
			gui_state.cur_j = (--gui_state.cur_j < 0) ? 0 : gui_state.cur_j; 
			break;
		case GLUT_KEY_DOWN:
			gui_state.cur_j = (++gui_state.cur_j >= gui_state.frame->columns) ? gui_state.frame->columns-1 : gui_state.cur_j; 
			break;
	}

	gui_state.need_redraw = 1;
}

void	set_color (GLfloat color[], float r, float g, float b, float a)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = a;
}

void	mark_region_selection ()
{
	int		i, j;
	double	x,y, x1,y1;
	frameT	*f = gui_state.frame;
	
	i = gui_state.region_i;	
	j = gui_state.region_j;
	x = denormalize_data(f->column[i], gui_state.region_x);
	y = denormalize_data(f->column[j], 1.0 - (gui_state.region_y));
	x1 = denormalize_data(f->column[i], gui_state.region_x + gui_state.region_w);
	y1 = denormalize_data(f->column[j], 1.0 - (gui_state.region_y + gui_state.region_h));

	mark_region (f, i, j, x, x1, y, y1);
}

void	mouse_to_pointer ()
{
	double	mx, my;
	int		pi, pj;
	double	px, py;

	// TODO: This isnt 100% right

	mx = gui_state.mouse.x;
	my = gui_state.mouse.y;

	pi = gui_state.mouse.x * gui_state.frame->columns;
	pj = gui_state.mouse.y * gui_state.frame->columns;

	if (pi < 0) pi = 0;
	if (pj < 0) pj = 0;
	if (pi >= gui_state.frame->columns) pi = gui_state.frame->columns-1;
	if (pj >= gui_state.frame->columns) pj = gui_state.frame->columns-1;

	gui_state.pointer_i = pi;
	gui_state.pointer_j = pj;

	px = gui_state.frame->columns * (mx - pi / gui_state.frame->columns) - pi;	
	py = gui_state.frame->columns * (my - pj / gui_state.frame->columns) - pj;	

	gui_state.pointer_x = px;
	gui_state.pointer_y = py;
}



void	mouseButton (int button, int state, int x, int y)
{
	gui_state.mouse.x = x / (float) gui_state.w;
	gui_state.mouse.y = y / (gui_state.h * (1.0 - gui_state.status_bar_height));
	gui_state.mouse.button = button;
	gui_state.mouse.state = state;

	mouse_to_pointer();

	if (state == 0)
	{
		// Button DOWN
		switch(button)
		{
			case GLUT_LEFT_BUTTON: 
				gui_state.region_x = gui_state.pointer_x;
				gui_state.region_y = gui_state.pointer_y;
				gui_state.region_i = gui_state.pointer_i;
				gui_state.region_j = gui_state.pointer_j;
				gui_state.region_w = 0;
				gui_state.region_h = 0;
				break;
		}
	} else
	{
		// Button UP
		switch (button)
		{
			case GLUT_LEFT_BUTTON: mark_region_selection(); break;
		}
	}

	gui_state.need_redraw = 1;
}

void	mouseMoved (int x, int y)
{
	gui_state.mouse.x = x / (float) gui_state.w;
	gui_state.mouse.y = y / (gui_state.h * (1.0 - gui_state.status_bar_height));

	mouse_to_pointer();

	gui_state.need_redraw = 1;
}	

void	mouseButtonMoved (int x, int y)
{
	gui_state.mouse.x = x / (float) gui_state.w;
	gui_state.mouse.y = y / (gui_state.h * (1.0 - gui_state.status_bar_height));

	mouse_to_pointer();

	if ((gui_state.pointer_i == gui_state.region_i) && (gui_state.pointer_j == gui_state.region_j))
	{
		switch (gui_state.mouse.button )
		{
			case GLUT_LEFT_BUTTON:
				gui_state.region_w = gui_state.pointer_x - gui_state.region_x;
				gui_state.region_h = gui_state.pointer_y - gui_state.region_y;
				break;
		}
	}

	gui_state.need_redraw = 1;
}	


void init_gui(int *argc, char **argv, int Width, int Height, frameT *frame)	        
{
	gui_state.frame = frame;
	
	gui_state.jitter = 0;
	gui_state.hide_axes = 0;
	gui_state.zoom = 0;
	gui_state.gradient = 0;
	gui_state.help = 0;

	gui_state.plot_padding = 0.05;
	gui_state.axis_padding = 0.05;
	gui_state.status_bar_height = 0.05;

	gui_state.scale = 1.0;

	gui_state.max_points = 10000;

	gui_state.w = Width;
	gui_state.h = Height;
	gui_state.cur_i = frame->columns / 2;
	gui_state.cur_j = frame->columns / 2;

	gui_state.selected_i = -1;
	gui_state.selected_j = -1;

	gui_state.region_i = -1;
	gui_state.region_j = -1;

	set_color(gui_state.color.fg,      1.0, 1.0, 0.6,  1.0);
	set_color(gui_state.color.bg,      0.0, 0.0, 0.0,  1.0);
	set_color(gui_state.color.axis,    1.0, 1.0, 0.3,  0.3);
	set_color(gui_state.color.faint,   1.0, 1.0, 0.3,  0.2);
	set_color(gui_state.color.point,   0.3, 1.0, 0.3,  0.5);
	set_color(gui_state.color.fit,     1.0, 0.3, 0.3,  0.5);
	set_color(gui_state.color.marked,  1.0, 0.3, 0.0,  0.5);
	set_color(gui_state.color.scrim,   0.05, 0.05, 0.05, 0.9);

	set_color(gui_state.color.gradient_xmin,  0.1, 0.8, 0.3, 0.3);
	set_color(gui_state.color.gradient_xmax,  0.8, 0.3, 0.1, 0.3);
	
	set_color(gui_state.color.gradient_ymin,  0.8, 0.1, 0.3, 0.3);
	set_color(gui_state.color.gradient_ymax,  0.3, 0.1, 0.8, 0.3);

	gui_state.stipple = 0x6666;

	snprintf (gui_state.message, GUI_MSG_LEN, "Welcome");

	gui_state.first_display_row = 0;

	glutInit(argc, argv);  

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA);

	glutInitWindowSize(gui_state.w, gui_state.h);  
	gui_state.window = glutCreateWindow("smatter / i2pi.com");
	glutDisplayFunc(&draw_gui);  
//	glutFullScreen();

#ifdef _TIMING
  	glutIdleFunc(&draw_gui);
#endif
	glutReshapeFunc(&resize_gui);
	glutKeyboardFunc(&keyPressed);
	glutSpecialFunc(&specialPressed);
	glutMouseFunc(&mouseButton);
	glutMotionFunc(&mouseButtonMoved);
	glutPassiveMotionFunc(&mouseMoved);
	glutTimerFunc(REDRAW_MSEC, draw_gui_timer, 0);

	glClearColor(gui_state.color.bg[0], gui_state.color.bg[1], gui_state.color.bg[2], 0);

	glDepthFunc(GL_LESS);			   

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_STIPPLE);
	glHint (GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();			

	gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);

//	glutSetCursor(GLUT_CURSOR_FULL_CROSSHAIR);
//	glutSetCursor(GLUT_CURSOR_NONE);
}
