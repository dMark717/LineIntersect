//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev : Dorozsmai Márk
// Neptun : F5SXE8
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const char* const vertexSource = R"(
	#version 330
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec3 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, vp.z, 1) * MVP ;		// transform vp from modeling space to normalized device space
	}
)";

const char* const fragmentSource = R"(
	#version 330
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram;
unsigned int vao;
class Object
{
public:
	unsigned int vao, vbo;
	std::vector<vec3> vtx;
	Object() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}
	void Draw(int type, vec3 color) {
		if (vtx.size() > 0) {
			glBindVertexArray(vao);
			gpuProgram.setUniform(color, "color");
			glDrawArrays(type, 0, vtx.size());
		}
	}
};


class PointCollection : Object {
public:
	std::vector<vec3>& Vtx() { return vtx; }
	void updateGPU() {
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vtx.size() * sizeof(vec3), &vtx[0], GL_DYNAMIC_DRAW);
	}

	void add(float x, float y) {
		if (!(x >= 1 || x <= -1 || y >= 1 || y <= -1))
		{
			Vtx().push_back(vec3(x, y, 1));
			printf("Added point: (%f, %f)\n", x, y);
		}
		else printf("Point not added\n");
	}
	vec3 findClosestPoint(float x, float y) {
		vec3 closestPoint;
		float minDistance = 100000;

		for (const vec3& point : Vtx()) {
			float dx = x - point.x;
			float dy = y - point.y;
			float distance = sqrt(dx * dx + dy * dy);

			if (distance < minDistance) {
				minDistance = distance;
				closestPoint = point;
			}
		}
		if (minDistance > 0.01) return vec3(0, 0, 0);

		return closestPoint;
	}

	void DrawAll() {
		updateGPU();
		Draw(GL_POINTS, vec3(1, 0, 0));
	}
};

class Line {
private:
	vec3 start;
	vec3 end;

public:
	Line() : start(2, 2, 1), end(2, 2, 1) {}
	Line(vec3 point1, vec3 point2) {

		float a = point2.y - point1.y;
		float b = point1.x - point2.x;
		float c = point2.x * point1.y - point1.x * point2.y;

		float x0 = point1.x;
		float y0 = point1.y;
		float t = 0;

		if (!(a == 0 && b == 0))
		{
			printf("Implicit equation of the line: %fx + %fy + %f = 0\n", a, b, c);
			printf("Parametric equation of the line: x = %f + %ft, y = %f + %ft\n", x0, t, y0, t);
		}
		else
		{
			printf("The two selected points are in the same location, line not added\n");
			start = vec3(2, 2, 1);
			end = vec3(2, 2, 1);
		}

		vec3 intersectionTop = vec3(0, 0, 0);
		vec3 intersectionBottom = vec3(0, 0, 0);
		vec3 intersectionRight = vec3(0, 0, 0);
		vec3 intersectionLeft = vec3(0, 0, 0);

		if (a != 0 && b != 0)
		{
			intersectionTop = vec3(-1, (-c - a * -1) / b, 1);
			intersectionBottom = vec3(1, (-c - a * 1) / b, 1);
			intersectionRight = vec3((-c - b * 1) / a, 1, 1);
			intersectionLeft = vec3((-c - b * -1) / a, -1, 1);

			if (intersectionTop.x < intersectionRight.y) {
				start = intersectionTop;
				end = intersectionBottom;
			}

			if (intersectionRight.y < intersectionTop.x) {
				start = intersectionRight;
				end = intersectionLeft;
			}
		}
		else if (a == 0) {
			intersectionTop = vec3(-1, -c / b, 1);
			intersectionBottom = vec3(1, -c / b, 1);
			start = intersectionTop;
			end = intersectionBottom;
		}
		else if (b == 0) {
			intersectionRight = vec3(-c / a, 1, 1);
			intersectionLeft = vec3(-c / a, -1, 1);
			start = intersectionRight;
			end = intersectionLeft;
		}

	}

