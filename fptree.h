#ifndef _FPTREE_H
#define _FPTREE_H
#include "tract.h"

#define MAX_SUPPORT 2147483647              /* maximum integer in 32 bits Linux */
extern int NumberOfFrequentSets;
typedef struct _FpNode{                     /* --- frequent pattern tree node --- */
	char *field;                              /* item name */
	int support;                              /* support(weight of transactions) */
	struct _FpNode *parent;                   /* parent node */
	struct _FpNode *eldest;                   /* first child(link the other children with eldest's sibling) */
	struct _FpNode *sibling;                  /* sibling with same parent */
	struct _FpNode *same;                     /* link together the nodes with same field */
} FpNode, *FpNodeLink, *Pattern, *PatternBase;

typedef struct{                             /* --- freq. pat. tree node list(item head table) --- */
	char *field;
	int support;
	FpNodeLink head;                          /* head of the item list */
} FpList, *FpListLink;

typedef struct{
	FpListLink *lists;                        /* array of f.p. lists */
	int count;                                /* number of lists */
} FpHeadTable;

typedef struct {                            /* --- frequent pattern tree --- */
	FpHeadTable *headTable;                   /* item head table */
	FpNode *root;                             /* root node connecting trees */
} FpTree;
static FpNode* _matched(FpNode *nodeLink, ItemLink itemLink);
static void _initTreeRoot(FpNode **root);
static void _insertTree(ItemLink tract, FpNode **node, FpHeadTable *headTable);
static void _linkSameNode(FpNode **node, FpHeadTable *headTable);
extern void createHeadTable(NodeLink *hashTable, int size, FpHeadTable *headTable, int numExceeded);
extern void _sortHeadTable(FpHeadTable *headTable);
extern void createFpTree(FpTree *fptree, FpHeadTable *headTable, ItemLink *tracts, int size);
static void _buildCondFpTree(FpTree *cfptree,  FpTree *fptree, int idx, int support);
static void _buildCondPatternBase(PatternBase *pbh, FpTree *fptree, int idx);
static void _buildHeadTable(FpHeadTable *ht, PatternBase *pbh, int support);
static int  _assistUtil(NodeLink *h, PatternBase pbh, int support);
static void _prunePatternBase(PatternBase pbh, NodeLink h, int n);
static void _insertCfpTree(PatternBase pbh, FpNodeLink root, FpHeadTable *ht);
static FpNode* _matchedCfp(FpNode *q, Pattern p);
extern void fpgrowth(FpTree *fptree, FpNode *node, int support, FILE *fp);
static int  _singlePath(FpTree *fptree);
static void _generatePatterns(FpNode *a, FpTree *fptree, FILE *fp);
static void _generateOnePattern(FpListLink ai, FpNode *a, FILE *fp);
static void _copyFpNode(FpNodeLink *nodeLink, const FpNode node);
extern void showFpTree(FpTree *fptree);
static void _showPatternBase(PatternBase *pb);
static void _showAssistList(NodeLink h);
static void _showHeadTable(FpHeadTable ht);
#endif
