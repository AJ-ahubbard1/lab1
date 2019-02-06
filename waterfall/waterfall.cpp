// Waterwheel
//modified by: Andrew Hubbard
//date: 2/3/2019
//
//3350 Spring 2018 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
// .general animation framework
// .animation loop
// .object definition and movement
// .collision detection
// .mouse/keyboard interaction
// .object constructor
// .coding style
// .defined constants
// .use of static variables
// .dynamic memory allocation
// .simple opengl components
// .git
//
//elements we will add to program...
//   .Game constructor
//   .multiple particles
//   .gravity
//   .collision detection
//   .more objects
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"


const int MAX_PARTICLES = 999999;
const float GRAVITY = -0.3;
int RADIUS = 3;
int WATERPRESSURE = 9;
bool faucet = true;
float spin;
//some structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

class Global {
    public:
	int xres, yres;
	Vec hose;
	Vec waterwheel;
	Shape box[5];
	int xmouse, ymouse;
	Particle particle[MAX_PARTICLES];
	int n;
	Global() {
	    xres = 1000;
	    yres = 750;
	    xmouse = 780;
	    ymouse = 526;
	    //center of waterwheel
	    waterwheel.x = 335;
	    waterwheel.y = 291;
	    //radius of waterwheel
	    waterwheel.z =	250;			
	    //define a box shape
	    for (int i = 0; i < 5; i++) {
		box[i].width = 50;
		box[i].height = 10;
		box[i].center.x = waterwheel.x + waterwheel.z * cos(i * 1.256);
		box[i].center.y = waterwheel.x + waterwheel.z * sin(i * 1.256);
	    }
	    n = 0;
	    hose.x = -2.8;
	    hose.y = 8.0;
	}
} g;

class X11_wrapper {
    private:
	Display *dpy;
	Window win;
	GLXContext glc;
    public:
	~X11_wrapper() {
	    XDestroyWindow(dpy, win);
	    XCloseDisplay(dpy);
	}
	X11_wrapper() {
	    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	    int w = g.xres, h = g.yres;
	    dpy = XOpenDisplay(NULL);
	    if (dpy == NULL) {
		cout << "\n\tcannot connect to X server\n" << endl;
		exit(EXIT_FAILURE);
	    }
	    Window root = DefaultRootWindow(dpy);
	    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	    if (vi == NULL) {
		cout << "\n\tno appropriate visual found\n" << endl;
		exit(EXIT_FAILURE);
	    } 
	    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	    XSetWindowAttributes swa;
	    swa.colormap = cmap;
	    swa.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	    set_title();
	    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	    glXMakeCurrent(dpy, win, glc);
	}
	void set_title() {
	    //Set the window title bar.
	    XMapWindow(dpy, win);
	    XStoreName(dpy, win, "3350 Lab1");
	}
	bool getXPending() {
	    //See if there are pending events.
	    return XPending(dpy);
	}
	XEvent getXNextEvent() {
	    //Get a pending event.
	    XEvent e;
	    XNextEvent(dpy, &e);
	    return e;
	}
	void swapBuffers() {
	    glXSwapBuffers(dpy, win);
	}
} x11;

//Function prototypes
void commands_print();
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render();


//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main() {
    commands_print();
    srand(time(NULL));
    init_opengl();
    //Main animation loop
    int done = 0;
    while (!done) {
	//Process external events.
	while (x11.getXPending()) {
	    XEvent e = x11.getXNextEvent();
	    check_mouse(&e);
	    done = check_keys(&e);
	}
	movement();
	render();
	x11.swapBuffers();
    }
    cleanup_fonts();
    return 0;
}

void commands_print() {
    cout << "Waterfall \"Waterwheel\" Model\n\n";
    cout << "Mouse:\n";
    cout << "\tLeft\twater on/off\n";
    cout << "\tRight\tSingle Drop\n";
    cout << "\tScroll\twaterflow inc/dec\n\n";
    cout << "Keyboard:\n";
    cout << "\tArrows\tAim water/inc pressure\n";
    cout << "\tWASD\tMove Water wheel\n";
    cout << "\tEsc\tEnd\n";
}

void init_opengl(void) {
    //OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

void makeParticle(int x, int y) {
    if (g.n >= MAX_PARTICLES)
	return;
    //cout << "makeParticle() " << x << " " << y << endl;
    //position of particle
    Particle *p = &g.particle[g.n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = g.hose.y;
    p->velocity.x = g.hose.x;
    ++g.n;
}

void check_mouse(XEvent *e) {
    //static int savex = 0;
    //static int savey = 0;

    if (e->type != ButtonRelease &&
	    e->type != ButtonPress &&
	    e->type != MotionNotify) {
	//This is not a mouse event that we care about.
	return;
    }
    //
    if (e->type == ButtonRelease) {
	return;
    }
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    //Left button was pressed
	    faucet = !faucet;
	    //int y = g.yres - e->xbutton.y;
	    //makeParticle(e->xbutton.x, y);
	    return;
	}
	if (e->xbutton.button==3) {
	    makeParticle(g.xmouse,g.ymouse);
	    return;
	}	
	if (e->xbutton.button==4) {
	    if (++RADIUS > 8)
		RADIUS = 8;
	    WATERPRESSURE = RADIUS * RADIUS;
	    return;
	}
	if (e->xbutton.button==5) {
	    if (--RADIUS < 1)
		RADIUS = 1;
	    WATERPRESSURE = RADIUS * RADIUS;
	    return;
	}
    }
    if (e->type == MotionNotify) {
	//The mouse moved!
	if (g.xmouse != e->xbutton.x || g.ymouse != e->xbutton.y) {
	    g.xmouse = e->xbutton.x;
	    g.ymouse = g.yres - e->xbutton.y;
	    //makeParticle(savex,savey);
	}
    }
}

