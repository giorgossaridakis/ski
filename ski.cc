/* ASCII SKI a simple game of SKI */
#include <unistd.h>
#include <iostream>
#include <conio.h>
#include <termios.h>

#define INITIAL_LIVES 5
#define OBSTACLES_RARITY 15
#define UP 'q'
#define LEFT 'n'
#define RIGHT 'm'
#define DOWN 'a'
#define PAUSE 'p'
#define ESC 27
#define ASTERISK 42

void design_line();
void scroll_screen();
void erase_screen();
void find_skier_edge(int y);
void show_message(const char *s);
void lose_life();
int check_rarity(int rarity);
void Sleep(int sleepMs);
void cleanstdin();

struct points {
 int x;
int y; } ;
struct skier_prototype {
 char character;
 int appearance;
struct points pt; } ;
struct skier_prototype skier;

int line_in_draw;
int lane_size;
int lane_position;
int lives=INITIAL_LIVES;
int total_score=0, score=0;
char screen[35][25];
char screen_copy[35][25];
int skier_edge_left, skier_edge_right;
int game_flag=0;
int bufferkeyboard=1;

WINDOW *win1=newwin(80, 25, 0, 0);

int main()
{ 
  int i;
  char c=0;
  int kbrd_flag;
  skier.character='X';
  win1=initscr();
  curs_set(0);
  start_color();			
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_BLACK, COLOR_BLACK);
  attron(COLOR_PAIR(1));
  srand(time(NULL));
	
   while (c!=ESC) {    
	if (!game_flag) {  	
	 erase_screen();
     line_in_draw=1;
	 c=0;
     total_score+=score;
	 score=0;
	 lane_size=0;
	 while (lane_size<5)
	  lane_size=rand() % 20;
     lane_position=rand() % 10+2;
	 skier.appearance=0;
	game_flag=1; }
	 /* keyboard controls */
    if (kbhit()) {
	 c=getch();
    kbrd_flag=1; }
    // transform arrow key input
    if (c==254 && kbrd_flag)
     c=0;
    if (kbrd_flag && c==0) {
     switch (getch()) {
      case 53:
       c=UP;
      break;
      case 52:
       c=DOWN;
      break;
      case 55:
       c=RIGHT;
      break;
      case 54:
       c=LEFT;
    break; } }
    if (bufferkeyboard)
     cleanstdin();
    kbrd_flag=0;
    // asterisk is same button as 'r'
    if (c==ASTERISK)
     c='r';
    switch (c) {
     case 'b':
      bufferkeyboard=bufferkeyboard ? 0 : 1;
     break;
	 case LEFT:
      skier.pt.x--;
	 break;
	 case RIGHT:
      skier.pt.x++;
	 break;
	 case UP:
      if (!screen[skier.pt.x][skier.pt.y-1])
	   skier.pt.y--;
	  c=0;
	 break;
	 case DOWN:
      if (!screen[skier.pt.x][skier.pt.y+1])
	   skier.pt.y++;
	  score+=25;
	  c=0;
	 break;
	 case PAUSE:
	  gotoxy(skier.pt.x, skier.pt.y);
      attron(COLOR_PAIR(2));
	  addch(skier.character);
      refresh();
	  c=getch();
	  c=0;
      attron(COLOR_PAIR(1));
	 break;
	 case 'r' :
	  score+=100;
	  c=0;
	 break; }	
	/* is skier.pt.y way too up or down ? */
    if (skier.pt.y>22)
	 skier.pt.y=22;
    if (skier.pt.y<5)
	 skier.pt.y=5;
	/* check for collisions */	
	if (skier.appearance) {
	 find_skier_edge(skier.pt.y);	
	 for (i=skier_edge_left;i<skier_edge_right;i++) {
	  if (((screen[i][skier.pt.y+1]) && skier.pt.x==i))
	 lose_life(); }
	  if ((skier.pt.x<skier_edge_left) || (skier.pt.x>skier_edge_right)) 
	 lose_life();     } 
	/* increace score, limit it and grant life for every 100 points */
    if (skier.appearance)
	 ++score;	
	if (score>999) 
	 score=999;
    if (lives<1)
	 c=ESC; 
	for (i=100;i<1000;i+=100) 
	 if (score==i)
	  ++lives;
	design_line(); 	}	
	/* prepare for exit */
	show_message("Goodbye ASCII Skier!");
    delwin(win1);
    endwin();
    curs_set(1);
    refresh();
    
  return 0;
}

