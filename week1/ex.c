#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define ERR_PATTERN "INVALID COMMAND, expected '%s' after '%s'\n"
#define ARGEXP_PATTERN "INVALID COMMAND, expected argument of type '%s'\n"
#define HELP_ANNOTATION "AVAILABLE COMMANDS: push(int), pop(), display(), peek(), " \
			"empty(), create(), stack_size(), exit()\n"
#define BUF_SIZE 512
#define ARG_SIZE 256
#define CMD_SIZE 256

typedef struct node NODE;

struct node {
	struct node *next;
	int data;
};

NODE* stack;
int count, pid, fds[2];
char buf[BUF_SIZE];

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
	return 1;
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
	return 1;
}



int handle_input() {
	char cmd[CMD_SIZE];
	char arg[ARG_SIZE];
	int i, k, j;
	i = k = j = 0;

	while (k < CMD_SIZE && i < BUF_SIZE && buf[i] != '(' && buf[i] != '\0')
		if (buf[i++] != ' ') cmd[k++] = buf[i-1];
	cmd[k] = '\0';
	if (buf[i] != '(') {
		printf(ERR_PATTERN, "(", cmd);
		return 1;
	}

	i++;
	while (j < ARG_SIZE && i < BUF_SIZE && buf[i] != ')' && buf[i] != '\0')
		if (buf[i++] != ' ') arg[j++] = buf[i-1];
	arg[j] = '\0';
	if (buf[i] != ')') {
		printf(ERR_PATTERN, ")", cmd);
		return 1;
	}

	if (strcmp(cmd, "push") == 0) {
		if (strlen(arg) > 0 && (atoi(arg) != 0 || strcmp(arg, "0") == 0))
			push(atoi(arg));
		else printf(ARGEXP_PATTERN, "int");
	}
	else if (strcmp(cmd, "pop") == 0) pop();
	else if (strcmp(cmd, "display") == 0) display();
	else if (strcmp(cmd, "peek") == 0) printf("PEEK: %d\n", peek());
	else if (strcmp(cmd, "empty") == 0) printf("%sEMPTY\n", empty() ? "" : "NOT ");
	else if (strcmp(cmd, "create") == 0) create();
	else if (strcmp(cmd, "stack_size") == 0) stack_size();
	else if (strcmp(cmd, "exit") == 0) {
		printf("PRESS ANY KEY TO PROCEED");
		exit(0);
	} else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0 || strlen(cmd) == 0) 
		printf(HELP_ANNOTATION);
	else printf("COMMAND NOT FOUND\n");

	return 1;
}

int read_input() {
	return readline(buf, BUF_SIZE, STDIN_FILENO);
}

int read_pipe() {
	return readline(buf, BUF_SIZE, fds[0]);
}

int main(int argc, char* argv[]) {
	pipe(fds);
	pid = fork();
	memset(buf, 0, BUF_SIZE);
	setvbuf(stdout, NULL, _IONBF, BUF_SIZE);
	if (pid > 0) {
		close(fds[0]);
		while(read_input()) writeline(buf, BUF_SIZE, fds[1]);
	} else if (pid == 0) {
		close(fds[1]);
		printf(HELP_ANNOTATION);
		printf("<command>: ");
		while(read_pipe() && handle_input()) printf("<command>: ");
	} else exit(-1);

	return 0;
}
