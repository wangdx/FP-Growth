#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fptree.h"
#include "tract.h"

void _initTreeRoot(FpNode **root)
{
	(*root) = (FpNode *)malloc(sizeof(FpNode));
	if(!(*root)){
		perror("out of memory!!!");
		exit(-1);
	}
	(*root)->field = NULL;
	(*root)->support = 0;
	(*root)->parent = NULL;
	(*root)->eldest = NULL;
	(*root)->sibling = NULL;
	(*root)->same = NULL;
}
void createFpTree(FpTree *fptree, FpHeadTable *headTable, ItemLink *tracts, int size)
{
	int i;
	fptree->headTable = headTable;
	_initTreeRoot(&fptree->root);             /* initialize the tree root */
	for(i = 0; i < size; ++i){
		ItemLink p = tracts[i];
		_insertTree(p, &fptree->root, fptree->headTable);
	}
}

FpNode* _matchedCfp(FpNode *q, Pattern p)
{                                           /* if matching, node's support++ and
																							 return the matched child(true), otherwise NULL(false) */
	for(; q; q = q->sibling){                 /* when using large data set, 
																							 here may cause some problems,
																							 e.g. segemental fault,
																							 I don't know why */
		if((q->field != NULL) && (p->field != NULL) && (strcmp(q->field, p->field)==0)){
			q->support += p->support;			        /* support increase */
			return q;                             /* find it */
		}
	}
	return NULL;                              /* not matched */
}
void _insertCfpTree(Pattern pt, FpNode *node, FpHeadTable *headTable)
{
	if(!pt || pt->field == NULL)return;
	if(node == NULL){
		printf("error parameter, node is null.\n");
		return;
	}
	FpNode *child = node->eldest;
	if((child != NULL) && (child = _matchedCfp(child, pt)));/*if node has first child and match, 
																														support++ and get the matched child */
	else{                                     /* else node does not have a child at all
																							 or no child(ren) can be matched */
		child = (FpNode *)malloc(sizeof(FpNode));
		if(!child){
			perror("out of memory!!!");exit(-1);
		}
		child->field = (char *)malloc(strlen(pt->field) + 1);
		if(!child->field){
			perror("out of memory!!!");
			exit(-1);
		}
		strcpy(child->field, pt->field);
		child->support = pt->support;           /* its support must be one */
		child->parent = node;                   /* as a child of node */
		child->eldest = NULL;                   /* there is not a child */
		child->sibling = node->eldest;          /* become a sibling of node's child */
		node->eldest = child;                   /* and as the first child (like a stack) */
		_linkSameNode(&child, headTable);       /* link together the nodes which have the same name to the item head table */
	}
	_insertCfpTree(pt->sibling, child, headTable);/* insert the tree recursively */
}
FpNode* _matched(FpNode *p, ItemLink itemLink)
{                                           /* if matching, node's support++ and 
																								 return the matched child(true), otherwise NULL(false) */
	for(; p; p = p->sibling){
		if(!strcmp(p->field, itemLink->field)){
			p->support++;                         /* support increase */
			return p;                             /* find it */
		}
	}
	return NULL;                              /* not matched */
}
void _insertTree(ItemLink itemLink, FpNode **node, FpHeadTable *headTable)
{
	int i;
	if(!itemLink || itemLink->field == NULL)return;
	if((*node) == NULL){
		printf("error parameter, node is null.\n");
		return;
	}
	FpNode *child = (*node)->eldest;
	if((child != NULL) && (child = _matched(child, itemLink)));/* if node has first child and 
																																match, support++ and get the matched child */
	else{                                     /* else node does not have a child at all 
																							 or no child(ren) can be matched */
		child = (FpNode *)malloc(sizeof(FpNode));
		if(!child){
			perror("out of memory!!!");exit(-1);
		}
		child->field = (char *)malloc(strlen(itemLink->field) + 1);
		if(!child->field){
			perror("out of memory!!!");exit(-1);
		}
		strcpy(child->field, itemLink->field);
		child->support = 1;                     /* its support must be one */
		child->parent = *node;                  /* as a child of node */
		child->eldest = NULL;                   /* there is not a child */
		child->sibling = (*node)->eldest;       /* become a sibling of node's child */
		(*node)->eldest = child;                /* and as the first child (like a stack) */
		_linkSameNode(&child, headTable);       /* link together the nodes which 
																							 have the same name to the item head table */
	}
	_insertTree(itemLink->next, &child, headTable);/* insert the tree recursively */
}
void _linkSameNode(FpNode **node, FpHeadTable *headTable)
{                                           /* link the same item-name nodes with item head table */
	int i;
	for(i = 0; i < headTable->count; ++i){
		if(!strcmp((*node)->field, headTable->lists[i]->field)){
			(*node)->same = headTable->lists[i]->head;
			headTable->lists[i]->head = (*node);
			break;
		}
	}
}
void createHeadTable(NodeLink *ht, int size, FpHeadTable *headTable, int numExceeded)
{                                           /* create a item head table for sorting and projecting */
	int i;                                    /* loop variables */
	headTable->count = 0;
	headTable->lists = (FpListLink *)calloc(numExceeded, sizeof(FpListLink));
	for(i = 0; i < size; ++i){                /* for every hash value */
		NodeLink p = ht[i];                     /* get the node pointer */
		for(; p; p = p->next){                  /* and for every node */
			if(p->exceeded == 'N')continue;       /* if not exceeded, do not put into head table, */
			FpListLink q = (FpListLink)malloc(sizeof(FpList));/* else create a node head */
			if(!q){
				perror("out of memory!!!");exit(-1);
			}
			q->field = (char *)malloc(strlen(p->field) + 1);/* item name */
			if(!q->field){
				perror("out of memory!!!");exit(-1);
			}
			strcpy(q->field, p->field);           /* copy from hash table */
			q->support = p->support;              /* and support */
			q->head = NULL;                       /* initial point to null, 
																							 ready to link nodes of freqent pattern tree */
			headTable->lists[headTable->count++] = q;/* assign to node table pointer */
		}
	}
	_sortHeadTable(headTable);
}
void _sortHeadTable(FpHeadTable *headTable)
{                                           /* binary insert sort */
	int i, j;
	int low, high, m;
	for(i = 1; i < headTable->count; ++i){
		FpListLink p = headTable->lists[i];
		low = 0; high = i-1;
		while(low <= high){
			m = (low + high)/2;
			if(p->support > headTable->lists[m]->support 
					/* || (p->support == headTable->lists[m]->support && strcmp(p->field, headTable->lists[m]->field) < 0)*/
					/* do not consider the lex. order */
				)
				high = m - 1;
			else low = m + 1;
		}
		for(j = i - 1; j >= high; --j)
			headTable->lists[j+1] = headTable->lists[j];
		headTable->lists[high+1] = p;
	}
}
void showFpTree(FpTree *fptree)
{
	FpHeadTable *ht = fptree->headTable;
	FpNode *r = fptree->root;
	FpListLink *lists = ht->lists;
	int i, c = ht->count;
	for(i = 0; i < c; ++i){
		printf("---[%d]---:\n", i);
		FpListLink t = lists[i];
		FpNodeLink p = t->head;
		while(p){
			printf("field=[%s:%d],parentfield=[%s:%d],eldest=[%s:%d],sibling=[%s:%d],same=[%s:%d]\n",
									p->field, p->support, p->parent->field,p->parent->support, 
									p->eldest==NULL?"-":p->eldest->field,p->eldest==NULL?0:p->eldest->support, 
									p->sibling==NULL?"-":p->sibling->field, p->sibling==NULL?0:p->sibling->support,
									p->same == NULL?"-":p->same->field,p->same == NULL?0:p->same->support);
			p = p->same;
		}
	}
}

