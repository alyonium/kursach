#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <SDL.h>
#include <SDL_draw.h>
#include "SDL_ttf.h"

#define N 6

int mainLength = 740, mazeLength = 640, length = 64, halfLength = 32;
int colorBackground = 0x039C7D, colorField = 0xB7D036, colorActive = 0xC91E2C; //фон, поле, персонаж + клетка выхода

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
        Draw_FillRect(field, x + 2, y, length, length, colorActive);
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
        Draw_FillCircle(field, this->x , this->y, 20, colorActive);
    }
};

struct Current { //хранит индексы расположения текущей клетки в матрице
    int x;
    int y;
};

typedef struct Current position;

int messageWin(SDL_Surface *screen, SDL_Rect *frameField, SDL_Surface *field){
    SDL_Surface *victory;
    SDL_Surface *text = NULL;
    SDL_Rect frameVictory;
    SDL_Rect dest;
    SDL_Event event;
    SDL_Color messageColor;
    TTF_Font *message = TTF_OpenFont("text.ttf", 30);

    frameVictory.w = 320; frameVictory.h = 100;
    frameVictory.x = 210; frameVictory.y = 320;
    victory = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frameVictory.w, frameVictory.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);

    if (victory == NULL) { //поверхность прямоугольника для сообщения
        cout << "SDL CREATE RGB SURFACE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 4;
    }

    SDL_FillRect(victory, NULL, colorActive);
    SDL_BlitSurface(victory, NULL, screen, &frameVictory);
    SDL_Flip(screen);

    if(message){
        messageColor.r = 183;
        messageColor.g = 208;
        messageColor.b = 54;
         dest.x = 250;
         dest.y = 352;
        text = TTF_RenderUTF8_Solid(message,"YOU ARE A WINNER", messageColor); //поверхность текста
        if(text){
            Draw_Rect(victory, 0, 0, frameVictory.w, frameVictory.h, 0x000000); //черный контур вокруг сообщения
            SDL_BlitSurface(victory, NULL, screen, &frameVictory);
            SDL_BlitSurface(text, NULL, screen, &dest);

            SDL_Flip(screen);
      }
    }
    while(SDL_WaitEvent(&event)){
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)){
            TTF_CloseFont(message);
            return 0;
        }
    }
}


int Game(SDL_Surface *screen, SDL_Surface *field, SDL_Rect *frameField, Cell (*Maze)[10]){
    SDL_Event event;
    SDL_FillRect(field, NULL, colorField);
    position currentCell;
    currentCell.x = 0;
    currentCell.y = 0;
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
                    if(Maze[currentCell.x][currentCell.y].bottom != 1){
                        Mur.Draw(field, Mur.x, Mur.y + length);
                        currentCell.y++;
                    }
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP){
                    if(Maze[currentCell.x][currentCell.y].top != 1){
                        Mur.Draw(field, Mur.x, Mur.y - length);
                        currentCell.y--;
                    }
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT){
                    if(Maze[currentCell.x][currentCell.y].right != 1){
                        Mur.Draw(field, Mur.x + length, Mur.y);
                        currentCell.x++;
                    }
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT){
                    if(Maze[currentCell.x][currentCell.y].left != 1){
                        Mur.Draw(field, Mur.x - length, Mur.y);
                        currentCell.x--;
                    }

                }
                Draw_Rect(field, 0, 0, frameField->w, frameField->h, 0x000000); //черный контур вокруг поля
                SDL_BlitSurface(field, NULL, screen, frameField);
                SDL_Flip(screen);

                if(currentCell.x == 9 && currentCell.y == 9){
                    return messageWin(screen, frameField, field); //вызываем поверхность с сообщением о победе

                }

                if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                    return 0;

            }
}


int main(int argc, char** argv ){
    Cell Maze[10][10]; //матрица расположения клеток
    SDL_Surface *screen, *maze; //основная поверхность и поверхность лабиринта
    SDL_Rect frameMaze; //площадь лабиринта
//считываем данные из файла, в котором описывается расположение стен лабиринта
   ifstream infoWall("walls.txt");
    if (infoWall.is_open()) {
        int count = 0; //счетчик кол-ва чисел в файле
        int temp; //временная переменная
        while (!infoWall.eof()) { //считаем количество пробелов в файле
            infoWall >> temp; //считываем из файла числа (нам нужно занть только их количество)
            count++; //увеличиваем счетчик количества чисел
        }
        infoWall.seekg(0, ios::beg); //переходим в начало фйла

        int count_space = 0; //количество пробелов в первой строчке равно 0
        char symbol;
        while (!infoWall.eof()) { //считываем до конца файла
            infoWall.get(symbol); //считываем текущий символ
            if (symbol == ' ') count_space++; //если пробел, увеличиваем счетчик
            if (symbol == '\n') break; //если конец строки, прерывваем цикл
        }
        infoWall.seekg(0, ios::beg); //переходим к началу файла

        int n = count / (count_space + 1); //количество строк
        int m = count_space + 1; //количество столбцов
        int **matrix;
        matrix = new int*[n];

        for (int i = 0; i<n; i++) matrix[i] = new int[m];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                infoWall >> matrix[i][j]; //считываем матрицу из файла

        int k, z = -1;
        for(int j = 0; j < 10; j++){
            for (int i = 0; i < 10; i++) {
            if(i % (n / 2) == 0){
                k = 0;
                z++;
                }
            Cell cell(k * 64, z * 64, matrix[z][k], matrix[k + n / 2][z], matrix[z][k + 1], matrix[k + n / 2][z + 1]);
            Maze[i][j] = cell; //вписываем клетку в матрицу
            k++;
            }
        }

        for (int i = 0; i<n; i++) delete[] matrix[i];
        delete[] matrix;
        infoWall.close(); //закрыли файл
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
    frameMaze.w = frameMaze.h = mazeLength;
    frameMaze.x = frameMaze.y = 50;
    maze = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frameMaze.w, frameMaze.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);
    if (maze == NULL) {
        cout << "SDL CREATE RGB SURFACE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 4;
    }

    if (SDLCALL TTF_Init()){
        cout << "ERROR INIT FONT";
        return 5;
    }

    SDL_FillRect(screen, NULL, colorBackground);
    if (!Game(screen, maze, &frameMaze, Maze))
        SDL_Quit();
    TTF_Quit();

    return 0;
}

