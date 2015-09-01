typedef struct node_tag
		{
			int data;
			struct node_tag *next;
		}
		node;

typedef struct
		{
			node *head;
			node *tail;
			int size;
		}
		queue;

void queue_init(queue*);
void queue_free(queue*);
int queue_isempty(queue*);
void queue_add(queue*,int);
int queue_remove(queue*);
int queue_delete(queue*,int);
void queue_print(queue*);
