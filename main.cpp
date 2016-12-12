#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_VERTICES_POR_FACE 4

struct Vertice {
  float x, y, z;
};

struct Face {
  int num_vertices;
  int vertices[MAX_VERTICES_POR_FACE];
};

struct Objeto {
  Vertice* vertices;
  const Face* faces;
  int num_faces;
};

void desenha_obj_modo(Objeto* objeto, GLenum modo)
{
  // Percorre todas as faces
  for(int f = 0; f < objeto->num_faces; f++) {
    glBegin(modo);
    // Percorre todos os v�rtices da face
    for(int v = 0; v < objeto->faces[f].num_vertices; v++) {
      Vertice* vertice = &objeto->vertices[objeto->faces[f].vertices[v]];
      glVertex3f(vertice->x, vertice->y, vertice->z);
    }
    glEnd();
  }
}

void desenha_objeto(Objeto* objeto)
{
    desenha_obj_modo(objeto, GL_POLYGON);
    glPushAttrib(GL_CURRENT_BIT);
    glColor3f(0, 0, 0);
    glLineWidth(0.5);
    desenha_obj_modo(objeto, GL_LINE_LOOP);
    glPopAttrib();
}

const Face faces_paralelepido[] = {
  { 4, { 0, 2, 3, 1 } },  // 0 - Frente ?
  { 4, { 4, 5, 7, 6 } },  // 1 - Tr�s ?
  { 4, { 4, 5, 1, 0 } },  // 2 - Baixo ?
  { 4, { 2, 3, 7, 6 } },  // 3 - Cima
  { 4, { 2, 6, 4, 0 } },  // 4 - Esquerda ?
  { 4, { 3, 1, 5, 7 } }   // 5 - Direita
};

Objeto novo_paralelepido(float comprimento, float largura, float altura)
{
  Vertice* vertices = new Vertice[8];
  vertices[0] = {0.0f, 0.0f, 0.0f};               // 0
  vertices[1] = {comprimento, 0.0f, 0.0f};        // 1
  vertices[2] = {0.0f, altura, 0.0f};             // 2
  vertices[3] = {comprimento, altura, 0.0f};      // 3
  vertices[4] = {0.0f, 0.0f, -largura};           // 4
  vertices[5] = {comprimento, 0.0f, -largura};    // 5
  vertices[6] = {0.0f, altura, -largura};         // 6
  vertices[7] = {comprimento, altura, -largura};  // 7

  Objeto paralelepido = {
    vertices,
    faces_paralelepido,
    6
  };

  return paralelepido;
}

void free_paralelepido(Objeto* paralelepido)
{
  free(paralelepido->vertices);
}

const float TAMANHO_SALA = 30.0f;

// Vari�veis para controles de navega��o
GLfloat angle, fAspect;
GLfloat rotX, rotY, rotX_ini, rotY_ini;
GLfloat obsX, obsY, obsZ, obsX_ini, obsY_ini, obsZ_ini;
int x_ini,y_ini,bot;

// Luz selecionada
int luz = 0;

GLfloat posLuz[4] = {  0, 70,  0, 1 };
GLfloat dirLuz[3] = { 0,-1,0 };
GLfloat luzDifusa[4] = { 0.4,0.4,0.4,1 };
GLfloat luzEspecular[4] = { 0.7,0.7,0.7,1 };
GLfloat luzAmbiente[4]= {0.0,0.0,0.0,1.0};

// Fun��o respons�vel pela especifica��o dos par�metros de ilumina�o
void DefineIluminacao(void)
{
  // Capacidade de brilho do material
  GLfloat especularidade[4]= {0.7,0.7,0.7,1.0};
  GLint especMaterial = 90;
  // Define a reflet�ncia do material
  glMaterialfv(GL_FRONT,GL_SPECULAR, especularidade);
  // Define a concentra��o do brilho
  glMateriali(GL_FRONT,GL_SHININESS,especMaterial);

  // Ativa o uso da luz ambiente
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);

  // Define os par�metros das fontes de luz
  glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);
  glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);
  glLightfv(GL_LIGHT0, GL_POSITION, posLuz);
  glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,dirLuz);
  glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,80.0);
  glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,5.0);
}

