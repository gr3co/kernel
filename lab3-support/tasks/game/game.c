
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <bits/fileno.h>

#define BUFSIZE 4
#define ROUNDS 10
int main() { 

	unsigned num1;
	unsigned num2;
	unsigned ans;
	int score;

	unsigned start;
	unsigned end;
	int n;

	char buf[4];

	printf("Welcome to Game!\n");
	printf("You must solve these addition problems as quickly as possible.\n");
	printf("You're score depends on how much time you have left, 2 seconds for each problem\n");
	srand(time());
	
	start = time();
	for(n=0;n < ROUNDS; n++) {
		num1 = ((unsigned)rand()) % 100;
		num2 = ((unsigned)rand()) % 100;
		ans = num1 + num2;
		printf("%d + %d = ?\n",num1,num2);
		
		read(STDIN_FILENO,buf,BUFSIZE);

		if (atoi(buf) == ans) {
			printf("correct\n");
			score = += 1;
		}

		else {
			printf("wrong\n");
		}

		printf("score:%d\n",score);
		srand(time() * atoi(buf));
	}
	end = time();
	unsigned playing = (end - start) / 1000;
	int final = (playing > 20) ? score - (playing - 20) : score + (20 - playing);
	printf("You took %d.%d seconds\n",(end - start)/1000, ((end - start) % 1000) / 100));
	printf("Your final score is %d\n",final);

}