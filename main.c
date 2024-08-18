#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define WIDTH 70
#define HEIGHT 20
#define PADDLE_WIDTH 6
#define PADDLE_SPEED 2
#define BALL_CHAR 'O'
#define PADDLE_CHAR '='
#define BRICK_CHAR '#'
#define PADDLE_OFFSET 1
#define BALL_SIZE 1
#define NUM_BRICKS 50
#define BRICK_WIDTH 5
#define BRICK_HEIGHT 1
#define GAME_SPEED 200000

int ball_x, ball_y, ball_dx, ball_dy;
int paddle_x;
char display[WIDTH][HEIGHT];
int score, lives;

struct Brick {
    int x;
    int y;
    int visible;
};

struct Brick bricks[NUM_BRICKS];

void clear_screen() {
    printf("\033[2J");
    printf("\033[H");
}

int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void init_game() {
    ball_x = WIDTH / 2;
    ball_y = HEIGHT / 2;
    ball_dx = 1;
    ball_dy = -1;
    paddle_x = WIDTH / 2 - PADDLE_WIDTH / 2;
    score = 0;
    lives = 3;

    // Initialize bricks
    int brick_count = 0;
    for (int i = 0; i < WIDTH - BRICK_WIDTH; i += BRICK_WIDTH + 1) {
        for (int j = 0; j < HEIGHT / 2; j += BRICK_HEIGHT + 1) {
            if (brick_count < NUM_BRICKS) {
                bricks[brick_count].x = i;
                bricks[brick_count].y = j;
                bricks[brick_count].visible = 1;
                brick_count++;
            }
        }
    }
}

void draw_display() {
    clear_screen();
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            putchar(display[j][i]);
        }
        putchar('\n');
    }
    printf("Score: %d  Lives: %d\n", score, lives);
}

int main() {
    init_game();

    while (1) {
        // Clear the display
        for (int i = 0; i < WIDTH; i++) {
            for (int j = 0; j < HEIGHT; j++) {
                display[i][j] = ' ';
            }
        }

        // Move the ball
        ball_x += ball_dx;
        ball_y += ball_dy;

        // Check for collision with walls
        if (ball_x <= 0 || ball_x >= WIDTH - 1) {
            ball_dx = -ball_dx;
        }
        if (ball_y <= 0) {
            ball_dy = -ball_dy;
        }

        // Check if the ball is out
        if (ball_y >= HEIGHT) {
            lives--;
            if (lives == 0) {
                printf("Game Over! Final Score: %d\n", score);
                break;
            }
            ball_x = WIDTH / 2;
            ball_y = HEIGHT / 2;
            ball_dy = -ball_dy;
        }

        // Move the paddle
        if (kbhit()) {
            char key = getchar();
            // Move the paddle to the right
            if (key == 'd' && paddle_x < WIDTH - PADDLE_WIDTH) {
                paddle_x += PADDLE_SPEED;
            }
            // Move the paddle to the left
            else if (key == 'a' && paddle_x > 0) {
                paddle_x -= PADDLE_SPEED;
            }
        }

        // Check for collision with the paddle
        if (ball_y + BALL_SIZE == HEIGHT - PADDLE_OFFSET &&
            ball_x >= paddle_x && ball_x <= paddle_x + PADDLE_WIDTH) {
            ball_dy = -ball_dy;
            score++;
        }

        // Check for collision with the bricks
        for (int i = 0; i < NUM_BRICKS; i++) {
            if (bricks[i].visible) {
                if (ball_x + BALL_SIZE >= bricks[i].x &&
                    ball_x <= bricks[i].x + BRICK_WIDTH &&
                    ball_y + BALL_SIZE >= bricks[i].y &&
                    ball_y <= bricks[i].y + BRICK_HEIGHT) {
                    bricks[i].visible = 0;
                    ball_dy = -ball_dy;
                    score++;
                    break;
                }
            }
        }

        // Draw the paddle
        for (int i = 0; i < PADDLE_WIDTH; i++) {
            display[paddle_x + i][HEIGHT - PADDLE_OFFSET] = PADDLE_CHAR;
        }

        // Draw the ball
        display[ball_x][ball_y] = BALL_CHAR;

        // Draw the bricks
        for (int i = 0; i < NUM_BRICKS; i++) {
            if (bricks[i].visible) {
                for (int j = 0; j < BRICK_WIDTH; j++) {
                    for (int k = 0; k < BRICK_HEIGHT; k++) {
                        display[bricks[i].x + j][bricks[i].y + k] = BRICK_CHAR;
                    }
                }
            }
        }

        // Draw the display
        draw_display();

        // Sleep for a short amount of time to control game speed
        usleep(GAME_SPEED);
    }

    return 0;
}
