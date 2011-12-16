/*
 * File: tract.c
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tract.h"

void findFItemset(FILE *file, ItemLink *tracts, NodeLink *np, int support, int transNum, int itemsNum)
{                                           /* find frequent itemset */
	int idx = 0;                              /* for transaction's loop */
	int read = 0;
	char *trans = NULL;
	trans = (char *)malloc(BUF_SIZE*sizeof(char));/* read a transaction per loop */
	if(!trans){
		perror("out of memory!!!");exit(-1);
	}
	read = BUF_SIZE;                          /* and initial size */
  assert(file);                             /* check the file */
	while((read = getline(&trans, &read, file)) != -1){/* get one transaction from a line */
		createFItemset(tracts, idx++, np, support, trans, itemsNum);
	}                                         /* and create itemset using hash table structure*/
	if(trans)free(trans);                     /* free transaction buffer */
}

void createFItemset(ItemLink *tracts, int idx, NodeLink *np, int support, char *trans, int itemsNum)
{                                           /* create freq. itemset */
	int hashSize = itemsNum;
	char *p;
	char seperators[3] = {DMN_SPRT, RCD_SPRT, '\0'};/* dimension and record seperator */
	const char *delim = seperators;           /* as delim */
	p = strtok(trans, delim);
	p = strtok(NULL, delim);                  /* transaction id */
	p = strtok(NULL, delim);                  /* items number */
	while(p){
		createTract(tracts, idx, p);            /* create a transaction array for sorting and projecting */
		createHashTable(np, p, support, hashSize);/* put into hash table for counting support */
		p = strtok(NULL, delim);                /* of each item(column) */
	}
}
void createTract(ItemLink *tracts, int idx, char *field)
{                                           /* put the idxth transaction into tracts array*/
	Item *i = (Item *)malloc(sizeof(Item));   /* create an item */
	if(!i){
		perror("out of memory!!!");exit(-1);
	}
	i->field = (char *)malloc(strlen(field) + 1);/* and its name */
	if(!i->field){
		perror("out of memory!!!");exit(-1);
	}
	strcpy(i->field, field);                  /* fill with data source */
	i->next = tracts[idx];                    /* then link to the first place*/
	tracts[idx] = i;                          /* of tracts array */
}
void deleteUnsupport(ItemLink *tracts, int length, int support, NodeLink *hashTable, int size)
{                                           /* delete the items, which does not reach the 
																							 support threshold,  in transactions */
	int i;
	for(i = 0; i < length; ++i){
		ItemLink p, q = tracts[i];
		while(q && getSupport(hashTable, size, q->field) < support){/* deal with the first item near the head */
			tracts[i] = q->next;
			free(q);
			q = tracts[i];
		}
		if(!q || !q->next)continue;
		p = q->next;                            /* others */
		while(p){
			if(getSupport(hashTable, size, p->field) < support){
				q->next = p->next;
				free(p);
				p = q->next;
				continue;
			}
			p = p->next;
			q = q->next;
		}
	}
}
void sortEveryTract(ItemLink *tracts, int length, NodeLink *hashTable, int size, int support)
{                                           /* sort all of the transactions with select sort */
	int i;
	for(i = 0; i < length; ++i){
		selectSort(&tracts[i], hashTable, size, support);
	}
}
void selectSort(ItemLink *head, NodeLink *hashTable, int size, int support)
{                                           /* descending sort transaction based on support, 
																							 and lexcographical if equal support */
	ItemLink p, q, t, s, h;
	h = (ItemLink)malloc(sizeof(Item));
	if(!h){
		perror("out of memory!!!");exit(-1);
	}
	h->next = (*head);
	p = h;
	while(p->next && p->next->next != NULL){
		for(s = p, q = p->next; q->next != NULL; q = q->next){
			int spt_q = getSupport(hashTable, size, q->next->field);
			int spt_s = getSupport(hashTable, size, s->next->field);
			if(spt_q > spt_s || (spt_q == spt_s &&
				 strcmp(q->next->field, s->next->field) < 0))
				s = q;
		}
		if(s != q){
			t = s->next;
			s->next = t->next;
			t->next = p->next;
			p->next = t;
		}
		p = p->next;
	}
	(*head) = h->next;
	free(h);
}

int hash(char *buf, int tableSize)
{                                           /* BKDR Hash Function */
	unsigned int seed = 5;                    /* namely seed is 31 */
	unsigned int h = 0;
	char *p = buf;
	while(*p){
		h = (h << seed) - h + (*p++);           /* h = h*31 + (*p++) */
	}
	return h % tableSize;
}


void createHashTable(NodeLink *np, char *field, int support, int tableSize)
{                                           /* use link address method to create hash table */
	int h = hash(field, tableSize);
	NodeLink p;
	for(p = np[h]; p; p=p->next){             /* traverse the whole hash table */
		if(strcmp(p->field, field) == 0){       /* if here it is */
			p->support++;                         /* increase its support */
			if(p->support >= support) p->exceeded = 'Y';/* exceed the threshold */
			return;                               /* and return */
		}
	}
	p = (NodeLink)malloc(sizeof(Node));       /* else create a new node */
	if(!p){
		perror("out of memory!!!");exit(-1);
	}
	p->field = (char *)malloc(strlen(field) + 1);
	if(!p->field){
		perror("out of memory!!!");exit(-1);
	}
	strcpy(p->field, field);
	p->support = 1;
	if(support > 1)	p->exceeded = 'N';        /* default value when initializing */
	else p->exceeded = 'Y';
	p->next = np[h];                          /* and insert it to the hash table */
	np[h] = p;                                /* at the first place of this hash value */
}

int getNumOfExceeded(NodeLink *hashTable, int size)
{                                           /* get the number of items whose support has exceeded the threshold */
	int i;
	int num = 0;
	for(i = 0; i < size; ++i){
		NodeLink p = hashTable[i];
		for(; p; p = p->next){
			if(p->exceeded == 'Y')
				++num;
		}
	}
	return num;
}

int getSupport(NodeLink *hashTable, int size, char *field)
{
	int h = hash(field, size);
	NodeLink p;
	for(p = hashTable[h]; p; p=p->next){
		if(strcmp(p->field, field) == 0)
			return p->support;
	}
	printf("input error, can not find the key[%s]\n", field);
	return -1;
}