	void translateLineToGoThroughPoint(vec3 point) {

		float a = end.y - start.y;
		float b = start.x - end.x;
		float c = end.x * start.y - start.x * end.y;

		vec3 normal = vec3(a, b, 0);

		c = -normal.x * point.x - normal.y * point.y;
		a = normal.x;
		b = normal.y;

		vec3 intersectionTop = vec3(0, 0, 0);
		vec3 intersectionBottom = vec3(0, 0, 0);
		vec3 intersectionRight = vec3(0, 0, 0);
		vec3 intersectionLeft = vec3(0, 0, 0);

		if (a != 0 && b != 0)
		{
			intersectionTop = vec3(-1, (-c - a * -1) / b, 1);
			intersectionBottom = vec3(1, (-c - a * 1) / b, 1);
			intersectionRight = vec3((-c - b * 1) / a, 1, 1);
			intersectionLeft = vec3((-c - b * -1) / a, -1, 1);

			if (intersectionTop.x < intersectionRight.y) {
				start = intersectionTop;
				end = intersectionBottom;
			}

			if (intersectionRight.y < intersectionTop.x) {
				start = intersectionRight;
				end = intersectionLeft;
			}
		}
		else if (a == 0) {
			intersectionTop = vec3(-1, -c / b, 1);
			intersectionBottom = vec3(1, -c / b, 1);
			start = intersectionTop;
			end = intersectionBottom;
		}
		else if (b == 0) {
			intersectionRight = vec3(-c / a, 1, 1);
			intersectionLeft = vec3(-c / a, -1, 1);
			start = intersectionRight;
			end = intersectionLeft;
		}
		

	}


	vec3 getStart() { return start; }
	vec3 getEnd() { return end; }
	void setStart(vec3 start) { this->start = start; }
	void setEnd(vec3 end) { this->end = end; }
	float getParameterA() { return end.y - start.y; }
	float getParameterB() { return start.x - end.x; }
	float getParameterC() { return end.x * start.y - start.x * end.y; }

};


class LineCollection : Object {
private:
	std::vector<Line> lines;
public:
	void updateGPU() {
		glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(Line), &lines[0], GL_DYNAMIC_DRAW);
	}

	void updateVtx() {
		vtx.clear();
		for (int i = 0; i < lines.size(); i++) {
			vtx.push_back(lines[i].getStart());
			vtx.push_back(lines[i].getEnd());
		}
	}

	void add(Line ln) {
		if (ln.getStart().x != 2 && ln.getStart().y != 2)
		{
			lines.push_back(ln);
		}
		else printf("Line not added\n");
			
	}

	Line findClosestLine(vec3 point) {
		Line closestLine;
		float minDistance = 100000;

		for (int i = 0; i < lines.size(); i++) {
			Line line = lines[i];
			float a = line.getParameterA();
			float b = line.getParameterB();
			float c = line.getParameterC();

			float distance = abs(a * point.x + b * point.y + c) / sqrt(a * a + b * b);

			if (distance < minDistance) {
				minDistance = distance;
				closestLine = line;
			}
		}
		if (minDistance > 0.01) return Line();
		else return closestLine;
	}

	vec3 findIntersection(Line line1, Line line2) {
		vec3 intersection;

		double det = line1.getParameterA() * line2.getParameterB() - line2.getParameterA() * line1.getParameterB();

		if (det == 0) {
			printf("The lines are parallel, no intersection point exists\n");
			intersection.x = 2;
			intersection.y = 2;
		}
		else {
			intersection.x = (line2.getParameterC() * line1.getParameterB() - line1.getParameterC() * line2.getParameterB()) / det;
			intersection.y = (line1.getParameterC() * line2.getParameterA() - line2.getParameterC() * line1.getParameterA()) / det;
		}

		return intersection;
	}

	int getPosition(Line line) {
		for (int i = 0; i < lines.size(); i++) {
			if (lines[i].getStart().x == line.getStart().x && lines[i].getStart().y == line.getStart().y && lines[i].getEnd().x == line.getEnd().x && lines[i].getEnd().y == line.getEnd().y) return i;
		}
		return -1;
	}


	void translateLineToGoThroughPoint(vec3 point, int position) {

		lines[position].translateLineToGoThroughPoint(point);
	}


	void drawAll()
	{
		updateVtx();
		updateGPU();
		Draw(GL_LINES, vec3(0, 1, 1));
	}

};

