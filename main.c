#include <stdio.h>
#include <stdlib.h>

#include "gui.h"
#include "data.h"
#include "config.h"

int main (int argc, char **argv)
{
	frameT	*frame;

	if (argc != 2)
	{
		fprintf (stderr, "%s data_file.csv\n", argv[0]);
		exit (-1);
	}

	nlo_init();
	init_types();
	init_transforms();
	init_config();

	frame = read_csv (argv[1]);

	init_gui(&argc, argv, 640, 480, frame);

	// TODO: yeah, need to do this properly
	if (load_config ("smatter.conf") == -1) load_config("~/.smatter.conf");

	glutMainLoop();

	return (0);
}
