#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>

#define EPS  (1e-9)
#define ISZERO(x) ((x) < EPS)
#define M_PI (3.14159265359)

const float DELAY = 10;

const double DELTA_T = 5;
const double ATRITO_MESA = 0.000009;
const double RAIO_BOLA = 1;
const double ATRITO_COLISAO = 0.000009;
const double MASSA_BOLAS = 15.0;
const double MASSA_BRANCA = 15.0;

const float COMPRIMENTO_MESA = 73;
const float LARGURA_MESA = 35;
const float ESPESSURA_MESA = 2;
const float TAM_BEIRADA = 3.0f;

const float esq = -COMPRIMENTO_MESA/2+TAM_BEIRADA;
const float dir = COMPRIMENTO_MESA/2-TAM_BEIRADA;
const float baixo = LARGURA_MESA/2-TAM_BEIRADA;
const float cima = -LARGURA_MESA/2+TAM_BEIRADA;

struct Vec2D {
  double x;
  double y;
};

struct Bola {
  int id;
  Vec2D pos;
  Vec2D vel;
  double accel;
  double massa;
};

const float cores[7][3] =   {
  {   1,    1,          1   },
  {   1,    1,          0   },
  {   0,    0,          1   },
  {   1,    0.65,       0   },
  {   0.5,  0,          0.5 },
  {   1,    0,          0   },
  {   0,    1,          0   },
};

const Vec2D posicao_inicial_bolas[7] = {
  { -COMPRIMENTO_MESA/2 + COMPRIMENTO_MESA/6, 0 },
  { COMPRIMENTO_MESA/2 - COMPRIMENTO_MESA/4.5, 0 },
  { COMPRIMENTO_MESA/2 - COMPRIMENTO_MESA/4.5 + 2, -1 },
  { COMPRIMENTO_MESA/2 - COMPRIMENTO_MESA/4.5 + 2, 1 },
  { COMPRIMENTO_MESA/2 - COMPRIMENTO_MESA/4.5 + 4, 2 },
  { COMPRIMENTO_MESA/2 - COMPRIMENTO_MESA/4.5 + 4, 0 },
  { COMPRIMENTO_MESA/2 - COMPRIMENTO_MESA/4.5 + 4, -2 },
};

const int num_bolas = 7;
Bola bolas[num_bolas];

void updatePos(Bola& b)
{
  b.pos.x += b.vel.x;
  b.pos.y += b.vel.y;
  //printf("id %d -- vely: %g - posy: %g / velx: %g - posx: %g -- accel: %g\n",
       //  b.id, b.vel.y, b.pos.y, b.vel.x, b.pos.x, b.accel);
}

void updateVeloc(Bola& b)
{
  b.vel.x = b.accel*b.vel.x;
  b.vel.y = b.accel*b.vel.y;
}

void updateAccel(Bola& b, double atrito)
{
  b.accel = b.accel * (1-atrito);
  if(ISZERO(b.accel)) {
    b.accel = 0;
    b.vel.x = 0;
    b.vel.y = 0;
  }
}

double distancia(Vec2D& a, Vec2D& b)
{
  double delta_x = b.x - a.x;
  double delta_y = b.y - a.y;
  return sqrt(delta_x*delta_x + delta_y*delta_y);
}

void snap(Bola& a, Bola& b)
{
  double dist = distancia(a.pos, b.pos);
  double delta = 1.05;
  double ang = atan2(a.pos.y - b.pos.y, a.pos.x - b.pos.x);

  double x = dist*delta*cos(ang);
  double y = dist*delta*sin(ang);

  a.pos.x = b.pos.x + x;
  a.pos.y = b.pos.y + y;
}

void onCollision(Bola& a, Bola& b)
{
  double novaVelAx = (a.vel.x * (a.massa - b.massa) + (2 * b.massa * b.vel.x)) / (a.massa + b.massa);
  double novaVelAy = (a.vel.y * (a.massa - b.massa) + (2 * b.massa * b.vel.y)) / (a.massa + b.massa);
  double novaVelBx = (b.vel.x * (b.massa - a.massa) + (2 * a.massa * a.vel.x)) / (a.massa + a.massa);
  double novaVelBy = (b.vel.y * (b.massa -  a.massa) + (2 * a.massa * a.vel.y)) / (a.massa + a.massa);

  double maxaccel = max(a.accel, b.accel);

  a.vel.x = novaVelAx;
  a.vel.y = novaVelAy;
  a.accel = maxaccel;
  updatePos(a);

  b.vel.x = novaVelBx;
  b.vel.y = novaVelBy;
  b.accel = maxaccel;
  updatePos(b);

  snap(a, b);
}