int _singlePath(FpTree *fptree)
{
	assert(fptree);
	FpHeadTable *ht = fptree->headTable;
	FpNode *p = fptree->root;
	if(ht->count == 1)return 1;               /* only one node */
	while(p){
		if(p->eldest && p->eldest->sibling)return 0;/* p has at least two children */
		p = p->eldest;
	}
	return 1;
}
void _copyFpNode(FpNodeLink *nodeLink, const FpNode node)
{
	(*nodeLink)->field = (char *)malloc(strlen(node.field) + 1);
	if(!(*nodeLink)->field){
		perror("out of memory!!!");exit(-1);
	}
	strcpy((*nodeLink)->field, node.field);
	(*nodeLink)->support = node.support;
	(*nodeLink)->parent = NULL;
	(*nodeLink)->eldest = NULL;
	(*nodeLink)->sibling = NULL;
	(*nodeLink)->same = NULL;
}
void fpgrowth(FpTree *fptree, FpNode *a, int support, FILE *fp)
{
	int i;
	if(!fptree)return;
	if(_singlePath(fptree)){
		_generatePatterns(a, fptree, fp);       /* output frequent item sets */
	} else {
		FpHeadTable *ht = fptree->headTable;
		FpNode *root = fptree->root;
		for(i = ht->count - 1; i >= 0; --i){
			FpListLink aiLink= ht->lists[i];
			_generateOnePattern(aiLink, a, fp);   /* output frequent item sets at first */
			Pattern b = (Pattern)malloc(sizeof(FpNode));/* and then generate the required pattern b */
			if(!b){
				printf("*** out of memory! *** failed to create pattern b \n");
				exit(-1);
			}
			_copyFpNode(&b, *(aiLink->head));     /* b is ai now */
			b->eldest = a;                        /* b = (ai U a) TODO check if we can use a, with assuming ok */
			FpTree *cfptree = (FpTree *)malloc(sizeof(FpTree));
			if(!cfptree){
				printf("*** out of memory! *** failed to create conditional fptree \n");
				exit(-1);
			}
			/* we can use ai rather than b to create c.f.p. tree, 
				 b, containing (ai U a), will be used in fpgrowth recursively. 
				 i is ai's index in headTable */
			_buildCondFpTree(cfptree, fptree, i, support);
			/*printf("-------------[%d] fptree\n", i);
			showFpTree(cfptree);
			_generatePatterns(NULL, cfptree, stdout);printf("----test---\n");return;*/
			if(cfptree->root){
				fpgrowth(cfptree, b, support, fp);
			}
		}
	}
}
void _buildCondFpTree(FpTree *cfptree, FpTree *fptree, int idx, int support)
{
	PatternBase *pbHead = (PatternBase *)malloc(sizeof(PatternBase));/* every suffix pattern has its own p.b. */
	if(!pbHead){
		perror("can not create PatternBase");
		exit(-1);
	}
	if(NULL == (cfptree->headTable = (FpHeadTable *)malloc(sizeof(FpHeadTable)))){
		perror("can not create head table ");
		exit(-1);
	}
	_buildCondPatternBase(pbHead, fptree, idx);
	//printf("-------------[%d]---\n", idx);// for test
	//_showPatternBase(pbHead);/* for test */
	_buildHeadTable(cfptree->headTable, pbHead, support);
	//_showHeadTable(*(cfptree->headTable));
	_initTreeRoot(&cfptree->root);
	PatternBase pbh = (*pbHead);
	for(pbh = pbh->eldest; pbh; pbh = pbh->eldest){
		_insertCfpTree(pbh->sibling, cfptree->root, cfptree->headTable);
	}
	//showFpTree(cfptree);
	//_showPatternBase(pbHead);
	//TODO free the memory!!!
}
void _buildHeadTable(FpHeadTable *ht, PatternBase *pbh, int support)
{
	NodeLink p, listHead = (NodeLink)malloc(sizeof(Node));/* put all p.b. into a link list for compute 
																												number and support of the same items, simple idea */
	if(!listHead){
		perror("can not create link list head ");exit(-1);
	}
	int size = _assistUtil(&listHead, *pbh, support);/* build a list, which has a head node */
	ht->count = 0;                            /* initialize the number of lists */
	if(size <= 0)return;                      /* need not to create head table any more */
	//_showAssistList(listHead);/* for test */
	ht->lists = (FpListLink *)calloc(size, sizeof(FpListLink));
	if(ht->lists == NULL){
		perror("can not create head table lists ");exit(-1);
	}
	for(p = listHead->next; p; p = p->next){  /* I'm NOT sure whether the link list is descend order or not */
		if(p->exceeded == 'N')continue;         /* so, traverse it then sort the head table again, ignoring the unsupport one */
		FpListLink q = (FpListLink)malloc(sizeof(FpList));/* else create a node head */
		if(!q){
			perror("out of memory!!!");exit(-1);
		}
		q->field = (char *)malloc(strlen(p->field) + 1);/* item name */
		if(!q->field){
			perror("out of memory!!!");exit(-1);
		}
		strcpy(q->field, p->field);             /* copy from hash table */
		q->support = p->support;                /* and support */
		q->head = NULL;                         /* initial point to null, ready to link nodes of freqent pattern tree */
		ht->lists[ht->count++] = q;             /* assign to node table pointer */
	}
	_sortHeadTable(ht);                       /* here, we sort it again, time consuming :( */
	//_showHeadTable(*ht);// for test
}
int _assistUtil(NodeLink *h, PatternBase pbh, int support)
{/* build a assistant list for building the head table,
		prune the p.b., w.r.t., delete the unsupport nodes int p.b.,
		return the number of nodes exceeding the support as well */
	int numOfExceeded = 0;
	NodeLink listHead = (*h);                 /* initialize the link list head */
	listHead->field = NULL;
	listHead->support = 0;                    /* regard as counter of the distinct nodes in the list */
	listHead->exceeded = 'A';                 /* nothing, just initialize it */
	listHead->next = NULL;
	PatternBase q = pbh->eldest;
	PatternBase p = NULL;
	while(q){
		p = q->sibling;                         /* sth segemental fault may occur here */
		while(p){
			NodeLink s = listHead->next;
			NodeLink r = listHead;
			for(; s; s = s->next, r = r->next){   /* r->next is s */
				if(!strcmp(p->field, s->field)){
					s->support += p->support;
					if(s->exceeded == 'N' && support <= s->support){/* if someone comes up to the threshold */
						s->exceeded = 'Y';              /* mark it */
						++numOfExceeded;                /* and increase the number */
					}
					break;
				}
			}
			if(!s){                               /* there is not a same node */
				listHead->support++;                /* regard head's support as counter of the list */
				if(NULL == (r->next = (NodeLink)malloc(sizeof(Node)))){
					perror("can not create Node ");
					exit(-1);
				}
				r->next->field = (char *)malloc(strlen(p->field) + 1);
				if(!r->next->field){
					perror("out of memory!!!");exit(-1);
				}
				strcpy(r->next->field, p->field);
				r->next->support = p->support;
				r->next->next = NULL;
				if(support <= p->support){
					r->next->exceeded = 'Y';
					++numOfExceeded;                  /* number of nodes exceeding the support */
				} else {
					r->next->exceeded = 'N';
				}
			}
			p = p->sibling;
		}
		q = q->eldest;                          /* next set */
	}
	if(numOfExceeded < listHead->support){    /* support nodes is less than total ones */
		_prunePatternBase(pbh, listHead, listHead->support - numOfExceeded);
	}
	//_showPatternBase(pbh);// for test
	return numOfExceeded;
}
void _prunePatternBase(PatternBase pbh, NodeLink list, int n)
{                                           /* delete n unsupport node(s) in the p.b. */
	Item items[n];
	NodeLink p = list->next;
	int i = 0;
	for(; p; p = p->next){
		if(p->exceeded == 'Y'){
			continue;
		}
		items[i].field = (char *)malloc(strlen(p->field) + 1);
		if(!items[i].field){
			perror("out of memory!!!");exit(-1);
		}
		strcpy(items[i++].field, p->field);
	}
	PatternBase r = pbh->eldest;
	for(; r; r = r->eldest){
		PatternBase s = r->sibling;
		PatternBase x = r;
		for(; s; s = s->sibling, x = x->sibling){
			for(i = 0; i < n; ++i){
				if(!strcmp(items[i].field, s->field)){
					x->sibling = s->sibling;
					free(s->field);                   /* we could not free the char pointer, why?*/
					free(s);
					break;
				}
			}
		}
	}
}
void _showPatternBase(PatternBase *pbHead)
{
	printf("------------\n");
	PatternBase q = *pbHead;
	for(q = q->eldest; q; q = q->eldest){
		PatternBase p = q->sibling;
		printf("***\n");
		while(p){
			printf("field=[%s:%d],sibling=[%s],eldest=[%s]\n",
									p->field, p->support, 
									p->sibling==NULL?"-":p->sibling->field, 
									p->eldest==NULL?"-":p->eldest->field
									);
			p = p->sibling;
		}
	}
}
void _showAssistList(NodeLink h)
{
	NodeLink p = h->next;
	printf("number of nodes in list:[%d]\n", h->support);
	while(p){
		printf("[%s:%d][%c]\n", p->field, p->support, p->exceeded);
		p = p->next;
	}
}
void _showHeadTable(FpHeadTable ht)
{
	printf("number of lists in head table:[%d]\n", ht.count);
	int i;
	for(i = 0; i < ht.count; ++i){
		printf("[%s:%d]\n", ht.lists[i]->field, ht.lists[i]->support);
	}
}
void _buildCondPatternBase(PatternBase *pbHead, FpTree *fptree, int idx)
{/* use FpNode to create nodes of pattern bases, 
		->eldest as linking the next set, while 
		->sibling as next pattern base in the same set,
		so, pattern base pb need have a head node with null field
		*/
	(*pbHead) = (PatternBase)malloc(sizeof(FpNode));/* head node of the sets' link list */
	if(!(*pbHead)){
		perror("out of memory!!!");exit(-1);
	}
	FpNodeLink p = fptree->headTable->lists[idx]->head;/* the first node pointered by idxth head table node in fptree */
	PatternBase *q = pbHead;
	while(p){                                 /* traverse the chain of node-links with same item name */
		FpNodeLink r = (FpNode *)malloc(sizeof(FpNode));
		if(!r){
			printf("*** out of memory! *** can not create a set link node \n");
			exit(-1);
		}
		_copyFpNode(&r, *p);                    /* r's support is assigned absolutely p's support */
		r->eldest = (*q)->eldest;               /* link the sets with ->eldest */
		(*q)->eldest = r;
		FpNodeLink s = p;
		s = s->parent;                          /* create conditional pattern base starts from it's parent */
		while(s && s->field){                   /* traverse the path along with ->parent, no root(->field is null)*/
			FpNodeLink t = (FpNode *)malloc(sizeof(FpNode));
			if(!t){
				printf("*** out of memory! *** can not create a same node \n");
				exit(-1);
			}
			_copyFpNode(&t, *s);
			t->support = r->support;              /* must be the support of node in chain of node-links */
			t->sibling = r->sibling;              /* link to the same set with ->sibling */
			r->sibling = t;
			s = s->parent;
		}
		p = p->same;
	}
}
void _generatePatterns(FpNode *a, FpTree *fptree, FILE *fp)
{
	int support = MAX_SUPPORT;
	int cnt = 0;                              /* for combination */
	int i, j;
	int n = fptree->headTable->count;         /* number of nodes in the single path */
	FpNode *path = fptree->root;
	FpNode *cur = path;
	FpNode *b;
	while(n--){                               /* traverse whole path which has at least one node */
		for(i = 0, b = cur->eldest; i <= n; ++i){/* for each combination (denoted as b) of nodes in the path  */
			FpNode *b_cur = b;
			for(support = MAX_SUPPORT, j = 0; j <= i && b_cur; ++j, b_cur = b_cur->eldest){/* traverse one combination */
				if(support > b_cur->support)support = b_cur->support;/* minimum support count of nodes in b */
				fprintf(fp, "%s ", b_cur->field);
			}
			FpNode *a_cur = a;
			while(a_cur){                         /* traverse the suffix pattern a */
				fprintf(fp, "%s ", a_cur->field);
				a_cur = a_cur->eldest;
			}
			fprintf(fp, "[%d]\n", support);
			++NumberOfFrequentSets;
		}
		cur = cur->eldest;                      /* cur and n have synchronous change */
	}
}
void _generateOnePattern(FpListLink ai, FpNode *a, FILE *fp)
{
	FpNode *p = a;
	fprintf(fp, "%s ", ai->field);
	while(p){
		fprintf(fp, "%s ", p->field);
		p = p->eldest;
	}
	fprintf(fp, "[%d]\n", ai->support);
	++NumberOfFrequentSets;
}
