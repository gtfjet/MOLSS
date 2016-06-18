/*!
	Maze of Lost Souls Solver (MOLSS)
 */

#include <stdio.h>
#include <windows.h>

#define DIMX 460
#define DIMY 540
#define DIMZ 4
#define NHOLES 10000

HDC DC;
int x, y, z, M[DIMX][DIMY][DIMZ] = {0};      
unsigned char R[DIMX][DIMY][DIMZ], G[DIMX][DIMY][DIMZ], B[DIMX][DIMY][DIMZ];

int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

void makeM() {
	int i, j, k;
	for(i=0; i<DIMX; i++) {	
		for(j=0; j<DIMY; j++) {
			for(k=0; k<DIMZ; k++) {
				if (R[i][j][k]==153 && G[i][j][k]==102 && B[i][j][k]==51) {
					M[i][j][k] = 1;  //walkable=1
				} else if (R[i][j][k]==255 && G[i][j][k]==255 && B[i][j][k]==0) {
					M[i][j][k] = 2;  //hole=2
				} else if (R[i][j][k]==153 && G[i][j][k]==153 && B[i][j][k]==153) {
					M[i][j][k] = 2;  //hole=2
				}
			}
		}
	}
}

void draw() {
	int i, j, k;
	COLORREF C;
	for(k=0; k<DIMZ; k++) {
		for(j=0; j<DIMY; j++) {
			for(i=0; i<DIMX; i++) {
				C = RGB(R[i][j][k], G[i][j][k], B[i][j][k]);
				SetPixelV(DC,i+DIMX*k,j,C); 
			}
		}
	}
	
}

void movein(int h) {
	int i, j, dx, dy;
	COLORREF C = RGB(255,255,255);
	
	/* Update x and y */
	if(h==0) {			//head N
		dx = 0;  dy = -3;
	} else if(h==1) {	//head NE
		dx = 3;  dy = -3;
	} else if(h==2) {	//head E
		dx = 3;  dy = 0;
	} else if(h==3) {	//head SE	
		dx = 3;  dy = 3;
	} else if(h==4) {	//head S
		dx = 0;  dy = 3;
	} else if(h==5) {	//head SW
		dx = -3; dy = 3;
	} else if(h==6) {  	//head W
		dx = -3; dy = 0;
	} else if(h==7) {	//head NW
		dx = -3; dy = -3;
	} else {
		dx = 0;  dy = 0;
	}
	x += dx; y += dy;

	/* Color pixels */
	for(i=-1; i<=1; i++) {
		for(j=-1; j<=1; j++) {
			SetPixelV(DC,x+i+DIMX*z,y+j,C); 
		}
	}
}

void walk() {
	int i, j, k=0, n, h, b[8];
	int	p[]={0,1,2,3,-1,-2,-3}, q[]={7,6,5,0,8,4,1,2,3};
	
	/* Initial heading and position */
	h = 0;	x = 289;  y = 214;	z = 0;
	//h = 2;	x = 301;  y = 226;	z = 0;
	//h = 4;	x = 289;  y = 235;	z = 0;
	//h = 6;    x = 274;  y = 229;  z = 0;
	
	while(1) {
		k++;
		movein(h);
		
		/* Look for a hole */
		n = 0;
		for(i=-3; i<=3; i+=3) {
			for(j=-3; j<=3; j+=3) {
				if(M[x+i][y+j][z]==2) {    //found a hole!
					if(rand()>26383) {					
						/* Go up or down? */
						if(z==0) {
							z++; 
						} else if(z==(DIMZ-1)) {
							z--;
						} else if(M[x+i][y+j][z-1]==2) {
							z--;
						} else {
							z++;
						}			
						h = q[n];
						movein(h);
						i = j = 4;
						break;
					}
				}
				n++;
			}
		}
		
		/* Wall following */
		for(i=0; i<8; i++) {b[i] = 0;}	
		
		if(h!=4 && M[x][y-3][z]>=1 && M[x+3][y-3][z]==0) {	//head N
			b[0] = 1;
		}
		if(h!=5 && M[x+3][y-3][z]>=1 && M[x+3][y][z]==0) {	//head NE
			b[1] = 1;
		}
		if(h!=6 && M[x+3][y][z]>=1 && M[x+3][y+3][z]==0) {	//head E
			b[2] = 1;
		}
		if(h!=7 && M[x+3][y+3][z]>=1 && M[x][y+3][z]==0) {	//head SE
			b[3] = 1;
		}
		if(h!=0 && M[x][y+3][z]>=1 && M[x-3][y+3][z]==0) {	//head S
			b[4] = 1;
		}
		if(h!=1 && M[x-3][y+3][z]>=1 && M[x-3][y][z]==0) {	//head SW
			b[5] = 1;
		}
		if(h!=2 && M[x-3][y][z]>=1 && M[x-3][y-3][z]==0) {  //head W
			b[6] = 1;
		}
		if(h!=3 && M[x-3][y-3][z]>=1 && M[x][y-3][z]==0) {	//head NW
			b[7] = 1;
		}
		
		n = 0;
		for(i=0; i<8; i++) {n += b[i];}
		
		if(n==0) {
			h = mod(h+4,8);
		} else if(n==1) {
			for(i=0; i<8; i++) {
				if(b[i]==1) {
					h = i;
					break;
				}
			}
		} else {
			for(i=0; i<7; i++) {
				if(b[mod(h+p[i],8)]==1) {
					h = mod(h+p[i],8);
					break;
				}
			}
		}
		
		//if(mod(k,10000)==0){draw();}
		Sleep(1);
	}
}

void main() {
	FILE *fp;
	int i, j, k;
	char fname[16];
	system("mode 300,34");
	DC = GetDC(GetConsoleWindow());

	for(k=0; k<DIMZ; k++) {
		/* Open ppm */
		sprintf(fname,"Mols-%i.ppm",k+3);
		fp = fopen(fname,"rb");
	
		/* Read header information */
		for(i=0; i<15; i++) {fgetc(fp);}

		/* Read image data and close file */
		for(j=0; j<DIMY; j++) {
			for(i=0; i<DIMX; i++) {
				R[i][j][k] = fgetc(fp);
				G[i][j][k] = fgetc(fp);
				B[i][j][k] = fgetc(fp);
			}
		}
		fclose(fp);
	}
	
	/* Create the map and start navigation*/
	makeM();	draw();		walk();
	
	/* End gracefully */
	ReleaseDC(GetConsoleWindow(),DC);
	getch();
}

