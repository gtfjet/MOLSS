/*!
	Maze of Lost Souls Solver (MOLSS)
 */

#include <stdio.h>
#include <windows.h>

#define DIMX 460
#define DIMY 540
#define DIMZ 4
#define MAXSTEPS 100000


/* Global variables:
	Device Context
	RGB data from images
	Compressed version of maze "maze"
	X Y Z trajectory for the solution
	Total number of steps taken by algorithm
*/
HDC DC;
unsigned char RedPixel[DIMX][DIMY][DIMZ], GreenPixel[DIMX][DIMY][DIMZ], BluePixel[DIMX][DIMY][DIMZ];
int Maze[DIMX][DIMY][DIMZ] = {0};   
int X[MAXSTEPS], Y[MAXSTEPS], Z[MAXSTEPS];  //SOLUTION VECTORS 
int TotalSteps = 0;

/* Function prototypes */
int  mod(int a, int b);
int  walkToDeath(int* x, int steps);
int  fixSolution(int n);
void timeStep(int* x);
void drawSolution(int n, int r, int g, int b);
void startNavigation(int n);
void drawMaze();
void parse();

void main() {
	FILE *fp;
	int numSteps;
	char fname[16];
	int x[4];
	DC = GetDC(GetConsoleWindow());
	system("mode 230,45");
	
	for(int k=0; k<DIMZ; k++) {
		/* Open ppm */
		sprintf(fname,"Mols-%i.ppm",k+3);
		fp = fopen(fname,"rb");
	
		/* Read header information */
		for(int i=0; i<15; i++) {fgetc(fp);}

		/* Read image data and close file */
		for(int j=0; j<DIMY; j++) {
			for(int i=0; i<DIMX; i++) {
				RedPixel[i][j][k]   = fgetc(fp);
				GreenPixel[i][j][k] = fgetc(fp);
				BluePixel[i][j][k]  = fgetc(fp);
			}
		}
		fclose(fp);
	}
	
	/* Parse RGB data and draw the maze */
	parse();	
	drawMaze();	

	/* Set target */
	Maze[373][502][3] = 8; // Entrance to Demona
	
	/* Initial heading and position */
	x[0] = 0;	  //heading
	x[1] = 289;   //x0
	x[2] = 214;   //y0
	x[3] = 0;     //z0
	x[0] = 2;	x[1] = 301;  x[2] = 226;  x[3] = 0;
	// x[0] = 4;	x[1] = 289;  x[2] = 235;  x[3] = 0;
	// x[0] = 6; 	x[1] = 274;  x[2] = 229;  x[3] = 0; 
	
	/* Set heading and start walking */
	numSteps = walkToDeath(x, 0);
	
	/* Print solution */
	drawSolution(numSteps, 0, 0, 255);
	
	/* Improve solution */
	numSteps = fixSolution(numSteps);
	
	/* Print improved solution */
	drawSolution(numSteps, 0, 255, 0);

	/* Start navigating user */
	getch();
	startNavigation(numSteps);
	
	/* End gracefully */
	ReleaseDC(GetConsoleWindow(),DC);
	getch();
}