void DesenhaParede(void)
{
  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(0.75f, 0.75f, 0.75f);
  // Desenha FACE DE TR�S
  glBegin(GL_POLYGON);
  glVertex3f(-TAMANHO_SALA,TAMANHO_SALA,-TAMANHO_SALA);
  glVertex3f(-TAMANHO_SALA, -TAMANHO_SALA,-TAMANHO_SALA);
  glVertex3f(TAMANHO_SALA, -TAMANHO_SALA, -TAMANHO_SALA);
  glVertex3f(TAMANHO_SALA,  TAMANHO_SALA,-TAMANHO_SALA);
  glEnd();

  glColor3f(0.65f, 0.65f, 0.65f);
  // Desenha FACE DA FRENTE
  glBegin(GL_POLYGON);
  glVertex3f(-TAMANHO_SALA,TAMANHO_SALA,TAMANHO_SALA);
  glVertex3f(-TAMANHO_SALA, -TAMANHO_SALA,TAMANHO_SALA);
  glVertex3f(TAMANHO_SALA, -TAMANHO_SALA, TAMANHO_SALA);
  glVertex3f(TAMANHO_SALA,  TAMANHO_SALA,TAMANHO_SALA);
  glEnd();

  glColor3f(0.85f, 0.85f, 0.85f);
  // Desenha FACE D0 LADO ESQUERDO
  glBegin(GL_POLYGON);
  glVertex3f(-TAMANHO_SALA,TAMANHO_SALA,-TAMANHO_SALA);
  glVertex3f(-TAMANHO_SALA, TAMANHO_SALA,TAMANHO_SALA);
  glVertex3f(-TAMANHO_SALA, -TAMANHO_SALA, TAMANHO_SALA);
  glVertex3f(-TAMANHO_SALA, -TAMANHO_SALA,-TAMANHO_SALA);
  glEnd();

  glColor3f(0.55f, 0.55f, 0.55f);
  // Desenha FACE D0 LADO DIREITO
  glBegin(GL_POLYGON);
  glVertex3f(TAMANHO_SALA,TAMANHO_SALA,-TAMANHO_SALA);
  glVertex3f(TAMANHO_SALA, TAMANHO_SALA,TAMANHO_SALA);
  glVertex3f(TAMANHO_SALA, -TAMANHO_SALA, TAMANHO_SALA);
  glVertex3f(TAMANHO_SALA, -TAMANHO_SALA,-TAMANHO_SALA);
  glEnd();

  glColor3f(0.95f, 0.95f, 0.95f);
  // Desenha FACE TOPO
  glBegin(GL_POLYGON);
    glVertex3f(-TAMANHO_SALA,TAMANHO_SALA,TAMANHO_SALA);
    glVertex3f(-TAMANHO_SALA, TAMANHO_SALA,-TAMANHO_SALA);
    glVertex3f(TAMANHO_SALA, TAMANHO_SALA, -TAMANHO_SALA);
    glVertex3f(TAMANHO_SALA, TAMANHO_SALA,TAMANHO_SALA);
  glEnd();

}

// Fun��o para desenhar um "ch�o" no ambiente
void desenha_chao(void)
{
  const float D = 1.0f;

  // Flags para determinar a cor de cada quadrado
  bool flagx,flagz;

  // Define a normal apontando para cima
  glNormal3f(0,1,0);
  glBegin(GL_QUADS);
  flagx = false;

  // X varia de -TAM a TAM, de D em D
  for(float x=-TAMANHO_SALA; x<TAMANHO_SALA; x+=D) {
    // Flagx determina a cor inicial
    if(flagx)
      flagz = false;
    else
      flagz = true;

    // Z varia de -TAM a TAM, de D em D
    for(float z=-TAMANHO_SALA; z<TAMANHO_SALA; z+=D) {
      // Escolhe cor
      if(flagz)
        glColor3f(0.4,0.4,0.4);
      else
        glColor3f(1,1,1);

      // E desenha o quadrado
      glVertex3f(x,-TAMANHO_SALA,z);
      glVertex3f(x+D,-TAMANHO_SALA,z);
      glVertex3f(x+D,-TAMANHO_SALA,z+D);
      glVertex3f(x,-TAMANHO_SALA,z+D);

      // Alterna cor
      flagz=!flagz;
    }

    // A cada coluna, alterna cor inicial
    flagx=!flagx;
  }
  glEnd();
}

const float ALTURA_MESA = 15.0f;

