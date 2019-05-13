#include<stdlib.h>
#include<stdio.h>
#include"conio2.h"

#define  REP(a) for(int i=0;i<a;i++)
#define  ESC 0x1b 
#define  ENTER 0x0d
#define  BACK 0x08
#define  MENU_POSITION_X 4 //4 po prawej
#define  MENU_POSITION_Y 2
#define  DRAW_POSITION_X 40 //1 po lewej
#define  DRAW_POSITION_Y 4
#define  MENU_COLORS_POSITION_X 100
#define  MENU_CORORS_POSITION_Y 2
#define  MAX_LENGHT_COLOR 20 
#define  LICZBA_KOLOROW 16
#define  MAX_LENGHT_STRING 40 
#define  LICZBA_OPCJI 14
#define  DL_NAZWY_PLIKU 50
#define  MAX_SZER_PIC 50
#define  MAX_WYS_PIC 20

enum RUCH {GORA = 0x48,DOL = 0x50,LEWO = 0x4b,PRAWO = 0x4d};
enum RUCHALT { GORAALT = 0x98, DOLALT = 0xa0, LEWOALT = 0x9b, PRAWOALT = 0x9d };
enum RUCHCTRL { GORACTRL = 0x8d, DOLCTRL = 0x91, LEWOCTRL = 0x73, PRAWOCTRL = 0x74 };
//******************STOS************************
typedef struct point
{
	int y, x;
}Point;

typedef Point Elem;
const int INIT_SIZE = 4;

struct MyStack
{
	Elem *e;
	int count;
	int size;
};

void initMyStack(MyStack *ms)
{
	ms->count = 0;
	ms->e = (Elem*)malloc(sizeof(Elem)*INIT_SIZE);
	ms->size = INIT_SIZE;
}

void ReallocStack(MyStack *ms, int new_size)
{
	Elem *tmp = (Elem*)malloc(sizeof(Elem)*new_size);
	if (!tmp)
		{
		clrscr();
		gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y);
		cputs("ZABRAKLO PAMIECI");
		}
	for (int i = 0; i<ms->count; i++)
		tmp[i] = ms->e[i];
	free(ms->e);
	ms->e = tmp;
	ms->size = new_size;
}

void pushMyStack(MyStack *ms, Elem e)
{
	if (ms->count == ms->size)
		ReallocStack(ms, ms->size * 2);
	ms->e[ms->count] = e;
	ms->count++;
}

void pushTableMyStack(MyStack *UNDO, int **tablica,int szerokosc,int wysokosc)
	{
	for (int i = 0; i < wysokosc; i++)
		for (int j = 0; j < szerokosc; j++)
			pushMyStack(UNDO, {tablica[i][j],0});
	}

Elem popMyStack(MyStack *ms)
{
	if (4 * ms->count <= ms->size)
		ReallocStack(ms, ms->size / 2);
	ms->count--;
	return ms->e[ms->count];
}

bool StackEmpty(MyStack *ms)
{
	return ms->count ? 1 : 0;
}

void ClearMyStack(MyStack *UNDO)
{
	while (StackEmpty(UNDO))
		popMyStack(UNDO);
}

//***********BMP_STRUCT*********************//
struct naglowek {
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	short bfOffBits;
};
naglowek plik;

struct obrazek {
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
};
obrazek grafika;

