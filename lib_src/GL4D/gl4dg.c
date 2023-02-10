/*!\file gl4dg.c
 *
 * \brief The GL4Dummies Geometry
 *
 * \author Fares BELHADJ amsi@ai.univ-paris8.fr
 * \date February 22, 2016
 */

#if defined(_MSC_VER)
#  define _USE_MATH_DEFINES
#endif
#include <math.h>
#include "linked_list.h"
#include "gl4dg.h"
#include "gl4dm.h"
#include <stdlib.h>
#include <assert.h>

/*!\brief permet de sélectionner une topologie à utiliser selon le
 * niveau d'optimisation géométrique choisie. */
#define SELECT_GEOMETRY_OPTIMIZATION(index, geom, w, h) do {		\
    switch(_geometry_optimization_level) {				\
    case 0:								\
      (index) = mkRegularGridTriangleIndices((w), (h));			\
      (geom)->index_mode = GL_TRIANGLES;				\
      (geom)->index_row_count = 6 * ((w) - 1) * ((h) - 1);		\
      (geom)->index_nb_rows = 1;					\
      break;								\
    case 1:								\
      (index) = mkRegularGridStripsIndices((w), (h));			\
      (geom)->index_mode = GL_TRIANGLE_STRIP;				\
      (geom)->index_row_count = 2 * (w);				\
      (geom)->index_nb_rows = ((h) - 1);				\
      break;								\
    case 2:								\
      index = mkRegularGridStripIndices((w), (h));			\
      (geom)->index_mode = GL_TRIANGLE_STRIP;				\
      (geom)->index_row_count = 2 * (w) * ((h) - 1);			\
      (geom)->index_nb_rows = 1;					\
      break;								\
    case 3:								\
      index = mkRegularGridTriangleAdjacencyIndices((w), (h));		\
      (geom)->index_mode = GL_TRIANGLES_ADJACENCY;			\
      (geom)->index_row_count = 12 * ((w) - 1) * ((h) - 1);		\
      (geom)->index_nb_rows = 1;					\
      break;								\
    default: /* GL_TRIANGLE_STRIP_ADJACENCY */				\
      (index) = mkRegularGridStripsAdjacencyIndices((w), (h));		\
      (geom)->index_mode = GL_TRIANGLE_STRIP_ADJACENCY;			\
      (geom)->index_row_count = 4 * (w);				\
      (geom)->index_nb_rows = ((h) - 1);				\
      break;								\
    }									\
  } while(0)

/*!\brief dessine un element-array selon la topologie de l'objet. */
#define DRAW_WITH_GEOMETRY_OPTIMIZATION(geom) do {			\
    int i, d;								\
    for(i = 0, d = 0; i <  (geom)->index_nb_rows; ++i) {		\
      glDrawElements((geom)->index_mode, (geom)->index_row_count, GL4D_VAO_INDEX, (const GLvoid *)(intptr_t)d); \
      d += (geom)->index_row_count * sizeof(GL4Dvaoindex);		\
    }									\
  } while(0)

typedef struct geom_t geom_t;
typedef struct gsphere_t gsphere_t;
typedef struct gstatic_t gstatic_t;
typedef struct gcone_t gcone_t;
typedef struct gcylinder_t gcylinder_t;
typedef struct gdisk_t gdisk_t;
typedef struct gtorus_t gtorus_t;
typedef struct ggrid2d_t ggrid2d_t;
typedef struct gteapot_t gteapot_t;
typedef enum   geom_e geom_e;

enum geom_e {
  GE_NONE = 0,
  GE_SPHERE,
  GE_QUAD,
  GE_CUBE,
  GE_CONE,
  GE_FAN_CONE,
  GE_CYLINDER,
  GE_DISK,
  GE_TORUS,
  GE_GRID2D,
  GE_TEAPOT
};

struct geom_t {
  GLuint id, vao;
  geom_e type;
  void * geom;
};

struct gsphere_t {
  GLuint buffers[2];
  GLuint slices, stacks;
  GLenum index_mode;
  GLsizei index_row_count;
  GLsizei index_nb_rows;
};

struct gstatic_t {
  GLuint buffer;
};

struct gcone_t {
  GLuint buffer;
  GLuint slices;
  GLboolean base;
};

struct gcylinder_t {
  GLuint buffer;
  GLuint slices;
  GLboolean base;
};

struct gdisk_t {
  GLuint buffer;
  GLuint slices;
};

struct gtorus_t {
  GLuint buffers[2];
  GLuint slices, stacks;
  GLdouble radius;
  GLenum index_mode;
  GLsizei index_row_count;
  GLsizei index_nb_rows;
};

struct ggrid2d_t {
  GLuint buffers[2];
  GLuint width, height;
  GLenum index_mode;
  GLsizei index_row_count;
  GLsizei index_nb_rows;
};

struct gteapot_t {
  GLuint buffer;
  GLuint slices;
};

static geom_t * _garray = NULL;
static GLint _garray_size = 256;
static linked_list_t * _glist = NULL;
static int _hasInit = 0;
static GLuint _geometry_optimization_level = 1;

static void            freeGeom(void * data);
static GLuint          genId(void);
static GLuint          mkStaticf(geom_e type);
static GLfloat       * mkSphereVerticesf(GLuint slices, GLuint stacks);
static GL4Dvaoindex  * mkRegularGridTriangleIndices(GLuint width, GLuint height);
static GL4Dvaoindex  * mkRegularGridStripsIndices(GLuint width, GLuint height);
static GL4Dvaoindex  * mkRegularGridStripIndices(GLuint width, GLuint height);
static GL4Dvaoindex  * mkRegularGridTriangleAdjacencyIndices(GLuint width, GLuint height);
static GL4Dvaoindex  * mkRegularGridStripsAdjacencyIndices(GLuint width, GLuint height);
static GLfloat       * mkConeVerticesf(GLuint slices, GLboolean base);
static GLfloat       * mkFanConeVerticesf(GLuint slices, GLboolean base);
static GLfloat       * mkCylinderVerticesf(GLuint slices, GLboolean base);
static GLfloat       * mkDiskVerticesf(GLuint slices);
static GLfloat       * mkTorusVerticesf(GLuint slices, GLuint stacks, GLfloat radius);
static GLfloat       * mkGrid2dVerticesf(GLuint width, GLuint height, GLfloat * heightmap);
static GLfloat       * mkTeapotVerticesf(GLuint slices);
static void            mkGrid2dNormalsf(GLuint width, GLuint height, GLfloat * data);
static inline void     triangleNormalf(GLfloat * out, GLfloat * p0, GLfloat * p1, GLfloat * p2);
static inline int      _maxi(int a, int b);
static inline int      _mini(int a, int b);

