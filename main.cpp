#include <iostream>
#include <fstream>
#include <cstdio>
#include <SDL.h>
#include <SDL_draw.h>
#include <SDL_ttf.h>

#define N 6

int mainLength = 746, mazeLength = 643, length = 64, halfLength = 32; //прибавили к длинам основного окна и поля по 3, 6 пикселей для  симметрии
int colorBackground = 0x039C7D, colorField = 0xB7D036, colorActive = 0xC91E2C, colorWall = 0xffffff; //фон, поле, персонаж + клетка выхода
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

    void Draw (SDL_Surface *field, SDL_Surface *harry, SDL_Rect *frameHarry, int x, int y){ //движение персонажа переименуй
        SDL_FillRect (harry, NULL, colorField); //перекрасить по координатам рамки
        SDL_BlitSurface(harry, NULL, field, frameHarry);
        this->x = x;
        this->y = y;
        frameHarry->x = x;
        frameHarry->y = y;
        harry = SDL_LoadBMP("harry.bmp");
        SDL_BlitSurface(harry, NULL, field, frameHarry);
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
        frameMessage.x = 250;
        frameMessage.y = 352;
        text = TTF_RenderUTF8_Solid(message,"YOU ARE A WINNER", messageColor); //поверхность текста
        if(text){
            Draw_Rect(victory, 0, 0, frameVictory.w, frameVictory.h, 0x000000); //черный контур вокруг сообщения
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

    Character Mur(3, 3);

    SDL_Surface *harry;
    SDL_Rect frameHarry;
    harry = SDL_LoadBMP("harry.bmp");
    frameHarry.x = 5;
    frameHarry.y = 4;
    frameHarry.w = 58;
    frameHarry.h = 58;
    SDL_BlitSurface(harry, NULL, field, &frameHarry);


        for (int j = 0; j < 10; j++){
            for (int i = 0; i < 10; i++){
                Maze[i][j].Wall(field);
            }
        }
    while(SDL_WaitEvent(&event)){

                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT){ //влево
                    if(Maze[currentCell.x][currentCell.y].left != 1){
                        Mur.Draw(field,harry, &frameHarry, Mur.x - length, Mur.y);
                        currentCell.x--;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP){ //вверх
                    if(Maze[currentCell.x][currentCell.y].top != 1){
                        Mur.Draw(field, harry, &frameHarry, Mur.x, Mur.y - length);
                        currentCell.y--;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT){ //вправо
                    if(Maze[currentCell.x][currentCell.y].right != 1){
                        Mur.Draw(field, harry, &frameHarry, Mur.x + length, Mur.y);
                        currentCell.x++;
                    }
                } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN){ //вниз
                    if(Maze[currentCell.x][currentCell.y].bottom != 1){
                        Mur.Draw(field, harry, &frameHarry, Mur.x, Mur.y + length);
                        currentCell.y++;
                    }
                }
                Draw_Rect(field, 0, 0, frameField.w, frameField.h, 0x000000); //черный контур
                SDL_BlitSurface(field, NULL, screen, &frameField);
                SDL_Flip(screen);

                if(currentCell.x == 9 && currentCell.y == 9){ //это как-то работает но хуй пойми как. настоятельно рекомендую посмотреть еще раз, и обратить внимание на фрейм, потому что ширина и высота по умолчанию те же как у картинки (имеет ли смысл их прописывать тут)
                        SDL_Surface *last = SDL_LoadBMP("last.bmp");
                        SDL_Rect frameLast;
                        frameLast.x = mazeLength - 64;
                        frameLast.y = mazeLength - 67;
                        frameLast.w = 61;
                        frameLast.h = 64;
                        SDL_BlitSurface(last, NULL, field, &frameLast);
                        SDL_BlitSurface(field, NULL, screen, &frameField);
                        SDL_Flip(screen);
                    return messageWin(screen, &frameField, field); //вызываем поверхность с сообщением о победе
                }

                if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
                 if(Menu(screen) != 3){
                   return 0;
                   }
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
    frameMenu.y = 53;
    frameMenu.w = 372;
    frameMenu.h = 643;

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
    SDL_Color color[2] = {{255, 255, 255}, //белый
                          {0,   0,   255}}; //синий

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

    SDL_Surface *surface = SDL_LoadBMP("fon.bmp");
    SDL_Rect frameSurface;
    frameSurface.x = frameSurface.y = 0;

    SDL_BlitSurface(surface, NULL, screen, &frameSurface);


    if (SDLCALL TTF_Init()){
        cout << "ERROR INIT FONT";
        return 5;
    }

    Menu(screen);

    return 0;
}