//***********DEKLARACJE*********************//
void pokaz_nazwe_pliku(char*);
void aktualny_kolor(const int);
void wybor_koloru(int*,int);
void wypisz_liczbe(int);
bool czy_w_polu(const int, const int, const int, const int);
void pozycja_kursora(const int,const int,const int,const int,int,int);
void sprawdz_dostepnosc_nazwy(char*,int);
int strlen(const char*);
void sklej(char*,char*);
void czysc_tablice(int, int, int**);
int **wczytaj_obrazek(int*,int*,char*);
void pokaz_obrazek(int,int,int**,int*,int*,int,int,int);
int czytaj_rozmiar(int,int);
void czytaj_nazwe(char*,int,int);
void wyczysc_nazwe(char*);
int **utworz_nowy_obrazek(int*, int*, char*);
void zapisz_obrazek(int,int,char*,int**);
int **pamiec_dla_rysowania(int**, int, int);
void rysuj_linie(int**,int*,int,int,int*,int*,bool,MyStack*,int*,int*);
void algorytm_prostokatow(int, int, int, int, int*, int**, MyStack*, int**);
void algorytm_linii(int,int,int,int,int*,int**,MyStack*,int**);
void Ramka(int, int,int,int);
void komunikat(char[]);
void wyswietl_menu(void);
int abs(int);
int min(int, int);
int max(int, int);
bool XPM2(char*);
bool BMP(char*);
void jaki_to_kolor(char*,int*,int);
int rowne(char*, char*);
int **kopiuj(int**,int,int,int*,int*,int*,int*,int*,int*);
bool sprawdź_czy_mozna_wkleic(int,int,int,int,int,int);
void wklej(int**,int**,int,int,int,int,int,int,int,int);
void pomaluj(int**,int,int,int,int,int,int);
void zwolnij_pamiec(int **, int);
void cofnij_akcje(MyStack*,int **,int,int);
void zaznacz(int **, int, int, int, int,int,int,int,int);
int round(double);
//***************MAIN*********************//
int main(int argc, char *argv[]) {
	gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y);
	bool schowek = false;
	int zn = 0, x = DRAW_POSITION_X+1, y = DRAW_POSITION_Y+1, attr = 0, back = 0,szerokosc=0,wysokosc=0,schowekx=0,schoweky=0;
	int przesunX = 0, przesunY = 0;
	char filename[DL_NAZWY_PLIKU + 10]="";
	int **tablica_obrazka, **skopiowany_fragment;
	MyStack UNDO;
	initMyStack(&UNDO);
	settitle("Bartosz Rychcik s165659");
	wyswietl_menu();
	if (argv[1] != '\0')
		{
		tablica_obrazka = wczytaj_obrazek(&szerokosc, &wysokosc, argv[1]);
		Ramka(szerokosc, wysokosc,0,0);
		sklej(filename, argv[1]);
		pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka, &x, &y, 1,przesunX,przesunY);
		pokaz_nazwe_pliku(argv[1]);
		gotoxy(DRAW_POSITION_X + 1, DRAW_POSITION_Y + 1);
		}
	do {
		textcolor(BLACK);
		aktualny_kolor(attr);
		pozycja_kursora(x,y,szerokosc,wysokosc,przesunX,przesunY);
		if(schowek)
			{
			gotoxy(MENU_POSITION_X, MENU_POSITION_Y + LICZBA_OPCJI + 4); cputs("Schowek: ");
			wypisz_liczbe(schoweky); cputs("x"); wypisz_liczbe(schowekx);
			}
		gotoxy(x, y);
		zn = getch();
		wybor_koloru(&attr, zn);
		if (zn == BACK)
			{
			if (filename[0] != '\0')
			{
				if (StackEmpty(&UNDO))
				{
					cofnij_akcje(&UNDO, tablica_obrazka, szerokosc, wysokosc);
					pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka, &x, &y, 0,przesunX,przesunY);
				}
				else
				{
					komunikat("To jest oryginalny obrazek                ");
					pokaz_nazwe_pliku(filename);
				}
			}
			else
				komunikat("Najpierw wczytaj jakis obrazek                ");
			}
		else if(zn == 0) 
			{
			zn = getch();
			if (zn == DOL) { if (czy_w_polu(x, y + 1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))y++; }
			else if(zn == GORA) {if (czy_w_polu(x, y - 1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))y--;}
			else if(zn == PRAWO) {if (czy_w_polu(x+1, y, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))x++;}
			else if(zn == LEWO) {if (czy_w_polu(x-1, y, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))x--;}
			else if (zn == GORAALT || zn == DOLALT || zn == LEWOALT || zn == PRAWOALT)
						{
						if (filename[0] != '\0')
							{
							if (czy_w_polu(x, y, min(MAX_SZER_PIC,szerokosc),min(MAX_WYS_PIC,wysokosc)))
								{
								if (schowek)
									{
									char text[] = "W schowku jest juz obraz, chcesz nadpisac? (Y)";
									textcolor(BLACK);
									gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y - 1);
									cputs(text);
									zn = getch();
									gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y - 1);
									REP(strlen(text))cputs(" ");
									pokaz_nazwe_pliku(filename);
									if (zn == 'y' || zn == 'Y')
										{
										zwolnij_pamiec(skopiowany_fragment, schoweky);
										skopiowany_fragment = kopiuj(tablica_obrazka, wysokosc, szerokosc, &x, &y, &schowekx, &schoweky,&przesunX,&przesunY);
										if(schowekx!=0)
											schowek = true;
										}
									}
								else
									{ 
									skopiowany_fragment = kopiuj(tablica_obrazka, wysokosc, szerokosc, &x, &y, &schowekx, &schoweky,&przesunX,&przesunY);
									if (schowekx != 0)
										schowek = true;
									}
								}
								else
									{
									komunikat("Nie mozesz nic kopiowac poza polem obrazka           ");
									pokaz_nazwe_pliku(filename);
									}
							}
							else
								komunikat("Aby cos skopiowac musisz miec obrazek              ");
						}
			else if (zn == DOLCTRL) 
				{
				if (filename[0] != '\0'&& wysokosc > MAX_WYS_PIC && (wysokosc - MAX_WYS_PIC) > przesunY)
					{ 
					przesunY++;
					pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka, &x, &y, 0, przesunX, przesunY);
					}
				}
			else if (zn == GORACTRL) 
				{ 
				if (filename[0] != '\0' && wysokosc > MAX_WYS_PIC && przesunY>0)
					{
					przesunY--;
					pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka, &x, &y, 0, przesunX, przesunY);
					}
				}
			else if (zn == PRAWOCTRL) 
				{ 
				if (filename[0] != '\0' && szerokosc > MAX_SZER_PIC && (szerokosc- MAX_SZER_PIC)>przesunX)
					{
					przesunX++;
					pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka, &x, &y, 0, przesunX, przesunY);
					}
				}
			else if (zn == LEWOCTRL) 
				{ 
				if (filename[0] != '\0' && szerokosc > MAX_SZER_PIC && przesunX>0)
					{
					przesunX--;
					pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka, &x, &y, 0, przesunX, przesunY);
					}
				}
			} 
		else if(zn=='f'||zn=='F')
			{
			if (filename[0] != '\0')
				{
				if (czy_w_polu(x, y, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))
					{
					int id = tablica_obrazka[y+przesunY - 1 - DRAW_POSITION_Y][x+przesunX - 1 - DRAW_POSITION_X];
					if (id != attr)
						{
						pushTableMyStack(&UNDO, tablica_obrazka, szerokosc, wysokosc);
						pomaluj(tablica_obrazka, wysokosc, szerokosc, x+przesunX, y+przesunY, id,attr);
						}
					}
				else
					{
					komunikat("Nie mozesz malowac poza polem obrazka         ");
					pokaz_nazwe_pliku(filename);
					}
				}
			else
				komunikat("Najpierw wczytaj jakis obrazek         ");
			pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka, &x, &y, 0,przesunX,przesunY);
			}
		else if(zn=='p'||zn=='P')
			{
			if (filename[0] != '\0')
				{
				if (czy_w_polu(x, y, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))
					{
					if (schowek)
						{
						if (sprawdź_czy_mozna_wkleic(schowekx, schoweky, x+przesunX, y+przesunY, szerokosc, wysokosc))
							{ 
							pushTableMyStack(&UNDO, tablica_obrazka, szerokosc, wysokosc);
							wklej(tablica_obrazka, skopiowany_fragment, wysokosc, szerokosc, schowekx, schoweky, x, y,przesunX,przesunY);
							}
						else
							{
							komunikat("Nie mozesz tu wkleic obrazka            ");
							pokaz_nazwe_pliku(filename);
							}
						}
					else
						{
						komunikat("Schowek jest pusty                      ");
						if(filename[0]!='\0')
							pokaz_nazwe_pliku(filename);
						}
					}
				else
					{
					komunikat("Nie mozesz nic wkleic poza polem obrazka           ");
					pokaz_nazwe_pliku(filename);
					}
				}
			else
				komunikat("Najpierw wczytaj jakis obrazek           ");
			}
		else if(zn=='l'||zn=='k'||zn=='L'||zn=='K') //rysowanie linii i prostokata
			{
			if (filename[0] != '\0')
				{
				if (czy_w_polu(x, y, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))
					{
					if (zn == 'k'||zn=='K')rysuj_linie(tablica_obrazka, &attr, szerokosc, wysokosc, &x, &y,0,&UNDO,&przesunX,&przesunY);
					else if(zn=='l'||zn=='L')rysuj_linie(tablica_obrazka, &attr, szerokosc, wysokosc, &x, &y,1,&UNDO,&przesunX,&przesunY);
					}
				else
					{
					komunikat("Nie mozesz rysowac poza polem obrazka               ");
					pokaz_nazwe_pliku(filename);
					}
				pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka, &x, &y, 0,przesunX,przesunY);
				}
			else
				komunikat("Musisz otworzyc jakis plik, aby moc rysowac             ");
			}
		else if (zn == 'o'||zn=='O') //wczytywanie dowolnego obrazka .obrazek/.xpm/.bmp
			{
			if (filename[0] != '\0')
				zwolnij_pamiec(tablica_obrazka, wysokosc);
			wyczysc_nazwe(filename);
			przesunX = przesunY = 0;
			szerokosc = wysokosc = 0;
			clrscr();
			gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y);
			cputs("Wpisz nazwe pliku:");
			czytaj_nazwe(filename,DRAW_POSITION_X+18,DRAW_POSITION_Y);
			if(!XPM2(filename)&&!BMP(filename))
				sklej(filename, ".obrazek");
			wyswietl_menu();
			ClearMyStack(&UNDO);
			tablica_obrazka = wczytaj_obrazek(&szerokosc, &wysokosc, filename);
			Ramka(szerokosc, wysokosc,0,0);
			pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka,&x,&y,1,przesunX,przesunY);
			pokaz_nazwe_pliku(filename);
			}
		else if (zn == 'i'||zn=='I') //wczytywanie domyslnego obrazka .obrazek
			{ 
			if(filename[0]!='\0')
				zwolnij_pamiec(tablica_obrazka, wysokosc);
			ClearMyStack(&UNDO);
			wyczysc_nazwe(filename);
			szerokosc = wysokosc = 0;
			przesunX = przesunY = 0;
			clrscr();
			wyswietl_menu();
			sklej(filename, "domyslny.obrazek"); 
			tablica_obrazka = wczytaj_obrazek(&szerokosc, &wysokosc, filename); 
			Ramka(szerokosc, wysokosc,0,0); 
			pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka,&x,&y,1,przesunX,przesunY);
			pokaz_nazwe_pliku(filename);
			gotoxy(DRAW_POSITION_X + 1, DRAW_POSITION_Y + 1);
			}
		else if (zn == 'n'||zn=='N') //tworzenie nowego obrazka .obrazek
			{
			if (filename[0] != '\0')
				zwolnij_pamiec(tablica_obrazka, wysokosc);
			wyczysc_nazwe(filename);
			szerokosc = wysokosc = 0;
			przesunX = przesunY = 0;
			ClearMyStack(&UNDO);
			tablica_obrazka=utworz_nowy_obrazek(&szerokosc,&wysokosc,filename);
			clrscr();
			wyswietl_menu();
			Ramka(szerokosc, wysokosc,0,0);
			pokaz_obrazek(wysokosc, szerokosc, tablica_obrazka,&x,&y,1,przesunX,przesunY);
			pokaz_nazwe_pliku(filename);
			gotoxy(DRAW_POSITION_X + 1, DRAW_POSITION_Y + 1);
			}
		else if (zn == 's'||zn=='S') //zapisywanie obrazka .obrazek/.xpm/.bmp
			{	
			if (filename[0]!='\0')
				{
				zapisz_obrazek(szerokosc, wysokosc, filename, tablica_obrazka);
				ClearMyStack(&UNDO);
				}
			else
				 komunikat("Musisz otworzyc jakis plik,aby go zapisac");
			wyczysc_nazwe(filename);
			clrscr();
			wyswietl_menu();
			gotoxy(DRAW_POSITION_X + 1, DRAW_POSITION_Y + 1);
			}
	} while (zn !=ESC);

	return 0;
	}