void gl4dgInit(void) {
  int i;
  if(_hasInit) return;
  _glist = llNew();
  _garray = calloc(_garray_size, sizeof *_garray);
  assert(_garray);
  for(i = _garray_size - 1 ; i >= 0; i--) {
    _garray[i].id = i;
    llPush(_glist, &_garray[i]);
  }
  _hasInit = 1;
}

void gl4dgClean(void) {
  if(_glist) {
    llFree(_glist, NULL);
    _glist = NULL;
  }
  if(_garray) {
    int i;
    for(i = 0; i < _garray_size; ++i)
      if(_garray[i].vao || _garray[i].geom)
	freeGeom(&_garray[i]);
    free(_garray);
    _garray = NULL;
  }
  _garray_size = 256;
  _hasInit = 0;
}

void gl4dgSetGeometryOptimizationLevel(GLuint level) {
  _geometry_optimization_level = level;
}

GLuint gl4dgGetVAO(GLuint id) {
  return _garray[--id].vao;
}

GLuint gl4dgGenSpheref(GLuint slices, GLuint stacks) {
  GLfloat * idata = NULL;
  GL4Dvaoindex * index = NULL;
  GLuint i = genId();
  gsphere_t * s = malloc(sizeof *s);
  assert(s);
  _garray[i].geom = s;
  _garray[i].type = GE_SPHERE;
  s->slices = slices; s->stacks = stacks;
  idata = mkSphereVerticesf(slices, stacks);
  SELECT_GEOMETRY_OPTIMIZATION(index, s, slices + 1, stacks + 1);
  glGenVertexArrays(1, &_garray[i].vao);
  glBindVertexArray(_garray[i].vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glGenBuffers(2, s->buffers);
  glBindBuffer(GL_ARRAY_BUFFER, s->buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, 5 * (slices + 1) * (stacks + 1) * sizeof *idata, idata, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (5 * sizeof *idata), (const void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (5 * sizeof *idata), (const void *)0);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (5 * sizeof *idata), (const void *)(3 * sizeof *idata));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, s->index_row_count * s->index_nb_rows * sizeof *index, index, GL_STATIC_DRAW);
  free(idata);
  free(index);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  return ++i;
}

GLuint gl4dgGenQuadf(void) {
  return mkStaticf(GE_QUAD);
}

GLuint gl4dgGenCubef(void) {
  return mkStaticf(GE_CUBE);
}

GLuint gl4dgGenConef(GLuint slices, GLboolean base) {
  GLfloat * data = NULL;
  GLuint i = genId();
  gcone_t * c = malloc(sizeof *c);
  assert(c);
  _garray[i].geom = c;
  _garray[i].type = GE_CONE;
  c->slices = slices; c->base = base;
  data = mkConeVerticesf(slices, base);
  glGenVertexArrays(1, &_garray[i].vao);
  glBindVertexArray(_garray[i].vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glGenBuffers(1, &(c->buffer));
  glBindBuffer(GL_ARRAY_BUFFER, c->buffer);
  glBufferData(GL_ARRAY_BUFFER, (16 * (slices + 1) + (base ? 8 : 0) * (slices + 2)) * sizeof *data, data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)(3 * sizeof *data));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)(6 * sizeof *data));
  free(data);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return ++i;
}

GLuint gl4dgGenFanConef(GLuint slices, GLboolean base) {
  GLfloat * data = NULL;
  GLuint i = genId();
  gcone_t * c = malloc(sizeof *c);
  assert(c);
  _garray[i].geom = c;
  _garray[i].type = GE_FAN_CONE;
  c->slices = slices; c->base = base;
  data = mkFanConeVerticesf(slices, base);
  glGenVertexArrays(1, &_garray[i].vao);
  glBindVertexArray(_garray[i].vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glGenBuffers(1, &(c->buffer));
  glBindBuffer(GL_ARRAY_BUFFER, c->buffer);
  glBufferData(GL_ARRAY_BUFFER, (base ? 16 : 8) * (slices + 1 + /* le sommet ou le centre de la base */ 1) * sizeof *data, data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)(3 * sizeof *data));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)(6 * sizeof *data));
  free(data);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return ++i;
}

GLuint gl4dgGenCylinderf(GLuint slices, GLboolean base) {
  GLfloat * data = NULL;
  GLuint i = genId();
  gcylinder_t * c = malloc(sizeof *c);
  assert(c);
  _garray[i].geom = c;
  _garray[i].type = GE_CYLINDER;
  c->slices = slices; c->base = base;
  data = mkCylinderVerticesf(slices, base);
  glGenVertexArrays(1, &_garray[i].vao);
  glBindVertexArray(_garray[i].vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glGenBuffers(1, &(c->buffer));
  glBindBuffer(GL_ARRAY_BUFFER, c->buffer);
  glBufferData(GL_ARRAY_BUFFER, (16 * (slices + 1) + (base ? 16 : 0) * (slices + 2)) * sizeof *data, data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)(3 * sizeof *data));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)(6 * sizeof *data));
  free(data);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return ++i;
}

GLuint gl4dgGenDiskf(GLuint slices) {
  GLfloat * data = NULL;
  GLuint i = genId();
  gdisk_t * c = malloc(sizeof *c);
  assert(c);
  _garray[i].geom = c;
  _garray[i].type = GE_DISK;
  c->slices = slices;
  data = mkDiskVerticesf(slices);
  glGenVertexArrays(1, &_garray[i].vao);
  glBindVertexArray(_garray[i].vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glGenBuffers(1, &(c->buffer));
  glBindBuffer(GL_ARRAY_BUFFER, c->buffer);
  glBufferData(GL_ARRAY_BUFFER, 8 * (slices + 1 + /* le centre du disk */ 1) * sizeof *data, data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)(3 * sizeof *data));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)(6 * sizeof *data));
  free(data);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return ++i;
}

