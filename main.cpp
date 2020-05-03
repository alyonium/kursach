#include <iostream>
#include <fstream>
#include <cstdio>
#include <SDL.h>
#include <SDL_draw.h>
#include <SDL_ttf.h>

#define N 6

int mainLength = 746, mazeLength = 643, length = 64; //прибавили к длинам основного окна и поля по 3, 6 пикселей для  симметрии
int colorField = 0xA0A0C0, colorActive = 0xE0A0C0, colorWall = 0x802080; //поле, клетка выхода, cтены
static int counter = 0;

using namespace std;

int Menu(SDL_Surface*);

class Coordinates{ //базовый
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
            Draw_FillRect(field, x + length, y, 3, length + 2, colorWall); //length + 2 чтобы была нормальная стыковка между горизонтальными и вертикальными стенами
        }
    }

    void Final (SDL_Surface *field){
        Draw_FillRect(field, x, y, length, length, colorActive); //картинка не во всю площадь клетки, поэтому прорисовываем тем же цветом заранее
        SDL_Surface *scepter;
        SDL_Rect frameScepter;
        frameScepter.x = this->x + 6;
        frameScepter.y = this->y + 5;
        frameScepter.w = 56;
        frameScepter.h = 56;
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

    void Draw (SDL_Surface *field, SDL_Surface *sailor, SDL_Rect *frameSailor, int x, int y){ //движение персонажа переименуй
        SDL_FillRect (sailor, NULL, colorField); //перекрасить по координатам рамки
        SDL_BlitSurface(sailor, NULL, field, frameSailor);
        this->x = x;
        this->y = y;
        frameSailor->x = x;
        frameSailor->y = y;
        sailor = SDL_LoadBMP("sailor.bmp");
        SDL_BlitSurface(sailor, NULL, field, frameSailor);
    }
};


int readFile(Cell (&fieldMaze)[10][10]){  //считываем данные из файла, в котором описывается расположение стен лабиринта
    ifstream infoWall("walls.txt");
    if (infoWall.is_open()) {
        int count = 0; //счетчик кол-ва чисел в файле
        int temp;
        while (!infoWall.eof()) { //считаем количество пробелов в файле
            infoWall >> temp; //считываем из файла числа (нам нужно знать только их количество)
            count++; //увеличиваем счетчик
        }
        infoWall.seekg(0, ios::beg); //в начало фйла

        int count_space = 0; //количество пробелов в первой строчке
        char symbol;
        while (!infoWall.eof()) { //считываем до конца файла
            infoWall.get(symbol); //считываем текущий символ
            if (symbol == ' ') count_space++; //если пробел, увеличиваем счетчик
            if (symbol == '\n') break; //если конец строки, прерывваем
        }
        infoWall.seekg(0, ios::beg); //к началу файла

        int n = count / (count_space + 1);
        int m = count_space + 1;
        int **matrix; //матрица, в которую мы считываем все наши позиции
        matrix = new int*[n];

        for (int i = 0; i < n; i++)
            matrix[i] = new int[m];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                infoWall >> matrix[i][j]; //считывам из файла в матрицу

        int k, z = -1; //индексы
            for (int i = 0; i < (m - 1) * (m - 1); i++){
                if(i % (m - 1) == 0){
                    k = 0;
                    z++;
                }
                Cell cell(k * 64, z * 64, matrix[z][k], matrix[k + n / 2][z], matrix[z][k + 1], matrix[k + n / 2][z + 1]);
                fieldMaze[k][z] = cell; //вписываем клетку в матрицу
                k++;
            }

        for (int i = 0; i<n; i++) delete[] matrix[i];
        delete[] matrix;
        infoWall.close();
    }
    else {
        return 1;
    }
    return 0;
}

int Win(SDL_Surface *screen, SDL_Rect *frameField, SDL_Surface *field){
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
    }

    while(SDL_WaitEvent(&event)){
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)){
            TTF_CloseFont(message);
            return 0;
        }
    }
}