/* Recursive function */
int walkToDeath(int* x, int steps) {
	int z_last, h_last, lives, mySteps, n, b[8], y[4], history[5];
	int	p[]={0,1,2,3,-1,-2,-3}; 	//priorities when number of options is > 2
	int q[]={7,6,5,0,8,4,1,2,3};	//x,y -> h transform
	
	mySteps=0;
	lives=10;
	while(1) {
		/* Check for a limit cycle */
		history[mySteps%5] = x[0];
		if(history[0]==1 && history[1]==3 && history[2]==5 && history[3]==7) {
			lives--;
			if(lives==0) { return(0); } //dead 
		}
		
		/* Take a step and save to solution */
		timeStep(x);
		if(steps<MAXSTEPS) {
			X[steps] = x[1];
			Y[steps] = x[2];
			Z[steps] = x[3];
		} else {
			printf("Out of space!\n");
			system("pause");
		}
		steps++; 
		mySteps++;
		
		/* Check for cheese */
		if(Maze[x[1]][x[2]][x[3]]==8) {
			return(steps);  //output number of steps required to find cheese
		}
		
		/* Look for birth hole */
		for(int i=-3; i<=3; i+=3) {
			for(int j=-3; j<=3; j+=3) {
				if(Maze[x[1]+i][x[2]+j][x[3]]==3) {    //found birth hole!	
					lives--;
					if(lives==0) { return(0); } //dead 
				}
			}
		}
		
		/* Look for a hole */
		n = 0;
		for(int i=-3; i<=3; i+=3) {
			for(int j=-3; j<=3; j+=3) {
				if(Maze[x[1]+i][x[2]+j][x[3]]==2) {    //found a hole!	
					z_last = x[3];
					h_last = x[0];
					/* Go up or down? */
					if(x[3]==0) {
						x[3]++; 
					} else if(x[3]==(DIMZ-1)) {
						x[3]--;
					} else if(Maze[x[1]+i][x[2]+j][x[3]-1]==2) {
						x[3]--;
					} else {
						x[3]++;
					}			
					Maze[x[1]+i][x[2]+j][x[3]] = 3;  	//mark as birth hole
					Maze[x[1]+i][x[2]+j][z_last] = 3;  	//mark as birth hole
					
					x[0] = q[n];  
					for(int i=0; i<4; i++) { y[i] = x[i]; } //copy x;
					n = walkToDeath(y, steps);
					if (n>0) { return(n); }				//someone found cheese!
					
					Maze[x[1]+i][x[2]+j][x[3]] = 4;  	//mark as decision hole
					Maze[x[1]+i][x[2]+j][z_last] = 4;  	//mark as decision hole
					x[3] = z_last;
					x[0] = h_last;
					i = j = 4;
					break;
				}
				n++;
			}
		}
		
		/* Wall following */
		for(int i=0; i<8; i++) {b[i] = 0;}	
		
		if(x[0]!=4 && Maze[x[1]][x[2]-3][x[3]]>=1 && Maze[x[1]+3][x[2]-3][x[3]]==0) {	//head N
			b[0] = 1;
		}
		if(x[0]!=5 && Maze[x[1]+3][x[2]-3][x[3]]>=1 && Maze[x[1]+3][x[2]][x[3]]==0) {	//head NE
			b[1] = 1;
		}
		if(x[0]!=6 && Maze[x[1]+3][x[2]][x[3]]>=1 && Maze[x[1]+3][x[2]+3][x[3]]==0) {	//head E
			b[2] = 1;
		}
		if(x[0]!=7 && Maze[x[1]+3][x[2]+3][x[3]]>=1 && Maze[x[1]][x[2]+3][x[3]]==0) {	//head SE
			b[3] = 1;
		}
		if(x[0]!=0 && Maze[x[1]][x[2]+3][x[3]]>=1 && Maze[x[1]-3][x[2]+3][x[3]]==0) {	//head S
			b[4] = 1;
		}
		if(x[0]!=1 && Maze[x[1]-3][x[2]+3][x[3]]>=1 && Maze[x[1]-3][x[2]][x[3]]==0) {	//head SW
			b[5] = 1;
		}
		if(x[0]!=2 && Maze[x[1]-3][x[2]][x[3]]>=1 && Maze[x[1]-3][x[2]-3][x[3]]==0) {  //head W
			b[6] = 1;
		}
		if(x[0]!=3 && Maze[x[1]-3][x[2]-3][x[3]]>=1 && Maze[x[1]][x[2]-3][x[3]]==0) {	//head NW
			b[7] = 1;
		}
		n = 0;
		for(int i=0; i<8; i++) {n += b[i];}
		if(n==0) {
			x[0] = mod(x[0]+4,8);  //turn around
		} else if(n==1) {
			for(int i=0; i<8; i++) {
				if(b[i]==1) {
					x[0] = i;
					break;
				}
			}
		} else {
			for(int i=0; i<7; i++) {
				if(b[mod(x[0]+p[i],8)]==1) {
					x[0] = mod(x[0]+p[i],8);
					break;
				}
			}
		}
		//Sleep(1);
	}
}

int fixSolution(int n) {
	int f, p=0, q=0;

	while(1) {
		for(int l=n-1; l>(p+3); l--){ 		//last position
			for(int i=-3; i<=3; i+=3) { 	//search in walkable x
				for(int j=-3; j<=3; j+=3) { //search in walkable y
					if((X[p]+i)==X[l] && (Y[p]+j)==Y[l] && Z[p]==Z[l]) {    //found a move!
						/* Shift data */
						f = 0;
						for(int k=l; k<n; k++) {
							f++;
							X[p+f] = X[k];
							Y[p+f] = Y[k];
							Z[p+f] = Z[k];
						}
						n = p+f+1;
						i = j = 4;
						l = -1;
						break; //break all for loops
					}
				}
			} 
		}
		p++;
		if(p>=(n-3)) {break; }
	}
	
	return(n);
}

int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