int check_keys(XEvent *e) {
    if (e->type != KeyPress && e->type != KeyRelease)
	return 0;
    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyPress) {
	switch (key) {
	    case XK_1:
		//Key 1 was pressed
		break;
	    case XK_Left:
		//Key Left was pressed
		g.hose.x -=.2;
		break;
	    case XK_Right:
		//Key Right was pressed
		g.hose.x +=.2;
		break;
	    case XK_Up:
		//Key Up was pressed
		g.hose.y +=.2;
		break;
	    case XK_Down:
		//Key Down was pressed
		g.hose.y -=.2;
		break;
	    case XK_equal:
		if (++RADIUS > 8)
		    RADIUS = 8;
		WATERPRESSURE = RADIUS * RADIUS;
		break;
	    case XK_minus:
		if (--RADIUS < 1)
		    RADIUS = 1;
		WATERPRESSURE = RADIUS * RADIUS;
		break;
	    case XK_w:
		g.waterwheel.y++;
		break;
	    case XK_s:
		g.waterwheel.y--;
		break;
	    case XK_a:
		g.waterwheel.x--;
		break;
	    case XK_d:
		g.waterwheel.x++;
		break;
	    case XK_Escape:
		/*Number of particles on screen 
		cout << "n = " << g.n << endl;
		*/
		return 1;
	}		
	//Troubleshooting Printout
	/*
	cout << "Hose Vel:" << g.hose.x << " " << g.hose.y << 
	    "  Rad: " << RADIUS << "  Spin: " << spin << 
	    "  Waterwheel cen: " << g.waterwheel.x << " " <<
	    g.waterwheel.y << "  mouse xy " << g.xmouse << " " <<
	    g.ymouse << endl;
	*/

    }
    return 0;
}

void movement() {
    int mx = g.xmouse;
    int my = g.ymouse;
    if (faucet) {
	for (int i = my+RADIUS; i > my-RADIUS; i--) {
	    for (int j = mx-RADIUS; j < mx+RADIUS; j++) {
		float circle = (j-mx)*(j-mx) + (i-my)*(i-my);
		if (circle <= WATERPRESSURE)
		    makeParticle(j,i);
	    }
	}
    }

    if (g.n <= 0)
	return;
    for (int i = 0; i <g.n; i++) {
	Particle *p = &g.particle[i];
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;
	p->velocity.y += GRAVITY;


	//check for collision with shapes...
	Shape *s;
	for (int i = 0; i < 5; i++) {
	    s = &g.box[i];
	    if ((p->s.center.y < s->center.y + s->height) && 
		    (p->s.center.y > s->center.y - s->height) &&
		    (p->s.center.x > s->center.x - s->width)  && 
		    (p->s.center.x < s->center.x + s->width)) {
		p->s.center.y = s->center.y + s->height;
		spin += .00001 * cos(i * 1.256 + spin) * p->velocity.y;
		p->velocity.y = -.01*(rand() % 41 + 30) * p->velocity.y;
		p->velocity.x = .01*(rand() % 251 - 100) * p->velocity.x;
	    }
	}
	//check for off-screen
	if (p->s.center.y < 0.0) {
	    //cout << "off screen" << endl;
	    g.particle[i] = g.particle[--g.n];
	}
    }
    for (int i = 0; i < 5; i++) {
	g.box[i].center.x = g.waterwheel.x + g.waterwheel.z * cos(i * 1.256 + spin);
	g.box[i].center.y = g.waterwheel.y + g.waterwheel.z * sin(i * 1.256 + spin);
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...
    //
    //draw a box
    Shape *s;
    float w, h; 
    for (int i = 0; i < 5; i++) {	   
	glColor3ub(90,140,90);
	s = &g.box[i];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);    
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
	glVertex2i(-w, -h);
	glVertex2i(-w,  h);
	glVertex2i( w,  h);
	glVertex2i( w, -h);
	glEnd();
	glPopMatrix();
    }
    //
    //Draw the particle here
    glPushMatrix();
    glColor3ub(150,160,220);
    for (int i = 0; i < g.n; i++) {
	Vec *c = &g.particle[i].s.center;
	w = h = 2;
	glBegin(GL_QUADS);
	glVertex2i(c->x-w, c->y-h);
	glVertex2i(c->x-w, c->y+h);
	glVertex2i(c->x+w, c->y+h);
	glVertex2i(c->x+w, c->y-h);
	glEnd();
    }    	
    glPopMatrix();

    //Draw your 2D text here
    Rect r[5];
    unsigned int c = 0x00ffff44;
    r[0].bot = g.box[0].center.y-5;
    r[0].left = g.box[0].center.x-39;
    r[0].center = 0;
    ggprint8b(&r[0], 16, c, "REQUIREMENTS");

    r[1].bot = g.box[1].center.y-5;
    r[1].left = g.box[1].center.x-18;
    r[1].center = 0;
    ggprint8b(&r[1], 16, c, "DESIGN");

    r[2].bot = g.box[2].center.y-5;
    r[2].left = g.box[2].center.x-47;
    r[2].center = 0;
    ggprint8b(&r[2], 16, c, "IMPLEMENTATION");

    r[3].bot = g.box[3].center.y-5;
    r[3].left = g.box[3].center.x-38;
    r[3].center = 0;
    ggprint8b(&r[3], 16, c, "VERIFICATION");

    r[4].bot = g.box[4].center.y-5;
    r[4].left = g.box[4].center.x-36;
    r[4].center = 0;
    ggprint8b(&r[4], 16, c, "MAINTENANCE");
}






