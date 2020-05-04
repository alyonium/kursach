#include <iostream>
#include <fstream>
#include <cstdio>
#include <SDL.h>
#include <SDL_draw.h>
#include <SDL_ttf.h>

#define N 6

int mainLength = 746, mazeLength = 643, length = 64; //прибавили к длинам основного окна и поля по 3, 6 пикселей для симметрии
int colorField = 0xA0A0C0, colorActive = 0xE0A0C0, colorWall = 0x802080; //поле, клетка выхода, cтены
static int counter = 0; //счетчик для меню

using namespace std;

int Menu(SDL_Surface*);

class Coordinates{ //базовый
public:
   int x;
   int y;
    Coordinates(){
        x = y = 0;
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
       if(top){
            Draw_FillRect(field, x , y, length + 2, 3, colorWall);
        }

        if(bottom){
            Draw_FillRect(field, x, y + length , length + 2, 3, colorWall);
        }

        if(left){
            Draw_FillRect(field, x , y, 3, length + 2, colorWall);
        }

        if(right){
            Draw_FillRect(field, x + length, y, 3, length + 2, colorWall); //length + 2 для стыковки между горизонтальными и вертикальными стенами
        }
    }

    void finalCell (SDL_Surface *field){
        Draw_FillRect(field, x, y, length, length, colorActive); //картинка не во всю площадь клетки, поэтому прорисовываем тем же цветом заранее
        SDL_Surface *scepter;
        SDL_Rect frameScepter;
        frameScepter.x = x + 6;
        frameScepter.y = y + 5;
        frameScepter.w = frameScepter.h = 56;
        scepter = SDL_LoadBMP("scepter.bmp");
        SDL_BlitSurface(scepter, NULL, field, &frameScepter);
    }
};

class Character : public Coordinates {
public:
    Character(int x, int y){
        this->x = x;
        this->y = y;
    }

    void Move(SDL_Surface *field, SDL_Surface *sailor, SDL_Rect *frameSailor, int x, int y){ //движение персонажа переименуй
        SDL_FillRect (field, frameSailor, colorField);
        frameSailor->x = this->x = x;
        frameSailor->y = this->y = y;
        SDL_BlitSurface(sailor, NULL, field, frameSailor);
    }
};


int readFile(Cell (&fieldMaze)[10][10]){  //считываем данные из файла, в котором описывается расположение стен лабиринта
    ifstream Walls("walls.txt");
    if (Walls.is_open()) {
        int count = 0;
        int temp;
        while (!Walls.eof()) { //считаем количество пробелов в файле
            Walls >> temp;
            count++;
        }
        Walls.seekg(0, ios::beg);

        int countSpace = 0;
        char symbol;
        while (!Walls.eof()) {
            Walls.get(symbol);
            if (symbol == ' ') countSpace++; //если пробел - увеличиваем счетчик
            if (symbol == '\n') break;
        }
        Walls.seekg(0, ios::beg);

        int n = count / (countSpace + 1);
        int m = countSpace + 1;
        int **Matrix;
        Matrix = new int*[n];

        for (int i = 0; i < n; i++)
            Matrix[i] = new int[m];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                Walls >> Matrix[i][j];

        int k, z = -1;
            for (int i = 0; i < (n / 2) * (n / 2); i++){
                if(i % (n / 2) == 0){
                    k = 0;
                    z++;
                }
                Cell cell(k * 64, z * 64, Matrix[z][k], Matrix[k + n / 2][z], Matrix[z][k + 1], Matrix[k + n / 2][z + 1]);
                fieldMaze[k][z] = cell; //вписываем клетку в матрицу
                k++;
            }

        for (int i = 0; i < n; i++) delete[] Matrix[i];
        delete[] Matrix;
        Walls.close();
    }
    else {
        return 1;
    }
    return 0;
}

