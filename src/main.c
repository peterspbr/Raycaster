#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glut.h>

typedef struct
{
    int w, a, s, d;
} handleKeyboardKeys; handleKeyboardKeys Keys;

const int windowWidth = 640, windowHeight = 480;
int mapW = 8, mapH = 8, mapS = 64;

float px, py, pdx, pdy, pa, walkVelocity;
float degToRad ( float a ) { return a * (3.1415 / 180); }
float fixedAngle ( float a ) { if(a > 359) { a -= 360; } if (a < 0) { a += 360; } return a; }

int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};

void drawPlayer()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(4);
    glLineWidth(2);

    glBegin(GL_POINTS);
    glVertex2i(px, py);
    glEnd();

    glBegin(GL_LINES);
    glVertex2i(px, py);
    glVertex2i(px + pdx * 20, py + pdy * 20);
    glEnd();
}

void drawMap()
{
    int xo, yo; // X offset and Y offset

    for (int y = 0; y < mapH; y++)
    {
        for (int x = 0; x < mapW; x++)
        {
            if(map[y * mapW + x] > 0){ glColor3f(1,1,1);} else{ glColor3f(0,0,0);}

            xo = x * mapS;
            yo = y * mapS;

            glBegin(GL_QUADS); 
            glVertex2i( 0 + xo + 1, 0 + yo + 1); 
            glVertex2i( 0 + xo + 1, mapS + yo - 1); 
            glVertex2i( mapS + xo - 1, mapS + yo - 1);  
            glVertex2i( mapS + xo - 1, 0 +yo + 1);
            glEnd();
        }
    }
}

float distance(int ax, int ay, int bx, int by, int ang){ return cos(degToRad(ang))*(bx-ax)-sin(degToRad(ang))*(by-ay);}

void drawRays ()
{
    int r, mx, my, mp, dof, side;
    float vx, vy, rx, ry, ra, xo, yo, disV, horiDist;

    // Roof
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(640, 0);
    glVertex2i(640, 240);
    glVertex2i(0, 240);
    glEnd();

    // Floor
    glColor3f(0.2f, 0.2f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2i(0, 480);
    glVertex2i(0, 240);
    glVertex2i(640, 240);
    glVertex2i(640, 480);
    glEnd();	 	

    ra = fixedAngle(pa + 30);

    for(r = 0; r < 60; r++)
    {
        //---Vertical--- 
        dof = 0; 
        side = 0;
        disV = 100000;

        float Tan = tan(degToRad(ra));

        if(cos(degToRad(ra)) > 0.001)
        {
            rx = (((int)px>>6)<<6) + 64;
            ry=(px-rx)*Tan+py;
            xo = 64;
            yo =- xo * Tan;
        } else if(cos(degToRad(ra))<-0.001){
            rx = (((int)px>>6)<<6) -0.0001;
            ry = (px-rx) * Tan + py;
            xo =- 64;
            yo =- xo * Tan;
        } else {
            rx = px;
            ry = py;
            dof=8;
        }

        while(dof<8) 
        { 
            mx=(int)(rx)>>6;
            my=(int)(ry)>>6;
            mp = my * mapW + mx;

            if(mp>0 && mp<mapW*mapH && map[mp]==1){
                dof = 8;
                disV=cos(degToRad(ra)) * (rx-px) - sin(degToRad(ra)) * (ry-py);
            } else{
                rx += xo;
                ry += yo;
                dof += 1;
            }
        } 

        vx = rx;
        vy = ry;

        //---Horizontal---
        dof = 0;
        horiDist = 100000;
        Tan = 1.0/Tan;

        if(sin(degToRad(ra))> 0.001){
            ry=(((int)py>>6)<<6) -0.0001;
            rx=(py-ry) * Tan + px;
            yo =- 64;
            xo =- yo * Tan;
        } else if(sin(degToRad(ra))<-0.001){
            ry = (((int)py>>6)<<6) + 64;
            rx = (py-ry) * Tan + px;
            yo = 64;
            xo =- yo * Tan;
        } else{
            rx = px;
            ry = py;
            dof=8;
        }

        while(dof<8) 
        { 
            mx = (int)(rx)>>6;
            my=(int)(ry)>>6;
            mp  =my * mapW + mx;

            if(mp>0 && mp<mapW*mapH && map[mp]==1)
            {
                dof = 8;
                horiDist = cos(degToRad(ra)) * (rx-px) - sin(degToRad(ra)) * (ry - py);
            } else {
                rx += xo;
                ry += yo;
                dof += 1;
            }
        } 

        glColor3f(0.8, 0.8, 0.8);

        if(disV < horiDist){
            rx = vx;
            ry = vy;
            horiDist = disV;
            
            glColor3f(0.6, 0.6, 0.6);
        }
        
        /* For debug
        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2i(px,py);
        glVertex2i(rx,ry);
        glEnd();
        */

        int ca = fixedAngle(pa-ra);
        horiDist *= cos(degToRad(ca));
        int lineHeight = (mapS * 480) / (horiDist);
        
        if(lineHeight > 480){ lineHeight = 480;}
        
        int lineOffset = 240 - (lineHeight>>1);

        glLineWidth(8);
        glBegin(GL_LINES);
        glVertex2i(r * 8, lineOffset);
        glVertex2i(r * 8, lineOffset + lineHeight);
        glEnd();

        ra = fixedAngle(ra - 1);
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(Keys.w == 1) {  px += pdx * walkVelocity; py += pdy * walkVelocity; }
    if(Keys.s == 1) {  px -= pdx * walkVelocity; py -= pdy * walkVelocity; }
    if(Keys.a == 1) { pa += 0.05f; pa = fixedAngle(pa); pdx = cos(degToRad(pa)); pdy =- sin(degToRad(pa)); }
    if(Keys.d == 1) { pa -= 0.05f; pa = fixedAngle(pa); pdx = cos(degToRad(pa)); pdy =- sin(degToRad(pa)); }

    //drawMap(); // Debug
    drawRays();
    //drawPlayer(); // Debug

    // printf("%f %f\n", px, py); // Player position

    glutSwapBuffers();
}

void init()
{
    glClearColor(0, 0, 0, 1);
    gluOrtho2D(0, windowWidth, windowHeight, 0);

    walkVelocity = 0.01f;
    px = 128;
    py = 211;
    pa = 90.0f;
    pdx = cos(degToRad(pa));
    pdy =- sin(degToRad(pa));
}

void resizeWindow(int width, int height)
{
    glutReshapeWindow(windowWidth, windowHeight);
}

void keyboardUpEvents(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
        Keys.w = 0;
        break;
    case 's':
        Keys.s = 0;
        break;
    case 'a':
        Keys.a = 0;
        break;
    case 'd':
        Keys.d = 0;
        break;
    
    default:
        break;
    }

    glutPostRedisplay();
}

void keyboardEvents(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
        Keys.w = 1;
        break;
    case 's':
        Keys.s = 1;
        break;
    case 'a':
        Keys.a = 1;
        break;
    case 'd':
        Keys.d = 1;
        break;
    
    default:
        break;
    }

    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Raycaster");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(resizeWindow);
    glutKeyboardFunc(keyboardEvents);
    glutKeyboardUpFunc(keyboardUpEvents);
    glutMainLoop();

    return 0;
}