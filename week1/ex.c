#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct node NODE;
typedef NODE* STACK;

struct node {
	struct node *next;
	int data;
};

STACK stack;
int count;

int fds[2];
char buf[512];

int peek() {
	if (stack == NULL) {
		printf("STACK IS EMPTY\n");
		exit(-1);
	} else return stack->data;
}

void push(int data) {
	NODE *node = (NODE*)malloc(sizeof(NODE));
	node->data = data;
	if (count == 0) {
		stack = node;
		node->next = NULL;
	} else {
		node->next = stack;
		stack = node;
	}
	count++;
	printf("PUSHED\n");
}

void pop() {
	if (count == 0) {
		printf("STACK IS EMPTY\n");
		return;
	} else if (count == 1) stack = NULL;
	else {
		stack = stack->next;
	}
	count--;
	printf("POPPED\n");
}

int empty() {
	return (count == 0);
}

void display() {
	NODE *node = stack;
	printf("STACK: { ");
	for (int i = 0; i < count; i++) {
		printf("%d ", node->data);
		node = node->next;
	}
	printf("}\n");
}

void create() {
	stack = NULL;
	count = 0;
	printf("CREATED NEW STACK\n");
}

void stack_size() {
	printf("STACK SIZE: %d\n", count);
}

int readline(char *buf, size_t sz, int fd) {
	size_t i, count;
	i = 0;
	while(i < sz && (count = read(fd, buf+i, 1)) && buf[i] != '\n')
		i++;
	if (count == -1) return 0;
	if (buf[i] == '\n') buf[i] = '\0';
	return i;
}

int writeline(char *buf, size_t sz, int fd) {
	size_t i, count;
	i = 0;
	while(buf[i] != '\n' && buf[i] != '\0') {
		write(fd, buf+i, 1);
		i++;
	}
	write(fd, "\n", 1);
	if (count == -1) return 0;
	return i;
}



int handle_input() {
	char cmd[100];
	char arg[100];
	int i, k, j;
	i = k = j = 0;

	while (k < 100 && i < 512 && buf[i] != '(' && buf[i] != '\0')
		if (buf[i++] != ' ') cmd[k++] = buf[i-1];
	cmd[k] = '\0';
	if (buf[i] != '(') {
		printf("NOT A COMMAND [expected '(' after '%s']\n", cmd);
		return 1;
	}

	i++;
	while (j < 100 && i < 512 && buf[i] != ')' && buf[i] != '\0')
		if (buf[i++] != ' ') arg[j++] = buf[i-1];
	arg[j] = '\0';
	if (buf[i] != ')') {
		printf("NOT A COMMAND [expected ')' after '%s']\n", arg);
		return 1;
	}

	if (strcmp(cmd, "push") == 0)
		if (strlen(arg) > 0 && (atoi(arg) != 0 | (strlen(arg) == 1 && arg[0] == '0')))
			push(atoi(arg));
	else if (strcmp(cmd, "pop") == 0) pop();
	else if (strcmp(cmd, "display") == 0) display();
	else if (strcmp(cmd, "peek") == 0) printf("PEEK: %d\n", peek());
	else if (strcmp(cmd, "empty") == 0) printf("%sEMPTY\n", empty() ? "" : "NOT ");
	else if (strcmp(cmd, "create") == 0) create();
	else if (strcmp(cmd, "stack_size") == 0) stack_size();
	else if (strcmp(cmd, "exit") == 0) return 0;
	else printf("COMMAND NOT FOUND\n");

	return 1;
}

int read_input() {
	return (fgets(buf, 512, stdin) != NULL);
}

int read_pipe() {
	return readline(buf, 512, fds[0]);
}

int main(int argc, char* argv[]) {
	pipe(fds);
	int pid = fork();
	memset(buf, 0, 512);
	if (pid > 0) {
		//close(fds[0]);
		while(read_input()) writeline(buf, 512, fds[1]);
	} else if (pid == 0) {
		//close(fds[1]);
		while(read_pipe()) handle_input();
	} else exit(-1);

	return 0;
}