int Win(SDL_Surface *screen){
    SDL_Surface *win;
    SDL_Rect frameWin;
    frameWin.x = frameWin.y = 0;
    frameWin.w = frameWin.h = mainLength;
    win = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frameWin.w, frameWin.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);
    if (win == NULL) {
        cout << "SDL CREATE RGB SURFACE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 4;
    }

    win = SDL_LoadBMP("sailorwin.bmp");
    SDL_BlitSurface(win, NULL, screen, &frameWin);

    SDL_Surface *rect;
    SDL_Surface *text = NULL;
    SDL_Rect frameRect;
    SDL_Rect frameMessage;
    SDL_Event event;
    SDL_Color messageColor;
    TTF_Font *message = TTF_OpenFont("text.ttf", 30);



    frameRect.w = 320; frameRect.h = 100;
    frameRect.x = 210; frameRect.y = 320;
    rect = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frameRect.w, frameRect.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);

    if (rect == NULL) { //поверхность прямоугольника для сообщения
        cout << "SDL CREATE RGB SURFACE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 4;
    }

    SDL_FillRect(rect, NULL, colorActive);
    SDL_BlitSurface(rect, NULL, screen, &frameRect);

    if(message){
        messageColor.r = 128;
        messageColor.g = 32;
        messageColor.b = 128;
        frameMessage.x = 250;
        frameMessage.y = 352;
        text = TTF_RenderUTF8_Solid(message,"YOU ARE A WINNER", messageColor); //поверхность текста
        if(text){
            Draw_Rect(rect, 0, 0, frameRect.w, frameRect.h, 0xA02080); //контур вокруг сообщения
            SDL_BlitSurface(rect, NULL, screen, &frameRect);
            SDL_BlitSurface(text, NULL, screen, &frameMessage);
            SDL_Flip(screen);
      }
    } else {
        cout << "ERROR OPENING FONT";
        return 5;
    }

    while(SDL_WaitEvent(&event)){
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)){
            TTF_CloseFont(message);
            SDL_FreeSurface(win);
            SDL_FreeSurface(rect);
            SDL_FreeSurface(text);
            SDL_FreeSurface(screen);
            return 0;
        }
    }
}

int Game(SDL_Surface *screen){

    Cell Maze[10][10];  //матрица расположения клеток
    if(readFile(Maze)){
        cout << "ERROR OPENING FILE";
        return 1;
    }

    SDL_Surface *field;//поверхность лабиринта
    SDL_Rect frameField; //площадь лабиринта
    frameField.w = frameField.h = mazeLength;
    frameField.x = frameField.y = 53;
    field = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frameField.w, frameField.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);
    if (field == NULL) {
        cout << "SDL CREATE RGB SURFACE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 4;
    }
    SDL_FillRect(field, NULL, colorField);

    SDL_Event event;
    Coordinates currentCell;
    Maze[9][9].finalCell(field);

    Character SailorMoon(3, 3);

    SDL_Surface *sailor;
    SDL_Rect frameSailor;
    frameSailor.x = frameSailor.y = 3;
    frameSailor.w = frameSailor.h = 58;
    sailor = SDL_LoadBMP("sailor.bmp");
    SDL_BlitSurface(sailor, NULL, field, &frameSailor);

        for (int j = 0; j < 10; j++)
            for (int i = 0; i < 10; i++)
                Maze[i][j].Wall(field);

    while(SDL_WaitEvent(&event)){

                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT){ //влево
                    if(Maze[currentCell.x][currentCell.y].left != 1){
                        SailorMoon.Move(field, sailor, &frameSailor, SailorMoon.x - length, SailorMoon.y);
                        currentCell.x--;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP){ //вверх
                    if(Maze[currentCell.x][currentCell.y].top != 1){
                        SailorMoon.Move(field, sailor, &frameSailor, SailorMoon.x, SailorMoon.y - length);
                        currentCell.y--;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT){ //вправо
                    if(Maze[currentCell.x][currentCell.y].right != 1){
                        SailorMoon.Move(field, sailor, &frameSailor, SailorMoon.x + length, SailorMoon.y);
                        currentCell.x++;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN){ //вниз
                    if(Maze[currentCell.x][currentCell.y].bottom != 1){
                        SailorMoon.Move(field, sailor, &frameSailor, SailorMoon.x, SailorMoon.y + length);
                        currentCell.y++;
                    }
                }

                SDL_BlitSurface(field, NULL, screen, &frameField);
                SDL_Flip(screen);

                if(currentCell.x == 9 && currentCell.y == 9){
                    SDL_FreeSurface(sailor);
                    SDL_FreeSurface(field);
                    return Win(screen); //вызываем поверхность с сообщением о победе
                }

                if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                    if(Menu(screen) != 3)
                        return 0;

                if (event.type == SDL_QUIT)
                    return 0;
            }
}

int Menu(SDL_Surface *screen){

    TTF_Font *font = font = TTF_OpenFont("text.ttf", 30);

    SDL_Surface *menu;
    SDL_Rect frameMenu;
    frameMenu.x = 187;
    frameMenu.y = 153;
    frameMenu.w = 372;
    frameMenu.h = 443;

    menu = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frameMenu.w, frameMenu.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);
    if (menu == NULL) { //поверхность прямоугольника для сообщения
        cout << "SDL CREATE RGB SURFACE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 4;
    }

    SDL_FillRect(menu, NULL, colorActive);
    SDL_BlitSurface(menu, NULL, screen, &frameMenu);

    SDL_Event event;

    int x, y;
    char *items[2] = {"PLAY", "EXIT"};
    int current[2] = {0, 0};
    SDL_Surface *choise[2];
    SDL_Rect spot[2];
    SDL_Color color[2];

    color[0].r = 255; color[0].g = 255; color[0].b = 255;
    color[1].r = 64; color[1].g = 32; color[1].b = 128;

    choise[0] = TTF_RenderText_Solid(font, items[0], color[0]);
    choise[1] = TTF_RenderText_Solid(font, items[1], color[0]);


    spot[0].x = screen->clip_rect.w/2 - choise[0]->clip_rect.w/2;
    spot[0].y = screen->clip_rect.h/2 - choise[0]->clip_rect.h;
    spot[1].x = screen->clip_rect.w/2 - choise[0]->clip_rect.w/2;
    spot[1].y = screen->clip_rect.h/2 + choise[0]->clip_rect.h;

    while (1) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_MOUSEMOTION:
                    x = event.motion.x;
                    y = event.motion.y;
                    for (int i = 0; i < 2; i++)
                        if (x >= spot[i].x && x <= spot[i].x + spot[i].w && y >= spot[i].y && y <= spot[i].y + spot[i].h){
                            if (!current[i]) {
                                current[i] = 1;
                                SDL_FreeSurface(choise[i]);
                                choise[i] = TTF_RenderText_Solid(font, items[i], color[1]);
                            }
                        } else if (current[i]) {
                            current[i] = 0;
                            SDL_FreeSurface(choise[i]);
                            choise[i] = TTF_RenderText_Solid(font, items[i], color[0]);
                        }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    x = event.button.x;
                    y = event.button.y;
                    for (int i = 0; i < 2; i++)
                        if (x >= spot[i].x && x <= spot[i].x + spot[i].w && y >= spot[i].y && y <= spot[i].y + spot[i].h) {
                            if (!i) {
                                if(!counter){ //если меню открыли не первый раз, надо продолжить движение с последней позиции
                                    ++counter; //для этого здесь ставим счетчик static, если больше 0 - значит не надо запускать Game еще раз
                                    if(!Game(screen))
                                        return 0;
                                } else {
                                      for (int i = 0; i < 2; i++)
                                         SDL_FreeSurface(choise[i]);
                                    SDL_FreeSurface(menu);
                                    return 3;
                                }
                            } else {
                                 for (int i = 0; i < 2; i++)
                                    SDL_FreeSurface(choise[i]);
                                SDL_FreeSurface(menu);
                                return 0;
                            }
                        }
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) { //при первом запуске - обеспечивает выход из игры, при запуске в процессе игры - возвращает к процессу
                          for (int i = 0; i < 2; i++)
                            SDL_FreeSurface(choise[i]);
                        SDL_FreeSurface(menu);
                        return 3;
                    }

                case SDL_QUIT:
                    for (int i = 0; i < 2; i++)
                        SDL_FreeSurface(choise[i]);
                    SDL_FreeSurface(menu);
                    return 1;
            }
        }
        for (int i = 0; i < 2; i++) {
            SDL_BlitSurface(choise[i], NULL, screen, &spot[i]);
            SDL_Flip(screen);
        }
    }
}

