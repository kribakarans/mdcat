
#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "stack.h"

#define ANSI_FRMT_RESET     "\e[0m"
#define ANSI_FRMT_BOLD      "\e[1m"
#define ANSI_FRMT_ITALICS   "\e[3m"
#define ANSI_FRMT_UNDERLINE "\e[4m"
#define ANSI_COLOR_RED      "\e[31m"
#define ANSI_COLOR_GREEN    "\e[32m"
#define ANSI_COLOR_YELLOW   "\e[33m"
#define ANSI_COLOR_BLUE     "\e[34m"
#define ANSI_COLOR_MAGENTA  "\e[35m"
#define ANSI_COLOR_CYAN     "\e[36m"
#define ANSI_BOLD_RED       "\e[31;1;1m"
#define ANSI_BOLD_GREEN     "\e[32;1;1m"
#define ANSI_BOLD_YELLOW    "\e[33;1;1m"
#define ANSI_BOLD_BLUE      "\e[34;1;1m"
#define ANSI_BOLD_MAGENTA   "\e[35;1;1m"
#define ANSI_BOLD_CYAN      "\e[36;1;1m"
#define ANSI_BOLD_WHITE     "\e[37;1;1m"

#ifndef DEBUG
#define logit(fmt, ...) ((void) 0)
#else
#define logit(fmt, ...) (fprintf(stderr, "%17s:%-4d %17s: " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__))
#endif

enum {
	MDCAT_ERR = -1,
	MDCAT_OK  =  0,
};

enum fmts {
	DO_RESET     = 0xFFF0,
	DO_BOLD      = 0xFFF1,
	DO_CODEBLOCK = 0xFFF2,
	DO_ITALICS   = 0xFFF3,
	DO_LIST      = 0xFFF4,
	DO_UNDERLINE = 0xFFF5,
};

typedef struct mdcat_st {
	int fmt;
} mdcat_t;

mdcat_t mdcat;
stack_t stack;

/* Print the formatted final output line */
int mdcat_print_line(char *line)
{
	char *ptr = line;
	size_t delay = 3000;

	while(*ptr != '\0') {
		printf("%c", *ptr++);
		usleep((useconds_t)delay);
		fflush(stdout);
	}

	printf("\n");

	return 0;
}

bool is_header(const char *str)
{
	bool retval = false;

	do {
		if ((str[0] == '#') && (str[1] == ' '))  {
			retval = true;
		} else if ((str[0] == '#') && (str[1] == '#') && (str[2] == ' ')) {
			retval = true;
		} else if ((str[0] == '#') && (str[1] == '#') && (str[2] == '#') && (str[3] == ' ')) {
			retval = true;
		} else {
			retval = false;
		}
	} while(0);

	return retval;
}

/*
  Interface: mdcat_render_header()
  Description:
    Render header text format in the input @str and save the ascii formatted line to @dstline.
    Use @dstline as final output to print to the console.
  Input:
    @dstline: Holds the ascii formatted line (final output)
    @str: Input source line
  Retval:
    On success:
      @lineptr: Pointer to the line to render further
      @dstline: Point the rendered text (ascii formatted text) to print to console
    On failure:
      @lineptr: NULL (input line is not a header)
      @dstline: Unchanged
*/
char *mdcat_render_header(char **dstline, char *str)
{
	char *lineptr = NULL;

	do {
		assert(str != NULL);
		assert(dstline != NULL);
		assert(*dstline != NULL);

		if ((str[0] == '#') && (str[1] == ' '))  {
			lineptr = &str[2];
			sprintf(*dstline, ANSI_BOLD_BLUE "%s" ANSI_FRMT_RESET, lineptr);
		} else if ((str[0] == '#') && (str[1] == '#') && (str[2] == ' ')) {
			lineptr = &str[3];
			sprintf(*dstline, ANSI_BOLD_YELLOW "%s" ANSI_FRMT_RESET, lineptr);
		} else if ((str[0] == '#') && (str[1] == '#') && (str[2] == '#') && (str[3] == ' ')) {
			lineptr = &str[4];
			sprintf(*dstline, ANSI_BOLD_MAGENTA "%s" ANSI_FRMT_RESET, lineptr);
		} else {
			lineptr = NULL;
		}
	} while(0);

	return lineptr;
}

/*
  Interface: mdcat_render_text()
  Description:
    Render MD formated text into the ANSI coloured text format.
  Input:
    @dstline: Holds the ascii formatted line (final output)
    @dstindx: Points the last character of @dstline
    @md_op: What MD operation need to perform (bold, itallics ...)
    @fmt: ASCII format for the MD operation @md_op
  Retval:
    On success:
      @dstline: Save the rendered text format and adjust the @dstindx
    On failure:
      @dstline: Unchanged
*/
char *mdcat_render_text(char **dstline, int *dstindx, int md_op, char *fmt)
{
	int retval = -1;

	do {
			assert(dstindx  != NULL);
			assert(dstline  != NULL);
			assert(*dstline != NULL);

			/* Get current format from the stack top */
			retval = peek(&stack);
			if (retval == md_op) {
					mdcat.fmt = DO_RESET;
					strcat(*dstline, ANSI_FRMT_RESET);        /* 1: Reset the current text format */
					*dstindx += (int)strlen(ANSI_FRMT_RESET); /* 2: Adjust the dstline pointer to the end of format specifier to load next text */
					retval = pop(&stack);                     /* 3: Popout the handled format from the stack */
					assert(retval == md_op);
			} else {
					mdcat.fmt = md_op;
					strcat(*dstline, fmt);        /* 1: Insert the text format specifier */
					*dstindx += (int)strlen(fmt); /* 2: Adjust the dstline pointer to the end of format specifier to load next text */
					retval = push(&stack, md_op); /* 3: Push the current format to the stack */
					assert(retval == STACK_OK);
			}
	} while(0);

	return *dstline;
}

