#include <stdio.h>
#include <stdlib.h>

#include "data.h"

int main (int argc, char **argv)
{
	frameT	*frame;
	transformT	*t;

	init_types();
	init_transforms();

	if (argc != 2)
	{
		fprintf (stderr, "%s data_file.csv\n", argv[0]);
		exit (-1);
	}

	frame = read_csv (argv[1]);

	/*
	t = clone_transform (&transform[2]);
	column_add_transform (frame->column[4], t);
	column_apply_transforms (frame->column[4]);
	*/

//	print_frame (frame);

	show_stats (&frame->column[3]->orig_stats);

	return (0);
}
