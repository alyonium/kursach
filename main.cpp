#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <SDL.h>
#include <SDL_draw.h>

#define N 6

int mainLength = 740, mazeLength = 640, length = 64;
int color = 0x0D1137, colorMaze = 0xE52165;

using namespace std;

class Cell{
public:
    int x;
    int y;
    int left;
    int top;
    int right;
    int bottom;
    Cell(){
        x = y = left = top = right = bottom = 0;
    }

    Cell(int x, int y, int left, int top, int right, int bottom){
        this->x = x;
        this->y = y;
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;
    }

    void Wall (SDL_Surface *maze){
       if(this->top){
            Draw_HLine(maze, x, y, x + length, color);
            Draw_FillRect(maze, x - 1, y - 1, length + 2, 3, color);
        }
        if(this->bottom){
            Draw_HLine(maze, x, y + length, x + length, color);
        }

        if(this->left){
            Draw_VLine(maze, x, y, y + length, color);
            Draw_FillRect(maze, x - 1, y, 3, length + 2, color);
        }
        if(this->right){
            Draw_VLine(maze, x + length, y, y + length, color);
        }
    }

    void Final (SDL_Surface *maze) {
        Draw_FillRect(maze, x + 2, y, length, length, 0x987439);
    }
};

class Character {
public:
   int x;
   int y;

    Character(int x, int y){
        this->x = x;
        this->y = y;
    }

    void Draw (SDL_Surface *maze, int a, int b){
        Draw_FillCircle(maze, this->x, this->y, 20, colorMaze);
        this->x = a;
        this->y = b;
        Draw_FillCircle(maze, this->x , this->y, 20, 0xffffff);
    }
};

struct Cur { //индексы матрицы
    int x;
    int y;
};

typedef struct Cur current;


int mazeDraw(SDL_Surface *screen, SDL_Surface *maze, SDL_Rect *frame_maze, SDL_Event event, Cell (*Maze)[10], current Current){
    SDL_FillRect(maze, NULL, colorMaze);
    for (int j = 0; j < 10; j++){
        for (int i = 0; i < 10; i++){
            Maze[i][j].Wall(maze);
        }
    }
    Current.x = 0;
    Current.y = 0;

    Maze[9][9].Final(maze);
    Character Mur(32, 32);
    Mur.Draw(maze, 32, 32);
    SDL_Event new_event;
    while(SDL_WaitEvent(&event)){
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN){
                    if(Maze[Current.x][Current.y].bottom != 1){
                        Mur.Draw(maze, Mur.x, Mur.y + 64);
                        Current.y++;
                    }
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP){
                    if(Maze[Current.x][Current.y].top != 1){
                        Mur.Draw(maze, Mur.x, Mur.y - 64);
                        Current.y--;
                    }
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT){
                    if(Maze[Current.x][Current.y].right != 1){
                        Mur.Draw(maze, Mur.x + 64, Mur.y);
                        Current.x++;
                    }
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT){
                    if(Maze[Current.x][Current.y].left != 1){
                        Mur.Draw(maze, Mur.x - 64, Mur.y);
                        Current.x--;
                    }
                }
            SDL_BlitSurface(maze, NULL, screen, frame_maze);
            SDL_Flip(screen);

            if(SDL_PollEvent(&event)){
                if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                     return 0;
            }


            }
    }



int main(int argc, char** argv ){
    int point, number = 0;
    Cell Maze[10][10];
    SDL_Surface *screen, *maze;
    SDL_Rect frame_maze;
    SDL_Event event;
    int exit;
    current Current;
//считываем данные из файла, в котором описывается расположение стен лабиринта
   ifstream in("walls.txt");
    if (in.is_open()) {
        int count = 0;
        int temp;
        while (!in.eof()) { //считаем количество пробелов в файле
            in >> temp;
            count++;
        }
        in.seekg(0, ios::beg);
        in.clear();
        int count_space = 0;
        char symbol;
        while (!in.eof()) {
            in.get(symbol);
            if (symbol == ' ') count_space++;
            if (symbol == '\n') break;
        }
        in.seekg(0, ios::beg);
        in.clear();

        int n = count / (count_space + 1);
        int m = count_space + 1;
        int **x;
        x = new int*[n];
        for (int i = 0; i<n; i++) x[i] = new int[m];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                in >> x[i][j];

        int k, z = -1;
        for(int j = 0; j < 10; j++){
            for (int i = 0; i < 10; i++) {
            if(i % (n / 2) == 0){
                k = 0;
                z++;
                }
            Cell cell(k * 64, z * 64, x[z][k], x[k + n / 2][z], x[z][k + 1], x[k + n / 2][z + 1]);
            Maze[i][j] = cell;
            k++;
            }
        }

        for (int i = 0; i<n; i++) delete[] x[i];
        delete[] x;
        in.close();
    }
    else {
        cout << "ERROR";
        return 1;
    }
//работа с поверхностями
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        cout << "ERROR SDL INIT: %s\n" << SDL_GetError();
        return 2;
    }
//основная поверхность
    screen = SDL_SetVideoMode(mainLength, mainLength, 32, SDL_HWSURFACE);
    if (!screen) {
        cout << "SDL MODE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 3;
    }
//поверхность лабиринта
    frame_maze.w = frame_maze.h = mazeLength;
    frame_maze.x = frame_maze.y = 50;
    maze = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frame_maze.w, frame_maze.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);
    if (maze == NULL) {
        cout << "SDL CREATE RGB SURFACE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 4;
    }

    SDL_FillRect(screen, NULL, color);
    exit = mazeDraw(screen, maze, &frame_maze, event, Maze, Current);
    if (exit == 0)
        SDL_Quit();

    return 0;
}

