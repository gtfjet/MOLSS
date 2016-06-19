/*!
	Maze of Lost Souls Solver (MOLSS)
 */

#include <stdio.h>
#include <windows.h>

#define DIMX 460
#define DIMY 540
#define DIMZ 4
#define MAXSTEPS 100000

HDC DC;
int TARGET[3] = {0};
int M[DIMX][DIMY][DIMZ] = {0};      
unsigned char R[DIMX][DIMY][DIMZ], G[DIMX][DIMY][DIMZ], B[DIMX][DIMY][DIMZ];
int D[3][MAXSTEPS]; 
int total = 0;
int totalSteps = 0;


int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

void makeMap() {
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

void drawMaze() {
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

void moveIn(int * S) {
	int x, y, z, h; 
	int i, j, dx, dy;
	COLORREF C = RGB(255,255,255);
	x = S[0];	y = S[1];	z = S[2];	h = S[3];
	
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
	
	S[0] = x; 	S[1] = y; 	S[2] = z; 	S[3] = h;
	return;
}

void drawSolution(int n) {
	int i, j, k;
	COLORREF C = RGB(0,255,0);
	
	/* Color pixels */
	for(k=0; k<n; k++) {
		for(i=-1; i<=1; i++) {
			for(j=-1; j<=1; j++) {
				SetPixelV(DC,D[0][k]+i+DIMX*D[2][k],D[1][k]+j,C); 
			}
		}
	}
	return;
}

int walkToDeath(int * S, int steps) {
	int x, y, z, z_last, h, h_last; 
	int i, j, k=0, n, b[8], count=0;
	int	p[]={0,1,2,3,-1,-2,-3}, q[]={7,6,5,0,8,4,1,2,3};
	int T[4], H[5];
	
	x = S[0];	y = S[1];	z = S[2];	h = S[3];
	while(1) {
		/* Check for limit cycle */
		H[k%5] = h;
		if(H[0]==1 && H[1]==3 && H[2]==5 && H[3]==7) {
			return 0; //dead
		}
		
		/* Move in h-direction */
		k++; totalSteps++;
		moveIn(S);
		x = S[0];	y = S[1];	z = S[2];	h = S[3];
		for(i=0; i<3; i++) {
			D[i][steps] = S[i];
		}
		steps++;
		//printf("%i, %i\n", totalSteps, steps);
		
		if(x==TARGET[0] && y==TARGET[1] && z==TARGET[2]) {
			printf("FOUND!\n");
			return steps;
		}
		
		/* Look for birth hole */
		for(i=-3; i<=3; i+=3) {
			for(j=-3; j<=3; j+=3) {
				if(M[x+i][y+j][z]==3) {    //found birth hole!	
					count++;
					if(count == 10) { 
						return 0; //dead 
					}
				}
			}
		}
		
		/* Look for a hole */
		n = 0;
		for(i=-3; i<=3; i+=3) {
			for(j=-3; j<=3; j+=3) {
				if(M[x+i][y+j][z]==2) {    //found a hole!	
					z_last = z;
					h_last = h;
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
					M[x+i][y+j][z] = 3;  	  //mark as birth hole
					M[x+i][y+j][z_last] = 3;  //mark as birth hole
					
					h = q[n];  
					T[0] = x; 	T[1] = y; 	T[2] = z; 	T[3] = h;
					total++;
					n = walkToDeath(T, steps);
					if (n>0) { return n; }
					total--;
					
					M[x+i][y+j][z] = 4;       //mark as decision hole
					M[x+i][y+j][z_last] = 4;  //mark as decision hole
					z = z_last;
					h = h_last;
					i = j = 4;
					break;
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
			h = mod(h+4,8);  //turn around
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

		S[0] = x; 	S[1] = y; 	S[2] = z; 	S[3] = h;
		//printf("%i\n",total);
		//Sleep(1);
	}
}

void main() {
	FILE *fp;
	int i, j, k, n;
	int x, y, z, h; //Current position and heading
	char fname[16];
	int S[4];
	system("mode 230,45");
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
	
	/* Create the map */
	makeMap();
	drawMaze();	
	
	/* Set target */
	TARGET[0] = 394;
	TARGET[1] = 508;
	TARGET[2] = 1;
	
	/* Initial heading and position */
	h = 0;	x = 289;  y = 214;	z = 0;
	// h = 2;	x = 301;  y = 226;	z = 0;
	// h = 4;	x = 289;  y = 235;	z = 0;
	// h = 6; 	x = 274;  y = 229;  z = 0; 
	
	/* Set heading and start walking */
	S[0] = x; 	S[1] = y; 	S[2] = z; 	S[3] = h;
	n = walkToDeath(S, 0);
	
	/* Print solution */
	drawSolution(n);
	
	/* End gracefully */
	ReleaseDC(GetConsoleWindow(),DC);
	getch();
}

