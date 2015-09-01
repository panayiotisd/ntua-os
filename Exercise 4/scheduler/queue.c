#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

void queue_init(queue *q)
{
	q->head=NULL;
	q->tail=NULL;
	q->size=0;
	return;
}

void queue_free(queue *q)
{
	while(q->head)
	{
		node *p=q->head;
		q->head=q->head->next;
		free(p);
	}
	return;
}

int queue_isempty(queue *q)
{
	return !(q->head);
}

void queue_add(queue *q,int x)
{
	node *p=(node*)malloc(sizeof(node));
	p->data=x;
	p->next=NULL;
	if(queue_isempty(q)) q->head=p;
	else q->tail->next=p;
	q->tail=p;
	q->size++;
	return;
}

int queue_remove(queue *q)
{
	if(queue_isempty(q))
	{
		fprintf(stderr,"Error: dequeue: Queue is empty\n");
		exit(1);
	}
	node *p=q->head;
	q->head=q->head->next;
	q->size--;
	if(queue_isempty(q)) q->tail=NULL;
	int x=p->data;
	free(p);
	return x;
}

int queue_delete(queue *q,int x)
{
	node *p=q->head;
	if(!p) return 0;
	if(p->data==x)
	{
		q->head=q->head->next;
		q->size--;
		if(queue_isempty(q)) q->tail=NULL;
		free(p);
		return 1;
	}
	while(p->next&&p->next->data!=x) p=p->next;
	if(p->next)
	{
		node *t=p->next;
		p->next=p->next->next;
		q->size--;
		if(q->tail==t) q->tail=p;
		free(t);
		return 1;
	}
	return 0;
}

void queue_print(queue *q)
{
	printf("QUEUE : [");
	node *p=q->head;
	while(p)
	{
		printf(" %d",p->data);
		p=p->next;
	}
	printf(" ]\n");
	return;
}
