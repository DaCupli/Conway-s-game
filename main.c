#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

#define MAX_CELLS 100000
#define WIDTH 50
#define HEIGHT 20
#define X_CURSOR 25
#define Y_CURSOR 10

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

long cam_x = 0;
long cam_y = 0;
long cells_x[MAX_CELLS];
long cells_y[MAX_CELLS];
long copy_x[MAX_CELLS];
long copy_y[MAX_CELLS];
long copy_num_cells = 0;

int num_cells = 0;

char numjump[5];
char ndigits = 0;
char states[(int) MAX_CELLS * 9 / 8];

char getch(void);

int intpow(int base, int exp);

void check_neighbours(long x, long y, char recheck);
void show(void);
void get_input(void);
void move_cam(char direction);
void placeremove(void);
void remove_cell(int element);
void update(void);
void remove_clones(void);

char getch(void) {
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if(tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if(read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}

void show(void) {

	system("clear");

	char screen[HEIGHT][WIDTH];

	int i;
	int j;

	for (i = 0; i < HEIGHT; i++) {

		for (j = 0; j < WIDTH; j++) {

			screen[i][j] = ' ';
		}
	}

	for (i = 0; i < num_cells; i++) {

		long x = cells_x[i] - cam_x;
		long y = cells_y[i] - cam_y;

		if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
			screen[(int) y][(int) x] = '#';
		}
	}

	if (screen[Y_CURSOR][X_CURSOR] == '#')
		screen[Y_CURSOR][X_CURSOR] = 'X';
	else
		screen[Y_CURSOR][X_CURSOR] = 'x';

	for (i = 0; i < HEIGHT; i++) {

		for (j = 0; j < WIDTH; j++) {

			printf("%c", screen[i][j]);
		}

		printf("-\n");
	}
}

void get_input(void) {
	
again:

	char ch = getch();
	
	if ((ch - 0x30) >= 0 && (ch - 0x30) <= 9) {
		
		numjump[ndigits] = ch - 0x30;	
		printf("%d", ch - 0x30);
		ndigits++;

		goto again;
	} else {

		switch (ch) {

		case 'h':
			move_cam(LEFT);
			break;
		case 'j':
			move_cam(DOWN);
			break;
		case 'k':
			move_cam(UP);
			break;
		case 'l':
			move_cam(RIGHT);
			break;
		case 'x':
			placeremove();
			break;
		case 'c':
			update();
			break;
		}	
	}
}

int intpow(int base, int esp) {
	
	int b = 1;
	int i;

	for (i = esp; i > 0; i--)
		b *= base;

	return b;
}

void check_neighbours(long x, long y, char recheck) {

	char i;
	char j;
	char alives = 0;

	for (i = -1; i <= 1; i++) {

		for (j = -1; j <= 1; j++) {

			if (!(i == 0 && j == 0)) {

				long n_x = x + (long) j;
				long n_y = y + (long) i;

				char alive = 0;

				int k;

				for (k = 0; k < num_cells; k++) {
					
					if (n_x == cells_x[k] && n_y == cells_y[k]) {
						alive = 1;
						break;
					}
				}
			
				if (recheck && !alive)
					check_neighbours(n_x, n_y, 0);
				else if (alive)
					alives++;
			}
		}
	}

	if (recheck && alives >= 2 && alives <= 3 || (!recheck && alives == 3)) {

		copy_x[copy_num_cells] = x;
		copy_y[copy_num_cells] = y;

		copy_num_cells++;
	}
}

void remove_clones(void) {

	int i;
	for (i = 0; i < num_cells - 1; i++) {

		int k;
		for (k = i + 1; k < num_cells; k++) {
			
			if (cells_x[i] == cells_x[k] && cells_y[i] == cells_y[k]) {
				
				remove_cell(k);
				k -= 1;
			}
		}
	}
}

void update(void) {

	for (int i = 0; i < num_cells; i++) {

		check_neighbours(cells_x[i], cells_y[i], 1);
	}

	num_cells = copy_num_cells;
	
	copy_num_cells = 0;

	for (int i = 0; i < num_cells; i++) {

		cells_x[i] = copy_x[i];
		cells_y[i] = copy_y[i];

	}
	
	remove_clones();
}

void remove_cell(int element) {

	int i;
	for (i = element; i < num_cells - 1; i++) {

		cells_x[i] = cells_x[i + 1];
		cells_y[i] = cells_y[i + 1];
	}

	num_cells--;
}

void placeremove(void) {

	long x = cam_x + X_CURSOR;
	long y = cam_y + Y_CURSOR;

	int i;
	for (i = 0; i < num_cells; i++) {

		if (cells_x[i] == x && cells_y[i] == y) {
			
			remove_cell(i);
			return;
		}
	} 
	
	cells_x[num_cells] = x;
	cells_y[num_cells] = y;

	num_cells++;
}

void move_cam(char direction) {

	int num = 0;
	
	char i;
	for (i = 0; i < ndigits; i++) {
		
		num += numjump[i] * intpow(10, ndigits - 1 - i);
		numjump[i] = 0;
	}

	if (num == 0)
		num = 1;

	switch(direction) {

	case UP:
		cam_y -= num;
		break;
	case DOWN:
		cam_y += num;
		break;
	case LEFT:
		cam_x -= num;
		break;
	case RIGHT:
		cam_x += num;
		break;
	}

	ndigits = 0;
}

int main(void) {

	for (int i = 0; i < 5; i++) {

		numjump[i] = 0;
	}

	while (1) {

		show();
		printf("X: %ld. Y: %ld.\n", cam_x, cam_y);
		get_input();
	}

	return 0;
}