void parse() {
	for(int i=0; i<DIMX; i++) {	
		for(int j=0; j<DIMY; j++) {
			for(int k=0; k<DIMZ; k++) { //wall=0
				if (RedPixel[i][j][k]==153 && GreenPixel[i][j][k]==102 && BluePixel[i][j][k]==51) {
					Maze[i][j][k] = 1;  //walkable=1
				} else if (RedPixel[i][j][k]==255 && GreenPixel[i][j][k]==255 && BluePixel[i][j][k]==0) {
					Maze[i][j][k] = 2;  //hole=2
				} else if (RedPixel[i][j][k]==153 && GreenPixel[i][j][k]==153 && BluePixel[i][j][k]==153) {
					Maze[i][j][k] = 2;  //hole=2
				}						//birth hole=3
			}							//decision hole=4
		}								//target=8
	}
}

void timeStep(int* x) {
	int dx, dy;
	COLORREF C = RGB(255,255,255);
	
	/* Update x and y */
	if(x[0]==0) {			//head N
		dx = 0;  dy = -3;
	} else if(x[0]==1) {	//head NE
		dx = 3;  dy = -3;
	} else if(x[0]==2) {	//head E
		dx = 3;  dy = 0;
	} else if(x[0]==3) {	//head SE	
		dx = 3;  dy = 3;
	} else if(x[0]==4) {	//head S
		dx = 0;  dy = 3;
	} else if(x[0]==5) {	//head SW
		dx = -3; dy = 3;
	} else if(x[0]==6) {  	//head W
		dx = -3; dy = 0;
	} else if(x[0]==7) {	//head NW
		dx = -3; dy = -3;
	} else {
		dx = 0;  dy = 0;
	}
	x[1] += dx; x[2] += dy;

	/* Color pixels */
	for(int i=-1; i<=1; i++) {
		for(int j=-1; j<=1; j++) {
			SetPixelV(DC,x[1]+i+DIMX*x[3],x[2]+j,C); 
		}
	}
	TotalSteps++;
}

void drawMaze() {
	COLORREF C;
	RECT rect;
	HDC hDCMem;
	HBITMAP bm;
	hDCMem = CreateCompatibleDC(DC);
	GetClientRect(GetConsoleWindow(), &rect); 
	bm = CreateCompatibleBitmap(DC, rect.right, rect.bottom);
	SelectObject(hDCMem, bm);
	
	for(int k=0; k<DIMZ; k++) {
		for(int j=0; j<DIMY; j++) {
			for(int i=0; i<DIMX; i++) {
				C = RGB(RedPixel[i][j][k], GreenPixel[i][j][k], BluePixel[i][j][k]);
				SetPixelV(hDCMem,i+DIMX*k,j,C); 
			}
		}
	}	

	BitBlt(DC, 0, 0, rect.right, rect.bottom, hDCMem, 0, 0, SRCCOPY);
	DeleteObject(bm);
	DeleteDC(hDCMem);
}

void drawSolution(int n, int r, int g, int b) {
	COLORREF C = RGB(r,g,b);
	for(int k=0; k<n; k++) {
		for(int i=-1; i<=1; i++) {
			for(int j=-1; j<=1; j++) {
				SetPixelV(DC,X[k]+i+DIMX*Z[k],Y[k]+j,C); 
			}
		}
	}
}

void startNavigation(int n) {
	int r = 50;
	COLORREF C = RGB(0,0,0);
	RECT rect;
	HDC hDCMem;
	HBITMAP bm;
	hDCMem = CreateCompatibleDC(DC);
	GetClientRect(GetConsoleWindow(), &rect); 
	bm = CreateCompatibleBitmap(DC, rect.right, rect.bottom);
	SelectObject(hDCMem, bm);
	
	/* Clear maze */
	BitBlt(DC, 0, 0, rect.right, rect.bottom, hDCMem, 0, 0, SRCCOPY);
	
	/* Resize bitmap */
	bm = CreateCompatibleBitmap(DC, 2*r, 2*r);
	SelectObject(hDCMem, bm);	
	
	/* Print minimap */
	for(int k=0; k<n; k++) {
		for(int i=-r; i<=r; i++) {
			for(int j=-r; j<=r; j++) {
				if((X[k]+i)<DIMX && (Y[k]+j)<DIMY && (X[k]+i)>=0  && (Y[k]+j)>=0) {
					if(i<-1 || i>1 || j<-1 || j>1) {
						C = RGB(RedPixel[X[k]+i][Y[k]+j][Z[k]], GreenPixel[X[k]+i][Y[k]+j][Z[k]], BluePixel[X[k]+i][Y[k]+j][Z[k]]);
						SetPixelV(hDCMem,r+i,r+j,C); 
					} else {
						SetPixelV(hDCMem,r+i,r+j,RGB(255,255,255)); 
					}
				}
			}
		}
		Sleep(10);
		StretchBlt(DC, 0, 0, 12*r, 12*r, hDCMem, 0, 0, 2*r, 2*r, SRCCOPY);
	}
	DeleteObject(bm);
	DeleteDC(hDCMem);
}

