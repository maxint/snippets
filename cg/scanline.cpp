#include <windows.h>   // use as needed for your system
#include <gl/gl.h>
#include <gl/glut.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <ctime>
using namespace std;

const int windowH = 480;
const int windowW = 640;
// if need to debug - print out information
ofstream outFile;

//<<<<<<< Data structures 
struct GLintPoint
{ GLint x,y;
};

class GLintPointArray
{
public:
	vector<GLintPoint> pt;
};

class Node
{ 
public:
	Node():yUpper(-1),xIntersect(0.0),dxPerScan(0.0)
	{ };
	int yUpper;
	float xIntersect,dxPerScan;
};
Node EmptyNode;  // an empty node

class EdgeTbl
{
public:
	void buildTable (const GLintPointArray &);
	int yNext (int,vector<GLintPoint>);
	void makeEdgeRecord (GLintPoint,GLintPoint,int);
	void printEdgeTable();

	vector<list<Node> > Edges;
};
list<Node> EmptyList;  // an empty list

//*************EDGE TABLE METHODS 
void insertEdge (list<Node>& orderedList, const Node& item)
{
	list<Node>::iterator curr = orderedList.begin(),
		stop = orderedList.end();
	while ((curr != stop) && ((*curr).xIntersect < item.xIntersect))
		curr++;
	orderedList.insert(curr,item);
} 
int EdgeTbl::yNext (int k, vector<GLintPoint> p)
{ int j;
// next subscript in polygon
if ((k+1) > (p.size()-1))
j = 0;
else
j = k+1;
while (p[k].y == p[j].y)
if ((j+1) > (p.size()-1))
j = 0;
else
j++;
return (p[j].y);
}

void EdgeTbl::makeEdgeRecord (GLintPoint lower, GLintPoint upper, 
							  int yComp)
{ Node n;

n.dxPerScan = (float)(upper.x-lower.x)/(upper.y-lower.y);
n.xIntersect = lower.x;
if (upper.y < yComp)
n.yUpper = upper.y-1;
else
n.yUpper = upper.y;
insertEdge (Edges[lower.y],n);
}

void EdgeTbl::buildTable (const GLintPointArray& Poly)
{
	GLintPoint v1,v2;
	int i, yPrev;

	yPrev = Poly.pt[Poly.pt.size()-2].y;
	v1.x = Poly.pt[Poly.pt.size()-1].x;
	v1.y = Poly.pt[Poly.pt.size()-1].y;
	for (i = 0; i < Poly.pt.size(); i++)
	{ v2 = Poly.pt[i];
	if (v1.y != v2.y)
	{ // non horizontal edge
		if (v1.y < v2.y) 
			makeEdgeRecord (v1,v2,yNext(i,Poly.pt)); //up edge
		else
			makeEdgeRecord (v2,v1,yPrev); // down edge
	}
	yPrev = v1.y;
	v1 = v2;
	}
}
//******* for debugging - print to a file
void writeListInfo (list<Node> & L)
{ list<Node>::iterator iter;
for (iter = L.begin(); iter != L.end(); iter++)
outFile << "  contents: " << (*iter).yUpper << " "
<< (*iter).xIntersect << " " << (*iter).dxPerScan << endl; 
}

void EdgeTbl::printEdgeTable()
{ 
	for (int i = 0; i < Edges.size(); i++)
	{  outFile << "Scan Line: " << i << "Information" << endl;
	writeListInfo (Edges[i]);
	}
}
//
// ************** AEL ROUTINES 
void buildAEL (list<Node> &AEL, list<Node> ET)
{ list<Node>::iterator iter;

iter = ET.begin();
// every Edge table list has a "empty" node at front
iter++;
while (iter != ET.end())
{ insertEdge (AEL,*iter);
iter++;
}
}
void fillScan (int y,list<Node> L)
{    // want to pull off pairs of x values from adjacent
	// nodes in the list - the y value = scan line
	list<Node>::iterator iter1 = L.begin(), iter2;
	int x1,x2;
	while (iter1 != L.end())
	{ iter2 = iter1;
	iter2++;
	x1 = (int)(*iter1).xIntersect;
	x2 = (int)(*iter2).xIntersect;
	glBegin(GL_LINES);
	glVertex2i(x1,y);
	glVertex2i(x2,y);
	glEnd();
	// move on to next pair of nodes
	iter1 = iter2;
	iter1++;
	}
}
void updateAEL (int y, list<Node>& L)
{   // delete completed edges
	// update the xIntersect field
	list<Node>::iterator iter = L.begin();
	while (iter != L.end())
		if (y >= (*iter).yUpper)
			L.erase(iter++);
		else
		{ (*iter).xIntersect += (*iter).dxPerScan;
	iter++;
	}
} 
void resortAEL (list<Node>& L)
{ Node n;
list<Node> L1;
list<Node>::iterator iter = L.begin();
// create a new list from the old
// note that the sort command for a list would
// need us to overload the comparisons operators in the
// Node class. This is probably just as simple
while (iter != L.end())
{ insertEdge (L1,*iter);
L.erase(iter++);
}
L = L1;
}
//*******************OTHER subprograms
typedef GLfloat colorType[3];