void checkCollision()
{
  for(Bola& a : bolas) {
    for(Bola& b : bolas) {
      if(a.id != b.id) {
        double dist = distancia(a.pos, b.pos);
        if(dist < 2 * RAIO_BOLA) {
          onCollision(a, b);
        }
      }
    }
  }
}

void checkCanto(Bola& b)
{
  if(b.pos.x - esq < RAIO_BOLA || dir - b.pos.x < RAIO_BOLA) {
    b.vel.x = -b.vel.x;
  }
  if(b.pos.y - baixo < RAIO_BOLA || cima - b.pos.y < RAIO_BOLA) {
    b.vel.y = -b.vel.y;
  }

  updatePos(b);
}

void updateWorld()
{
  checkCollision();

  for(Bola& b : bolas) {
    updateAccel(b, ATRITO_MESA);
    updateVeloc(b);
    updatePos(b);

    checkCanto(b);
  }
}

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
    // Percorre todos os vértices da face
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
  { 4, { 0, 1, 3, 2 } },  // 0 - Frente ?*
  { 4, { 4, 5, 7, 6 } },  // 1 - Trás ?
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

const float TAMANHO_SALA = 70.0f;

// Variáveis para controles de navegação
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
GLfloat luzAmbiente[4]= {0.2,0.2,0.2,1.0};

// Função responsável pela especificação dos parâmetros de iluminaç¿o
void DefineIluminacao(void)
{
  // Capacidade de brilho do material
  GLfloat especularidade[4]= {0.7,0.7,0.7,1.0};
  GLint especMaterial = 90;
  // Define a refletância do material
  glMaterialfv(GL_FRONT,GL_SPECULAR, especularidade);
  // Define a concentração do brilho
  glMateriali(GL_FRONT,GL_SHININESS,especMaterial);

  // Ativa o uso da luz ambiente
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);

  // Define os parâmetros das fontes de luz
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
  // Desenha FACE DE TRÁS
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

  /*glColor3f(0.95f, 0.95f, 0.95f);
  // Desenha FACE TOPO
  glBegin(GL_POLYGON);
  glVertex3f(-TAMANHO_SALA,TAMANHO_SALA,TAMANHO_SALA);
  glVertex3f(-TAMANHO_SALA, TAMANHO_SALA,-TAMANHO_SALA);
  glVertex3f(TAMANHO_SALA, TAMANHO_SALA, -TAMANHO_SALA);
  glVertex3f(TAMANHO_SALA, TAMANHO_SALA,TAMANHO_SALA);
  glEnd();*/

}

// Função para desenhar um "ch¿o" no ambiente
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
  /*Objeto pe_da_mesa = novo_paralelepido(comprimento, largura, altura);
  desenha_objeto(&pe_da_mesa);
  free_paralelepido(&pe_da_mesa);*/
  glPushMatrix();
  glScalef(comprimento, altura, largura);
  glutSolidCube(1.0);
  glPopMatrix();
}

void desenha_pe_mesa(float x, float z)
{
  glPushMatrix();
  glTranslatef(x, -TAMANHO_SALA + ALTURA_MESA/2, z);

  glColor3f(0.55, 0.34, 0.26);
  desenha_paralelepido(5, 5, ALTURA_MESA);

  glPopMatrix();
}

void desenha_pes_mesa()
{
  desenha_pe_mesa(-33, -12.5);
  desenha_pe_mesa(-33, 12.5);
  desenha_pe_mesa(0, -12.5);
  desenha_pe_mesa(0, 12.5);
  desenha_pe_mesa(33, -12.5);
  desenha_pe_mesa(33, 12.5);
}