void desenha_paralelepido(float comprimento, float largura, float altura)
{
  Objeto pe_da_mesa = novo_paralelepido(comprimento, largura, altura);
  desenha_objeto(&pe_da_mesa);
  free_paralelepido(&pe_da_mesa);
}

void desenha_pe_mesa(float x, float z)
{
  glPushMatrix();
  glTranslatef(x, -TAMANHO_SALA, z);

  glColor3f(0.55, 0.34, 0.26);
  desenha_paralelepido(5, 5, ALTURA_MESA);

  glPopMatrix();
}

void desenha_pes_mesa()
{
  desenha_pe_mesa(0, 5);
  desenha_pe_mesa(0, -15);
  desenha_pe_mesa(0, 20);
  desenha_pe_mesa(10, 5);
  desenha_pe_mesa(10, -15);
  desenha_pe_mesa(10, 20);
}

void desenha_tampa_mesa()
{
  glPushMatrix();
  glTranslatef(-3, ALTURA_MESA - TAMANHO_SALA, 23);

  glColor3f(0.55, 0.34, 0.26);
  desenha_paralelepido(20, 45, 2);

  glPopMatrix();
}

void desenha_beirada_mesa()
{
  const float altura_beirada = 3.0f;

  glPushMatrix();
  glTranslatef(-3, ALTURA_MESA-TAMANHO_SALA+2, 21);
  desenha_paralelepido(2, 41, altura_beirada);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(15, ALTURA_MESA-TAMANHO_SALA+2, 21);
  desenha_paralelepido(2, 41, altura_beirada);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-3, ALTURA_MESA-TAMANHO_SALA+2, -20);
  desenha_paralelepido(20, 2, altura_beirada);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-3, ALTURA_MESA-TAMANHO_SALA+2, 23);
  desenha_paralelepido(20, 2, altura_beirada);
  glPopMatrix();
}

void desenha_tabuleiro()
{
  const float y = ALTURA_MESA-TAMANHO_SALA+2.5;

  glColor3f(0, 1, 0);
  glBegin(GL_POLYGON);
  glVertex3f(-3, y, 23);
  glVertex3f(17, y, 23);
  glVertex3f(17, y, -20);
  glVertex3f(-3, y, -20);
  glEnd();
}

void desenha_mesa()
{
  desenha_pes_mesa();
  desenha_tampa_mesa();
  desenha_beirada_mesa();
  desenha_tabuleiro();
}

// Fun�o callback de redesenho da janela de visualiza��o
void Desenha(void)
{
  // Limpa a janela de visualiza��o com a cor
  // de fundo definida previamente
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  DefineIluminacao();

  DesenhaParede();
  desenha_chao();
  desenha_mesa();

  // Executa os comandos OpenGL
  glutSwapBuffers();
}

// Fun��o usada para especificar a posi��o do observador virtual
void PosicionaObservador(void)
{
  // Especifica sistema de coordenadas do modelo
  glMatrixMode(GL_MODELVIEW);
  // Inicializa sistema de coordenadas do modelo
  glLoadIdentity();
  // Posiciona e orienta o observador
  glTranslatef(-obsX,-obsY,-obsZ);

  printf("%f %f %f\n", obsX, obsY, obsZ);

  glRotatef(rotX,1,0,0);
  glRotatef(rotY,0,1,0);
}

// Fun��o usada para especificar o volume de visualiza��o
void EspecificaParametrosVisualizacao(void)
{
  // Especifica sistema de coordenadas de proje��o
  glMatrixMode(GL_PROJECTION);
  // Inicializa sistema de coordenadas de proje��o
  glLoadIdentity();

  // Especifica a proje��o perspectiva(angulo,aspecto,zMin,zMax)
  gluPerspective(angle,fAspect,0.5,500);

  PosicionaObservador();
}

// Fun��o callback chamada para gerenciar eventos de teclas normais (ESC)
void Teclado(unsigned char tecla, int x, int y)
{
  if (tecla == 27) // ESC ?
    exit(0);
}

// Fun��o callback para tratar eventos de teclas especiais
void TeclasEspeciais(int tecla, int x, int y)
{
  switch(tecla) {

  }
  PosicionaObservador();
  glutPostRedisplay();
}