//**************************************DEFINICJE**************************************************
void cofnij_akcje(MyStack *UNDO, int **tablica, int szerokosc, int wysokosc)
	{
	Elem m;
	for (int i = 0; i < wysokosc; i++)
		{
		int k = (wysokosc - 1) - i;
		for (int j = 0; j < szerokosc; j++)
			{
			int p = (szerokosc - 1) - j;
			m = popMyStack(UNDO);
			tablica[k][p] = m.y;
			}
		}
	}

void zwolnij_pamiec(int **obrazek,int wysokosc)
	{
	for (int i = 0; i < wysokosc; i++)
		free(obrazek[i]);
	free(obrazek);
	}

void pomaluj(int **oryginal, int wysokosc, int szerokosc, int x, int y,int stary,int nowy)
	{
	if (oryginal[y - DRAW_POSITION_Y - 1][x - DRAW_POSITION_X - 1] != nowy)
		oryginal[y - DRAW_POSITION_Y - 1][x - DRAW_POSITION_X - 1] = nowy;
	if (x - DRAW_POSITION_X - 1 + 1 <szerokosc && oryginal[y - DRAW_POSITION_Y - 1][x - DRAW_POSITION_X - 1 + 1] == stary)
		pomaluj(oryginal, wysokosc, szerokosc, x+1, y, stary, nowy);
	if (x - DRAW_POSITION_X - 1 - 1 >=0 && oryginal[y - DRAW_POSITION_Y - 1][x - DRAW_POSITION_X - 1 - 1] == stary)
		pomaluj(oryginal, wysokosc, szerokosc, x - 1, y, stary, nowy);
	if (y - DRAW_POSITION_Y - 1 + 1 <wysokosc && oryginal[y - DRAW_POSITION_Y - 1 + 1][x - DRAW_POSITION_X - 1] == stary)
		pomaluj(oryginal, wysokosc, szerokosc, x, y + 1, stary, nowy);
	if (y - DRAW_POSITION_Y - 1 - 1 >=0 && oryginal[y - DRAW_POSITION_Y - 1 - 1][x - DRAW_POSITION_X - 1] == stary)
		pomaluj(oryginal, wysokosc, szerokosc, x, y - 1, stary, nowy);
	}

void wklej(int**oryginal, int **fragment,int wysokosc,int szerokosc,int schowekx,int schoweky,int x,int y,int przesunx,int przesuny)
	{
	for(int i=0;i<schoweky;i++)
		for (int j = 0; j < schowekx; j++)
			oryginal[y+przesuny+i-DRAW_POSITION_Y-1][x+przesunx+j-DRAW_POSITION_X-1] = fragment[i][j];	
	pokaz_obrazek(wysokosc, szerokosc, oryginal, &x, &y, 0,przesunx,przesuny);
	}

bool sprawdź_czy_mozna_wkleic(int szerkopii, int wyskopii, int x,int y,int szerokosc,int wysokosc)
	{
	if (x - DRAW_POSITION_X - 1 + szerkopii <= szerokosc&&y - DRAW_POSITION_Y - 1 + wyskopii <= wysokosc)return true;
	return false;
	}

void zaznacz(int **oryginal,int x,int y,int x1,int y1,int px,int py,int wys,int szer)
	{
	for (int i = min(y, y1); i <= max(y, y1); i++)
		for (int j = min(x, x1); j <= max(x, x1); j++)
		{
			if(DRAW_POSITION_X+j+1-px>DRAW_POSITION_X&&DRAW_POSITION_Y+i+1-py>DRAW_POSITION_Y&&DRAW_POSITION_X+j+1-px<DRAW_POSITION_X+1+min(szer, MAX_SZER_PIC)&&DRAW_POSITION_Y+i+1-py<DRAW_POSITION_Y+1+min(wys, MAX_WYS_PIC))
			{
			gotoxy(DRAW_POSITION_X+j+1-px,DRAW_POSITION_Y+i+1-py);
			if (oryginal[i][j] >= 0 && oryginal[i][j] <= 15) //kolory sa od 0 do 15
				{
				textcolor(oryginal[i][j]);
				putch('*');
				}
			else
				putch(' ');
			}
		}
	}

