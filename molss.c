/*!
	Maze of Lost Souls Solver (MOLSS)
*/

#include <stdio.h>
#include <windows.h>

#define DIMX 460
#define DIMY 540

HDC DC;
unsigned char R[DIMX][DIMY], G[DIMX][DIMY], B[DIMX][DIMY];

int moveto(int x, int y) {
	int i, j, k=0;
	COLORREF C = RGB(255,255,255);
	for(j=-1; j<2; j++) {
		for(i=-1; i<2; i++) {
			if (R[x+i][y+j]==153 && G[x+i][y+j]==102 && B[x+i][y+j]==51) {
				k++;
			}
		}
	}
	if (k==9) {
		for(j=-1; j<2; j++) {
			for(i=-1; i<2; i++) {
				SetPixelV(DC,x+i,y+j,C); 
			}
		}
		return 1;
	}
	return 0;
}
 
void main() {
	int i, j, k;
	COLORREF C;
	FILE *fp = fopen("Mols-3.ppm","rb");
	DC = GetDC(GetConsoleWindow());
	system("mode 58,34");

	/* Read header information */
	for(i=0; i<15; i++) {fgetc(fp);}

	/* Read image */
	for(j=0; j<DIMY; j++) {
		for(i=0; i<DIMX; i++) {
			R[i][j] = fgetc(fp);
			G[i][j] = fgetc(fp);
			B[i][j] = fgetc(fp);
		}
	}
	fclose(fp);
	
	/* Print image */
	for(j=0; j<DIMY; j++) {
		for(i=0; i<DIMX; i++) {
			C = RGB(R[i][j], G[i][j], B[i][j]);
			SetPixelV(DC,i,j,C); 
		}
	}
	
	for(i=0; i<300; i+=2) {moveto(176+i,194);}
	
	/* End gracefully */
	ReleaseDC(GetConsoleWindow(),DC);
	getch();
}