GLuint gl4dgGenTorusf(GLuint slices, GLuint stacks, GLfloat radius) {
  GLfloat * idata = NULL;
  GL4Dvaoindex * index = NULL;
  GLuint i = genId();
  gtorus_t * s = malloc(sizeof *s);
  assert(s);
  _garray[i].geom = s;
  _garray[i].type = GE_TORUS;
  s->slices = slices; s->stacks = stacks;
  s->radius = radius;
  idata = mkTorusVerticesf(slices, stacks, radius);
  SELECT_GEOMETRY_OPTIMIZATION(index, s, slices + 1, stacks + 1);
  glGenVertexArrays(1, &_garray[i].vao);
  glBindVertexArray(_garray[i].vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glGenBuffers(2, s->buffers);
  glBindBuffer(GL_ARRAY_BUFFER, s->buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, 8 * (slices + 1) * (stacks + 1) * sizeof *idata, idata, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *idata), (const void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *idata), (const void *)(3 * sizeof *idata));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof *idata), (const void *)(6 * sizeof *idata));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, s->index_row_count * s->index_nb_rows * sizeof *index, index, GL_STATIC_DRAW);
  free(idata);
  free(index);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  return ++i;
}

GLuint gl4dgGenGrid2df(GLuint width, GLuint height) {
  return gl4dgGenGrid2dFromHeightMapf(width, height, NULL);
}

GLuint gl4dgGenGrid2dFromHeightMapf(GLuint width, GLuint height, GLfloat * heightmap) {
  GLfloat * idata = NULL;
  GL4Dvaoindex * index = NULL;
  GLuint i = genId();
  ggrid2d_t * s = malloc(sizeof *s);
  assert(s);
  _garray[i].geom = s;
  _garray[i].type = GE_GRID2D;
  s->width = width; s->height = height;
  idata = mkGrid2dVerticesf(width, height, heightmap);
  SELECT_GEOMETRY_OPTIMIZATION(index, s, width, height);
  glGenVertexArrays(1, &_garray[i].vao);
  glBindVertexArray(_garray[i].vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glGenBuffers(2, s->buffers);
  glBindBuffer(GL_ARRAY_BUFFER, s->buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, 8 * width * height * sizeof *idata, idata, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *idata), (const void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *idata), (const void *)(3 * sizeof *idata));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof *idata), (const void *)(6 * sizeof *idata));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, s->index_row_count * s->index_nb_rows * sizeof *index, index, GL_STATIC_DRAW);
  free(idata);
  free(index);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  return ++i;
}

GLuint gl4dgGenTeapotf(GLuint slices) {
  GLfloat * data = NULL;
  GLuint i = genId();
  gteapot_t * c = malloc(sizeof *c);
  assert(c);
  _garray[i].geom = c;
  _garray[i].type = GE_TEAPOT;
  c->slices = slices;
  data = mkTeapotVerticesf(slices);
  glGenVertexArrays(1, &_garray[i].vao);
  glBindVertexArray(_garray[i].vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glGenBuffers(1, &(c->buffer));
  glBindBuffer(GL_ARRAY_BUFFER, c->buffer);
  glBufferData(GL_ARRAY_BUFFER,(392 * slices + 8) * sizeof *data, data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)(3 * sizeof *data));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof *data), (const void *)(6 * sizeof *data));
  free(data);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return ++i;
}

