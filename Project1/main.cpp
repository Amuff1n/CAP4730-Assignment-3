/*
	Kolby Rottero
	CAP4730
	Assignment #3

	Cubic Bezier curve implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> //used for memset()
#include <GL/glut.h>
#include <cmath> //used for pow()
#include <iostream> //used currently for stepping through subdivision

using namespace std;

GLuint implementation = 0; //state bit for toggling implementations
static GLint mouse_state; //for mouse callback
static GLint mouse_button; //for mouse callback
static int xyIndex; //for saving mouse click positions
static int xPos[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //zero out your arrays, kids!
static int yPos[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //max of 16 saved points for surface patch



void drawControlPoint(int x, int y) {
	//draws red control point 
	glPointSize(5);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	glVertex2d(x, y);
	glEnd();
	glFlush();
}

//blending function implementation of curve drawing, takes in arrays of x and y coordinates
void drawCurveMatrix(int x[4], int y[4]) {
	//dumbly pick an abritrary t to increment by
	//use ugly equation instead of matrices because i'm dumb
	glPointSize(3);
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);

	double prevX = x[0];
	double prevY = y[0];
	for (double t = 0.0; t <= 1.0; t += 0.02) {
		double winX = (pow((1 - t),3)*x[0]) + (3 * t*pow((1 - t), 2)*x[1]) + (3 * pow(t, 2)*(1 - t)*x[2]) + (pow(t, 3)*x[3]);
		double winY = (pow((1 - t), 3)*y[0]) + (3 * t*pow((1 - t), 2)*y[1]) + (3 * pow(t, 2)*(1 - t)*y[2]) + (pow(t, 3)*y[3]);
		
		//have to draw line from starting point, update as we go along
		glVertex2d(prevX, prevY);
		glVertex2d(winX, winY);
		prevX = winX;
		prevY = winY;
	}

	glEnd();
	glFlush();
}

//linear interpolation function
//sourced from here http://www.cubic.org/docs/bezier.htm
void lerp(double * xy, double * a, double * b, double t) {
	xy[0] = a[0] + (b[0] - a[0]) * t;
	xy[1] = a[1] + (b[1] - a[1]) * t;
}

void drawCurveCasteljau(int x[4], int y[4]) {
	glPointSize(3);
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);

	double prevX = x[0];
	double prevY = y[0];
	double a[2] = { x[0], y[0] };
	double b[2] = { x[1], y[1] };
	double c[2] = { x[2], y[2] };
	double d[2] = { x[3], y[3] };

	//also sourced from http://www.cubic.org/docs/bezier.htm
	//deCasteljau's uses parameter t
	for (double t = 0.0; t < 1.0; t += 0.02) {
		double ab[2];
		double bc[2];
		double cd[2];
		double abbc[2];
		double bccd[2];
		double dest[2];

		lerp(ab, a, b, t); //Q0
		lerp(bc, b, c, t); //Q1
		lerp(cd, c, d, t); //Q2
		lerp(abbc, ab, bc, t); //R0
		lerp(bccd, bc, cd, t); //R1
		lerp(dest, abbc, bccd, t); //P(1/2)

		//draw line between previous point and new point
		glVertex2d(prevX, prevY);
		glVertex2d(dest[0], dest[1]);
		prevX = dest[0];
		prevY = dest[1];
	}

	glEnd();
	glFlush();
}

void drawCurveOpenGL(int x[4], int y[4]) {
	
	//set up control points from input, fake z coord because there isn't GL_MAP_VERTEX_2
	GLdouble controlPoints[4][3] = {
		{x[0], y[0], 0},
		{x[1], y[1], 0},
		{x[2], y[2], 0},
		{x[3], y[3], 0}
	};

	//cubic, parametric curve from 0 to 1
	glMap1d(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, *controlPoints);
	glEnable(GL_MAP1_VERTEX_3);


	glPointSize(3);
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINE_STRIP); //line strip will work better than point
	for (double t = 0.0; t < 1.0; t += 0.02) {
		glEvalCoord1d(t);
	}

	glEnd();
	glFlush();
}

void drawCurveSubdivision(int x[4], int y[4]) {
	glPointSize(3);
	glColor3f(0.5f, 0.0f, 0.5f);
	glBegin(GL_LINES);

	int length = sqrt(pow(x[0] - x[3], 2) + pow(y[0] - y[3], 2));
	//Line length of 10 is good middle ground between levels of recursion and noticable change on screen
	//There's probably a cheaper way to determine a stopping point for recursion
	if (length <= 10) {
		return;
	}

	//definitions for days
	double prevX = x[0];
	double prevY = y[0];
	double a[2] = { x[0], y[0] };
	double b[2] = { x[1], y[1] };
	double c[2] = { x[2], y[2] };
	double d[2] = { x[3], y[3] };

	double ab[2];
	double bc[2];
	double cd[2];
	double abbc[2];
	double bccd[2];
	double dest[2];

	double t = 0.5; //for subdivision, just use the midpoint
	lerp(ab, a, b, t); //Q0
	lerp(bc, b, c, t); //Q1
	lerp(cd, c, d, t); //Q2
	lerp(abbc, ab, bc, t); //R0
	lerp(bccd, bc, cd, t); //R1
	lerp(dest, abbc, bccd, t); //P(1/2)

	//draw line between first point and midpoint
	glVertex2d(a[0], a[1]);
	glVertex2d(dest[0], dest[1]);
	//draw line between midpoint and last point
	glVertex2d(dest[0], dest[1]);
	glVertex2d(d[0], d[1]);

	glEnd();
	glFlush();

	int tempXLeft[4] = { a[0], ab[0], abbc[0], dest[0] };
	int tempYLeft[4] = { a[1], ab[1], abbc[1], dest[1] };
	int tempXRight[4] = { dest[0], bccd[0], cd[0], d[0] };
	int tempYRight[4] = { dest[1], bccd[1], cd[1], d[1] };

	printf("Press Enter to continue"); //TODO find a better way to do this 
	cin.ignore();

	//recursively subdivide left then right side of curve
	drawCurveSubdivision(tempXLeft, tempYLeft);
	drawCurveSubdivision(tempXRight, tempYRight);
}

void drawPatchOpenGL(int x[16], int y[16]) {
	//setup control points, need 16
	GLdouble controlPoints[16][3] = {
		{ x[0], y[0], 0 },
		{ x[1], y[1], 0 },
		{ x[2], y[2], 0 },
		{ x[3], y[3], 0 },
		{ x[4], y[4], 0 },
		{ x[5], y[5], 0 },
		{ x[6], y[6], 0 },
		{ x[7], y[7], 0 },
		{ x[8], y[8], 0 },
		{ x[9], y[9], 0 },
		{ x[10], y[10], 0 },
		{ x[11], y[11], 0 },
		{ x[12], y[12], 0 },
		{ x[13], y[13], 0 },
		{ x[14], y[14], 0 },
		{ x[15], y[15], 0 },
	};

	//Help in setup from here: https://users.cs.jmu.edu/bernstdh/web/common/lectures/slides_opengl-bezier-surfaces.php
	//Couldn't figure out shading though

	glMap2d(GL_MAP2_VERTEX_3, 0.0, 1.0, 3, 4, 0.0, 1.0, 3*4, 4, *controlPoints);
	glEnable(GL_MAP2_VERTEX_3);

	glMapGrid2d(50.0, 0.0, 1.0, 50.0, 0.0, 1.0); //50 partitions, overkill?
	//Interestingly, step size of 1 gives you a straight up plane

	glColor3f(0.0f, 0.75f, 0.75f);
	glEvalMesh2(GL_FILL, 0.0, 50.0, 0.0, 50.0);

	glEnd();
	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'h':
		printf("Help!\n\n");
		printf("Left click in window to input control points.\n");
		printf("h - Print this help menu.\n");
		printf("r - Reset control points.\n");
		printf("f - Change implementation. Order is matrix form (0), de Casteljau (1), OpenGL (2), Subdivision (3), Bezier surface patch (4).\n");
		printf("t - Evaluate test polygon. Useful for seeing differences between implementations.\n");
		break;
	case 'r':
		//reset control points
		memset(xPos, 0, sizeof(xPos));
		memset(yPos, 0, sizeof(yPos));
		xyIndex = 0;
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		break;
	
	case 'f':
		//change implementation state, resets control points
		memset(xPos, 0, sizeof(xPos));
		memset(yPos, 0, sizeof(yPos));
		xyIndex = 0;
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		implementation++;
		if (implementation > 4) {
			implementation = 0;
		}
		printf("impelemntation = %d\n", implementation);
		break;
	
	case 't':
		printf("Drawing test control polygon\n");
		memset(xPos, 0, sizeof(xPos));
		memset(yPos, 0, sizeof(yPos));
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		xPos[0] = 57;
		xPos[1] = 112;
		xPos[2] = 276;
		xPos[3] = 450;
		yPos[0] = 400;
		yPos[1] = 200;
		yPos[2] = 167;
		yPos[3] = 273;
		//draw control points for test polygon
		drawControlPoint(xPos[0], yPos[0]);
		drawControlPoint(xPos[1], yPos[1]);
		drawControlPoint(xPos[2], yPos[2]);
		drawControlPoint(xPos[3], yPos[3]);

		switch (implementation) {
		case 0: 
			drawCurveMatrix(xPos, yPos); //blending function/matrix form
			break;

		case 1:
			drawCurveCasteljau(xPos, yPos); //de Casteljau's algorithm
			break;
			
		case 2:
			drawCurveOpenGL(xPos, yPos); //Native OpenGL implementation
			break;

		case 3:
			drawCurveSubdivision(xPos, yPos); //draw curve using subdivision
			printf("Done with subdivision!\n");
			glEnd();
			glFlush();
			break;

		case 4:
			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();
			printf("No test polygon for patch, resetting...");
			break;
		}

		memset(xPos, 0, sizeof(xPos));
		memset(yPos, 0, sizeof(yPos));
		xyIndex = 0;
	}
}

void mouse(int button, int state, int x, int y) {
	//used to get x, y coordinates to input point 
	mouse_state = state;
	mouse_button = button;
	
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		xPos[xyIndex] = x;
		yPos[xyIndex] = 512 - y; //gotta flip points because of window weirdness
		drawControlPoint(xPos[xyIndex], yPos[xyIndex]);
		xyIndex++;
		if (xyIndex >= 4) {
			//time to draw a curve!
			//for all but the patch, want to reuse last control point as first of new polygon
			if (implementation == 0) {
				drawCurveMatrix(xPos, yPos); //blending function/matrix form
				xPos[0] = xPos[3];
				yPos[1] = yPos[3];
				xyIndex = 1;
			}
			else if (implementation == 1) {
				drawCurveCasteljau(xPos, yPos); //de Casteljau's algorithm
				xPos[0] = xPos[3];
				yPos[1] = yPos[3];
				xyIndex = 1;
			}
			else if (implementation == 2) {
				drawCurveOpenGL(xPos, yPos); //Native OpenGL implementation
				xPos[0] = xPos[3];
				yPos[1] = yPos[3];
				xyIndex = 1;
			}
			else if (implementation == 3) {
				drawCurveSubdivision(xPos, yPos); //draw curve using subdivision
				printf("Done with subdivision!\n");
				glEnd();
				glFlush();
				xPos[0] = xPos[3];
				yPos[1] = yPos[3];
				xyIndex = 1;
			}
			else {
				if (xyIndex >= 16) {
					drawPatchOpenGL(xPos, yPos); //draw patch using OpenGL
					//clear values
					memset(xPos, 0, sizeof(xPos));
					memset(yPos, 0, sizeof(yPos));
					xyIndex = 0;
				}
			}
		}
	}
}

void display(void) {

}

void init(void) {
	//Clear Window
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, 512, 512);
	glMatrixMode(GL_PROJECTION); // we are essentially working in 2d for now
	glOrtho(0.0, 512.0, 0.0, 512.0, 1.0, -1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char** argv) {
	glutInitWindowSize(512, 512);
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE); //single buffer simpler for this assignment
	glutCreateWindow("Assignment 3");

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	printf("Help!\n\n");
	printf("Left click in window to input control points.\n");
	printf("h - Print this help menu.\n");
	printf("r - Reset control points.\n");
	printf("f - Change implementation. Order is matrix form (0), de Casteljau (1), OpenGL (2), Subdivision (3), Bezier surface patch (4).\n");
	printf("t - Evaluate test polygon. Useful for seeing differences between implementations.\n");

	init();

	glutMainLoop(); //Let's do this
}