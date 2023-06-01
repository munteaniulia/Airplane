#include "GLOS.h"
#include "gl.h"
#include "glu.h"
#include "glaux.h"
#include "glut.h"

static int rotire = 0, a = 0, b = 0, spin = 0, poz = 0;
bool umbra2 = false;
GLUquadric* quad;

const int x = 0, y = 1, z = 2, w = 3;
const int A = 0, B = 1, C = 2, D = 3;

static float lightSourcePosition[4] = {-60, 60, -150, 0 };
static float modelX = 0;
static float angle = 0;

GLfloat punctePlanIarba[][3] = {
	 { -150.0f, -60.0f, -150.0f },
	{ -150.0f, -60.0f, 150.0f },
	{ 150.0f, -60.0f, 150.0f } ,
	{ 150.0f, -60.0f, -150.0f }
};

float coeficientiPlanIarba[4]; 
float matriceUmbrire[4][4];


void CALLBACK mutaSursaFata() {
	if (lightSourcePosition[z] < 0) {
		lightSourcePosition[z] += 5;
	}
}
void CALLBACK mutaSursaSpate() {
	if (lightSourcePosition[z] > -150) {
		lightSourcePosition[z] -= 5;
	}
}
void CALLBACK mutaSursaDreapta() {
	if (lightSourcePosition[x] < 100) {
		lightSourcePosition[x] += 5;
	}
}
void CALLBACK mutaSursaStanga() {
	if (lightSourcePosition[x] > -100) {
		lightSourcePosition[x] -= 5;
	}
}

void CALLBACK Zbor()
{
	a = (a - 5) % 360;
}
void CALLBACK Rotiref()
{
	b = (b + 15) % 360;
}
void CALLBACK Rotires()
{
	b = (b - 15) % 360;
}

void computePlaneCoefficientsFromPoints(float points[3][3]) {
	// calculeaza 2 vectori din 3 puncte
	float v1[3]{ points[0][x] - points[1][x], points[0][y] - points[1][y], points[0][z] - points[1][z] };
	float v2[3]{ points[1][x] - points[2][x], points[1][y] - points[2][y], points[1][z] - points[2][z] };

	// produsul vectorial al celor 2 vectori => al 3lea vector cu componentele A,B,C chiar coef din ec. planului
	coeficientiPlanIarba[A] = v1[y] * v2[z] - v1[z] * v2[y];
	coeficientiPlanIarba[B] = v1[z] * v2[x] - v1[x] * v2[z];
	coeficientiPlanIarba[C] = v1[x] * v2[y] - v1[y] * v2[x];

	// determinam D - ecuatia planului in punctul random ales trebuie sa fie zero
	int randomPoint = 1; // poate fi orice punct de pe planul ierbii, asa ca alegem unul din cele 3 folosite pentru calcule
	coeficientiPlanIarba[D] =
		-(coeficientiPlanIarba[A] * points[randomPoint][x] +
			coeficientiPlanIarba[B] * points[randomPoint][y] +
			coeficientiPlanIarba[C] * points[randomPoint][z]);

}

void computeShadowMatrix(float points[3][3], float lightSourcePosition[4]) {
	// determina coef planului	
	computePlaneCoefficientsFromPoints(points);

	// temp = AxL + ByL + CzL + Dw
	float temp =
		coeficientiPlanIarba[A] * lightSourcePosition[x] +
		coeficientiPlanIarba[B] * lightSourcePosition[y] +
		coeficientiPlanIarba[C] * lightSourcePosition[z] +
		coeficientiPlanIarba[D] * lightSourcePosition[w];

	//prima coloana
	matriceUmbrire[0][0] = temp - coeficientiPlanIarba[A] * lightSourcePosition[x];
	matriceUmbrire[1][0] = 0.0f - coeficientiPlanIarba[B] * lightSourcePosition[x];
	matriceUmbrire[2][0] = 0.0f - coeficientiPlanIarba[C] * lightSourcePosition[x];
	matriceUmbrire[3][0] = 0.0f - coeficientiPlanIarba[D] * lightSourcePosition[x];
	//a 2a coloana
	matriceUmbrire[0][1] = 0.0f - coeficientiPlanIarba[A] * lightSourcePosition[y];
	matriceUmbrire[1][1] = temp - coeficientiPlanIarba[B] * lightSourcePosition[y];
	matriceUmbrire[2][1] = 0.0f - coeficientiPlanIarba[C] * lightSourcePosition[y];
	matriceUmbrire[3][1] = 0.0f - coeficientiPlanIarba[D] * lightSourcePosition[y];
	//a 3a coloana
	matriceUmbrire[0][2] = 0.0f - coeficientiPlanIarba[A] * lightSourcePosition[z];
	matriceUmbrire[1][2] = 0.0f - coeficientiPlanIarba[B] * lightSourcePosition[z];
	matriceUmbrire[2][2] = temp - coeficientiPlanIarba[C] * lightSourcePosition[z];
	matriceUmbrire[3][2] = 0.0f - coeficientiPlanIarba[D] * lightSourcePosition[z];
	//a 4a coloana
	matriceUmbrire[0][3] = 0.0f - coeficientiPlanIarba[A] * lightSourcePosition[w];
	matriceUmbrire[1][3] = 0.0f - coeficientiPlanIarba[B] * lightSourcePosition[w];
	matriceUmbrire[2][3] = 0.0f - coeficientiPlanIarba[C] * lightSourcePosition[w];
	matriceUmbrire[3][3] = temp - coeficientiPlanIarba[D] * lightSourcePosition[w];
}