// Fun�o callback para eventos de bot�es do mouse
void GerenciaMouse(int button, int state, int x, int y)
{
  if(state==GLUT_DOWN) {
    // Salva os par�metros atuais
    x_ini = x;
    y_ini = y;
    obsX_ini = obsX;
    obsY_ini = obsY;
    obsZ_ini = obsZ;
    rotX_ini = rotX;
    rotY_ini = rotY;
    bot = button;
  } else bot = -1;
}

// Fun��o callback para eventos de movimento do mouse
#define SENS_ROT  5.0
#define SENS_OBS  10.0
#define SENS_TRANSL 10.0
void GerenciaMovim(int x, int y)
{
  // Bot�o esquerdo ?
  if(bot==GLUT_LEFT_BUTTON) {
    // Calcula diferen�as
    int deltax = x_ini - x;
    int deltay = y_ini - y;
    // E modifica �ngulos
    rotY = rotY_ini - deltax/SENS_ROT;
    rotX = rotX_ini - deltay/SENS_ROT;
  }
  // Bot�o direito ?
  else if(bot==GLUT_RIGHT_BUTTON) {
    // Calcula diferen�a
    int deltaz = y_ini - y;
    // E modifica dist�ncia do observador
    obsZ = obsZ_ini + deltaz/SENS_OBS;
  }
  // Bot�o do meio ?
  else if(bot==GLUT_MIDDLE_BUTTON) {
    // Calcula diferen�as
    int deltax = x_ini - x;
    int deltay = y_ini - y;
    // E modifica posi��es
    obsX = obsX_ini + deltax/SENS_TRANSL;
    obsY = obsY_ini - deltay/SENS_TRANSL;
  }
  PosicionaObservador();
  glutPostRedisplay();
}

// Fun��o callback chamada quando o tamanho da janela � alterado
void AlteraTamanhoJanela(GLsizei w, GLsizei h)
{
  // Para previnir uma divis�o por zero
  if(h == 0) h = 1;

  // Especifica as dimens�es da viewport
  glViewport(0, 0, w, h);

  // Calcula a corre��o de aspecto
  fAspect = (GLfloat)w/(GLfloat)h;

  EspecificaParametrosVisualizacao();
}

// Fun��o respons�vel por inicializar par�metros e vari�veis
void Inicializa(void)
{
  // Define a cor de fundo da janela de visualiza��o como branca
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // Habilita a defini��o da cor do material a partir da cor corrente
  glEnable(GL_COLOR_MATERIAL);
  //Habilita o uso de ilumina��o
  glEnable(GL_LIGHTING);
  // Habilita as fontes de luz
  glEnable(GL_LIGHT0);
  // Habilita o depth-buffering
  glEnable(GL_DEPTH_TEST);

  // Habilita o modelo de coloriza��o de Gouraud
  glShadeModel(GL_SMOOTH);

  // Inicializa a vari�vel que especifica o �ngulo da proje��o
  // perspectiva
  angle=60;

  // Inicializa as vari�veis usadas para alterar a posi��o do
  // observador virtual
  rotX = 50;
  rotY = 90;
  obsX = -4.5;
  obsY = 2.3;
  obsZ = 33;
}

// Programa Principal
int main(void)
{
  // Define o modo de opera��o da GLUT
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  // Especifica a posi��o inicial da janela GLUT
  glutInitWindowPosition(5,5);

  // Especifica o tamanho inicial em pixels da janela GLUT
  glutInitWindowSize(750,450);

  // Cria a janela passando como argumento o t�tulo da mesma
  glutCreateWindow("Desenho de um teapot iluminado por spots");

  // Registra a fun��o callback de redesenho da janela de visualiza��o
  glutDisplayFunc(Desenha);

  // Registra a fun��o callback de redimensionamento da janela de visualiza��o
  glutReshapeFunc(AlteraTamanhoJanela);

  // Registra a fun��o callback para tratamento das teclas normais
  glutKeyboardFunc(Teclado);

  // Registra a fun��o callback para tratamento das teclas especiais
  glutSpecialFunc(TeclasEspeciais);

  // Registra a fun��o callback para eventos de bot�es do mouse
  glutMouseFunc(GerenciaMouse);

  // Registra a fun��o callback para eventos de movimento do mouse
  glutMotionFunc(GerenciaMovim);

  // Chama a fun��o respons�vel por fazer as inicializa��es
  Inicializa();

  // Inicia o processamento e aguarda intera��o do usu�rio
  glutMainLoop();

  return 0;
}

