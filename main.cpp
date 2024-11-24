#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600
#define TILE_SIZE 25

typedef struct {
    int x,y;
} Point;

typedef struct {
    Point body[SCREEN_WIDTH*SCREEN_HEIGHT/(TILE_SIZE * TILE_SIZE)];
    int length;
    Point direction;
} Snake;


// upload texture from the file
SDL_Texture *load_texture(SDL_Renderer *renderer,const char *file) {
    SDL_Surface *surface=IMG_Load(file);
    if (!surface) {
        printf("IMG_Load Error: %s\n",IMG_GetError());
        return NULL;
    }
    SDL_Texture *texture=SDL_CreateTextureFromSurface(renderer,surface);
    SDL_FreeSurface(surface);
    return texture;
}


// text showing on the screen
void render_text(SDL_Renderer *renderer,TTF_Font *font,const char *text,SDL_Color color,int x,int y) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {x,y,surface->w,surface->h};
    SDL_RenderCopy(renderer,texture,NULL,&dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// scaning high score from a file
int load_high_score(const char *filename) {
    FILE *file=fopen(filename,"r");
    int highScore = 0;
    if (file) {
        fscanf(file,"%d",&highScore);
        fclose(file);
    }
    return highScore;
}

// Save high score to a file
void save_high_score(const char *filename,int highScore) {
    FILE *file = fopen(filename,"w");
    if (file) {
        fprintf(file,"%d",highScore);
        fclose(file);
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0||TTF_Init()==-1||Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048)<0)
        return 1;

    SDL_Window *window = SDL_CreateWindow("Snake Game By Tawfiq Bin Wahed",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    if (!window||!renderer)
        return 1;

    TTF_Font *font=TTF_OpenFont("arial.ttf", 24);
    Mix_Chunk *eatSound=Mix_LoadWAV("eat.wav");
    SDL_Texture *foodTexture = load_texture(renderer,"food.png");
    SDL_Texture *snakeTexture = load_texture(renderer,"snake.png");
    SDL_Texture *backgroundTexture = load_texture(renderer,"background.png");
    if (!font||!eatSound||!foodTexture||!snakeTexture||!backgroundTexture)
        return 1;

    srand(time(NULL));

    // High score 
    const char *highScoreFile = "highscore.txt";
    int highScore = load_high_score(highScoreFile);

    // Main game loop
    bool running=true;
    while (running) {
        // Game init
        Snake snake={{0}, 5, {0, 1}};
        for (int i=0;i<snake.length;++i)
            snake.body[i]=(Point){snake.length - i - 1, 0};
        Point food={rand() % (SCREEN_WIDTH / TILE_SIZE ), rand() % (SCREEN_HEIGHT / TILE_SIZE )};
        bool gameOver=false;
        int score=0;

        while (!gameOver) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type==SDL_QUIT)
                    running=gameOver = false;
                if (e.type==SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_UP:
                            if (snake.direction.y == 0)
                                snake.direction = (Point){0, -1};
                            break;
                        case SDLK_DOWN:
                            if (snake.direction.y == 0)
                                snake.direction = (Point){0, 1};
                            break;
                        case SDLK_LEFT:
                            if (snake.direction.x == 0)
                                snake.direction = (Point){-1, 0};
                            break;
                        case SDLK_RIGHT:
                            if (snake.direction.x == 0)
                                snake.direction = (Point){1, 0};
                            break;
                    }
                }
            }

            // Moving the snake
            for (int i = snake.length - 1; i > 0; --i)
                snake.body[i] = snake.body[i - 1];
            snake.body[0].x += snake.direction.x;
            snake.body[0].y += snake.direction.y;

            // collisions
            if (snake.body[0].x < 0 || snake.body[0].x >= SCREEN_WIDTH / TILE_SIZE || snake.body[0].y < 0 || snake.body[0].y >= SCREEN_HEIGHT / TILE_SIZE)
                gameOver = true;
            for (int i = 1; i < snake.length; ++i)
                if (snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y)
                    gameOver = true;

            // snake eat food
            if (snake.body[0].x == food.x && snake.body[0].y == food.y) {
                snake.length++;
                score++;
                food = (Point){rand() % (SCREEN_WIDTH / TILE_SIZE), rand() % (SCREEN_HEIGHT / TILE_SIZE)};
                Mix_PlayChannel(-1, eatSound, 0);
            }

            // Update high score
            if (score > highScore) highScore = score;

            
            SDL_RenderClear(renderer);

            
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

            SDL_Rect foodRect = {food.x * TILE_SIZE, food.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_RenderCopy(renderer, foodTexture, NULL, &foodRect);

            for (int i = 0; i < snake.length; ++i) {
                SDL_Rect snakeRect = {snake.body[i].x * TILE_SIZE, snake.body[i].y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                SDL_RenderCopy(renderer, snakeTexture, NULL, &snakeRect);
            }

            char scoreText[32];
            sprintf(scoreText, "Score: %d", score);
            render_text(renderer, font, scoreText, (SDL_Color){255, 255, 255, 255}, 10, 10);

            char highScoreText[32];
            sprintf(highScoreText, "High Score: %d", highScore);
            render_text(renderer, font, highScoreText, (SDL_Color){255, 255, 255, 255}, 10, 40);

            SDL_RenderPresent(renderer);
            SDL_Delay(100);
        }

        // Game over screening
        save_high_score(highScoreFile, highScore);
        while (true) {
            SDL_RenderClear(renderer);

            //  background on the game-over screening
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

            render_text(renderer, font, "Game Over", (SDL_Color){255, 0, 0, 255}, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 100);

            // Define button dimensions
            SDL_Rect newGameButton = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 30, 200, 50};
            SDL_Rect quitGameButton = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 50, 200, 50};

            //  buttons
            SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255); // Green for "New Game"
            SDL_RenderFillRect(renderer, &newGameButton);
            SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255); // Red for "Quit Game"
            SDL_RenderFillRect(renderer, &quitGameButton);

            //  button text
            render_text(renderer, font, "New Game", (SDL_Color){255, 255, 255, 255}, newGameButton.x + 40, newGameButton.y + 10);
            render_text(renderer, font, "Quit Game", (SDL_Color){255, 255, 255, 255}, quitGameButton.x + 40, quitGameButton.y + 10);

            SDL_RenderPresent(renderer);

            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = false;
                    break;
                }
                if (e.type==SDL_MOUSEBUTTONDOWN) {
                    int mouseX=e.button.x;
                    int mouseY=e.button.y;

                    // the user clicked on "New Game" button
                    if (mouseX>=newGameButton.x && mouseX <= newGameButton.x + newGameButton.w &&
                        mouseY>=newGameButton.y && mouseY <= newGameButton.y + newGameButton.h) {
                        gameOver=false; // Restart the game
                        break;
                    }

                    //  the user clicked on "Quit Game" button
                    if (mouseX>=quitGameButton.x && mouseX <= quitGameButton.x + quitGameButton.w &&
                        mouseY>=quitGameButton.y && mouseY <= quitGameButton.y + quitGameButton.h) {
                        running=false; // Exit the game
                        break;
                    }
                }
            }
            if (!running||!gameOver) break;
        }
    }

    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(foodTexture);
    SDL_DestroyTexture(snakeTexture);
    Mix_FreeChunk(eatSound);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