GLuint textureId1;
GLuint incarcaTextura(const char* s)
{
	GLuint textureId = 0;
	AUX_RGBImageRec* pImagineTextura = auxDIBImageLoad(s);

	if (pImagineTextura != NULL)
	{
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, pImagineTextura->sizeX, pImagineTextura->sizeY,
			0, GL_RGB, GL_UNSIGNED_BYTE, pImagineTextura->data);
	}
	if (pImagineTextura)
	{
		if (pImagineTextura->data) {
			free(pImagineTextura->data);
		}
		free(pImagineTextura);
	}
	return textureId;
}

void myInit(void) {

	glClearColor(0.0, 0.0, 0.0, 0.0);

	textureId1 = incarcaTextura("fundal4.bmp");

	glEnable(GL_NORMALIZE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	float lightAmbient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	float lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lightSourcePosition);

	float materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float materialShininess[] = { 128.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_SMOOTH);
	glShadeModel(GL_SMOOTH);
}

void corp() {
	//glEnable(GL_COLOR_MATERIAL);
	if (umbra2) {
		umbra2 = true;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0, 0.0, 0, 0.5);
	}
	else {
		glDisable(GL_BLEND);
		umbra2 = false;
		glColor3f(1.0, 1.0, 1.0);
	}
	glFrontFace(GL_CW);
	
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glTranslatef(-65.0, -10.0, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	gluDisk(quad, 0, 10, 40, 100);
	glFrontFace(GL_CCW);
	gluCylinder(quad, 10, 10, 80, 45, 1000);

	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glTranslatef(80.0, 0.0, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	gluCylinder(quad, 10, 1, 30, 45, 1000);
}

void cabina() {
	if (umbra2) {
		umbra2 = true;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0, 0.0, 0, 0.5);
	}
	else {
		glDisable(GL_BLEND);
		umbra2 = false;
		glColor3f(0.6, 0.6, 0.8);
	}

	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glTranslatef(45.0, 10.0, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	gluCylinder(quad, 1, 6, 10, 45, 20);

	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glTranslatef(10.0, 0.0, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	gluCylinder(quad, 6, 6, 10, 45, 1000);

	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glTranslatef(10.0, 0.0, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	gluCylinder(quad, 6, 1, 10, 45, 1000);
}

void elice() {
	if (umbra2) {
		umbra2 = true;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0, 0.0, 0, 0.5);
	}
	else {
		glDisable(GL_BLEND);
		umbra2 = false;
		glColor3f(0.5, 0.5, 0.7);
	}
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glTranslatef(30.0, 0.0, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	gluDisk(quad, 0, 4, 40, 100);
	glRotatef((GLfloat)rotire, 0.0, 0.0, 1.0);
	gluPartialDisk(quad, 4, 20, 5, 100, 0, 30);
	gluPartialDisk(quad, 4, 20, 5, 100, 90, 30);
	gluPartialDisk(quad, 4, 20, 5, 100, 180, 30);
	gluPartialDisk(quad, 4, 20, 5, 100, 270, 30);
}

void aripas() {

	glBegin(GL_QUADS);

	//fata
	glVertex3f(-35, 2.5, 70);
	glVertex3f(-10, 2.5, 70);
	glVertex3f(-10, -2.5, 70);
	glVertex3f(-35, -2.5, 70);

	//lateral d
	glVertex3f(-10, 2.5, 70);
	glVertex3f(-10, 2.5, 50);
	glVertex3f(-10, -2.5, 50);
	glVertex3f(-10, -2.5, 70);

	//fspate

	glVertex3f(-10, 2.5, 50);
	glVertex3f(-35, 2.5, 50);
	glVertex3f(-35, -2.5, 50);
	glVertex3f(-10, -2.5, 50);

	//laterals
	glVertex3f(-35, 2.5, 50);
	glVertex3f(-35, 2.5, 70);
	glVertex3f(-35, -2.5, 70);
	glVertex3f(-35, -2.5, 50);

	//sus
	glVertex3f(-10, 2.5, 50);
	glVertex3f(-35, 2.5, 50);
	glVertex3f(-35, 2.5, 70);
	glVertex3f(-10, 2.5, 70);

	//jos
	glVertex3f(-10, -2.5, 50);
	glVertex3f(-35, -2.5, 50);
	glVertex3f(-35, -2.5, 70);
	glVertex3f(-10, -2.5, 70);

	glEnd();

}

void aripad() {

	glBegin(GL_QUADS);

	//fata
	glVertex3f(35, 2.5, 70);
	glVertex3f(10, 2.5, 70);
	glVertex3f(10, -2.5, 70);
	glVertex3f(35, -2.5, 70);

	//lateral d
	glVertex3f(35, 2.5, 70);
	glVertex3f(35, 2.5, 50);
	glVertex3f(35, -2.5, 50);
	glVertex3f(35, -2.5, 70);

	//fspate

	glVertex3f(35, 2.5, 50);
	glVertex3f(35, -2.5, 50);
	glVertex3f(10, -2.5, 50);
	glVertex3f(10, 2.5, 50);

	//laterals
	glVertex3f(10, 2.5, 50);
	glVertex3f(10, 2.5, 70);
	glVertex3f(10, -2.5, 70);
	glVertex3f(10, -2.5, 50);

	//sus
	glVertex3f(35, 2.5, 50);
	glVertex3f(10, 2.5, 50);
	glVertex3f(10, 2.5, 70);
	glVertex3f(35, 2.5, 70);

	//jos
	glVertex3f(35, -2.5, 50);
	glVertex3f(10, -2.5, 50);
	glVertex3f(10, -2.5, 70);
	glVertex3f(35, -2.5, 70);

	glEnd();

}

void aripaspate() {
	glBegin(GL_QUADS);
	//fata
	glVertex3f(20, 2.5, 10);
	glVertex3f(-20, 2.5, 10);
	glVertex3f(-20, -2.5, 10);
	glVertex3f(20, -2.5, 10);

	//lateral d
	glVertex3f(20, 2.5, 10);
	glVertex3f(20, 2.5, 0);
	glVertex3f(20, -2.5, 0);
	glVertex3f(20, -2.5, 10);

	//fspate

	glVertex3f(-20, 2.5, 0);
	glVertex3f(-20, -2.5, 0);
	glVertex3f(20, -2.5, 0);
	glVertex3f(20, 2.5, 0);

	//laterals
	glVertex3f(-20, 2.5, 0);
	glVertex3f(-20, 2.5, 10);
	glVertex3f(-20, -2.5, 10);
	glVertex3f(-20, -2.5, 0);

	//sus
	glVertex3f(20, 2.5, 0);
	glVertex3f(-20, 2.5, 0);
	glVertex3f(-20, 2.5, 10);
	glVertex3f(20, 2.5, 10);

	//jos
	glVertex3f(-20, -2.5, 0);
	glVertex3f(20, -2.5, 0);
	glVertex3f(20, -2.5, 10);
	glVertex3f(-20, -2.5, 10);

	glEnd();

	glBegin(GL_QUADS);
	//fata
	glVertex3f(-2.5, 9.0, 10);
	glVertex3f(-2.5, 20.0, 0);
	glVertex3f(2.5, 20.0, 0);
	glVertex3f(2.5, 9.0, 10);

	//spate
	glVertex3f(-2.5, 20.0, 0);
	glVertex3f(2.5, 20.0, 0);
	glVertex3f(2.5, 9.0, 0);
	glVertex3f(-2.5, 9.0, 0);
	glEnd();

	glBegin(GL_TRIANGLES);
	//lateral d
	glVertex3f(2.5, 20, 0);
	glVertex3f(2.5, 9.0, 10);
	glVertex3f(2.5, 9.0, 0);

	//Laterals
	glVertex3f(-2.5, 20, 0);
	glVertex3f(-2.5, 9.0, 10);
	glVertex3f(-2.5, 9.0, 0);
	glEnd();
}

void avion(bool umbra) {
	if (umbra) {
		umbra2 = true;
	}
	else {
		
		umbra2 = false;
	}
	glPushMatrix();
	glTranslatef(0.0, 30.0, -50.0);

	glPushMatrix();
	glTranslated(0.0, -50.0, 0.0);
	glRotatef((GLfloat)a, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 0.0, -50.0);
	glRotatef((GLfloat)b, 1.0, 0.0, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	cabina();
	corp();
	elice();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, -50.0, 0.0);
	glRotatef((GLfloat)a, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 0.0, -50.0);
	glRotatef((GLfloat)b, 1.0, 0.0, 0.0);
	glTranslatef(2.0, 0.0, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	aripas();
	aripad();
	aripaspate();
	glDisable(GL_BLEND);
	glPopMatrix();

	glPopMatrix();
}

void podea() {
	glPushMatrix();
	glColor3f(0.5, 0.5, 0.5);

	glTranslatef(0, -1, 0);
	glBegin(GL_QUADS);
	{
		glNormal3f(0, 1, 0);
		for (int i = 0; i < 4; i++) {
			glVertex3fv(punctePlanIarba[i]);
		}
	}
	glEnd();
	glPopMatrix();
}

void fundal() {

	glPushMatrix();
	glTranslatef(0.0, 0.0, -50.0);
	//lateral s
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-250, 280, 0);

	glTexCoord2f(0.33f, 1.0f);
	glVertex3f(-100, 280, -150);

	glTexCoord2f(0.33f, 0.0f);
	glVertex3f(-100, -70, -150);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-250, -100, 0);
	glEnd();

	//lspate
	glBegin(GL_QUADS);
	glTexCoord2f(0.33f, 1.0f);
	glVertex3f(-100, 280, -150);

	glTexCoord2f(0.66f, 1.0f);
	glVertex3f(100, 280, -150);

	glTexCoord2f(0.66f, 0.0f);
	glVertex3f(100, -70, -150);

	glTexCoord2f(0.33f, 0.0f);
	glVertex3f(-100, -70, -150);

	glEnd();

	//lateral d
	glBegin(GL_QUADS);
	glTexCoord2f(0.66f, 1.0f);
	glVertex3f(100, 280, -150);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(250, 280, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(250, -100, 0);

	glTexCoord2f(0.66f, 0.0f);
	glVertex3f(100, -70, -150);

	glEnd();
	glPopMatrix();

}

void deseneazaLumina()
{
	glPushMatrix();
	glTranslatef(lightSourcePosition[x], lightSourcePosition[y], lightSourcePosition[z]);
	glColor3f(1.0, 0.9, 0);
	auxWireSphere(5);
	glPopMatrix();
}

void CALLBACK display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureId1);
	glPushMatrix();
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glDisable(GL_LIGHTING);
	glTranslatef(0.0, 0.0, -250.0);
	fundal();
	glDisable(GL_TEXTURE_2D);
	glColor4f(0.5, 0.5, 0.5, 0.0);
	computeShadowMatrix(punctePlanIarba, lightSourcePosition);
	glLightfv(GL_LIGHT0, GL_POSITION, lightSourcePosition);
	podea();
	glEnable(GL_LIGHTING);
	avion(false);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(lightSourcePosition[x], lightSourcePosition[y], lightSourcePosition[z]);
	glDisable(GL_LIGHTING);
	glColor3f(0.8, 0.8, 0.0);
	auxSolidSphere(3);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glTranslatef(0, 0, -200);
	glPushMatrix();
	glMultMatrixf((GLfloat*)matriceUmbrire);
	avion(true);
	glPopMatrix();
	glEnable(GL_LIGHTING);

	auxSwapBuffers();
}
void CALLBACK IdleFunction(void)
{
	rotire = (rotire + 5) % 360;
	display();
	Sleep(2500 / 60);
}
void CALLBACK myReshape(GLsizei w, GLsizei h) {
	if (!h) return;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0 * (GLfloat)w / (GLfloat)h, 3, 700.0);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
	auxInitDisplayMode(AUX_DOUBLE | AUX_DEPTH16 | AUX_RGB);
	auxInitPosition(0, 0, 800, 600);
	auxKeyFunc(AUX_RIGHT, Zbor);
	auxKeyFunc(AUX_UP, Rotiref);
	auxKeyFunc(AUX_DOWN, Rotires);
	auxKeyFunc(AUX_s, mutaSursaFata);
	auxKeyFunc(AUX_w, mutaSursaSpate);
	auxKeyFunc(AUX_d, mutaSursaDreapta);
	auxKeyFunc(AUX_a, mutaSursaStanga);
	auxInitWindow("Umbra");
	myInit();
	auxReshapeFunc(myReshape);
	auxIdleFunc(IdleFunction);
	auxMainLoop(display);
	return 0;
}