void gl4dgDraw(GLuint id) {
  switch(_garray[--id].type) {
  case GE_SPHERE:
    glBindVertexArray(_garray[id].vao);
    DRAW_WITH_GEOMETRY_OPTIMIZATION((gsphere_t *)(_garray[id].geom));
    glBindVertexArray(0);
    break;
  case GE_TORUS:
    glBindVertexArray(_garray[id].vao);
    DRAW_WITH_GEOMETRY_OPTIMIZATION((gtorus_t *)(_garray[id].geom));
    glBindVertexArray(0);
    break;
  case GE_GRID2D:
    glBindVertexArray(_garray[id].vao);
    DRAW_WITH_GEOMETRY_OPTIMIZATION((ggrid2d_t *)(_garray[id].geom));
    glBindVertexArray(0);
    break;
  case GE_QUAD:
    glBindVertexArray(_garray[id].vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    break;
  case GE_CUBE:
    glBindVertexArray(_garray[id].vao);
    glDrawArrays(GL_TRIANGLE_STRIP,  0, 4);
    glDrawArrays(GL_TRIANGLE_STRIP,  4, 4);
    glDrawArrays(GL_TRIANGLE_STRIP,  8, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
    glBindVertexArray(0);
    break;
  case GE_CONE:
  case GE_FAN_CONE:
    glBindVertexArray(_garray[id].vao);
    if(_garray[id].type == GE_CONE)
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * (((gcone_t *)(_garray[id].geom))->slices + 1));
    else
      glDrawArrays(GL_TRIANGLE_FAN, 0, ((gcone_t *)(_garray[id].geom))->slices + 2);
    if(((gcone_t *)(_garray[id].geom))->base) {
      if(_garray[id].type == GE_CONE)
	glDrawArrays(GL_TRIANGLE_FAN, 2 * (((gcone_t *)(_garray[id].geom))->slices + 1), ((gcone_t *)(_garray[id].geom))->slices + 2);
      else
	glDrawArrays(GL_TRIANGLE_FAN, ((gcone_t *)(_garray[id].geom))->slices + 2, ((gcone_t *)(_garray[id].geom))->slices + 2);
    }
    glBindVertexArray(0);
    break;
  case GE_CYLINDER:
    glBindVertexArray(_garray[id].vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * (((gcylinder_t *)(_garray[id].geom))->slices + 1));
    if(((gcylinder_t *)(_garray[id].geom))->base) {
      glDrawArrays(GL_TRIANGLE_FAN, 2 * (((gcylinder_t *)(_garray[id].geom))->slices + 1), ((gcylinder_t *)(_garray[id].geom))->slices + 2);
      glDrawArrays(GL_TRIANGLE_FAN, 2 * (((gcylinder_t *)(_garray[id].geom))->slices + 1) + ((gcylinder_t *)(_garray[id].geom))->slices + 2,
		   ((gcylinder_t *)(_garray[id].geom))->slices + 2);
    }
    glBindVertexArray(0);
    break;
  case GE_DISK:
    glBindVertexArray(_garray[id].vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, ((gdisk_t *)(_garray[id].geom))->slices + 2);
    glBindVertexArray(0);
    break;
  case GE_TEAPOT:
    glBindVertexArray(_garray[id].vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 49 * (((gteapot_t *)(_garray[id].geom))->slices )+1);
    glBindVertexArray(0);
    break;
  default:
    break;
  }
}

void gl4dgDelete(GLuint id) {
  --id;
  freeGeom(&_garray[id]);
  _garray[id].vao  = 0;
  _garray[id].type = 0;
  llPush(_glist, &_garray[id]);
}

static void freeGeom(void * data) {
  geom_t * geom = (geom_t *)data;
  switch(geom->type) {
  case GE_SPHERE:
    glDeleteVertexArrays(1, &(geom->vao));
    glDeleteBuffers(2, ((gsphere_t *)(geom->geom))->buffers);
    break;
  case GE_TORUS:
    glDeleteVertexArrays(1, &(geom->vao));
    glDeleteBuffers(2, ((gtorus_t *)(geom->geom))->buffers);
    break;
  case GE_GRID2D:
    glDeleteVertexArrays(1, &(geom->vao));
    glDeleteBuffers(2, ((ggrid2d_t *)(geom->geom))->buffers);
    break;
  case GE_QUAD:
  case GE_CUBE:
    glDeleteVertexArrays(1, &(geom->vao));
    glDeleteBuffers(1, &(((gstatic_t *)(geom->geom))->buffer));
    break;
  case GE_CONE:
  case GE_FAN_CONE:
    glDeleteVertexArrays(1, &(geom->vao));
    glDeleteBuffers(1, &(((gcone_t *)(geom->geom))->buffer));
    break;
  case GE_CYLINDER:
    glDeleteVertexArrays(1, &(geom->vao));
    glDeleteBuffers(1, &(((gcylinder_t *)(geom->geom))->buffer));
    break;
  case GE_DISK:
    glDeleteVertexArrays(1, &(geom->vao));
    glDeleteBuffers(1, &(((gdisk_t *)(geom->geom))->buffer));
    break;
  case GE_TEAPOT:
    glDeleteVertexArrays(1, &(geom->vao));
    glDeleteBuffers(1, &(((gteapot_t *)(geom->geom))->buffer));
    break;
  default:
    break;
  }
  if(geom->geom)
    free(geom->geom);
  geom->geom = NULL;
}

static GLuint genId(void) {
  int i;
  if(llEmpty(_glist)) {
    int s = _garray_size;
    _garray = realloc(_garray, (_garray_size *= 2) * sizeof *_garray);
    assert(_garray);
    for(i = _garray_size - 1 ; i >= s; i--) {
      _garray[i].id   = i;
      _garray[i].vao  = 0;
      _garray[i].type = 0;
      _garray[i].geom = NULL;
      llPush(_glist, &_garray[i]);
    }
  }
  return ((geom_t *)llPop(_glist))->id;
}

static GLuint mkStaticf(geom_e type) {
  static GLfloat quad_data[] = {
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
     1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f
  };
  static GLfloat cube_data[] = {
    /* front */
    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
     1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    /* back */
     1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
     1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
    -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
    /* right */
    1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    /* left */
    -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    /* top */
    -1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    /* bottom */
    -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f
  };
  GLuint i = genId();
  gstatic_t * q = malloc(sizeof *q);
  assert(q);
  _garray[i].geom = q;
  _garray[i].type = type;
  glGenVertexArrays(1, &_garray[i].vao);
  glBindVertexArray(_garray[i].vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glGenBuffers(1, &(q->buffer));
  glBindBuffer(GL_ARRAY_BUFFER, q->buffer);
  switch(type) {
  case GE_QUAD:
    glBufferData(GL_ARRAY_BUFFER, sizeof quad_data, quad_data, GL_STATIC_DRAW);
    break;
  case GE_CUBE:
    glBufferData(GL_ARRAY_BUFFER, sizeof cube_data, cube_data, GL_STATIC_DRAW);
    break;
  default:
    assert(0);
    break;
  }
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *quad_data), (const void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof *quad_data), (const void *)(3 * sizeof *quad_data));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof *quad_data), (const void *)(6 * sizeof *quad_data));
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return ++i;
}

static GLfloat * mkSphereVerticesf(GLuint slices, GLuint stacks) {
  int i, j, k;
  GLdouble phi, theta, r, y;
  GLfloat * data;
  GLdouble c2MPI_Long = 2.0 * M_PI / slices;
  GLdouble cMPI_Lat = M_PI / stacks;
  data = malloc(5 * (slices + 1) * (stacks + 1) * sizeof *data);
  assert(data);
  for(i = 0, k = 0; i <= (int)stacks; ++i) {
    theta  = -M_PI_2 + i * cMPI_Lat;
    y = sin(theta);
    r = cos(theta);
    for(j = 0; j <= (int)slices; ++j) {
      phi = j * c2MPI_Long;
      data[k++] = r * cos(phi);
      data[k++] = y;
      data[k++] = r * sin(phi);
      data[k++] = 1.0f - phi / (2.0 * M_PI);
      data[k++] = (theta + M_PI_2) / M_PI;
    }
  }
  return data;
}

static GL4Dvaoindex * mkRegularGridTriangleIndices(GLuint width, GLuint height) {
  int z, nz, x, nx, k, zw, nzw, wm1 = width - 1, hm1 = height - 1;
  GLuint * index;
  assert(height > 1 && width > 1);
  index = malloc(6 * wm1 * hm1 * sizeof *index);
  assert(index);
  for(z = 0, k = 0; z < hm1; ++z) {
    nz = z + 1;
    zw = z * width;
    nzw = nz * width;
    for(x = 0; x < wm1; ++x) {
      nx = x + 1;

      index[k++] = zw  + x;
      index[k++] = nzw + x;
      index[k++] = zw  + nx;

      index[k++] = zw + nx;
      index[k++] = nzw + x;
      index[k++] = nzw + nx;
    }
  }
  return index;
}

static GL4Dvaoindex * mkRegularGridStripsIndices(GLuint width, GLuint height) {
  int z, x, k, zw, nzw, hm1 = height - 1;
  GLuint * index;
  assert(height > 1 && width > 1);
  index = malloc(2 * width * hm1 * sizeof *index);
  assert(index);
  for(z = 0, k = 0; z < hm1; ++z) {
    zw = z * width;
    nzw = zw + width;
    for(x = 0; x < (int)width; ++x) {
      index[k++] = zw  + x;
      index[k++] = nzw + x;
    }
  }
  return index;
}

