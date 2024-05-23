
#ifndef _UTILITY_H
#define _UTILITY_H

/* Macro that determines number of elemets in an array */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/* Macro to access text of a #define in code. */
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif


/* Check whether the given string is numeric */
extern int isNumeric(const char *);

extern char * rightJustify(char *, int, long, char filler = ' ', char seperator = '\0');

extern char * rightJustifyF(char *, int, double, int, char = ' ', char = '\0');
extern char * rightJustifyF(char *, int, double, char = ' ', char = '\0');

#endif