int **kopiuj(int **oryginal,int wysokosc,int szerokosc,int *x,int *y,int *schowekx,int *schoweky,int *przesunx,int *przesuny)
	{
	int x1 = *x, y1 = *y,zn=0;
	int px = *przesunx, py = *przesuny;
	do {
		pokaz_obrazek(wysokosc, szerokosc, oryginal, x, y, 0,px,py);
		pozycja_kursora(x1, y1, szerokosc, wysokosc,px,py);
		gotoxy(x1, y1);
		textbackground(LIGHTGRAY);
		zaznacz(oryginal, x1 + px - DRAW_POSITION_X - 1, y1 + py - DRAW_POSITION_Y - 1, *x + *przesunx - DRAW_POSITION_X - 1, *y + *przesuny - DRAW_POSITION_Y - 1,px,py,wysokosc,szerokosc);
		textbackground(WHITE);
		zn = getch();
		if (zn == 0)
			{
			zn = getch();
			if (zn == DOLALT) { if (czy_w_polu(x1, y1 + 1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))y1++; }
			else if (zn == GORAALT) { if (czy_w_polu(x1, y1 - 1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))y1--; }
			else if (zn == PRAWOALT) { if (czy_w_polu(x1 + 1, y1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))x1++; }
			else if (zn == LEWOALT) { if (czy_w_polu(x1 - 1, y1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))x1--; }
			else if (zn == DOLCTRL) { if (wysokosc > MAX_WYS_PIC && (wysokosc - MAX_WYS_PIC) > py) { if (czy_w_polu(x1, y1 - 1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))y1--; py++; pokaz_obrazek(wysokosc, szerokosc, oryginal, x, y, 0, px, py); } }
			else if (zn == GORACTRL) { if (wysokosc > MAX_WYS_PIC && py > 0) { if (czy_w_polu(x1, y1 + 1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))y1++; py--; pokaz_obrazek(wysokosc, szerokosc, oryginal, x, y, 0, px, py); } }
			else if (zn == PRAWOCTRL) { if (szerokosc > MAX_SZER_PIC && (szerokosc - MAX_SZER_PIC) > px) { if (czy_w_polu(x1 - 1, y1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))x1--; px++; pokaz_obrazek(wysokosc, szerokosc, oryginal, x, y, 0, px, py); } }
			else if (zn == LEWOCTRL) { if (szerokosc > MAX_SZER_PIC && px > 0) { if (czy_w_polu(x1 + 1, y1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))x1++; px--; pokaz_obrazek(wysokosc, szerokosc, oryginal, x, y, 0, px, py); } }
			}
	} while (zn != 'c'&& zn!='C');
		zaznacz(oryginal, x1 + px - DRAW_POSITION_X - 1, y1 + py - DRAW_POSITION_Y - 1, *x + *przesunx - DRAW_POSITION_X - 1, *y + *przesuny - DRAW_POSITION_Y - 1,px,py,wysokosc,szerokosc);
		*schowekx = abs(*x+*przesunx - (x1+px)) + 1;
		*schoweky = abs(*y+*przesuny - (y1+py)) + 1;
		int **kopia = (int **)malloc(*schoweky * sizeof(*kopia));
		for (int i = 0; i < *schoweky; ++i)
			kopia[i] = (int *)malloc(*schowekx * sizeof(**kopia));
		for (int i = 0; i < *schoweky; i++)
			for (int j = 0; j < *schowekx; j++)
				kopia[i][j] = oryginal[min(*y+*przesuny,y1+py)- DRAW_POSITION_Y + i - 1][min(*x+*przesunx,x1+px)- DRAW_POSITION_X + j - 1];
		*przesunx = px;
		*przesuny = py;
		return kopia;	
	}

bool XPM2(char *filename)
	{
	int i = 0;
	while (filename[i] != '\0')i++;
	if (filename[i - 1] == 'm'&&filename[i - 2] == 'p'&&filename[i - 3] == 'x') return true;
	return false;
	}

bool BMP(char *filename)
{
	int i = 0;
	while (filename[i] != '\0')i++;
	if (filename[i - 1] == 'p'&&filename[i - 2] == 'm'&&filename[i - 3] == 'b') return true;
	return false;
}

int abs(int val)
	{
	return val > 0 ? val : -val;
	}

int min(int a, int b)
	{
	return a > b ? b : a;
	}

int max(int a, int b)
	{
	return a > b ? a : b;
	}

void algorytm_prostokatow(int x0, int y0, int x1, int y1, int *attr, int **kopia, MyStack *ms, int **oryginal)
{
	gotoxy(MENU_POSITION_X, MENU_POSITION_Y + LICZBA_OPCJI + 3); cputs("rysowanie prostokatow");
	Elem e;
	while (StackEmpty(ms))
	{
		e = popMyStack(ms);
		kopia[e.y][e.x] = oryginal[e.y][e.x];
	}

		for (int i =min(x0,x1);i<=max(x0,x1);i++)
			{
			e = { y0,i };
			pushMyStack(ms, e);
			e = { y1,i };
			pushMyStack(ms, e);
			kopia[y0][i] = *attr;
			kopia[y1][i] = *attr;
			}	

		for (int i = min(y0,y1); i<=max(y0,y1); i++)
		{
			e = { i,x0 };
			pushMyStack(ms, e);
			e = { i,x1 };
			pushMyStack(ms, e);
			kopia[i][x0] = *attr;
			kopia[i][x1] = *attr;
		}
}

void algorytm_linii(int x0, int y0, int x1, int y1,int *attr,int **kopia,MyStack *ms,int **oryginal)
	{	
	gotoxy(MENU_POSITION_X, MENU_POSITION_Y + LICZBA_OPCJI + 3); cputs("rysowanie linii");
	Elem e;
	while (StackEmpty(ms))
		{
		e=popMyStack(ms);
		kopia[e.y][e.x] = oryginal[e.y][e.x];
		}
		
	int sx, dx, dy, sy, str, mode;
	dx = abs(x1 - x0);
	dy = abs(y1 - y0);
	if (x0 < x1) sx = 1;
	else sx = -1;
	if (y0 < y1) sy = 1;
	else sy = -1;
	if (dx > dy) str = (dx)/2;
	else str = (-dy)/2;
	while(true) 
		{
		e = { y0,x0 };
		pushMyStack(ms, e);
		kopia[y0][x0] = *attr; 
		if (x0 == x1 && y0 == y1) break;
		mode = str;
		if (mode >-dx) { str -= dy; x0 += sx; }
		if (mode < dy) { str += dx; y0 += sy; }
		}
	}

int **pamiec_dla_rysowania(int **obrazek,int szerokosc,int wysokosc)
	{
	int **kopia = (int **)malloc(wysokosc * sizeof(*kopia));
	for (int i = 0; i<wysokosc; ++i)
		kopia[i] = (int *)malloc(szerokosc * sizeof(**kopia));

	for (int i = 0; i < wysokosc; i++)
		for (int j = 0; j < szerokosc; j++)
			kopia[i][j] = obrazek[i][j];

	return kopia;
	}

void rysuj_linie(int **obrazek,int *attr,int szerokosc,int wysokosc,int *x,int *y,bool tryb,MyStack *UNDO,int *przesunx,int *przesuny)
	{
	int zn = 0,y1=*y,x1=*x,px=*przesunx,py=*przesuny;
	int **kopia = pamiec_dla_rysowania(obrazek, szerokosc, wysokosc);
	MyStack ms;
	initMyStack(&ms);
	char znak = tryb ? 'l' : 'k';
	
	do
		{
		if(tryb)algorytm_linii(x1+px-DRAW_POSITION_X-1, y1+py-DRAW_POSITION_Y-1,*x+*przesunx-DRAW_POSITION_X-1,*y+*przesuny-DRAW_POSITION_Y-1, attr, kopia,&ms,obrazek);
		else if(!tryb)algorytm_prostokatow(x1 + px - DRAW_POSITION_X - 1, y1 + py - DRAW_POSITION_Y - 1, *x + *przesunx - DRAW_POSITION_X - 1, *y + *przesuny - DRAW_POSITION_Y - 1, attr, kopia, &ms, obrazek);
		pokaz_obrazek(wysokosc, szerokosc, kopia, x, y,0,px,py);
		textcolor(*attr);
		aktualny_kolor(*attr);
		pozycja_kursora(x1, y1, szerokosc, wysokosc,px,py);
		gotoxy(x1, y1);
		zn = getch();
		if (zn == 0)
			{
			zn = getch();
			if (zn == DOL) { if (czy_w_polu(x1, y1+1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))y1++; }
			else if (zn == GORA) { if (czy_w_polu(x1, y1-1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))y1--; }
			else if (zn == PRAWO) { if (czy_w_polu(x1+1, y1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))x1++; }
			else if (zn == LEWO) { if (czy_w_polu(x1-1, y1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))x1--; }
			else if (zn == DOLCTRL) { if (wysokosc > MAX_WYS_PIC && (wysokosc - MAX_WYS_PIC) > py) { if (czy_w_polu(x1, y1 - 1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))y1--; py++; pokaz_obrazek(wysokosc, szerokosc, obrazek, x, y, 0, px, py); } }
			else if (zn == GORACTRL) { if (wysokosc > MAX_WYS_PIC && py > 0) { if (czy_w_polu(x1, y1 + 1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))y1++; py--; pokaz_obrazek(wysokosc, szerokosc, obrazek, x, y, 0, px, py); } }
			else if (zn == PRAWOCTRL) { if (szerokosc > MAX_SZER_PIC && (szerokosc - MAX_SZER_PIC) > px) { if (czy_w_polu(x1 - 1, y1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))x1--; px++; pokaz_obrazek(wysokosc, szerokosc, obrazek, x, y, 0, px, py); } }
			else if (zn == LEWOCTRL) { if (szerokosc > MAX_SZER_PIC && px > 0) { if (czy_w_polu(x1 + 1, y1, min(MAX_SZER_PIC, szerokosc), min(MAX_WYS_PIC, wysokosc)))x1++; px--; pokaz_obrazek(wysokosc, szerokosc, obrazek, x, y, 0, px, py); } }
			}
		else if(zn!=0)
			wybor_koloru(attr, zn);
		} while (zn != ESC && zn != znak);

		if (zn=='l'||zn=='k'||zn=='L'||zn=='K')
		{
		pushTableMyStack(UNDO, obrazek, szerokosc, wysokosc);
		*x = x1;
		*y = y1;
		for (int i = 0; i < wysokosc; i++)
			for (int j = 0; j < szerokosc; j++)
				obrazek[i][j] = kopia[i][j];
		}
		*przesunx = px;
		*przesuny = py;
		gotoxy(MENU_POSITION_X, MENU_POSITION_Y + LICZBA_OPCJI + 3); cputs("                         ");
	for (int i = 0; i < wysokosc; i++)
		free(kopia[i]);
	free(kopia);
	
	}

