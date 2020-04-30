#include <iostream>
#include <fstream>
#include <cstdio>
#include <SDL.h>
#include <SDL_draw.h>
#include <SDL_ttf.h>

#define N 6

int mainLength = 746, mazeLength = 643, length = 64, halfLength = 32; //��������� � ������ ��������� ���� � ���� �� 3, 6 �������� ���  ���������
int colorBackground = 0x039C7D, colorField = 0xB7D036, colorActive = 0xC91E2C, colorWall = 0xffffff; //���, ����, �������� + ������ ������

using namespace std;

class Coordinates{ //�������
public:
    int x;
    int y;
    Coordinates(){
        x = 0;
        y = 0;
    }

};

class Cell : public Coordinates{
public:
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
            Draw_FillRect(field, x , y, length + 2, 3, colorWall);
        }

        if(this->bottom){
            Draw_FillRect(field, x, y + length , length + 2, 3, colorWall);
        }

        if(this->left){
            Draw_FillRect(field, x , y, 3, length + 2, colorWall);
        }

        if(this->right){
            Draw_FillRect(field, x + length, y, 3, length + 2, colorWall); //length + 2 ����� ���� ���������� �������� ����� ��������������� � ������������� �������
        }
    }

    void Final (SDL_Surface *field){
        Draw_FillRect(field, x, y, length, length, colorActive);
        SDL_Surface *goblet;
        SDL_Rect frameGoblet;
        frameGoblet.x = this->x + 6;
        frameGoblet.y = this->y + 5;
        frameGoblet.w = 56;
        frameGoblet.h = 56;
        goblet = SDL_LoadBMP("goblet (2).bmp");
        SDL_BlitSurface(goblet, NULL, field, &frameGoblet);
    }
};

class Character : public Coordinates {
public:
    Character(int x, int y){
        this->x = x;
        this->y = y;
    }

    void Draw (SDL_Surface *field, int x, int y){ //�������� ���������
        Draw_FillCircle(field, this->x, this->y, 20, colorField);
        this->x = x;
        this->y = y;
        Draw_FillCircle(field, this->x , this->y, 20, colorActive);
    }
};

int messageWin(SDL_Surface *screen, SDL_Rect *frameField, SDL_Surface *field){
    SDL_Surface *victory;
    SDL_Surface *text = NULL;
    SDL_Rect frameVictory;
    SDL_Rect frameMessage;
    SDL_Event event;
    SDL_Color messageColor;
    TTF_Font *message = TTF_OpenFont("text.ttf", 30);

    frameVictory.w = 320; frameVictory.h = 100;
    frameVictory.x = 210; frameVictory.y = 320;
    victory = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frameVictory.w, frameVictory.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);

    if (victory == NULL) { //����������� �������������� ��� ���������
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
        frameMessage.x = 250;
        frameMessage.y = 352;
        text = TTF_RenderUTF8_Solid(message,"YOU ARE A WINNER", messageColor); //����������� ������
        if(text){
            Draw_Rect(victory, 0, 0, frameVictory.w, frameVictory.h, 0x000000); //������ ������ ������ ���������
            SDL_BlitSurface(victory, NULL, screen, &frameVictory);
            SDL_BlitSurface(text, NULL, screen, &frameMessage);
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
    Coordinates currentCell;
    Maze[9][9].Final(field);
    Character Mur(halfLength, halfLength);
    Mur.Draw(field, halfLength, halfLength);
        for (int j = 0; j < 10; j++){
            for (int i = 0; i < 10; i++){
                Maze[i][j].Wall(field);
            }
        }
    while(SDL_WaitEvent(&event)){

                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT){ //�����
                    if(Maze[currentCell.x][currentCell.y].left != 1){
                        Mur.Draw(field, Mur.x - length, Mur.y);
                        currentCell.x--;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP){ //�����
                    if(Maze[currentCell.x][currentCell.y].top != 1){
                        Mur.Draw(field, Mur.x, Mur.y - length);
                        currentCell.y--;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT){ //������
                    if(Maze[currentCell.x][currentCell.y].right != 1){
                        Mur.Draw(field, Mur.x + length, Mur.y);
                        currentCell.x++;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN){ //����
                    if(Maze[currentCell.x][currentCell.y].bottom != 1){
                        Mur.Draw(field, Mur.x, Mur.y + length);
                        currentCell.y++;
                    }
                }
//                Draw_Rect(field, 0, 0, frameField->w, frameField->h, 0x000000); //������ ������
                SDL_BlitSurface(field, NULL, screen, frameField);
                SDL_Flip(screen);

                if(currentCell.x == 9 && currentCell.y == 9){
                    return messageWin(screen, frameField, field); //�������� ����������� � ���������� � ������

                }

                if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                    return 0;

            }
}


int main(int argc, char** argv ){
    Cell fieldMaze[10][10]; //������� ������������ ������
    SDL_Surface *screen, *maze; //�������� �����������, ����������� ���������
    SDL_Rect frameMaze; //������� ���������
//��������� ������ �� �����, � ������� ����������� ������������ ���� ���������
   ifstream infoWall("walls.txt");
    if (infoWall.is_open()) {
        int count = 0; //������� ���-�� ����� � �����
        int temp;
        while (!infoWall.eof()) { //������� ���������� �������� � �����
            infoWall >> temp; //��������� �� ����� ����� (��� ����� ����� ������ �� ����������)
            count++; //����������� �������
        }
        infoWall.seekg(0, ios::beg); //� ������ ����

        int count_space = 0; //���������� �������� � ������ �������
        char symbol;
        while (!infoWall.eof()) { //��������� �� ����� �����
            infoWall.get(symbol); //��������� ������� ������
            if (symbol == ' ') count_space++; //���� ������, ����������� �������
            if (symbol == '\n') break; //���� ����� ������, ����������
        }
        infoWall.seekg(0, ios::beg); //� ������ �����

        int n = count / (count_space + 1);
        int m = count_space + 1;
        int **matrix; //�������, � ������� �� ��������� ��� ���� �������
        matrix = new int*[n];

        for (int i = 0; i < n; i++)
            matrix[i] = new int[m];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                infoWall >> matrix[i][j]; //�������� �� ����� � �������

        int k, z = -1; //�������
            for (int i = 0; i < (m - 1) * (m - 1); i++){
                if(i % (m - 1) == 0){
                k = 0;
                z++;
                }
                Cell cell(k * 64, z * 64, matrix[z][k], matrix[k + n / 2][z], matrix[z][k + 1], matrix[k + n / 2][z + 1]);
                fieldMaze[k][z] = cell; //��������� ������ � �������
                k++;
            }



        for (int i = 0; i<n; i++) delete[] matrix[i];
        delete[] matrix;
        infoWall.close();
    }
    else {
        cout << "ERROR OPENING FILE";
        return 1;
    }
//������ � �������������
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        cout << "ERROR SDL INIT: %s\n" << SDL_GetError();
        return 2;
    }
//�������� �����������
    screen = SDL_SetVideoMode(mainLength, mainLength, 32, SDL_HWSURFACE);
    if (!screen) {
        cout << "SDL MODE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 3;
    }
//����������� ���������
    frameMaze.w = frameMaze.h = mazeLength;
    frameMaze.x = frameMaze.y = 53;
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
    if (!Game(screen, maze, &frameMaze, fieldMaze))
        SDL_Quit();
    TTF_Quit();

    return 0;
}