/*
  Interface: mdcat_render_list()
  Description:
    Render MD formated bullet list entries
  Input:
    @dstline: Holds the ascii formatted line (final output)
    @str: Source input line
  Retval:
    On success:
      @dstline: Save the rendered text format
    On failure:
      @dstline: Unchanged
*/
char *mdcat_render_list(char *dstline, char *str)
{
	int        i = 0;
	int  dstindx = 0;
	char *retval = NULL;
	bool applied = false;
	char *bullet = "•"; /* "➨" */
	int     blen = (int)strlen(bullet);

	do {
		assert(str != NULL);
		assert(dstline != NULL);

		if (mdcat.fmt == DO_CODEBLOCK) {
			retval = NULL;
			break;
		}

		while (str[i] == ' ' || str[i] == '\t') {
			i++;
		}

		if (((str[i] == '-') && (str[i + 1] == ' ')) ||
		    ((str[i] == '*') && (str[i + 1] == ' '))) {
			applied = false;
			for (i = 0, dstindx = 0; str[i] != '\0'; i++, dstindx++) {
				if ((applied != true) && ((str[i] == '-') || (str[i] == '*'))) {
					i++;
					strcat(dstline, bullet);
					dstindx += blen;
					applied = true;
				}
				dstline[dstindx] = str[i];
			}
		} else {
			retval = NULL;
			break;
		}

		retval = dstline;
	} while(0);

	return retval;
}

int mdcat_render_line(char *str, size_t len)
{
	int         i =  0;
	int    retval = -1;
	int   dstindx =  0;
	char *dstline = NULL; /* Holds the final ASCII formatted line */
	char *lineptr = NULL; /* Holds the starting of next line to render */


	do {
		lineptr = str;
		dstline = calloc(len * 2, sizeof(char)); /* @len: double the space of source line length to adapt the format specifiers */
		assert(dstline != NULL);

		/* Render #ed header line format */
		if (mdcat_render_header(&dstline, str) != NULL) {
			goto PRINT_OUTPUT;
			retval = MDCAT_OK;
			break;
		}

		/* Render list format */
		if (mdcat_render_list(dstline, str) != NULL) {
			goto PRINT_OUTPUT;
			retval = MDCAT_OK;
			break;
		}

		/* Render remaining formats */
		for (i = 0, dstindx = 0; lineptr[i] != '\0'; i++, dstindx++) {
			switch (lineptr[i]) {
				case '*':
					if (mdcat.fmt == DO_CODEBLOCK) {
						break;
					}

					if (lineptr[i + 1] == '*') {
						i += 2;
						/* Render Bold format */
						mdcat_render_text(&dstline, &dstindx, DO_BOLD, ANSI_FRMT_BOLD);
					} else {
						i += 1;
						/* Render Italic format */
						mdcat_render_text(&dstline, &dstindx, DO_ITALICS, ANSI_FRMT_ITALICS);
					}

					break;
				case '_': /* Render Underline format */
					if (mdcat.fmt == DO_CODEBLOCK) {
						break;
					}

					if ((lineptr[i + 1] == '_') && (lineptr[i + 2] == '_')) {
						i += 3;
						mdcat_render_text(&dstline, &dstindx, DO_UNDERLINE, ANSI_FRMT_UNDERLINE);
					}

					break;

				case '`': /* Render code snippet */
					if ((lineptr[i + 1] == '`') && (lineptr[i + 2] == '`')) {
						i += 3;
					} else if (lineptr[i + 1] == '`') {
						i += 2;
					} else {
						i += 1;
					}

					mdcat_render_text(&dstline, &dstindx, DO_CODEBLOCK, ANSI_COLOR_MAGENTA);
					break;
				default:
					break;
			}

			dstline[dstindx] = lineptr[i];
			fflush(stdout);
		}

PRINT_OUTPUT:
		mdcat_print_line(dstline);
		free(dstline);
	} while(0);

	return retval;
}


int mdcat_worker(const char *file)
{
	int retval = -1;
	size_t len =  0;
	FILE   *fp = NULL;
	char *linebuff = NULL;

	logit("%s", file);

	do {
		retval = initialize_stack(&stack);
		if (retval != STACK_OK) {
			fprintf(stderr, "failed to init stack");
			break;
		}

		fp = fopen(file, "r");
		if (fp == NULL) {
			fprintf(stderr, "mdcat: cannot open '%s' (%s)\n", file, strerror(errno));
			break;
		}

		/*
		  Getline dynamically allocates memory for the line, and it automatically resizes the buffer if needed.
		  The function returns the number of characters read, and -1 if there's an error or the end of the file.
		*/
		while (getline(&linebuff, &len, fp) != -1) {
			linebuff[strlen(linebuff) - 1] = '\0';
			mdcat_render_line(linebuff, len - 1);
		}

		//display_stack(&stack);
		free(linebuff);
		fclose(fp);
	} while(0);

	logit("retval: %d", retval);

	return retval;

}

int main(int argc, char **argv)
{
	int i = 0;
	int retval = -1;

	if (argc < 2) {
		fprintf(stderr, "mdcat: missing file operand.\n");
		fprintf(stderr, "usage: mdcat FILEs ...\n");
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		retval = mdcat_worker(argv[i]);
	}

	return retval;
}

/* EOF */