void komunikat(char tresc[])
	{
	textcolor(BLACK);
	gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y - 1);
	cputs(tresc);
	getch();
	gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y - 1);
	REP(strlen(tresc))cputs(" ");
	}

void wypisz_liczbe(int n)
	{
	if (n == 0) 
		{
		putch('0'); 
		return; 
		}
	char tab[12];
	int p = 0;
	while (n != 0) 
		{ 
		tab[p++] = (n % 10) + '0';
		n /= 10;
		}
	while (p--)
		putch(tab[p]); 
	}

bool czy_w_polu(const int x, const int y, const int szerokosc, const int wysokosc)
	{
	if ((x - DRAW_POSITION_X > 0 && y - DRAW_POSITION_Y > 0) && (x - DRAW_POSITION_X <= szerokosc && y - DRAW_POSITION_Y <= wysokosc)) return true;
	return false;
	}

void pozycja_kursora(const int x,const int y,const int szerokosc,const int wysokosc,int przesunx,int przesuny)
	{
	textcolor(BLACK);
	gotoxy(MENU_POSITION_X, MENU_POSITION_Y + LICZBA_OPCJI + 2);
	cputs("                              ");
	gotoxy(MENU_POSITION_X, MENU_POSITION_Y + LICZBA_OPCJI + 2);
	cputs("pozycja kursora:");
	if (szerokosc == 0 && wysokosc == 0) cputs("nie ma obrazka");
	else if (czy_w_polu(x,y,min(szerokosc, MAX_SZER_PIC),min(wysokosc, MAX_WYS_PIC)))
		{
		wypisz_liczbe(x+przesunx - DRAW_POSITION_X); 
		putch(' '); 
		wypisz_liczbe(y+przesuny - DRAW_POSITION_Y);
		}
	else cputs("Poza obrazkiem");
	}

void aktualny_kolor(const int a)
	{
	textcolor(BLACK);
	gotoxy(MENU_POSITION_X,MENU_POSITION_Y+LICZBA_OPCJI + 1);
	cputs("aktualny kolor:");
	textbackground(a);
	cputs(" ");
	textbackground(WHITE);
	}

void wybor_koloru(int *attr,int a)
	{
	if ((a >= '0'&& a <= '9') || (a == 'q') || (a == 'w') || (a == 'e') || (a == 'r') || (a == 't') || (a == 'y'))
		{
		if (a <= '9')*attr = a - '0';
		else if (a == 'q')*attr = 10;
		else if (a == 'w')*attr = 11;
		else if (a == 'e')*attr = 12;
		else if (a == 'r')*attr = 13;
		else if (a == 't')*attr = 14;
		else if (a == 'y')*attr = 15;
		}
	}

void pokaz_nazwe_pliku(char*filename)
	{
	textcolor(BLACK);
	gotoxy(DRAW_POSITION_X + 15, DRAW_POSITION_Y - 1);
	cputs(filename); cputs("              ");
	gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y);
	}

void sklej(char *a,char *b)
	{
	int i = 0,k = 0;
	while (a[i] != '\0')i++;
	while (b[k] != '\0')
		{
		a[i] = b[k];
		i++;
		k++;
		}
	a[i] = '\0';
	}

void czysc_tablice(int yR,int xR,int **obrazek)
	{
	for (int i = 0; i < yR; i++)
		for (int j = 0; j < xR; j++)
			obrazek[i][j] = 15;
	}

int rowne(char *a, char*b)
{
	while (*a && (*a == *b))
		a++, b++;
	return *(char*)a - *(char*)b;
}

void jaki_to_kolor(char *jaki, int *tab, int c)
{
	if (!rowne(jaki, "#000000")) tab[c] = 0;
	else if (!rowne(jaki, "#C0C0C0")) tab[c] = 7;
	else if (!rowne(jaki, "#808080")) tab[c] = 8;
	else if (!rowne(jaki, "#FFFFFF")) tab[c] = 15;
	else if (!rowne(jaki, "#800000")) tab[c] = 4;
	else if (!rowne(jaki, "#FF0000")) tab[c] = 12;
	else if (!rowne(jaki, "#800080")) tab[c] = 5;
	else if (!rowne(jaki, "#FF00FF")) tab[c] = 13;
	else if (!rowne(jaki, "#008000")) tab[c] = 2;
	else if (!rowne(jaki, "#00FF00")) tab[c] = 10;
	else if (!rowne(jaki, "#808000")) tab[c] = 6;
	else if (!rowne(jaki, "#FFFF00")) tab[c] = 14;
	else if (!rowne(jaki, "#000080")) tab[c] = 1;
	else if (!rowne(jaki, "#0000FF")) tab[c] = 9;
	else if (!rowne(jaki, "#008080")) tab[c] = 3;
	else if (!rowne(jaki, "#00FFFF")) tab[c] = 11;
}

