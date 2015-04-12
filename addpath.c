/* addpath.c
 *
 * Name: addpath
 * Description: tool to add elements to PATH style variables
 *
 * Author: Tony Aiuto
 * Source: http://tony.aiu.to/sa
 *
 * Copyright Tony Aiuto (c) 1990-2006
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern int	optind;
extern char	*optarg;

typedef enum {	SH, CSH, CMD } ShellType;

#if defined(_WIN32)
static	char	delimiter = ';';		/* input path separator */
static	char	pathSep = ';';			/* output path separator */
static	int	slashToBackSlash = 1;		/* emit / as \, for windoze */
static	ShellType	shellType = CMD;
#else
static	char	delimiter = ':';		/* input path sep */
static	char	pathSep = ':';			/* output path sep */
static	int	slashToBackSlash = 0;		/* emit / as \, for windoze */
static	ShellType	shellType = SH;
#endif

static	int	allowDuplicates = 0;		/* allow duplicates */
static	int	sourceform = 0;			/* print a source-able output */
static	int	checkDir = 0;			/* check for presence of path elem */
static	const char	*pathName = NULL;	/* path name we are checking */


static void pversion()
{
	printf("addpath %s, http://tony.aiu.to/sa/\n", RELEASE);
}

static void usage()
{
	pversion();
	printf("usage: addpath [-bcw] [-df] [-e VAR] [-p VAR] elem ...\n");

	printf("\tOnly one of bcw should be specified.  Default is detected by\n");
	printf("\texamining $SHELL.\n");
	printf("\t-b\temit Bourne style path.  e.g. 'a:b:c:...'\n");
	printf("\t-c\temit Csh style path.  e.g. 'a b c ...'\n");
	printf("\t-w\temit Windoze style path.  e.g. 'a;b;c;...'\n");

	printf("\t-d\tforce new elements in even if they are Duplicates\n");
	printf("\t-e\tonly do it if environment variable env_var is defined.\n");
	printf("\t-f\tprepend elements at Front of path. (default: at end)\n");
	printf("\t-p VAR\tuse VAR for path rather than PATH.\n");
	printf("\t-s\tprint a command that you could \"source\".\n");
	printf("\t-x\tcheck for existence of each element before adding.\n");
}


/* Check if a path contains a particular element
 * return 0 for no, non-zero for yes
 * note: uses global delimiter settings to split path
 */

static int path_contains(const char *path, const char *elem)
{
	const char	*estart, *eend;
	int	elen = strlen(elem);

	if(path == NULL) return 0;
	if(elen <= 0) return 0;

	estart = path;
	eend = path;
	while(*eend) {
		while(*eend != '\0' && *eend != delimiter) eend++;
		/* assert: eend points at : or null at end of string */
		if((eend - estart == elen) && strncmp(elem, estart, elen) == 0) return 1;
		while(*eend == delimiter) eend++;
		estart = eend;
	}
	/* if(strncmp(elem, estart, eend - estart)) return 1; */
	return 0;
}

static void printPath(FILE *out, const char *path)
{
	const char	*p;

	if(path == NULL) return;

	for(p = path; *p; p++) {
		if(*p == delimiter) {
			fputc(pathSep, out);
		} else if(slashToBackSlash && *p == '/') {
			fputc('\\', out);
		} else {
			fputc(*p, out);
		}
	}
}


/* replace environment variable callouts in a string
 * returns:
 *	malloc'ed string (caller frees)
 *	status is:
 *	0	no $var callouts
 *	1	successful variable substitution
 *	-1	failed substitution
 */
