/*!\file window.c
 * \brief géométries lumière diffuse, transformations de base et chargement de
 * textures en GL4Dummies \author Farès BELHADJ, amsi@ai.univ-paris8.fr \date
 * May 13 2018 */
#include <stdio.h>
//#include <GL4D/gl4du.h>
//#include <GL4D/gl4df.h>
#include <GL4D/gl4duw_SDL2.h>
#include <SDL_image.h>
#include <assert.h>

/* Prototypes des fonctions statiques contenues dans ce fichier C */
static void init(void);
static void loadTexture(GLuint id, const char *filename);
static void resize(int w, int h);
static void draw(void);
static void quit(void);
/*!\brief dimensions de la fenêtre */
static int _wW = 1600, _wH = 1000;
/*!\brief identifiant du programme GLSL */
static GLuint _pId = 0;
/*!\brief quelques objets géométriques */
static GLuint _ball = 0, _sphere = 0, _chap = 0, _spout = 0, _handle = 0, _quad = 0,
              _tex[4] = {0};
/*!\brief La fonction principale créé la fenêtre d'affichage,
 * initialise GL et les données, affecte les fonctions d'événements et
 * lance la boucle principale d'affichage.*/
int main(int argc, char **argv) {
  if (!gl4duwCreateWindow(argc, argv, "GL4Dummies", 0, 0, _wW, _wH,
                          GL4DW_RESIZABLE | GL4DW_SHOWN))
    return 1;
  init();
  atexit(quit);
  gl4duwResizeFunc(resize);
  gl4duwDisplayFunc(draw);
  gl4duwMainLoop();
  return 0;
}
/*!\brief initialise les paramètres OpenGL et les données */
static void init(void) {

gl4dgSetGeometryOptimizationLevel(2);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.7f, 0.5f, 1.0f, 1.0f);
  _pId = gl4duCreateProgram("<vs>shaders/depTex.vs", "<fs>shaders/depTex.fs",
                            NULL);
  gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  resize(_wW, _wH);
  //_sphere = gl4dgGenTeapotf(3, 3);
  _quad = gl4dgGenQuadf();
  /* génération de plusieurs identifiants de texture */
  glGenTextures(sizeof _tex / sizeof *_tex, _tex);
  /* chargement et transfert des images dans des textures OpenGL */
  // loadTexture(_tex[0], "images/teapot_motifs.jpg");
  loadTexture(_tex[0], "images/drap_02.jpg");
  loadTexture(_tex[1], "images/teapot_motifs.jpg");
  loadTexture(_tex[2], "images/metal_02.jpeg");

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  /* une dernière texture vide pour y faire des calculs d'effets */
  glBindTexture(GL_TEXTURE_2D, _tex[3]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _wW / 2, _wH, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glEnable(GL_TEXTURE_2D);
}
static void loadTexture(GLuint id, const char *filename) {
  SDL_Surface *t;
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  if ((t = IMG_Load(filename)) != NULL) {
#ifdef __APPLE__
    int mode = t->format->BytesPerPixel == 4 ? GL_BGRA : GL_BGR;
#else
    int mode = t->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, mode,
                 GL_UNSIGNED_BYTE, t->pixels);
    SDL_FreeSurface(t);
  } else {
    fprintf(stderr, "can't open file %s : %s\n", filename, SDL_GetError());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 NULL);
  }
}
/*!\brief Cette fonction paramétre la vue (viewport) OpenGL en
 * fonction des dimensions de la fenêtre.*/
static void resize(int w, int h) {
  _wW = w;
  _wH = h;
  glViewport(0, 0, _wW, _wH);
  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  gl4duFrustumf(-0.5, 0.5, -0.5 * _wH / _wW, 0.5 * _wH / _wW, 1.0, 1000.0);
  gl4duBindMatrix("modelViewMatrix");
}
/*!\brief dessine dans le contexte OpenGL actif. */
static void draw(void) {
  static GLfloat angle = 0.0f;
  static GLfloat a = 0;
  static GLfloat nb_faces = 15.0f;
  static GLfloat pas_chap = 0.0f;
  static GLfloat sens_chap = 0.0015;
  _ball = gl4dgGenSpheref((int)nb_faces, (int)nb_faces);
  _chap = gl4dgGenTeapotChapf_V2((int)nb_faces, (int)nb_faces);
  _sphere = gl4dgGenTeapotf_V2((int)nb_faces, (int)nb_faces);
  _spout = gl4dgGenTeapotSpoutf_V2((int)nb_faces, (int)nb_faces, 1);
  _handle = gl4dgGenTeapotSpoutf_V2((int)nb_faces, (int)nb_faces, 0);
  // nb_faces += 0.003;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  glUseProgram(_pId);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(_pId, "tex"), 0);
  gl4duTranslatef(0, 0, -8.0);
  gl4duPushMatrix();
  {
    gl4duTranslatef(0, -2.0, 0.0);
    gl4duRotatef(-90, 1, 0, 0);
    gl4duScalef(3.5f, 3, 3);
    gl4duSendMatrices();
  }
  gl4duPopMatrix();
  glBindTexture(GL_TEXTURE_2D, _tex[0]);
  // gl4dgDraw(_quad);


  gl4duPopMatrix();
  gl4duTranslatef(0, -1, 0);
  gl4duRotatef(angle, 0, 1, 0);
  gl4duSendMatrices();
  glBindTexture(GL_TEXTURE_2D, _tex[1]);
  gl4dgDraw(_sphere);

  gl4duPopMatrix();
  gl4duScalef(0.6, 0.6, 0.6);
  gl4duTranslatef(0, 3.53 /*+ pas_chap*/, 0);
  gl4duRotatef(angle, 0, 1, 0);
  gl4duSendMatrices();
  glBindTexture(GL_TEXTURE_2D, _tex[2]);
  gl4dgDraw(_chap);

  gl4duPopMatrix();
  gl4duTranslatef(0, -1, 0);
  gl4duRotatef(angle, 0, 1, 0);
  gl4duSendMatrices();
  glBindTexture(GL_TEXTURE_2D, _tex[1]);
  gl4dgDraw(_ball);

  gl4duPopMatrix();
  gl4duScalef(0.25, 0.25, 0.25);
  gl4duTranslatef(0, -1.5, 0);
  gl4duRotatef(angle, 0, 1, 0);
  gl4duTranslatef(3, -13, 0);
  gl4duSendMatrices();
  glBindTexture(GL_TEXTURE_2D, _tex[2]);
  gl4dgDraw(_spout);

  gl4duPopMatrix();
  gl4duScalef(1.25, 1.25, 1.25);
  gl4duTranslatef(0, -1.5, 0);
  gl4duRotatef(angle, 0, 1, 0);
  gl4duTranslatef(3, -5, 0);
  gl4duSendMatrices();
  glBindTexture(GL_TEXTURE_2D, _tex[2]);
  gl4dgDraw(_handle);

  pas_chap += sens_chap;
  if (pas_chap > 0.5)
    sens_chap *= -1;
  if (pas_chap < 0.0)
    sens_chap *= -1;
  angle += (1.0f / 1000.0f) * 360.0f;
}
/*!\brief appelée au moment de sortir du programme (atexit), libère les éléments
 * utilisés */
static void quit(void) {
  /* suppression de plusieurs identifiants de texture */
  glDeleteTextures(sizeof _tex / sizeof *_tex, _tex);
  gl4duClean(GL4DU_ALL);
}
