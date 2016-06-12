/*!
	Maze of Lost Souls Solver (MOLSS)
 */

#include <stdio.h>
#include <windows.h>

#define DIMX 460
#define DIMY 540

HDC DC;
int x, y;
int M[DIMX][DIMY] = {0};      
unsigned char R[DIMX][DIMY], G[DIMX][DIMY], B[DIMX][DIMY];

void movein(int h) {
	int i, j, dx, dy;
	COLORREF C = RGB(255,255,255);
	
	if(h==0) {			//head N
		dx = 0; dy = -3;
	} else if(h==1) {	//head NE
		dx = 3; dy = -3;
	} else if(h==2) {	//head E
		dx = 3; dy = 0;
	} else if(h==3) {	//head SE	
		dx = 3; dy = 3;
	} else if(h==4) {	//head S
		dx = 0; dy = 3;
	} else if(h==5) {	//head SW
		dx = -3; dy = 3;
	} else if(h==6) {  	//head W
		dx = -3; dy = 0;
	} else if(h==7) {	//head NW
		dx = -3; dy = -3;
	}
	
	x+=dx; y+=dy;
		
	for(i=-1; i<=1; i++) {
		for(j=-1; j<=1; j++) {
			SetPixelV(DC,x+i,y+j,C); 
		}
	}
}

void makeM() {
	int i, j;
	for(i=0; i<DIMX; i++) {	
		for(j=0; j<DIMY; j++) {
			if (R[i][j]==153 && G[i][j]==102 && B[i][j]==51) {
				M[i][j] = 1;  //walkable=1
			}
		}
	}
}

void walk() {
	int i, n, h, itWorked;
	int b[8];
	
	/* Initial position and heading */
	h = 4;	x = 175;  y = 190;
	
	while(1) {
		movein(h);
		for(i=0; i<8; i++) {b[i]=0;}
		
		if(M[x][y-3]==1 && h!=4 && M[x+3][y-3] == 0) {	//head N
			b[0]=1;
		}
		if(M[x+3][y-3]==1 && h!=5 && M[x+3][y] == 0) {	//head NE
			b[1]=1;
		}
		if(M[x+3][y]==1 && h!=6 && M[x+3][y+3] == 0) {	//head E
			b[2]=1;
		}
		if(M[x+3][y+3]==1 && h!=7 && M[x][y+3] == 0) {	//head SE	
			b[3]=1;
		}
		if(M[x][y+3]==1 && h!=0 && M[x-3][y+3] == 0) {	//head S
			b[4]=1;
		}
		if(M[x-3][y+3]==1 && h!=1 && M[x-3][y] == 0) {	//head SW
			b[5]=1;
		}
		if(M[x-3][y]==1 && h!=2 && M[x-3][y-3] == 0 ) {  //head W
			b[6]=1;
		}
		if(M[x-3][y-3]==1 && h!=3 && M[x][y-3] == 0) {	//head NW
			b[7]=1;
		}
		
		n=0;
		for(i=0; i<8; i++) {n += b[i];}
		
		if (n == 0) {
			h=(h+4)%8;
		}
		else if (n == 1) {
			for(i=0; i<8; i++) {
				if(b[i] == 1) {
					h=i;
				}
			}
		}
		else if (n == 2) {
			itWorked = 0;
			for(i=0; i<4; i++) {
				if(b[(h+i)%8] == 1) {
					h = (h+i)%8;
					itWorked=1;
					break;
				}
			}
			
			if(itWorked==0) {
				for(i=0; i<4; i++) {
					if(b[(h-i)%8 < 0 ? 8-(h-i)%8 : (h-i)%8] == 1) {
						h = (h-i)%8;
						break;
					}
				}
			}
		}
		//printf("%i,%i\n",n,h);
		Sleep(1);
	}
}

void main() {
	int i, j;
	COLORREF C;
	FILE *fp = fopen("Mols-3.ppm","rb");
	DC = GetDC(GetConsoleWindow());
	system("mode 58,34");

	/* Read header information */
	for(i=0; i<15; i++) {fgetc(fp);}

	/* Read image data and print to console */
	for(j=0; j<DIMY; j++) {
		for(i=0; i<DIMX; i++) {
			R[i][j] = fgetc(fp);
			G[i][j] = fgetc(fp);
			B[i][j] = fgetc(fp);
			C = RGB(R[i][j], G[i][j], B[i][j]);
			SetPixelV(DC,i,j,C); 
		}
	}
	fclose(fp);
	
	/* Read walkable spaces */
	makeM();
	
	/* Start navigation */
	walk();
	
	/* End gracefully */
	ReleaseDC(GetConsoleWindow(),DC);
	getch();
}