void drawDot (GLint x, GLint y, GLfloat r, GLfloat g, GLfloat b)
{ glColor3f(r,g,b);
glBegin (GL_POINTS);
glVertex2i (x,y);
glEnd();
}

void drawintPolygon (GLintPointArray P, colorType c)
{ glColor3fv (c);
glBegin(GL_LINE_LOOP);
for (int i=0; i < P.pt.size(); i++)
glVertex2i (P.pt[i].x,P.pt[i].y);
glEnd();
}

//*********************** myInit >>>>>>>>>>>>>>>>>>>>
void myInit(void)
{
	glClearColor(1.0,1.0,1.0,0.0);  // set white background color
	glColor3f (0.0f,0.0f,0.0f);     //default color
	//glPointSize(2.0);	      // a 'dot' is 4 by 4 pixels
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
	gluOrtho2D(0.0, 640.0, 0.0, 480.0);
	// open a debug file - if needed
	outFile.open ("fillDump.txt",ios::out);
}

//*********************  drawing subprograms go here
// create a sample polygon
void buildintPolygon (GLintPointArray &P)
{ GLintPoint p;
p.x = 270; p.y = 50; P.pt.push_back(p);
p.x = 350; p.y = 50; P.pt.push_back(p);
p.x = 450; p.y = 250; P.pt.push_back(p);
//p.x = 375; p.y = 325; P.pt.push_back(p);
//p.x = 200; p.y = 325; P.pt.push_back(p);
//p.x = 100; p.y = 250; P.pt.push_back(p);
//p.x = 200; p.y = 100; P.pt.push_back(p);
//p.x = 270; p.y = 225; P.pt.push_back(p);
}

void scanFill (GLintPointArray P, colorType c)
{   // need an edge table and AEL
	EdgeTbl EdgeTable;
	list<Node> AEL;

	EmptyList.push_front(EmptyNode); // and empty list
	// build the edge table - need the window size
	for (int i = 0; i < windowH; i++)
		EdgeTable.Edges.push_back(EmptyList);
	EdgeTable.buildTable(P);
	// if needed - print the table here
	// EdgeTable.printEdgeTable();
	// filling requires building and using AEL
	glColor3fv (c);
	for (int scanLine = 0; scanLine < windowH; scanLine++)
	{    // could add output data on table
		//outFile <<"Scan line: " << scanLine << endl;
		buildAEL (AEL,EdgeTable.Edges[scanLine]);
		if (!AEL.empty())
		{    // if needed print the table
			//writeListInfo(AEL);
			fillScan(scanLine,AEL);
			updateAEL (scanLine,AEL);
			resortAEL(AEL);
		}
	}
}
//************************ myDisplay >>>>>>>>>>>>>>>>>
void myDisplay(void)
{      // polygon  
	glClear(GL_COLOR_BUFFER_BIT);     // clear the screen 
	// 1. create a polygon to clip
	clock_t t0 = clock();
	for(int i=0;i<100;i++)
	{
		GLintPointArray Poly;
		colorType polyC = {0.0f,1.0f,0.0f}; 
		// color to fill
		colorType fillC = {1.0f,0.0f,0.0f};
		buildintPolygon(Poly);
		drawintPolygon (Poly,polyC);
	// 2. fill it
		scanFill (Poly,fillC);
		EmptyList.clear();
	}
	clock_t t1 = clock();
	std::cout<<t1-t0<<std::endl;
	glFlush();
}

//*************************** main >>>>>>>>>>>>>>>>>>>>>>
int main(int argc, char** argv)
{
	glutInit(&argc, argv);          // initialize the toolkit
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // set display mode
	glutInitWindowSize(640,480);     // set window size
	glutInitWindowPosition(100, 150); // set window position on screen
	glutCreateWindow("Filling: Edge Table and AEL"); // open the screen window
	glutDisplayFunc(myDisplay);     // register redraw function
	myInit();                   
	glutMainLoop(); 		     // go into a perpetual loop
	return 0;
}