int **wczytaj_obrazek(int *szerokosc,int *wysokosc,char *filename)
	{
	int x, y, color, i, yR = *wysokosc, xR = *szerokosc, ile_kol, pix, konkolor[17] = {};
	char kolor[8] = {"\0"}, format[5];
	char c, d, e, f, eol,h;
	FILE *fp;
	if ((fp = fopen(filename, "r")) == NULL)
		komunikat("Nie ma takiego pliku");
	else
		{ 
		if (BMP(filename))
			{
			fread(&plik.bfType, sizeof(plik.bfType), 1, fp);
			fread(&plik.bfSize, sizeof(plik.bfSize), 1, fp);
			fread(&plik.bfReserved1, sizeof(plik.bfReserved1), 1, fp);
			fread(&plik.bfReserved2, sizeof(plik.bfReserved2), 1, fp);
			fread(&plik.bfOffBits, sizeof(plik.bfOffBits), 1, fp);
			fseek(fp, 14, SEEK_SET);
			fread(&grafika.biSize, sizeof(grafika.biSize), 1, fp);
			fread(&grafika.biWidth, sizeof(grafika.biWidth), 1, fp);
			fread(&grafika.biHeight, sizeof(grafika.biHeight), 1, fp);
			fread(&grafika.biPlanes, sizeof(grafika.biPlanes), 1, fp);
			fread(&grafika.biBitCount, sizeof(grafika.biBitCount), 1, fp);
			fread(&grafika.biCompression, sizeof(grafika.biCompression), 1, fp);
			fread(&grafika.biSizeImage, sizeof(grafika.biSizeImage), 1, fp);
			fread(&grafika.biXPelsPerMeter, sizeof(grafika.biXPelsPerMeter), 1, fp);
			fread(&grafika.biYPelsPerMeter, sizeof(grafika.biYPelsPerMeter), 1, fp);
			fread(&grafika.biClrUsed, sizeof(grafika.biClrUsed), 1, fp);
			fread(&grafika.biClrImportant, sizeof(grafika.biClrImportant), 1, fp);
			yR = grafika.biHeight;
			xR = grafika.biWidth;
			}
		else if(XPM2(filename))
			{
			fscanf(fp,"%c %s", &h,&format);
			fscanf(fp,"%d%d%d%d&c", &xR, &yR,&ile_kol,&pix,&eol);
			for (int i = 0; i < ile_kol; i++)
				{
				fscanf(fp,"%c%c%c%c%s%c",&c,&d,&e,&f,&kolor,&eol);
				jaki_to_kolor(kolor, konkolor, i);
				}
			}
		else
			{
			fscanf(fp, "%d %d", &yR,&xR);
			}
		*wysokosc = yR;
		*szerokosc = xR;
		int **obrazek = (int **)malloc(yR * sizeof(*obrazek));
		for (i = 0; i<yR; ++i)
			obrazek[i] = (int *)malloc(xR * sizeof(**obrazek));
		czysc_tablice(yR, xR, obrazek);
		if (BMP(filename))
			{
			char bmpImg[3];
			char padd[4];
			for (int i = 0; i < grafika.biHeight; i++)
				{
				int j = (grafika.biHeight - 1) - i;
				for (int k = 0; k<grafika.biWidth; k++)
					{
					fread(&bmpImg, 1, 3, fp);
					if (bmpImg[0] == 0 && bmpImg[1] == 0 && bmpImg[2] == -1)obrazek[j][k] = 12;
					else if (bmpImg[0] == -128 && bmpImg[1] == 0 && bmpImg[2] == -128)obrazek[j][k] = 5;
					else if (bmpImg[0] == 0 && bmpImg[1] == -128 && bmpImg[2] == 0)obrazek[j][k] = 2;
					else if (bmpImg[0] == 0 && bmpImg[1] == 0 && bmpImg[2] == 0)obrazek[j][k] = 0;
					else if (bmpImg[0] == -1 && bmpImg[1] == 0 && bmpImg[2] == 0)obrazek[j][k] = 9;
					else if (bmpImg[0] == -64 && bmpImg[1] == -64 && bmpImg[2] == -64)obrazek[j][k] = 7;
					else if (bmpImg[0] == -128 && bmpImg[1] == -128 && bmpImg[2] == -128)obrazek[j][k] = 8;
					else if (bmpImg[0] == -1 && bmpImg[1] == -1 && bmpImg[2] == -1)obrazek[j][k] = 15;
					else if (bmpImg[0] == 0 && bmpImg[1] == 0 && bmpImg[2] == -128)obrazek[j][k] = 4;
					else if (bmpImg[0] == -1 && bmpImg[1] == 0 && bmpImg[2] == -1)obrazek[j][k] = 13;
					else if (bmpImg[0] == 0 && bmpImg[1] == -128 && bmpImg[2] == -128)obrazek[j][k] = 6;
					else if (bmpImg[0] == 0 && bmpImg[1] == -1 && bmpImg[2] == 0)obrazek[j][k] = 10;
					else if (bmpImg[0] == 0 && bmpImg[1] == -1 && bmpImg[2] == -1)obrazek[j][k] = 14;
					else if (bmpImg[0] == -128 && bmpImg[1] == 0 && bmpImg[2] == 0)obrazek[j][k] = 1;
					else if (bmpImg[0] == -128 && bmpImg[1] == -128 && bmpImg[2] == 0)obrazek[j][k] = 3;
					else if (bmpImg[0] == -1 && bmpImg[1] == -1 && bmpImg[2] == 0)obrazek[j][k] = 11;
					}
				int dopelnienie = 4 - ((grafika.biWidth * 3) % 4);
				if(dopelnienie!=4)
					fread(padd, 1, dopelnienie, fp);
				}
			}
		else if (XPM2(filename))
			{
			for(int i=0;i<yR;i++)
				for(int j=0;j<xR;j++)
				{
				fscanf(fp,"%c", &c);
				if (xR - 1 == j)fscanf(fp,"%c", &eol);
				obrazek[i][j] = konkolor[c - 'a'];
				}
			}
		else
			{
			while (~fscanf(fp, "%d%d%d", &y, &x, &color))
				obrazek[y][x] = color;
			}
		fclose(fp);
		return obrazek;
		}
	return NULL;
	}

int round(double a)
	{
	int b = int(a * 10);
	if (b % 10>4)return int(a) + 1;
	else return int(a);
	}

void pokaz_obrazek(int y, int x,int **obrazek,int*xS,int*yS,int mode,int przesunx,int przesuny)
	{
	if (x > MAX_SZER_PIC)
		{
		Ramka(min(MAX_SZER_PIC, x), 1, 0, min(MAX_WYS_PIC,y)+3);
		for (int i = 0; i<MAX_SZER_PIC; i++)
			{
			gotoxy(DRAW_POSITION_X + 1+i, DRAW_POSITION_Y + min(MAX_WYS_PIC, y) + 4);
			putch(' ');
			}
		gotoxy(DRAW_POSITION_X + round(przesunx*(48.0 / (x - MAX_SZER_PIC))) + 1, DRAW_POSITION_Y + min(MAX_WYS_PIC, y) + 4); putch(219); //'█'
		gotoxy(DRAW_POSITION_X + round(przesunx*(48.0 / (x - MAX_SZER_PIC))) + 2, DRAW_POSITION_Y + min(MAX_WYS_PIC, y) + 4); putch(219); //'█'
		x = MAX_SZER_PIC;
		}
	if (y > MAX_WYS_PIC)
	{
		Ramka(1, min(MAX_WYS_PIC, y), min(MAX_SZER_PIC,x)+3, 0);
		for (int i = 0; i<MAX_WYS_PIC; i++)
			{
			gotoxy(DRAW_POSITION_X + min(MAX_SZER_PIC, x) + 4, DRAW_POSITION_Y + 1+i);
			putch(' ');
			}
		gotoxy(DRAW_POSITION_X + min(MAX_SZER_PIC, x) + 4, DRAW_POSITION_Y + round(przesuny*(18.0 / (y - MAX_WYS_PIC))) + 1); putch(219); //'█'
		gotoxy(DRAW_POSITION_X + min(MAX_SZER_PIC, x) + 4, DRAW_POSITION_Y + round(przesuny*(18.0 / (y - MAX_WYS_PIC))) + 2); putch(219); //'█'
		y = MAX_WYS_PIC;
	}
	for(int i=0+przesuny;i<y+przesuny;i++)
		for (int j = 0+przesunx; j < x+przesunx; j++)
			{
			gotoxy(DRAW_POSITION_X+1+j-przesunx, DRAW_POSITION_Y+1+i-przesuny);
			if (obrazek[i][j] >= 0 && obrazek[i][j] <= 15)
				{
				textcolor(obrazek[i][j]);
				putch('*');
				}
			else
				putch(' ');
			}
	if (mode)
		{
		*xS = DRAW_POSITION_X + 1;
		*yS = DRAW_POSITION_Y + 1;
		}
	}
	
