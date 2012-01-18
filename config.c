#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "gui.h"

extern gui_stateT	gui_state;
void      set_color (GLfloat color[], float r, float g, float b, float a);

typedef enum
{
	BOOLEAN,
	COLOR,
	PERCENT,
	NUMBER,
	HEX
} ctype;

typedef struct 
{
	char	*str;
	ctype	type;
	void	*p;
} config_itemT;

config_itemT	conf[128];
int				confs=0;

void	register_config_item (char *str, ctype type, void *p)
{
	int	c = confs++;

	conf[c].str  = strdup (str);
	conf[c].type = type;
	conf[c].p    = p;
}

char	*lower_ltrim (char *str)
{
	char	*p, *start;
	start = str;
	while (isspace (*start)) start++;
	for (p=start; *p && !isspace(*p) && (*p != '#'); p++) *p = tolower(*p);
	*p = '\0';

	return start;
}

void	parse_value (config_itemT *item, char *str)
{
	void	*p = item->p;

	str = lower_ltrim (str);

	switch (item->type)
	{
		case BOOLEAN:
			if (strstr(str, "true") == str)  *(char *)p = 1; else  *(char *) p = 0; 
			break;
		case COLOR:
		{
			float	*color = (float *) p;
			int		i;
			if (strlen (str) < 8) 
			{
				fprintf (stderr, "Invalid color hex '%s'\n", str);
				exit (-1);
			}
			for (i=0; i<4; i++)
			{
				char	hv[3];
				hv[0] = str[2*i];
				hv[1] = str[2*i+1];
				hv[2] = '\0';
				color[i] = strtoul (hv, NULL, 16) / 255.0f;
			}
		}; break;
		case NUMBER:
			*(double *) p = atof (str);		
			break;
		case PERCENT:
			*(double *) p = atof(str) / 100.0;
			break;
		case HEX:
			*(unsigned long *)p = strtoul (str, NULL, 16);
			break;
	}
}

void	parse_config_line (char *str)
{
	char	*eq = strchr (str, '=');
	int		i;

	str = lower_ltrim (str);

	if (str[0] == '#') return;

	if (!eq)	return;

	i = 0; 
	while ((i < confs) && (strstr(str, conf[i].str) != str)) i++;
	if (i < confs) parse_value (&conf[i], eq+1);
}

int	load_config (char *fname)
{
	FILE 	*fp = fopen (fname, "r");
	char	buf[4096];

	if (!fp) return (-1);

	while (fgets (buf, 4095, fp)) parse_config_line (buf);

	fclose (fp);

	return (0);
}

	
void	init_config (void)
{
	register_config_item ("hide_axes", BOOLEAN, &gui_state.hide_axes);
	register_config_item ("plot_padding", PERCENT, &gui_state.plot_padding);
	register_config_item ("axis_padding", PERCENT, &gui_state.axis_padding);
	register_config_item ("max_display_points", NUMBER, &gui_state.max_points);
	register_config_item ("width", NUMBER, &gui_state.w);
	register_config_item ("height", NUMBER, &gui_state.h);


	register_config_item ("fg", COLOR, &gui_state.color.fg);
	register_config_item ("bg", COLOR, &gui_state.color.bg);
	register_config_item ("axis", COLOR, &gui_state.color.axis);
	register_config_item ("faint", COLOR, &gui_state.color.faint);
	register_config_item ("point", COLOR, &gui_state.color.point);
	register_config_item ("fit", COLOR, &gui_state.color.fit);
	register_config_item ("marked", COLOR, &gui_state.color.marked);
	register_config_item ("scrim", COLOR, &gui_state.color.scrim);

	register_config_item ("gradient_xmin", COLOR, &gui_state.color.gradient_xmin);
	register_config_item ("gradient_xmax", COLOR, &gui_state.color.gradient_xmax);
	
	register_config_item ("gradient_ymin", COLOR, &gui_state.color.gradient_ymin);
	register_config_item ("gradient_ymax", COLOR, &gui_state.color.gradient_ymax);

	register_config_item ("stipple", HEX, &gui_state.stipple);
}
