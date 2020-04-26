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
private:
    int x;
    int y;
    int left;
    int top;
    int right;
    int bottom;
public:
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
        Draw_FillRect(maze, x, y, length, length, 0x000000);
    }
};

class Character {
private:
   int x;
   int y;
public:
    Character(int x, int y){
        this->x = x;
        this->y = y;
    }

    void Draw (SDL_Surface *maze){
        Draw_FillCircle(maze, x, y, 20, 0xffffff);
    }
};

int mazeDraw(SDL_Surface *screen, SDL_Surface *maze, SDL_Rect *frame_maze, SDL_Event event, Cell Maze[]){
    SDL_FillRect(maze, NULL, colorMaze);
    for (int i = 0; i < 100; i++){
        Maze[i].Wall(maze); //массив со всеми клетками
    }

    Maze[99].Final(maze);
    Character Mur(32, 32);
    Mur.Draw(maze);

    while(1){

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
    Cell Maze[100];
    SDL_Surface *screen, *maze;
    SDL_Rect frame_maze;
    SDL_Event event;
    int exit;
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
        for (int i = 0; i < (m - 1) *  (m - 1); i++) {
            if(i % (n / 2) == 0){
                k = 0;
                z++;
            }
            Cell cell(k * 64, z * 64, x[z][k], x[k + n / 2][z], x[z][k + 1], x[k + n / 2][z + 1]);
            Maze[i] = cell;
            k++;
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
    exit = mazeDraw(screen, maze, &frame_maze, event, Maze);
    if (exit == 0)
        SDL_Quit();

    return 0;
}