void desenha_tampa_mesa()
{
  glPushMatrix();
  glTranslatef(0, ALTURA_MESA - TAMANHO_SALA, 0);

  glColor3f(0.55, 0.34, 0.26);
  desenha_paralelepido(COMPRIMENTO_MESA, LARGURA_MESA, ESPESSURA_MESA);

  glPopMatrix();
}

void desenha_beirada_mesa()
{
  glPushMatrix();
  glTranslatef(0, ALTURA_MESA-TAMANHO_SALA+2, LARGURA_MESA/2 - TAM_BEIRADA/2);
  desenha_paralelepido(COMPRIMENTO_MESA, TAM_BEIRADA, TAM_BEIRADA);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0, ALTURA_MESA-TAMANHO_SALA+2, -LARGURA_MESA/2 + TAM_BEIRADA/2);
  desenha_paralelepido(COMPRIMENTO_MESA, TAM_BEIRADA, TAM_BEIRADA);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(COMPRIMENTO_MESA/2 - TAM_BEIRADA/2, ALTURA_MESA-TAMANHO_SALA+2, 0);
  desenha_paralelepido(TAM_BEIRADA, LARGURA_MESA, TAM_BEIRADA);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-COMPRIMENTO_MESA/2 + TAM_BEIRADA/2, ALTURA_MESA-TAMANHO_SALA+2, 0);
  desenha_paralelepido(TAM_BEIRADA, LARGURA_MESA, TAM_BEIRADA);
  glPopMatrix();
}

void desenha_tabuleiro()
{
  const float y = ALTURA_MESA-TAMANHO_SALA+1.2;

  glColor3f(0, 1, 0);
  glBegin(GL_POLYGON);
  glVertex3f(esq, y, baixo);
  glVertex3f(esq, y, cima);
  glVertex3f(dir, y, cima);
  glVertex3f(dir, y, baixo);
  glEnd();
}

void desenha_mesa()
{
  desenha_pes_mesa();
  desenha_tampa_mesa();
  desenha_beirada_mesa();
  desenha_tabuleiro();
}

void desenha_bola(double x, double z)
{
  const double ALTURA_BOLA = -TAMANHO_SALA+ALTURA_MESA+2.5;
  glPushMatrix();
  glTranslated(x, ALTURA_BOLA, z);
  glutSolidSphere(1, 10, 10);
  glPopMatrix();
}

void desenha_bolas()
{
  for (int i = 0; i < num_bolas; i++) {
    glColor3fv(cores[i]);
    desenha_bola(bolas[i].pos.x, bolas[i].pos.y);
  }
}

void desenha_triangulo()
{
  const float y = ALTURA_MESA-TAMANHO_SALA+1.3;
  const float C = COMPRIMENTO_MESA;
  const float L = LARGURA_MESA;

  glColor3f(1, 1, 1);
  glBegin(GL_LINE_LOOP);
  glVertex3f(C/2 - C/7, y, -L/6);
  glVertex3f(C/2 - C/7, y, L/6);
  glVertex3f(C/2 - C/4, y, 0);
  glEnd();
}

void desenha_semicirculo()
{
  const float y = ALTURA_MESA-TAMANHO_SALA+1.6;
  const float raio = 7;
  const float delta = 0.001;

  glColor3f(1,1,1);

  glPushMatrix();
  glTranslatef(-COMPRIMENTO_MESA/2 + COMPRIMENTO_MESA/8, 0, 0);
  glBegin(GL_LINE_LOOP);
  for (float ang = 0.0; ang <= M_PI; ang += delta) {
    glVertex3f(raio*sin(ang), y, raio*cos(ang));
  }
  glEnd();
  glPopMatrix();
}

float taco_y = ALTURA_MESA-TAMANHO_SALA+1.6 + 9;
float taco_x = -4.5;