char *replaceEnv(const char *s, int *status)
{
	int	olen = strlen(s) + 1;
	char	*out = (char *) malloc(olen);
	int	optr = 0;
	const char	*ebegin;
	char	*ename, *eval;

	if(status) *status = 0;
	while(*s) {
		/* This block is only for the poor souls running on some
		 * OS where you normally put $ in file names
		 */
		if(*s == '\\' && *(s+1) == '$') {
			out[optr++] = '$';
			s += 2;
			continue;
		}

		if(*s != '$') {
			out[optr++] = *s++;
			continue;
		}

		/* We got a $something */
		if(status && *status == 0) *status = 1;
		s++;		/* skip the $ */
		ebegin = s;
		while(isalnum(*s) || *s == '_') s++;
		ename = (char *) malloc(s - ebegin + 1);
		strncpy(ename, ebegin, s-ebegin+1);
		ename[s-ebegin] = '\0';
		/* printf("\ngetenv(%s)\n", ename); */
		eval = getenv(ename);
		if(eval == NULL) {
			if(status) *status = -1;
		} else {
			int need = strlen(eval) - (s - ebegin + 1);
			if(need > 0) {		/* must expand output */
				char	*newo = (char *) malloc(olen+need);
				strncpy(newo, out, optr+1);
				free(out);
				out = newo;
			}
			strcpy(&out[optr], eval);
			optr += strlen(eval);
		}
		free(ename);
	}
	out[optr] = '\0';
	return out;
}


static int dirExists(const char *path)
{
	struct stat pstats;
	int err = stat(path, &pstats);
	if((err == 0) && S_ISDIR(pstats.st_mode)) return 1;
	return 0;
}

static setDefaultsForCSH()
{
	pathSep = ' ';
	slashToBackSlash = 0;
	shellType = CSH;
}

static setDefaultsForSH()
{
	pathSep = ':';
	slashToBackSlash = 0;
	shellType = SH;
}

int main(int argc, char *argv[])
{
	char	*path;
	int	c;

	int	front = 0;
	int	quickExit = 0;
	int	anything = 0;
	const char	*shell;


	/* set up defaults based on the SHELL */
	shell = getenv("SHELL");
	if(shell != NULL) {
		int l = strlen(shell);
		if((l > 3) && (strcmp(&shell[l-3], "csh") == 0) ) {
			setDefaultsForCSH();
		}
	}


	while((c = getopt(argc, argv, "bcdfe:hp:swVx")) != EOF) {
		switch(c) {
		case 'b':
			setDefaultsForSH();
			break;
		case 'c':
			setDefaultsForCSH();
			break;

		case 'd':
			allowDuplicates = 1;
			break;

		case 'e':
			if(getenv(optarg) == NULL) quickExit = 1;
			break;

		case 'f':
			front = 1;
			break;

		case 'h':
			usage();
			return 1;

		case 'p':
			pathName = strdup(optarg);
			break;

		case 's':
			sourceform = 1;
			break;

		case 'V':
			pversion();
			return 0;

		case 'w':
			pathSep = ';';
			slashToBackSlash = 1;
			break;

		case 'x':
			checkDir = 1;
			break;

		default:
			break;
		}
	}

	/* Now get the path variable */

	if(pathName == NULL) {
		pathName = "PATH";
	}
	path = getenv(pathName);
	if(path == NULL) path = "";

	/* printf("%s = '%s'\n", pathName, path); */

	if(quickExit) {
		printPath(stdout, path);
		printf("\n");
		return 0;
	}

	if(sourceform) {
		if(shellType == CSH) {
			fprintf(stdout, "set path = ( ");
		} else if(shellType == SH) {
			fprintf(stdout, "%s=", pathName);
		}

	}

	if(front == 0) {
		printPath(stdout, path);
		if(path != NULL && *path != '\0') anything = 1;
	}
	for(; optind < argc; optind++) {
		char *newelem = argv[optind];
		int	status = 0;
		newelem = replaceEnv(newelem, &status);
		if(status < 0) continue;
		if(checkDir && !dirExists(newelem)) {
			free(newelem);
			continue;
		}
		if(allowDuplicates || !path_contains(path, newelem)) {
			if(front) {
				if(anything > 0) fputc(pathSep, stdout);
				printPath(stdout, newelem);
			} else {
				if(anything > 0) fputc(pathSep, stdout);
				printPath(stdout, newelem);
			}
			anything += 1;
		}
		free(newelem);
	}
	if(front != 0) {
		if(anything > 0 && path != NULL && *path != '\0') {
			fputc(pathSep, stdout);
		}
		printPath(stdout, path);
	}
	if(sourceform) {
		if(shellType == CSH) {
			fprintf(stdout, " )");
		}
	}
	printf("\n");

	exit(0);
}