static GL4Dvaoindex * mkRegularGridStripIndices(GLuint width, GLuint height) {
  int z, x, k, zw, nzw, nnzw, wm1 = width - 1, hm1 = height - 1, hi = hm1 - ((height&1) ? 0 : 1);
  GLuint * index;
  assert(height > 1 && width > 1);
  index = malloc(2 * width * hm1 * sizeof *index);
  assert(index);
  for(z = 0, k = 0; z < hi; z += 2) {
    zw = z * width;
    nzw = zw + width;
    nnzw = nzw + width;
    for(x = 0; x < (int)width; ++x) {
      index[k++] = zw  + x;
      index[k++] = nzw + x;
    }
    for(x = wm1; x >= 0; x--) {
      index[k++] = nnzw + x;
      index[k++] = nzw  + x;
    }
  }
  if(!(height&1)) {
    zw = z * width;
    nzw = zw + width;
    for(x = 0; x < (int)width; ++x) {
      index[k++] = zw + x;
      index[k++] = nzw + x;
    }
  }
  return index;
}

static GL4Dvaoindex * mkRegularGridTriangleAdjacencyIndices(GLuint width, GLuint height) {
  int z, zw, pzw, nzw, nnzw, x, px, nx, nnx, k, wm1 = width - 1, hm1 = height - 1;
  GLuint * index;
  assert(height > 1 && width > 1);
  index = malloc(12 * wm1 * hm1 * sizeof *index);
  assert(index);
  for(z = 0, k = 0; z < hm1; ++z) {
    zw = z * width;
    pzw = _maxi(z - 1, 0) * width;
    nzw = (z + 1) * width;
    nnzw = _mini(z + 2, hm1) * width;
    for(x = 0; x < wm1; ++x) {
      px = _maxi(x - 1, 0);
      nx = x + 1;
      nnx = _mini(nx + 1, wm1);

      index[k++] = zw  + x;    /* (x, z) */
      index[k++] = nzw + px;   /* (x - 1, z + 1) */
      index[k++] = nzw + x;    /* (x, z + 1) */
      index[k++] = nzw + nx;   /* (x + 1, z + 1) */
      index[k++] = zw  + nx;   /* (x + 1, z) */
      index[k++] = pzw + nx;   /* (x + 1, z - 1) */

      index[k++] = zw   + nx;  /* (x + 1, ) */
      index[k++] = zw   + x;   /* (x, z) */
      index[k++] = nzw  + x;   /* (x, z + 1) */
      index[k++] = nnzw + x;   /* (x, z + 2) */
      index[k++] = nzw  + nx;  /* (x + 1, z + 1) */
      index[k++] = zw   + nnx; /* (x + 2, z) */
    }
  }
  return index;
}

static GL4Dvaoindex * mkRegularGridStripsAdjacencyIndices(GLuint width, GLuint height) {
  int z, zw, pzw, nzw, nnzw, x, k, hm1 = height - 1;
  GLuint * index;
  assert(height > 1 && width > 1);
  index = malloc(4 * width * hm1 * sizeof *index);
  assert(index);
  for(z = 0, k = 0; z < hm1; ++z) {
    zw = z * width;
    pzw = _maxi(z - 1, 0) * width;
    nzw = zw + width;
    nnzw = zw + 2 * width;
    index[k++] = zw;  /* (0, z) */
    index[k++] = nzw; /* (-1, z + 1) -> (0, z + 1) */
    index[k++] = nzw; /* (0, z + 1) */
    for(x = 1; x < (int)width; ++x) {
      index[k++] = pzw  + x;     /* (x, z - 1) */
      index[k++] = zw   + x;     /* (x, z) */
      index[k++] = nnzw + x - 1; /* (x - 1, z + 2) */
      index[k++] = nzw  + x;     /* (x, z + 1) */
    }
    index[k++] = zw + width - 1;  /* (w, z) -> (w - 1, z) */
  }
  return index;
}

static inline void fcvNormals(GLfloat * p, GLfloat y, int i) {
  (void)y; /* warning silenced */
  p[i] = 2.0f * p[i - 3] / sqrt(5.0);
  p[i + 1] = 1.0f / sqrt(5.0);
  p[i + 2] = 2.0f * p[i - 1] / sqrt(5.0);
}

static inline void fcvbNormals(GLfloat * p, GLfloat y, int i) {
  p[i] = 0;
  p[i + 1] = y;
  p[i + 2] = 0;
}

/*!\brief Macro servant à remplir des FANs, soit en disk soit en cone.
 * d est data, i est indice à partir duquel remplir, ym est le y du
 * point central, ye est le y du point extreme, slices est le nombre
 * de longitudes et normals est la fonction calculant les normales.
*/
#define DISK_FAN(d, i, ym, ye, slices, normals) do {			\
    int j;								\
    const GLdouble _1pi_4 = M_PI / 4, _3pi_4 = 3 * M_PI / 4;		\
    const GLdouble _5pi_4 = 5 * M_PI / 4, _7pi_4 = 7 * M_PI / 4;	\
    GLdouble c2MPI_Long = 2 * M_PI / slices, sens = SIGN(ym), phi;	\
    (d)[(i)++] = 0; (d)[(i)++] = ym; (d)[(i)++] = 0;			\
    (d)[(i)++] = 0; (d)[(i)++] = sens; (d)[(i)++] = 0;			\
    (d)[(i)++] = 0.5; (d)[(i)++] = 0.5;					\
    for(j = 0; j <= (slices); ++j) {					\
      phi = j * c2MPI_Long;						\
      (d)[(i)++] = -cos(sens * phi);					\
      (d)[(i)++] = (ye);						\
      (d)[(i)++] = sin(sens * phi);					\
      (normals)((d), (ye), (i)); (i) += 3;				\
      if(phi < _1pi_4 || phi > _7pi_4) {				\
	(d)[(i)++] = 1.0;						\
	(d)[(i)++] = 0.5 + tan(phi) / 2.0;				\
      } else if(phi < _3pi_4) {						\
	(d)[(i)++] = 0.5 - tan(phi - GL4DM_PI_2) / 2.0;			\
	(d)[(i)++] = 1.0;						\
      } else if(phi < _5pi_4) {						\
	(d)[(i)++] = 0.0;						\
	(d)[(i)++] = 0.5 - tan(phi) / 2.0;				\
      } else {								\
	(d)[(i)++] = 0.5 + tan(phi - GL4DM_PI_2) / 2.0;			\
	(d)[(i)++] = 0.0;						\
      }									\
    }									\
  } while(0)