/* design next line */
void design_line()
{
  int i,n;
  int r;
     /* determine increace or decreace of lane size */
	 if (!(check_rarity(score/100)))  
	  --lane_size;
     else 
 	  lane_size+=rand() % 3 - 1; 
 	 lane_position+=rand() % 3 - 1; 
     /* adjust lane position and size if it goes out of bounds */
     while (lane_position<2)
      ++lane_position;
     while (lane_position>14)
      --lane_position;
     while (lane_size<5)
	  lane_size++;
     while (lane_size>20) 
      --lane_size;
     /* place obstacles */
	 i=rand() % lane_size; // how many 
 	 for (n=0;n<i;n++) {
	  while (r<33 || r>128)
	   r=rand() % 127; /* obstacle character */
	   if (check_rarity(OBSTACLES_RARITY-(score/100))) 
   	    screen[lane_position+rand() % lane_size][line_in_draw]=(char) r; 
       r=0; } 
     /* place borders */
     screen[lane_position][line_in_draw]=ASTERISK;
	 screen[lane_position+lane_size][line_in_draw]=ASTERISK;	   
     // check to see if next line finds skier out of edges 
     if (skier.appearance) {
      find_skier_edge(skier.pt.y+1);
      if ((skier.pt.x<skier_edge_left) || (skier.pt.x>skier_edge_right)) 
     lose_life(); }
     
     ++line_in_draw;	
	/* is the screen full, lets scroll it */ 
	 if (line_in_draw>24) {
      line_in_draw=24;
     scroll_screen(); }
}

/* copy, rearrange, clear and show new screen and Skier */
void scroll_screen()
{
  int i,n;
    /* clear screen copy */
   	 for (i=1;i<25;i++) 
	  for (n=1;n<35;n++) 
	   screen_copy[n][i]=0; 
    /* make a copy */
    for (i=1;i<25;i++) 
	 for (n=1;n<35;n++) 
	  screen_copy[n][i]=screen[n][i];
    /* clear array and actual screen */
    erase_screen();
    clear();	
    refresh();
	/* show current game information */
	gotoxy(35,1);
    printw("ASCII SKI");
    gotoxy(35,2);
    printw("~~~~~~~~~");
    gotoxy(35,5);
    printw("keys");
    gotoxy(35,6);
    printw("q up");
    gotoxy(35,7);
    printw("a down");
    gotoxy(35,8);
    printw("n left");
    gotoxy(35,9);
    printw("m right");
    gotoxy(35,11);
    printw("p pause");
    gotoxy(35,12);
    printw("r run"); 
	gotoxy(1,24);
	printw("total score:%d|hill score:%d|lives:%d|lane:%d|x:%d|y:%d                ", total_score, score, lives, lane_size, skier.pt.x, skier.pt.y);	
	/* move contents one line up in screen array */
	for (i=2;i<25;i++) 
	 for (n=1;n<35;n++) 
	  screen[n][i-1]=screen_copy[n][i];   
   /* show the new screen */
	for (i=1;i<25;i++) {
	 for (n=1;n<35;n++) {
	  gotoxy(n,i);
	  if (screen[n][i])
	addch(screen[n][i]); } }
    /* turn on appearance for Skier, this is scroll routine, so a full screen has been drawn */
	if (!(skier.appearance)) {
	 skier.appearance=1;
	 find_skier_edge(7);
     skier.pt.x=skier_edge_left+(skier_edge_right-skier_edge_left)/2;
	 skier.pt.y=7;
	screen[skier.pt.x][skier.pt.y]=0; }
	 /* show skier */
    if (skier.appearance) {
	 gotoxy(skier.pt.x, skier.pt.y);
     attron(COLOR_PAIR(2));
	addch(skier.character); }
    attron(COLOR_PAIR(1));
	// refresh screen
    refresh();
	/* wait for next screen */ 
    Sleep(1000-score);
}

/* clear screen array */
void erase_screen()
{
  int i,n;	
  
	for (i=0;i<25;i++) 
	 for (n=0;n<35;n++) 
	  screen[n][i]=0;
}

/* assign values for skier edges */
void find_skier_edge(int y)
{
  	 skier_edge_left=0;
	 while (!(screen[skier_edge_left][y]))
      skier_edge_left++;
     skier_edge_right=34;
     while (!(screen[skier_edge_right][y]))
      skier_edge_right--;
}

/* show message */
void show_message(const char *s)
{
	gotoxy(7,10);
	printw("%s", s);
    refresh();
	Sleep(1500);
    attron(COLOR_PAIR(3));
    gotoxy(7,10);
	printw("%s", s);
    attron(COLOR_PAIR(1));
}

/* lose life */
void lose_life()
{	
  if (lives>1)
   show_message("Get ready Skier!");
  --lives;
  game_flag=0; 
}

/* return 1 if object is lucky enough to appear */
int check_rarity(int rarity)
{
  int i,n,l;
  int success[3];
  
   if (!rarity)
	return 1;
   else
	i=rand()% rarity;
    n=rarity/3;   
    for (l=1;l<4;l++) 
     success[l-1]=n*l;
	for (l=0;l<3;l++) {
	 if (i==success[l])
	  return 1; }
  return 0;
}

// Sleep for miliseconds
void Sleep(int sleepMs)
{
  usleep(sleepMs*1000);
}

/* remove garbage from stdin */
void cleanstdin()
{
 int stdin_copy = dup(STDIN_FILENO);

  tcdrain(stdin_copy);
  tcflush(stdin_copy, TCIFLUSH);
 close(stdin_copy);
}