int Game(SDL_Surface *screen){

    Cell Maze[10][10];  //матрица расположения клеток
    if(readFile(Maze)){ //функция чтения файла
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

    SDL_Event event;
    SDL_FillRect(field, NULL, colorField);
    Coordinates currentCell;
    Maze[9][9].Final(field);

    Character Moon(3, 3);

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
                        Moon.Draw(field, sailor, &frameSailor, Moon.x - length, Moon.y);
                        currentCell.x--;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP){ //вверх
                    if(Maze[currentCell.x][currentCell.y].top != 1){
                        Moon.Draw(field, sailor, &frameSailor, Moon.x, Moon.y - length);
                        currentCell.y--;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT){ //вправо
                    if(Maze[currentCell.x][currentCell.y].right != 1){
                        Moon.Draw(field, sailor, &frameSailor, Moon.x + length, Moon.y);
                        currentCell.x++;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN){ //вниз
                    if(Maze[currentCell.x][currentCell.y].bottom != 1){
                        Moon.Draw(field, sailor, &frameSailor, Moon.x, Moon.y + length);
                        currentCell.y++;
                    }
                }

                SDL_BlitSurface(field, NULL, screen, &frameField);
                SDL_Flip(screen);

                if(currentCell.x == 9 && currentCell.y == 9){
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

                            return Win(screen, &frameField, field); //вызываем поверхность с сообщением о победе
                }

                if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
                    if(Menu(screen) != 3)
                        return 0;
                }

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

    int x, y;
    const int NUMMENU = 2;
    const char *items[NUMMENU] = {"PLAY", "EXIT"};
    SDL_Surface *choise[NUMMENU];
    bool select[NUMMENU] = {false, false};
    SDL_Color color[2] = {{255, 255, 255},
                          {64,   32,   128}};

    choise[0] = TTF_RenderText_Solid(font, items[0], color[0]);
    choise[1] = TTF_RenderText_Solid(font, items[1], color[0]);
    SDL_Rect pos[NUMMENU];

    pos[0].x = screen->clip_rect.w/2 - choise[0]->clip_rect.w/2;
    pos[0].y = screen->clip_rect.h/2 - choise[0]->clip_rect.h;
    pos[1].x = screen->clip_rect.w/2 - choise[0]->clip_rect.w/2;
    pos[1].y = screen->clip_rect.h/2 + choise[0]->clip_rect.h;


    SDL_Event event;
    while (1) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: //крестик
                    SDL_FreeSurface(choise[0]);
                    SDL_FreeSurface(choise[1]);
                    return 1;

                case SDL_MOUSEMOTION:
                    x = event.motion.x;
                    y = event.motion.y;
                    for (int i = 0; i < NUMMENU; i++) {

                        if (x >= pos[i].x && x <= pos[i].x + pos[i].w &&
                            y >= pos[i].y && y <= pos[i].y + pos[i].h) {
                            if (!select[i]) {
                                select[i] = true;
                                SDL_FreeSurface(choise[i]);
                                choise[i] = TTF_RenderText_Solid(font, items[i], color[1]);
                            }
                        } else if (select[i]) {
                            select[i] = false;
                            SDL_FreeSurface(choise[i]);
                            choise[i] = TTF_RenderText_Solid(font, items[i], color[0]);
                        }

                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:

                    x = event.button.x;
                    y = event.button.y;
                    for (int i = 0; i < NUMMENU; i += 1) {
                        if (x >= pos[i].x && x <= pos[i].x + pos[i].w &&
                            y >= pos[i].y && y <= pos[i].y + pos[i].h) {
                            if (i == 0) {

                                if(!counter){ //если меню открыли не первый раз, надо продолжить с последней позиции
                                    ++counter;
                                    if(!Game(screen)){ //cработает только когда сломается
                                        return 0;
                                    }
                                } else {
                                    SDL_FreeSurface(choise[0]);
                                    SDL_FreeSurface(choise[1]);
                                    return 3;
                                }

                            } else {
                                SDL_FreeSurface(choise[0]);
                                SDL_FreeSurface(choise[1]);
                                return i;
                            }
                        }
                    }
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        SDL_FreeSurface(choise[0]);
                        SDL_FreeSurface(choise[1]);
                        return 0;
                    }
            }
        }

        for (int i = 0; i < NUMMENU; i += 1) {
            SDL_BlitSurface(choise[i], NULL, screen, &pos[i]);
            SDL_Flip(screen);
        }
    }
}


int main(int argc, char** argv ){

    SDL_Surface *screen; //основная поверхность
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

    SDL_Surface *background;
    SDL_Rect frameBackground;
    frameBackground.x = frameBackground.y = 0;
    frameBackground.w = frameBackground.h = mainLength;
    background = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, frameBackground.w, frameBackground.h, 32, screen -> format -> Rmask, screen -> format -> Gmask, screen -> format -> Bmask, screen -> format -> Amask);

    if (background == NULL) {
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