static GLfloat * mkConeVerticesf(GLuint slices, GLboolean base) {
  int j, k = 0;
  GLdouble phi;
  GLfloat * data;
  GLdouble c2MPI_Long = 2.0 * M_PI / slices, s;
  data = malloc((16 * (slices + 1) + (base ? 8 : 0) * (slices + 2)) * sizeof *data);
  assert(data);
  for(j = 0; j <= (int)slices; ++j) {
    data[k++] = 0; data[k++] = 1; data[k++] = 0;
    data[k++] = 0; data[k++] = 1; data[k++] = 0;
    data[k++] = (s = j / (GLdouble)slices); data[k++] = 1;
    phi = j * c2MPI_Long;
    data[k++] = -cos(phi);
    data[k++] = -1;
    data[k++] = sin(phi);
    fcvNormals(data, 0, k); k += 3;
    data[k++] = s; data[k++] = 0;
  }
  if(base)
    DISK_FAN(data, k, -1, -1, (int)slices, fcvbNormals);
  return data;
}

static GLfloat * mkFanConeVerticesf(GLuint slices, GLboolean base) {
  int k = 0;
  GLfloat * data;
  data = malloc((base ? 16 : 8) * (slices + 2) * sizeof *data);
  assert(data);
  DISK_FAN(data, k, 1, -1, (int)slices, fcvNormals);
  if(base)
    DISK_FAN(data, k, -1, -1, (int)slices, fcvbNormals);
  return data;
}

static GLfloat * mkCylinderVerticesf(GLuint slices, GLboolean base) {
  int j, k = 0;
  GLdouble phi;
  GLfloat * data;
  GLdouble c2MPI_Long = 2.0 * M_PI / slices, s;
  data = malloc((16 * (slices + 1) + (base ? 16 : 0) * (slices + 2)) * sizeof *data);
  assert(data);
  for(j = 0; j <= (int)slices; ++j) {
    phi = j * c2MPI_Long;
    data[k++] = -cos(phi);
    data[k++] = 1;
    data[k++] = sin(phi);
    fcvNormals(data, 0, k); k += 3;
    data[k++] = (s = j / (GLdouble)slices); data[k++] = 1;
    data[k] = data[k - 8]; k++;
    data[k++] = -1;
    data[k] = data[k - 8]; k++;
    fcvNormals(data, 0, k); k += 3;
    data[k++] = s; data[k++] = 0;
  }
  if(base) {
    DISK_FAN(data, k,  1,  1, (int)slices, fcvbNormals);
    DISK_FAN(data, k, -1, -1, (int)slices, fcvbNormals);
  }
  return data;
}

static GLfloat * mkDiskVerticesf(GLuint slices) {
  int k = 0;
  GLfloat * data;
  data = malloc(8 * (slices + 2) * sizeof *data);
  assert(data);
  DISK_FAN(data, k, 0, 0, (int)slices, fcvbNormals);
  return data;
}

static GLfloat * mkTorusVerticesf(GLuint slices, GLuint stacks, GLfloat radius) {
  int i, j, k;
  GLdouble phi, theta, r, x, y, z;
  GLfloat * data;
  GLdouble c2MPI_Long = 2.0 * M_PI / slices;
  GLdouble c2MPI_Lat  = 2.0 * M_PI / stacks;
  data = malloc(8 * (slices + 1) * (stacks + 1) * sizeof *data);
  assert(data);
  for(i = 0, k = 0; i <= (int)stacks; ++i) {
    theta  = i * c2MPI_Lat;
    y = radius * sin(theta);
    r = radius * cos(theta);
    for(j = 0; j <= (int)slices; ++j) {
      phi = j * c2MPI_Long;
      x = cos(phi);
      z = sin(phi);
      data[k++] = (1 - radius + r) * x;
      data[k++] = y;
      data[k++] = (1 - radius + r) * z;
      data[k + 0] = data[k - 3] - (1 - radius) * x;
      data[k + 1] = data[k - 2] - (1 - radius) * y;
      data[k + 2] = data[k - 1] - (1 - radius) * z;
      MVEC3NORMALIZE(&data[k]); k += 3;
      data[k++] = phi   / (2.0 * M_PI);
      data[k++] = theta / (2.0 * M_PI);
    }
  }
  return data;
}

static GLfloat * mkGrid2dVerticesf(GLuint width, GLuint height, GLfloat * heightmap) {
  int i, j, k, iw;
  GLdouble x, z, tx, tz;
  GLfloat * data;
  data = malloc(8 * width * height * sizeof *data);
  assert(data);
  if(heightmap) {
    for(i = 0, k = 0; i < (int)height; ++i) {
      z = -1.0f + 2.0f * (tz = i / (height - 1.0f));
      iw = i * width;
      for(j = 0; j < (int)width; ++j) {
	x = -1.0f + 2.0f * (tx = j / (width - 1.0f));
	data[k++] = x; data[k++] = 2.0f * heightmap[iw + j] - 1.0f; data[k++] = z;
	k += 3;
	data[k++] = tx; data[k++] = tz;
      }
    }
    mkGrid2dNormalsf(width, height, data);
  } else {
    for(i = 0, k = 0; i < (int)height; ++i) {
      z = -1.0f + 2.0f * (tz = i / (height - 1.0f));
      for(j = 0; j < (int)width; ++j) {
	x = -1.0f + 2.0f * (tx = j / (width - 1.0f));
	data[k++] = x;  data[k++] = 0; data[k++] = z;
	data[k++] = 0;  data[k++] = 1; data[k++] = 0;
	data[k++] = tx; data[k++] = tz;
      }
    }
  }
  return data;
}