void desenha_taco()
{
  glPushMatrix();
  glTranslatef(-50 + taco_x, taco_y, 0);
  glRotatef(90, 0, 1, 0);
  glRotatef(15, 1, 0, 0);

  const float inc = 2*M_PI/40;
  const float raio_base = 0.7;
  const float raio_ponta = 0.5;
  const float tamanho = 30;

  glColor3f(1, 1, 0);

  glBegin(GL_QUAD_STRIP);
  for(GLfloat ang = 0; ang <= 2*M_PI; ang+=inc) {
    glVertex3f(raio_base*cos(ang),raio_base*sin(ang),0);
    glVertex3f(raio_ponta*cos(ang),raio_ponta*sin(ang),tamanho-1);
  }
  glEnd();

  glColor3f(0, 0, 0);
  glBegin(GL_QUAD_STRIP);
  for(GLfloat ang = 0; ang <= 2*M_PI; ang+=inc) {
    glVertex3f((raio_ponta+0.1)*cos(ang),(raio_ponta+0.1)*sin(ang),tamanho-0.8);
    glVertex3f(raio_ponta*cos(ang),raio_ponta*sin(ang),tamanho);
  }
  glEnd();

  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0,0,tamanho);
  for(GLfloat ang = 0; ang <= 2*M_PI; ang+=inc) {
    glVertex3f(raio_ponta*cos(ang),raio_ponta*sin(ang),tamanho);
  }
  glEnd();

  glutSolidTorus(raio_base-raio_ponta,raio_ponta,20,20);
  glPopMatrix();
}

// Funç¿o callback de redesenho da janela de visualização
void Desenha(void)
{
  // Limpa a janela de visualização com a cor
  // de fundo definida previamente
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  DefineIluminacao();

  DesenhaParede();
  desenha_chao();
  desenha_mesa();

  desenha_triangulo();
  desenha_semicirculo();
  desenha_bolas();
  desenha_taco();

  // Executa os comandos OpenGL
  glutSwapBuffers();
}

// Função usada para especificar a posição do observador virtual
void PosicionaObservador(void)
{
  // Especifica sistema de coordenadas do modelo
  glMatrixMode(GL_MODELVIEW);
  // Inicializa sistema de coordenadas do modelo
  glLoadIdentity();
  // Posiciona e orienta o observador
  glTranslatef(-obsX,-obsY,-obsZ);

  glRotatef(rotX,1,0,0);
  glRotatef(rotY,0,1,0);
}

// Função usada para especificar o volume de visualização
void EspecificaParametrosVisualizacao(void)
{
  // Especifica sistema de coordenadas de projeção
  glMatrixMode(GL_PROJECTION);
  // Inicializa sistema de coordenadas de projeção
  glLoadIdentity();

  // Especifica a projeção perspectiva(angulo,aspecto,zMin,zMax)
  gluPerspective(angle,fAspect,0.5,500);

  PosicionaObservador();
}

// Função callback para tratar eventos de teclas especiais
void TeclasEspeciais(int tecla, int x, int y)
{
  switch(tecla) {

  }
  PosicionaObservador();
  glutPostRedisplay();
}