int Saver(SDL_Surface *screen){
SDL_Surface *saver;
    SDL_Rect frameSaver;
    frameSaver.x = frameSaver.y = 0;
    frameSaver.w = frameSaver.h = mainLength;
    saver = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frameSaver.w, frameSaver.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);
    if (saver == NULL) {
        cout << "SDL CREATE RGB SURFACE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 1;
    }

    saver = SDL_LoadBMP("saver.bmp");
    SDL_BlitSurface(saver, NULL, screen, &frameSaver);
    SDL_Flip(screen);
    SDL_Delay(1000);
    SDL_FreeSurface(saver);
    return 0;
}

int main(int argc, char** argv){
    SDL_Surface *screen;
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        cout << "ERROR SDL INIT: %s\n" << SDL_GetError();
        return 2;
    }

    screen = SDL_SetVideoMode(mainLength, mainLength, 32, SDL_HWSURFACE);//основная поверхность
    if (!screen) {
        cout << "SDL MODE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 3;
    }

    if(Saver(screen))
        return 4;

    SDL_Surface *background;
    SDL_Rect frameBackground;
    frameBackground.x = frameBackground.y = 0;
    frameBackground.w = frameBackground.h = mainLength;
    background = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frameBackground.w, frameBackground.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);

    if (background == NULL) { //поверхность с фоном
        cout << "SDL CREATE RGB SURFACE FAILED: %s\n" << SDL_GetError();
        atexit(SDL_Quit);
        return 4;
    }

    background = SDL_LoadBMP("fon.bmp");
    SDL_BlitSurface(background, NULL, screen, &frameBackground);

    if (SDLCALL TTF_Init()){
        cout << "ERROR INIT FONT";
        return 5;
    }

    Menu(screen);

    return 0;
}