PointCollection* pc;
LineCollection* lc;

void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	glLineWidth(3);
	glPointSize(10);
	lc = new LineCollection();
	pc = new PointCollection();

	pc->DrawAll();
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}


void onDisplay() {
	glClearColor(0.5, 0.5, 0.5, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	float MVPtransf[4][4] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 };
	int location = glGetUniformLocation(gpuProgram.getId(), "MVP");
	glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);

	lc->drawAll();
	pc->DrawAll();
	glutSwapBuffers();
}

int x = 0;

vec3 selectedPoint;
vec3 prevSelectedPoint;
Line selectedLine;
Line prevSelectedLine;
int selectedPointsCount = 0;
int selectedLinesCount = 0;
bool lineSelectedForMoving = false;
int lineToMove = -1;



void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == 'd') glutPostRedisplay();
	if (key == 'p') { x = 1; printf("Add point mode activated\n"); }
	if (key == 'l') { x = 2; printf("Add line mode activated\n"); }
	if (key == 'm') { x = 3; printf("Move line mode activated\n"); }
	if (key == 'i') { x = 4; printf("Add intersection mode activated\n"); }
}

void onKeyboardUp(unsigned char key, int pX, int pY) {

}

void onMouseMotion(int pX, int pY) {

	float cX = 2.0f * pX / windowWidth - 1;
	float cY = 1.0f - 2.0f * pY / windowHeight;
	printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
	if (lineSelectedForMoving)
	{
		lc->translateLineToGoThroughPoint(vec3(cX, cY, 1), lineToMove);
	}
	glutPostRedisplay();
}


void onMouse(int button, int state, int pX, int pY) {

	float cX = 2.0f * pX / windowWidth - 1;
	float cY = 1.0f - 2.0f * pY / windowHeight;

	char* buttonStat;
	switch (state) {
	case GLUT_DOWN: buttonStat = "pressed"; break;
	case GLUT_UP: buttonStat = "released"; break;
	}

	switch (button) {
	case GLUT_LEFT_BUTTON: printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
	case GLUT_MIDDLE_BUTTON: printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
	case GLUT_RIGHT_BUTTON: printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (x == 1) {
			pc->add(cX, cY);
		}
		if (x == 2) {
			if (selectedPointsCount == 0) {
				selectedPoint = pc->findClosestPoint(cX, cY);
				selectedPointsCount++;
				if (selectedPoint.x == 0 && selectedPoint.y == 0) selectedPointsCount = 0;
			}
			else if (selectedPointsCount == 1) {
				prevSelectedPoint = selectedPoint;
				selectedPoint = pc->findClosestPoint(cX, cY);
				if (selectedPoint.x == 0 && selectedPoint.y == 0) selectedPointsCount = 0;
				else
				{
					selectedPointsCount++;
					Line ln = Line(prevSelectedPoint, selectedPoint);
					lc->add(ln);
					selectedPointsCount = 0;
				}
			}

		}
		if (x == 3) {
			lineToMove = lc->getPosition(lc->findClosestLine(vec3(cX, cY, 1)));
			lineSelectedForMoving = true;
		}
		if (x == 4) {

			if (selectedLinesCount == 0) {
				selectedLine = lc->findClosestLine(vec3(cX, cY, 1));
				selectedLinesCount++;
				if (selectedLine.getEnd().x == 0 && selectedLine.getEnd().y == 0 && selectedLine.getStart().x == 0 && selectedLine.getStart().y == 0) selectedLinesCount = 0;
			}
			else if (selectedLinesCount == 1) {
				prevSelectedLine = selectedLine;
				selectedLine = lc->findClosestLine(vec3(cX, cY, 1));
				if (selectedLine.getEnd().x == 0 && selectedLine.getEnd().y == 0 && selectedLine.getStart().x == 0 && selectedLine.getStart().y == 0) selectedLinesCount = 0;
				else
				{
					selectedLinesCount++;
					vec3 intersectionPoint = lc->findIntersection(prevSelectedLine, selectedLine);
					pc->add(intersectionPoint.x, intersectionPoint.y);
					selectedLinesCount = 0;
				}
			}
		}
		glutPostRedisplay();
	}
}


void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME);
}