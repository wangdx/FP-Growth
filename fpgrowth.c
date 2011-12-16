/*
 * fpgrowth.c
 * FP-growth algorithm for finding frequent item sets
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "tract.h"
#include "fptree.h"

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "fpgrowth"
#define DESCRIPTION "find frequent item sets " \
                    "with the fpgrowth algorithm"

#define TIME_FLY(start)  ((clock()-(start)) /(double)CLOCKS_PER_SEC)
int NumberOfFrequentSets = 0;               /* like the variable name */
static FILE *in = NULL;                     /* input file */
static FILE *out = NULL;                    /* output file */
static NodeLink *hashTable = NULL;          /* array of Nodes in a hash table */
static FpHeadTable *headTable = NULL;       /* item head table */
static ItemLink *tracts = NULL;             /* array of transactions */
static FpTree *fptree = NULL;               /* frequent pattern tree */
int main(int argc, char *argv[])
{
  clock_t start = clock();                  /* to compute the execution time */
  char *appname = argv[0];
  char *s;                                  /* to traverse the options */
  char *inFile;                             /* File containing all the transactions(input file name) */
	char *outFile = NULL;                     /* output file name */
  double support;                           /* Support Threshold (in percent) */
	int transNum = 0;													/* transaction number */
	int itemsNum = 0;                         /* distinct item number */
  int i;                                    /* for traversing the argvs */
  if(argc >= 9){
		printf("%s - %s\n", PRGNAME, DESCRIPTION);
  } else {
		printf("usage: %s -i inFile -o outFile -t support -n transNum -m itemsNum\n", PRGNAME);
		printf("-i <[required]File containing all the transactions>\n");
		printf("-o <[optional]File we can output the results>\n");
		printf("-n <[required]total number of transactions>\n");
		printf("-m <[required]total number of distinct items>\n");
		printf("-t <[required]Support Threshold>\n");
    printf("   (positive: percentage, "
               "negative: absolute number)\n");
		return 0;
  }
  for(i = 1; i < argc; ++i){                /* traverse the arguments */
		s = argv[i];                            /* get an option argument */
		if((*s == '-') && *++s){                /* if argument is an option */
			while (*s) {                          /* traverse the options */
				switch(*s++){
				case 'i': inFile = argv[++i];break;
				case 't': support = strtod(argv[++i], NULL);break;
				case 'n': transNum = atoi(argv[++i]);break;
				case 'm': itemsNum = atoi(argv[++i]);break;
				case 'o': outFile = argv[++i];break;
				default:continue;
				}
			}
		}
  }
  if(support > 100) {                       /* check the limit for support threshold */
		printf("invalid minimum support %g%%\n", support);
		exit(-11);
  }
  support = ceil((support >= 0) ? 
			0.01 * support * transNum : -support);/* compute absolute support value */
  if(inFile && *inFile){
		in = fopen(inFile, "r");
  } else {
		inFile = "<stdin>";
		in = stdin;
  }
	if(outFile && *outFile){
		out = fopen(outFile, "a+");
	} else {
		outFile = "<stdout>";
		out = stdout;
	}
	/*--- initial some structures ---*/
	hashTable = (NodeLink *)calloc(itemsNum, sizeof(NodeLink));
	tracts = (ItemLink *)calloc(transNum, sizeof(ItemLink));
	findFItemset(in, tracts, hashTable, (int)support, transNum, itemsNum);
	deleteUnsupport(tracts, transNum, (int)support, hashTable, itemsNum);
	sortEveryTract(tracts, transNum, hashTable, itemsNum, (int)support);
	int numExceeded = getNumOfExceeded(hashTable, itemsNum);
	headTable = (FpHeadTable *)malloc(sizeof(FpHeadTable));
	if(!headTable){
		perror("out of memory!!!");exit(-1);
	}
	createHeadTable(hashTable, itemsNum, headTable, numExceeded);
	fptree = (FpTree *)malloc(sizeof(FpTree));
	if(!fptree){
		perror("out of memory!!!");exit(-1);
	}
	createFpTree(fptree, headTable, tracts, transNum);
	fpgrowth(fptree, NULL, (int)support, out);
  fprintf(out, "Number of frequent sets: [%d]\n", NumberOfFrequentSets);
	fprintf(out, "Time consume: [%.2fs]\n", TIME_FLY(start));
  return 0;
}