int czytaj_rozmiar(int x,int y)
	{
	int zn = 0, a=0, i=0;
		while (zn != ENTER)
		{
		zn = getch();
		putch(zn);
		if(zn>='0'&&zn<='9')
			{
			a=a*10+(zn-'0');
			i++;
			}
		else
			if (zn == BACK && i>0)
				{
				gotoxy(x + i - 1, y); putch(' '); gotoxy(x + i - 1, y);
				i--;
				a /= 10;
				}
		}
	return a;
	}

void czytaj_nazwe(char *filename,int x,int y)
	{
	int zn = 0,i=0;
	while (zn != ENTER)
		{
		zn = getch();
		if ((zn >= '0'&&zn <= '9') || (zn >= 'A'&&zn <= 'Z') || (zn >= 'a'&&zn <= 'z')||zn=='.')
			{
			putch(zn);
			if (i<MAX_SZER_PIC)filename[i++] = char(zn);
			else
				komunikat("nazwa moze zawierac do 50 znakow");
			}
		else
			if (zn == BACK && i>0)
				{
				gotoxy(x+i-1, y); putch(' '); gotoxy(x+i-1,y);
				filename[i--] = '\0';
				}
		}
	}

void wyczysc_nazwe(char *a)
	{
	int i = 0;
	while (a[i] != '\0')
		{
		a[i] ='\0';
		i++;
		}
	}

void sprawdz_dostepnosc_nazwy(char* filename,int x)
	{
	FILE *fr;
	while(fr = fopen(filename, "r"))
		{
		gotoxy(DRAW_POSITION_X + x, DRAW_POSITION_Y);
		cputs("                      ");
		komunikat("plik juz istnieje, wpisz inna nazwe!");
		gotoxy(DRAW_POSITION_X + x, DRAW_POSITION_Y);
		wyczysc_nazwe(filename);
		czytaj_nazwe(filename, DRAW_POSITION_X + x, DRAW_POSITION_Y);
		if(!XPM2(filename)&&!BMP(filename))
			sklej(filename, ".obrazek");
		fclose(fr);
		}
	}

void wyswietl_menu(void)
{
	char MENU_OPCJI[LICZBA_OPCJI][MAX_LENGHT_STRING] = { "esc = zamknij/anuluj","strzalki = poruszanie","i = wczytaj domyslny obraz","o = wczytaj wlasny obraz","n = utworz nowy obraz","s = zapisz obraz","l = narysuj linie","k = narysuj prostokat","ctrl+strzalki = przewijanie ekranu","backspace = cofnij akcje","alt+strzalki = zaznacz obszar","c = kopiuj zaz. fragment","p = wklej ze schowka","f = wypelnienie obszaru" };
	textbackground(WHITE);
	clrscr();
	textcolor(BLACK);
	REP(LICZBA_OPCJI)
	{
		gotoxy(MENU_POSITION_X, MENU_POSITION_Y + i);
		cputs(MENU_OPCJI[i]);
	}
	REP(LICZBA_KOLOROW)
	{
		gotoxy(MENU_COLORS_POSITION_X + i, MENU_POSITION_Y);
		textbackground(i);
		cputs(" ");
		textbackground(WHITE);
		gotoxy(MENU_COLORS_POSITION_X + i, MENU_POSITION_Y + 1);
		if (i < 10)putch('0' + i);
		else if (i == 10)putch('q');
		else if (i == 11)putch('w');
		else if (i == 12)putch('e');
		else if (i == 13)putch('r');
		else if (i == 14)putch('t');
		else if (i == 15)putch('y');
	}
}


int **utworz_nowy_obrazek(int *szerokosc,int *wysokosc,char *plik)
	{
	clrscr();
	gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y);
	cputs("Wpisz nazwe nowego obrazka:");
	char filename[60] ="";
	czytaj_nazwe(filename,DRAW_POSITION_X+27,DRAW_POSITION_Y);
	sklej(filename, ".obrazek");
	sprawdz_dostepnosc_nazwy(filename,27);
	gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y+1);
	cputs("Podaj wysokosc:");
	int sizeY = czytaj_rozmiar(DRAW_POSITION_X+15,DRAW_POSITION_Y+1);
	gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y+2);
	cputs("Podaj szerokosc:");
	int sizeX=czytaj_rozmiar(DRAW_POSITION_X+16,DRAW_POSITION_Y+2);
	int **obrazek = (int **)malloc(sizeY * sizeof(*obrazek));   
	for (int i = 0; i<sizeY; ++i)
		obrazek[i] = (int *)malloc(sizeX * sizeof(**obrazek));
	czysc_tablice(sizeY, sizeX, obrazek);
	FILE *fp;
	int i = 0,k=0;
	fp = fopen(filename, "w");
	fprintf(fp,"%d %d\n",sizeY,sizeX);
	*wysokosc = sizeY;
	*szerokosc = sizeX;
	sklej(plik, filename);
	fclose(fp);
	return obrazek;
	}

int strlen(const char *a)
	{
	int dl = 0;
	while (a[dl] != '\0')dl++;
	return dl;
	}	

