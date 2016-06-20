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
	Direct Context or something
	RGB data from images
	Compressed version of maze "maze"
	X Y Z trajectory for the solution
*/
HDC DC;
unsigned char RedPixel[DIMX][DIMY][DIMZ], GreenPixel[DIMX][DIMY][DIMZ], BluePixel[DIMX][DIMY][DIMZ];
int Maze[DIMX][DIMY][DIMZ] = {0};   
int Solution[3][MAXSTEPS]; 
int TotalSteps = 0;

/* Function prototypes */
int  mod(int a, int b);
void parse();
void drawMaze();
void drawSolution(int n, int r, int g, int b);
int  fixSolution(int n);
int  walkToDeath(int* x, int steps);
void timeStep(int* x);



void main() {
	FILE *fp;
	int numSteps;
	char fname[16];
	int x[4];
	system("mode 230,45");
	DC = GetDC(GetConsoleWindow());
	
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
	
	/* Create the map */
	parse();	drawMaze();	
	
	/* Set target */
	Maze[373][502][3] = 8;
	
	/* Initial heading and position */
	x[0] = 0;	  //heading
	x[1] = 289;   //x0
	x[2] = 214;   //y0
	x[3] = 0;     //z0
	// x[0] = 2;	x[1] = 301;  x[2] = 226;  x[3] = 0;
	// x[0] = 4;	x[1] = 289;  x[2] = 235;  x[3] = 0;
	// x[0] = 6; 	x[1] = 274;  x[2] = 229;  x[3] = 0; 
	
	/* Set heading and start walking */
	numSteps = walkToDeath(x, 0);
	
	/* Print solution */
	drawSolution(numSteps, 0, 0, 255);
	Sleep(1000);
	
	/* Improve solution */
	numSteps = fixSolution(numSteps);
	
	/* Print solution */
	drawSolution(numSteps, 0, 255, 0);
	
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
		
		/* Take a step and save to Solution */
		timeStep(x);
		steps++;  mySteps++;
		for(int i=0; i<3; i++) { Solution[i][steps] = x[i+1]; }

		/* Check for Cheese */
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
	for(int k=0; k<DIMZ; k++) {
		for(int j=0; j<DIMY; j++) {
			for(int i=0; i<DIMX; i++) {
				C = RGB(RedPixel[i][j][k], GreenPixel[i][j][k], BluePixel[i][j][k]);
				SetPixelV(DC,i+DIMX*k,j,C); 
			}
		}
	}
	
}

void drawSolution(int n, int r, int g, int b) {
	COLORREF C = RGB(r,g,b);
	for(int k=0; k<n; k++) {
		for(int i=-1; i<=1; i++) {
			for(int j=-1; j<=1; j++) {
				SetPixelV(DC,Solution[0][k]+i+DIMX*Solution[2][k],Solution[1][k]+j,C); 
			}
		}
	}
}