// Funç¿o callback para eventos de botões do mouse
void GerenciaMouse(int button, int state, int x, int y)
{
  if(state==GLUT_DOWN) {
    // Salva os parâmetros atuais
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

// Função callback para eventos de movimento do mouse
#define SENS_ROT  5.0
#define SENS_OBS  10.0
#define SENS_TRANSL 10.0
void GerenciaMovim(int x, int y)
{
  // Botão esquerdo ?
  if(bot==GLUT_LEFT_BUTTON) {
    // Calcula diferenças
    int deltax = x_ini - x;
    int deltay = y_ini - y;
    // E modifica ângulos
    rotY = rotY_ini - deltax/SENS_ROT;
    rotX = rotX_ini - deltay/SENS_ROT;
  }
  // Botão direito ?
  else if(bot==GLUT_RIGHT_BUTTON) {
    // Calcula diferença
    int deltaz = y_ini - y;
    // E modifica distância do observador
    obsZ = obsZ_ini + deltaz/SENS_OBS;
  }
  // Botão do meio ?
  else if(bot==GLUT_MIDDLE_BUTTON) {
    // Calcula diferenças
    int deltax = x_ini - x;
    int deltay = y_ini - y;
    // E modifica posições
    obsX = obsX_ini + deltax/SENS_TRANSL;
    obsY = obsY_ini - deltay/SENS_TRANSL;
  }
  PosicionaObservador();
  glutPostRedisplay();
}

// Função callback chamada quando o tamanho da janela é alterado
void AlteraTamanhoJanela(GLsizei w, GLsizei h)
{
  // Para previnir uma divisão por zero
  if(h == 0) h = 1;

  // Especifica as dimensões da viewport
  glViewport(0, 0, w, h);

  // Calcula a correção de aspecto
  fAspect = (GLfloat)w/(GLfloat)h;

  EspecificaParametrosVisualizacao();
}

void inicializa_mundo()
{
  for (int i = 0; i < num_bolas; i++) {
    bolas[i].id = i;
    bolas[i].accel = 0;
    bolas[i].vel.x = 0;
    bolas[i].vel.y = 0;
    bolas[i].massa = MASSA_BOLAS + 2*i;
    bolas[i].pos = posicao_inicial_bolas[i];
  }
  bolas[0].massa = MASSA_BRANCA;
  bolas[0].vel.x = 0.3;
  bolas[0].accel = 1;
}

// Função responsável por inicializar parâmetros e variáveis
void Inicializa(void)
{
  // Define a cor de fundo da janela de visualização como branca
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // Habilita a definição da cor do material a partir da cor corrente
  glEnable(GL_COLOR_MATERIAL);
  //Habilita o uso de iluminação
  glEnable(GL_LIGHTING);
  // Habilita as fontes de luz
  glEnable(GL_LIGHT0);
  // Habilita o depth-buffering
  glEnable(GL_DEPTH_TEST);

  // Habilita o modelo de colorização de Gouraud
  glShadeModel(GL_SMOOTH);

  // Inicializa a variável que especifica o ângulo da projeção
  // perspectiva
  angle=80;

  // Inicializa as variáveis usadas para alterar a posição do
  // observador virtual
  rotX = 46;
  rotY = 190;
  obsX = -0.2;
  obsY = -36;
  obsZ = 37;

  inicializa_mundo();
}

bool animacao = false;

void anima(int)
{
  updateWorld();
  glutPostRedisplay();

  if (animacao)
    glutTimerFunc(DELAY, anima, 0);
}

static float tempo_anima_taco = 0;
void anima_taco(int)
{
  const float max_tempo = 10;
  const float delta = 0.3;

  tempo_anima_taco += delta;
  if (tempo_anima_taco >= 2 * max_tempo) {
    animacao = true;
    glutTimerFunc(DELAY, anima, 0);
    return;
  } else if (tempo_anima_taco >= max_tempo) {
    taco_x += delta;
    taco_y -= delta;
  } else {
    taco_x -= delta;
    taco_y += delta;
  }
  glutPostRedisplay();
  glutTimerFunc(DELAY, anima_taco, 0);
}

// Função callback chamada para gerenciar eventos de teclas normais (ESC)
void Teclado(unsigned char tecla, int x, int y)
{
  if (tecla == 27) // ESC ?
    exit(0);
  if (tecla == 'x') {
    tempo_anima_taco = 0;
    animacao = false;
    glutTimerFunc(1000, anima_taco, 0);
    inicializa_mundo();
  }
}

// Programa Principal
int main(void)
{
  // Define o modo de operação da GLUT
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  // Especifica a posição inicial da janela GLUT
  glutInitWindowPosition(5,5);

  // Especifica o tamanho inicial em pixels da janela GLUT
  glutInitWindowSize(1350,700);

  // Cria a janela passando como argumento o título da mesma
  glutCreateWindow("Desenho de um teapot iluminado por spots");

  // Registra a função callback de redesenho da janela de visualização
  glutDisplayFunc(Desenha);

  // Registra a função callback de redimensionamento da janela de visualização
  glutReshapeFunc(AlteraTamanhoJanela);

  // Registra a função callback para tratamento das teclas normais
  glutKeyboardFunc(Teclado);

  // Registra a função callback para tratamento das teclas especiais
  glutSpecialFunc(TeclasEspeciais);

  // Registra a função callback para eventos de botões do mouse
  glutMouseFunc(GerenciaMouse);

  // Registra a função callback para eventos de movimento do mouse
  glutMotionFunc(GerenciaMovim);

  glutTimerFunc(1000, anima_taco, 0);

  // Chama a função responsável por fazer as inicializações
  Inicializa();

  // Inicia o processamento e aguarda interação do usuário
  glutMainLoop();

  return 0;
}