void zapisz_obrazek(int szerokosc, int wysokosc,char *nazwa_plik,int **obrazek)
	{
	clrscr();
	gotoxy(DRAW_POSITION_X, DRAW_POSITION_Y);
	cputs("Podaj nowa nazwe pliku lub wcisnij enter zostawiajac stara:");
	char filename[60] = "";
	czytaj_nazwe(filename, DRAW_POSITION_X + 59, DRAW_POSITION_Y);
	if (strlen(filename))
	{
		if(!XPM2(filename)&&!BMP(filename))
			sklej(filename, ".obrazek");
		sprawdz_dostepnosc_nazwy(filename, 59);
		int res = rename(nazwa_plik, filename);
		if (res != 0)
			komunikat("Zmiana nazwy nie powiodla sie");
		nazwa_plik = filename;
	}
	FILE *fp;
	fp = fopen(nazwa_plik, "w");
	if (BMP(nazwa_plik))
		{
		plik.bfType = 0x4d42;
		plik.bfSize = szerokosc*wysokosc * 3 + 2 * wysokosc + 54;
		plik.bfReserved1 = 0x0;
		plik.bfReserved2 = 0x0;
		plik.bfOffBits = 54;
		grafika.biSize = 0x28;
		grafika.biWidth = szerokosc;
		grafika.biHeight = wysokosc;
		grafika.biPlanes = 1;
		grafika.biBitCount = 24;
		grafika.biCompression = 0;
		grafika.biSizeImage = plik.bfSize-54;
		grafika.biXPelsPerMeter = 0;
		grafika.biYPelsPerMeter = 0;
		grafika.biClrUsed = 0;
		grafika.biClrImportant = 0;
		fwrite(&plik.bfType, sizeof(plik.bfType), 1, fp);
		fwrite(&plik.bfSize, sizeof(plik.bfSize), 1, fp);
		fwrite(&plik.bfReserved1, sizeof(plik.bfReserved1), 1, fp);
		fwrite(&plik.bfReserved2, sizeof(plik.bfReserved2), 1, fp);
		fwrite(&plik.bfOffBits, sizeof(plik.bfOffBits), 1, fp);
		fseek(fp, 14, SEEK_SET);
		fwrite(&grafika.biSize, sizeof(grafika.biSize), 1, fp);
		fwrite(&grafika.biWidth, sizeof(grafika.biWidth), 1, fp);
		fwrite(&grafika.biHeight, sizeof(grafika.biHeight), 1, fp);
		fwrite(&grafika.biPlanes, sizeof(grafika.biPlanes), 1, fp);
		fwrite(&grafika.biBitCount, sizeof(grafika.biBitCount), 1, fp);
		fwrite(&grafika.biCompression, sizeof(grafika.biCompression), 1, fp);
		fwrite(&grafika.biSizeImage, sizeof(grafika.biSizeImage), 1, fp);
		fwrite(&grafika.biXPelsPerMeter, sizeof(grafika.biXPelsPerMeter), 1, fp);
		fwrite(&grafika.biYPelsPerMeter, sizeof(grafika.biYPelsPerMeter), 1, fp);
		fwrite(&grafika.biClrUsed, sizeof(grafika.biClrUsed), 1, fp);
		fwrite(&grafika.biClrImportant, sizeof(grafika.biClrImportant), 1, fp);

		char bmpImg[3] = {};
		char padd[1] = {};
		padd[0] = 0;
		for (int i = 0; i < grafika.biHeight; i++)
			{
			int j = (grafika.biHeight - 1) - i;
			for (int k = 0; k<grafika.biWidth; k++)
			{
				if (obrazek[j][k] == 12) { bmpImg[0] = 0; bmpImg[1] = 0; bmpImg[2] = -1; }
				else if (obrazek[j][k] == 5) { bmpImg[0] = -128; bmpImg[1] = 0; bmpImg[2] = -128; }
				else if (obrazek[j][k] == 2) { bmpImg[0] = 0; bmpImg[1] = -128; bmpImg[2] = 0; }
				else if (obrazek[j][k] == 0) { bmpImg[0] = 0; bmpImg[1] = 0; bmpImg[2] = 0; }
				else if (obrazek[j][k] == 9) { bmpImg[0] = -1; bmpImg[1] = 0; bmpImg[2] = 0; }
				else if (obrazek[j][k] == 7) { bmpImg[0] = -64; bmpImg[1] = -64; bmpImg[2] = -64; }
				else if (obrazek[j][k] == 8) { bmpImg[0] = -128; bmpImg[1] = -128; bmpImg[2] = -128; }
				else if (obrazek[j][k] == 15) { bmpImg[0] = -1; bmpImg[1] = -1; bmpImg[2] = -1; }
				else if (obrazek[j][k] == 4) { bmpImg[0] = 0; bmpImg[1] = 0; bmpImg[2] = -128; }
				else if (obrazek[j][k] == 13) { bmpImg[0] = -1; bmpImg[1] = 0; bmpImg[2] = -1; }
				else if (obrazek[j][k] == 6) { bmpImg[0] = 0; bmpImg[1] = -128; bmpImg[2] = -128; }
				else if (obrazek[j][k] == 10) { bmpImg[0] = 0; bmpImg[1] = -1; bmpImg[2] = 0; }
				else if (obrazek[j][k] == 14) { bmpImg[0] = 0; bmpImg[1] = -1; bmpImg[2] = -1; }
				else if (obrazek[j][k] == 1) { bmpImg[0] = -128; bmpImg[1] = 0; bmpImg[2] = 0; }
				else if (obrazek[j][k] == 3) { bmpImg[0] = -128; bmpImg[1] = -128; bmpImg[2] = 0; }
				else if (obrazek[j][k] == 11) { bmpImg[0] = -1; bmpImg[1] = -1; bmpImg[2] = 0; }
				fwrite(bmpImg, sizeof(bmpImg), 1, fp);
			}
			int dopelnienie = 4 - ((grafika.biWidth * 3) % 4);
			if (dopelnienie != 4)
				fwrite(padd, 1, dopelnienie, fp);
			}
		}
	else if(XPM2(nazwa_plik))
		{
		int ile_kol = 0;
		bool jakie_kolory[16] = {};
		for (int i = 0; i < wysokosc; i++)
			for (int j = 0; j < szerokosc; j++)
				{
				if (obrazek[i][j] != -1)
					jakie_kolory[obrazek[i][j]] = true;
				}
		for (int i = 0; i < 16; i++)
			if (jakie_kolory[i])ile_kol++;
		fprintf(fp,"! XPM2\n");
		fprintf(fp, "%d %d %d 1\n", szerokosc, wysokosc, ile_kol);
		char litera = 'a';
		char kolory[16][8] = { "#000000","#000080","#008000","#008080","#800000","#800080","#808000","#C0C0C0","#808080","#0000FF","#00FF00","#00FFFF","#FF0000","#FF00FF","#FFFF00","#FFFFFF"};
		for (int i = 0; i < 16; i++)
			{
			if (jakie_kolory[i])
				{
				fprintf(fp, "%c c %s\n", litera, kolory[i]);
				litera++;
				}
			}

		for (int i = 0; i < wysokosc; i++)
			{
			for (int j = 0; j < szerokosc; j++)
				{
				if (obrazek[i][j] == -1)obrazek[i][j] = 15;
				int ktory_kolor = 0;
				for (int k = 0; k < obrazek[i][j]; k++)
					if (jakie_kolory[k])
						ktory_kolor++;
				char znak = (ktory_kolor+'a');
				fprintf(fp,"%c",znak);
				}
			fprintf(fp, "\n");
			}
		}
	else
		{
		fprintf(fp, "%d %d\n", wysokosc, szerokosc);
		for (int i = 0; i < wysokosc; i++)
			for (int j = 0; j < szerokosc; j++)
				fprintf(fp, "%d %d %d\n", i, j, obrazek[i][j]);
				
		}
	for (int i = 0; i < wysokosc; i++)
		free(obrazek[i]);
	free(obrazek);

	fclose(fp);
	}

void Ramka(int x, int y,int sx,int sy)
{
	x = x > MAX_SZER_PIC ? MAX_SZER_PIC : x;
	y = y > MAX_WYS_PIC ? MAX_WYS_PIC : y;
	textcolor(BLACK);
	textbackground(WHITE);
	gotoxy(DRAW_POSITION_X+sx, DRAW_POSITION_Y+sy);
	putch(201);											//'╔'
	REP(x)putch(205);									//'═'
	putch(187);											//'╗'
	REP(y + 1)
	{
		gotoxy(DRAW_POSITION_X+sx, DRAW_POSITION_Y+sy + 1 + i);
		putch(186);										//'║'
		gotoxy(DRAW_POSITION_X+sx + x + 1, DRAW_POSITION_Y+sy+ 1 + i);
		putch(186);										//'║'
	}
	gotoxy(DRAW_POSITION_X+sx, DRAW_POSITION_Y+sy + y + 1);
	putch(200);											//'╚'
	REP(x)putch(205);									//'═'
	putch(188);											//'╝'
}