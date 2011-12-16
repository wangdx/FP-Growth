#ifndef _TRACT_H
#define _TRACT_H

#define BUF_SIZE 1024                       /* initialize 1k buffer for getline() */
#define DMN_SPRT '\t'                        /* seperator for dimension */
#define RCD_SPRT '\n'                       /* seperator for record */

typedef struct Node{
	char				exceeded;                     /* whether exceed the support threshold('Y') or not('N') */
	char        *field;                       /* item name field */
	int         support;                      /* support threshold*/
	struct Node *next;                        /* link to successor */
} Node, *NodeLink;                          /* item set node */

typedef struct Item{                        /* item */
	char *field;                              /* name */
	struct Item *next;                        /* link */
}Item, *ItemLink;

extern void findFItemset(FILE *file, ItemLink *trans, NodeLink *np, int support, int transNum, int itemsNum);
extern int  hash(char *buf, int tableSize);
extern void createFItemset(ItemLink *tracts, int idx,  NodeLink *np, int support, char *trans, int itemsNum);
extern void createTract(ItemLink *tracts, int idx, char *field);
extern void deleteUnsupport(ItemLink *tracts, int length, int support, NodeLink *hashTable, int size);
extern void sortEveryTract(ItemLink *tracts, int length, NodeLink *ht, int size, int support);
extern void selectSort(ItemLink *head, NodeLink *ht, int size, int support);
extern void createHashTable(NodeLink *np, char *field, int support, int hashSize);
extern int getNumOfExceeded(NodeLink *hashTable, int size);
extern int getSupport(NodeLink *hashTable, int size, char *field);
#endif