static void mkGrid2dNormalsf(GLuint width, GLuint height, GLfloat * data) {
  int x, z, zw, i, wm1 = width - 1, hm1 = height - 1;
  GLfloat n[18];
  /* pour les sommets ayant 8 voisins */
  for(z = 1; z < hm1; ++z) {
    zw = z * width;
    for(x = 1; x < wm1; ++x) {
      triangleNormalf(&n[0],  &data[8 * (x + zw)], &data[8 * (x + 1 + zw)], &data[8 * (x + 1 + (z - 1) * width)]);
      triangleNormalf(&n[3],  &data[8 * (x + zw)], &data[8 * (x + 1 + (z - 1) * width)], &data[8 * (x + (z - 1) * width)]);
      triangleNormalf(&n[6],  &data[8 * (x + zw)], &data[8 * (x + (z - 1) * width)], &data[8 * (x - 1 + zw)]);
      triangleNormalf(&n[9],  &data[8 * (x + zw)], &data[8 * (x - 1 + zw)], &data[8 * (x - 1 + (z + 1) * width)]);
      triangleNormalf(&n[12], &data[8 * (x + zw)], &data[8 * (x - 1 + (z + 1) * width)], &data[8 * (x + (z + 1) * width)]);
      triangleNormalf(&n[15], &data[8 * (x + zw)], &data[8 * (x + (z + 1) * width)], &data[8 * (x + 1 + zw)]);
      data[8 * (x + zw) + 3] = 0.0f;
      data[8 * (x + zw) + 4] = 0.0f;
      data[8 * (x + zw) + 5] = 0.0f;
      for(i = 0; i < 6; ++i) {
        data[8 * (x + zw) + 3] += n[3 * i + 0];
        data[8 * (x + zw) + 4] += n[3 * i + 1];
        data[8 * (x + zw) + 5] += n[3 * i + 2];
      }
      data[8 * (x + zw) + 3] /= 6.0f;
      data[8 * (x + zw) + 4] /= 6.0f;
      data[8 * (x + zw) + 5] /= 6.0f;
    }
  }
  /* pour les 4 coins */
  /* haut-gauche (x = 0, z = 0) */
  triangleNormalf(&data[3], &data[0], &data[8 * width], &data[8]);
  /* bas-droite (x = w - 1, z = h - 1) */
  triangleNormalf(&data[8 * (wm1 + hm1 * width) + 3],  &data[8 * (wm1 + hm1 * width)], &data[8 * (wm1 + (height - 2) * width)], &data[8 * (width - 2 + hm1 * width)]);
  /* haut-droite (x = w - 1, z = 0) */
  triangleNormalf(&n[0], &data[8 * wm1], &data[8 * (wm1 - 1)], &data[8 * (wm1 - 1 + width)]);
  triangleNormalf(&n[3], &data[8 * wm1], &data[8 * (wm1 - 1 + width)], &data[8 * (wm1 + width)]);
  for(i = 0; i < 3; ++i)
    data[8 * wm1 + 3 + i] = (n[i] + n[3 + i]) / 2.0f;
  /* bas-gauche (x = 0, z = hm1) */
  zw = hm1 * width;
  triangleNormalf(&n[0], &data[8 * zw], &data[8 * (1 + zw)], &data[8 * (1 + (hm1 - 1) * width)]);
  triangleNormalf(&n[3], &data[8 * zw], &data[8 * (1 + (hm1 - 1) * width)], &data[8 * ((hm1 - 1) * width)]);
  for(i = 0; i < 3; ++i)
    data[8 * zw + 3 + i] = (n[i] + n[3 + i]) / 2.0f;
  /* le bord gauche et droit */
  for(z = 1; z < hm1; ++z) {
    zw = z * width;
    /* gauche */
    x = 0;
    triangleNormalf(&n[0], &data[8 * (x + zw)], &data[8 * (x + 1 + zw)], &data[8 * (x + 1 + (z - 1) * width)]);
    triangleNormalf(&n[3], &data[8 * (x + zw)], &data[8 * (x + 1 + (z - 1) * width)], &data[8 * (x + (z - 1) * width)]);
    triangleNormalf(&n[6], &data[8 * (x + zw)], &data[8 * (x + (z + 1) * width)], &data[8 * (x + 1 + zw)]);
    for(i = 0; i < 3; ++i)
      data[8 * (x + zw) + 3 + i] = (n[i] + n[3 + i] + n[6 + i]) / 3.0f;
    /* droit */
    x = wm1;
    triangleNormalf(&n[0], &data[8 * (x + zw)], &data[8 * (x + (z - 1) * width)], &data[8 * (x - 1 + zw)]);
    triangleNormalf(&n[3], &data[8 * (x + zw)], &data[8 * (x - 1 + zw)], &data[8 * (x - 1 + (z + 1) * width)]);
    triangleNormalf(&n[6], &data[8 * (x + zw)], &data[8 * (x - 1 + (z + 1) * width)], &data[8 * (x + (z + 1) * width)]);
    for(i = 0; i < 3; ++i)
      data[8 * (x + zw) + 3 + i] = (n[i] + n[3 + i] + n[6 + i]) / 3.0f;
  }
  /* le bord haut et bas */
  for(x = 1; x < wm1; ++x) {
    /* haut */
    z = 0; zw = 0;
    triangleNormalf(&n[0], &data[8 * (x + zw)], &data[8 * (x - 1 + zw)], &data[8 * (x - 1 + (z + 1) * width)]);
    triangleNormalf(&n[3], &data[8 * (x + zw)], &data[8 * (x - 1 + (z + 1) * width)], &data[8 * (x + (z + 1) * width)]);
    triangleNormalf(&n[6], &data[8 * (x + zw)], &data[8 * (x + (z + 1) * width)], &data[8 * (x + 1 + zw)]);
    for(i = 0; i < 3; ++i)
      data[8 * (x + zw) + 3 + i] = (n[i] + n[3 + i] + n[6 + i]) / 3.0f;
    /* bas */
    z = hm1; zw = z * width;
    triangleNormalf(&n[0], &data[8 * (x + zw)], &data[8 * (x + 1 + zw)], &data[8 * (x + 1 + (z - 1) * width)]);
    triangleNormalf(&n[3], &data[8 * (x + zw)], &data[8 * (x + 1 + (z - 1) * width)], &data[8 * (x + (z - 1) * width)]);
    triangleNormalf(&n[6], &data[8 * (x + zw)], &data[8 * (x + (z - 1) * width)], &data[8 * (x - 1 + zw)]);
    for(i = 0; i < 3; ++i)
      data[8 * (x + zw) + 3 + i] = (n[i] + n[3 + i] + n[6 + i]) / 3.0f;
  }
}

