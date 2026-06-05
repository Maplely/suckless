static int
issel(size_t id)
{
	for (int i = 0;i < selidsize;i++)
		if (selid[i] == id)
			return 1;
	return 0;
}

static void
printselected(unsigned int state)
{
	for (int i = 0; i < selidsize; i++) {
		if (selid[i] != -1 && (!sel || sel->id != selid[i])) {
			printitem(&items[selid[i]]);
		}
	}

	printcurrent(state);
}

static void
selsel(void)
{
	if (!sel)
		return;
	if (issel(sel->id)) {
		for (int i = 0; i < selidsize; i++)
			if (selid[i] == sel->id)
				selid[i] = -1;
	} else {
		for (int i = 0; i < selidsize; i++)
			if (selid[i] == -1) {
				selid[i] = sel->id;
				return;
			}
		selidsize++;
		int *tmp = realloc(selid, (selidsize + 1) * sizeof(int));
		if (!tmp)
			die("realloc:");
		selid = tmp;
		selid[selidsize - 1] = sel->id;
	}
}
