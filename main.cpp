#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <SDL.h>
#include <SDL_draw.h>

#define N 6

int mainLength = 740, mazeLength = 640, length = 64, halfLength = 32;
int colorBackground = 0x0D1137, colorField = 0xE52165;

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

    void Wall (SDL_Surface *field){
       if(this->top){
            Draw_FillRect(field, x - 1, y - 1, length + 2, 3, colorBackground);
        }
        if(this->bottom){
            Draw_HLine(field, x, y + length, x + length, colorBackground);
        }

        if(this->left){
            Draw_FillRect(field, x - 1, y, 3, length + 2, colorBackground);
        }
        if(this->right){
            Draw_VLine(field, x + length, y, y + length, colorBackground);
        }
    }

    void Final (SDL_Surface *field){
        Draw_FillRect(field, x + 2, y, length, length, 0x987439);
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

    void Draw (SDL_Surface *field, int x, int y){
        Draw_FillCircle(field, this->x, this->y, 20, colorField);
        this->x = x;
        this->y = y;
        Draw_FillCircle(field, this->x , this->y, 20, 0xffffff);
    }
};

struct Current { //хранит индексы расположения в матрице текущей клетки
    int x;
    int y;
};

typedef struct Current position;


int Game(SDL_Surface *screen, SDL_Surface *field, SDL_Rect *frame_field, Cell (*Maze)[10]){
    SDL_Event event;
    SDL_FillRect(field, NULL, colorField);
    position CurrentCell;
    CurrentCell.x = 0;
    CurrentCell.y = 0;
    Maze[9][9].Final(field);
    Character Mur(halfLength, halfLength);
    Mur.Draw(field, halfLength, halfLength);
        for (int j = 0; j < 10; j++){
            for (int i = 0; i < 10; i++){
                Maze[i][j].Wall(field);
            }
        }
    while(SDL_WaitEvent(&event)){
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN){
                    if(Maze[CurrentCell.x][CurrentCell.y].bottom != 1){
                        Mur.Draw(field, Mur.x, Mur.y + length);
                        CurrentCell.y++;
                    }
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP){
                    if(Maze[CurrentCell.x][CurrentCell.y].top != 1){
                        Mur.Draw(field, Mur.x, Mur.y - length);
                        CurrentCell.y--;
                    }
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT){
                    if(Maze[CurrentCell.x][CurrentCell.y].right != 1){
                        Mur.Draw(field, Mur.x + length, Mur.y);
                        CurrentCell.x++;
                    }
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT){
                    if(Maze[CurrentCell.x][CurrentCell.y].left != 1){
                        Mur.Draw(field, Mur.x - length, Mur.y);
                        CurrentCell.x--;
                    }
                }
                if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                    return 0;
            SDL_BlitSurface(field, NULL, screen, frame_field);
            SDL_Flip(screen);
            }
}




int main(int argc, char** argv ){
    int point, number = 0;
    Cell Maze[10][10];
    SDL_Surface *screen, *maze;
    SDL_Rect frame_maze;
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
        cout << "ERROR OPENING FILE";
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

    SDL_FillRect(screen, NULL, colorBackground);
    exit = Game(screen, maze, &frame_maze, Maze);
    if (exit == 0)
        SDL_Quit();

    return 0;
}