static GLfloat * mkTeapotVerticesf(GLuint slices) {
  /* Vertices data */
  static GLfloat teapot_data[][5] = {
    /*vertx  verty  normx  normy  texCoord    lid*/
    { 0.000, 0.394, 0.000, 1.000, 0.000000 },
    { 0.085, 0.382, 0.825, 0.565, 0.055058 },
    { 0.081, 0.352, 0.940,-0.335, 0.074406 },
    { 0.049, 0.315, 0.960,-0.285, 0.105557 },
    { 0.050, 0.281, 0.845, 0.530, 0.126978 },
    { 0.114, 0.259, 0.260, 0.965, 0.170294 },
    { 0.206, 0.244, 0.180, 0.985, 0.230067 },
    { 0.289, 0.229, 0.380, 0.925, 0.283858 },
    { 0.325, 0.206, 0.170, 0.985, 0.310912 },
    /*vertx  verty  normx  normy  texCoord    rim*/
    { 0.350, 0.206,-0.970,-0.255, 0.310912 },
    { 0.345, 0.225,-0.970, 0.255, 0.323163 },
    { 0.351, 0.231,-0.090, 1.000, 0.328592 },
    { 0.362, 0.225, 0.680, 0.730, 0.336960 },
    /*vertx  verty  normx  normy  texCoord    body*/
    { 0.489,-0.081, 0.870, 0.495, 0.351323 },
    { 0.421, 0.108, 0.915, 0.405, 0.420675 },
    { 0.461, 0.012, 0.940, 0.335, 0.487306 },
    { 0.489,-0.081, 0.980, 0.205, 0.549441 },
    { 0.500,-0.169, 1.000,-0.065, 0.605798 },
    { 0.481,-0.243, 0.900,-0.435, 0.654719 },
    { 0.438,-0.298, 0.730,-0.685, 0.699350 },
    { 0.395,-0.335, 0.695,-0.720, 0.735718 },
    /*vertx  verty  normx  normy  texCoord    bottom*/
    { 0.375,-0.356, 0.795,-0.610, 0.754156 },
    { 0.367,-0.370, 0.625,-0.780, 0.764490 },
    { 0.321,-0.382, 0.175,-0.985, 0.794571 },
    { 0.209,-0.391, 0.050,-1.000, 0.866376 },
    { 0.000,-0.394, 0.000,-1.000, 1.000000 }
  };
  /* Initialization of variables */
  int i, j, k = 0;
  GLfloat * data;
  GLdouble angle, theta = 2.0 * M_PI / slices;
  data = malloc((392 * slices + 8) * sizeof *data);
  assert(data);
  /* Filling the buffer */
  data[k++] = teapot_data[0][0];
  data[k++] = teapot_data[0][1];
  data[k++] = 0;
  data[k++] = teapot_data[0][2];
  data[k++] = teapot_data[0][3];
  data[k++] = 0;
  data[k++] = 0;
  data[k++] = teapot_data[0][4];
  for(i = 0; i < (int)slices; ++i) {
    angle = i*theta;
    if(i%2) {
      for(j = 24; j > 0; --j) {
        data[k++] = teapot_data[j][0]*cos(angle);
        data[k++] = teapot_data[j][1];
        data[k++] = teapot_data[j][0]*sin(angle);
        data[k++] = teapot_data[j][2]*cos(angle);
        data[k++] = teapot_data[j][3];
        data[k++] = teapot_data[j][2]*sin(angle);
        data[k++] = (angle)/(2.0*M_PI);
        data[k++] = teapot_data[j][4];

        data[k++] = teapot_data[j][0]*cos(angle+theta);
        data[k++] = teapot_data[j][1];
        data[k++] = teapot_data[j][0]*sin(angle+theta);
        data[k++] = teapot_data[j][2]*cos(angle+theta);
        data[k++] = teapot_data[j][3];
        data[k++] = teapot_data[j][2]*sin(angle+theta);
        data[k++] = (angle+theta)/(2.0*M_PI);
        data[k++] = teapot_data[j][4];
      }
      data[k++] = teapot_data[j][0];
      data[k++] = teapot_data[j][1];
      data[k++] = 0.0;
      data[k++] = teapot_data[j][2];
      data[k++] = teapot_data[j][3];
      data[k++] = 0.0;
      data[k++] = (angle)/(2.0*M_PI);
      data[k++] = teapot_data[j][4];
    }
    else {
      for(j = 1; j < 25; ++j) {
        data[k++] = teapot_data[j][0]*cos(angle);
        data[k++] = teapot_data[j][1];
        data[k++] = teapot_data[j][0]*sin(angle);
        data[k++] = teapot_data[j][2]*cos(angle);
        data[k++] = teapot_data[j][3];
        data[k++] = teapot_data[j][2]*sin(angle);
        data[k++] = (angle)/(2.0*M_PI);
        data[k++] = teapot_data[j][4];

        data[k++] = teapot_data[j][0]*cos(angle+theta);
        data[k++] = teapot_data[j][1];
        data[k++] = teapot_data[j][0]*sin(angle+theta);
        data[k++] = teapot_data[j][2]*cos(angle+theta);
        data[k++] = teapot_data[j][3];
        data[k++] = teapot_data[j][2]*sin(angle+theta);
        data[k++] = (angle+theta)/(2.0*M_PI);
        data[k++] = teapot_data[j][4];
      }
      data[k++] = teapot_data[25][0];
      data[k++] = teapot_data[25][1];
      data[k++] = 0.0;
      data[k++] = teapot_data[25][2];
      data[k++] = teapot_data[25][3];
      data[k++] = 0.0;
      data[k++] = (angle)/(2.0*M_PI);
      data[k++] = teapot_data[25][4];
    }
  }
  return data;
}

static inline void triangleNormalf(GLfloat * out, GLfloat * p0, GLfloat * p1, GLfloat * p2) {
  GLfloat v0[3], v1[3];
  v0[0] = p1[0] - p0[0];
  v0[1] = p1[1] - p0[1];
  v0[2] = p1[2] - p0[2];
  v1[0] = p2[0] - p1[0];
  v1[1] = p2[1] - p1[1];
  v1[2] = p2[2] - p1[2];
  MVEC3CROSS(out, v0, v1);
  MVEC3NORMALIZE(out);
}

static inline int _maxi(int a, int b) {
  return a > b ? a : b;
}

static inline int _mini(int a, int b) {
  return a < b ? a : b;
}
