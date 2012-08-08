/*
      glm.h
      Nate Robins, 1997
      ndr@pobox.com, http://www.pobox.com/~ndr/

      Wavefront OBJ model file format reader/writer/manipulator.

      Includes routines for generating smooth normals with
      preservation of edges, welding redundant vertices & texture
      coordinate generation (spheremap and planar projections) + more.

 */

#ifndef _GLM_H_
#define _GLM_H_

#ifdef _WIN32
#	include <windows.h>
#endif
#include <gl/gl.h>

#ifndef M_PI
#define M_PI 3.14159265f
#endif

#define GLM_NONE     (0)			/* render with only vertices */
#define GLM_FLAT     (1 << 0)		/* render with facet normals */
#define GLM_SMOOTH   (1 << 1)		/* render with vertex normals */
#define GLM_TEXTURE  (1 << 2)		/* render with texcoords */
#define GLM_COLOR    (1 << 3)		/* render with colors */
#define GLM_MATERIAL (1 << 4)		/* render with materials */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* GLMmaterial: Structure that defines a material in a model.
 */
typedef struct _GLMmaterial
{
  char* name;					/* name of material */
  GLfloat diffuse[4];			/* diffuse component */
  GLfloat ambient[4];			/* ambient component */
  GLfloat specular[4];			/* specular component */
  GLfloat emmissive[4];			/* emmissive component */
  GLfloat shininess;			/* specular exponent */
  char* kdMapName;				/* map_Kd texture name*/
  GLenum format;				/* pixel format */
  GLuint kdMapId;				/* texture id*/
} GLMmaterial;

/* GLMtriangle: Structure that defines a triangle in a model.
 */
typedef struct _GLMtriangle {
  GLuint vindices[3];			/* array of triangle vertex indices */
  GLuint nindices[3];			/* array of triangle normal indices */
  GLuint tindices[3];			/* array of triangle texcoord indices*/
  GLuint findex;				/* index of triangle facet normal */
} GLMtriangle;

/* GLMgroup: Structure that defines a group in a model.
 */
typedef struct _GLMgroup {
  char*             name;			/* name of this group */
  GLuint            numtriangles;	/* number of triangles in this group */
  GLuint*           triangles;		/* array of triangle indices */
  GLuint            material;       /* index to material for group */
  struct _GLMgroup* next;			/* pointer to next group in model */
} GLMgroup;

/* GLMmodel: Structure that defines a model.
 */
typedef struct _GLMmodel {
  char*    pathname;			/* path to this model */
  char*    mtllibname;			/* name of the material library */

  GLuint   numvertices;			/* number of vertices in model */
  GLfloat* vertices;			/* array of vertices  */

  GLuint   numnormals;			/* number of normals in model */
  GLfloat* normals;				/* array of normals */

  GLuint   numtexcoords;		/* number of texcoords in model */
  GLfloat* texcoords;			/* array of texture coordinates */

  GLuint   numfacetnorms;		/* number of facetnorms in model */
  GLfloat* facetnorms;			/* array of facetnorms */

  GLuint   numtriangles;		/* number of triangles in model */
  GLMtriangle* triangles;		/* array of triangles */

  GLuint       nummaterials;		/* number of materials in model */
  GLMmaterial* materials;		/* array of materials */

  GLuint       numgroups;		/* number of groups in model */
  GLMgroup*    groups;			/* linked list of groups */
} GLMmodel;

/* glmDot: compute the dot product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
GLfloat 
glmDot(GLfloat* u, GLfloat* v);

/* glmCross: compute the cross product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 * n - array of 3 GLfloats (GLfloat n[3]) to return the cross product in
 */
GLvoid
glmCross(GLfloat* u, GLfloat* v, GLfloat* n);

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
GLvoid 
glmNormalize(GLfloat* v);

/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scale factor used.
 *
 * model - properly initialized GLMmodel structure
 */
GLfloat
glmUnitize(GLMmodel* model);

/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model      - initialized GLMmodel structure
 * dimensions - array of 3 GLfloats (GLfloat dimensions[3])
 */
GLvoid
glmDimensions(GLMmodel* model, GLfloat* dimensions);

/* glmScale: Scales a model by a given amount.
 *
 * model - properly initialized GLMmodel structure
 * scale - scale factor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid
glmScale(GLMmodel* model, GLfloat scale);

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.  Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 *
 * model - properly initialized GLMmodel structure
 */
GLvoid
glmReverseWinding(GLMmodel* model);

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmFacetNormals(GLMmodel* model);

/* glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.  Then
 * loops through each vertex in the the list averaging all the facet
 * normals of the triangles each vertex is in.  Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.  If the dot product of a facet normal and the facet normal
 * associated with the first triangle in the list of triangles the
 * current vertex is in is greater than the cosine of the angle
 * parameter to the function, that facet normal is not added into the
 * average normal calculation and the corresponding vertex is given
 * the facet normal.  This tends to preserve hard edges.  The angle to
 * use depends on the model, but 90 degrees is usually a good start.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */
GLvoid
glmVertexNormals(GLMmodel* model, GLfloat angle);

/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmLinearTexture(GLMmodel* model);

/* glmSpheremapTexture: Generates texture coordinates according to a
 * spherical projection of the texture map.  Sometimes referred to as
 * sphere map, or reflection map texture coordinates.  It generates
 * these by using the normal to calculate where that vertex would map
 * onto a sphere.  Since it is impossible to map something flat
 * perfectly onto something spherical, there is distortion at the
 * poles.  This particular implementation causes the poles along the X
 * axis to be distorted.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmSpheremapTexture(GLMmodel* model);

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be freed with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 */
GLMmodel*
glmReadOBJ(const char* filename);

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model    - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode     - a bitwise or of values describing what is written to the file
 *            GLM_NONE    -  write only vertices
 *            GLM_FLAT    -  write facet normals
 *            GLM_SMOOTH  -  write vertex normals
 *            GLM_TEXTURE -  write texture coordinates
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid 
glmWriteOBJ(GLMmodel* model, const char* filename, GLuint mode);

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmDelete(GLMmodel* model);

/* glmDraw: Render the model to the current OpenGL context using the
 * mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE    -  render with only vertices
 *            GLM_FLAT    -  render with facet normals
 *            GLM_SMOOTH  -  render with vertex normals
 *            GLM_TEXTURE -  render with texture coordinates
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid
glmDraw(GLMmodel* model, GLuint mode);

/* glmList: Generates and returns a display list for the model using
 * the mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE    -  render with only vertices
 *            GLM_FLAT    -  render with facet normals
 *            GLM_SMOOTH  -  render with vertex normals
 *            GLM_TEXTURE -  render with texcoords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLuint
glmList(GLMmodel* model, GLuint mode);

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model      - initialized GLMmodel structure
 * epsilon    - maximum difference between vertices
 *              ( 0.00001 is a good start for a unitized model)
 *
 */
GLvoid 
glmWeld(GLMmodel* model, GLfloat epsilon);


/* glmFindGroup: Find a group in the model
 */
GLMgroup* 
glmFindGroup(GLMmodel* model, const char* name);


GLboolean 
glmBuildTexture(GLMmodel *model, GLMmaterial* material);

GLvoid 
glmBuildTexture2(int width, int heigh, GLenum format, GLenum type, const void* pData, GLuint* texid);

// GLvoid
// glmGetTexture(const GLMmodel *model, int groupid, unsigned char* pData);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _GLM_H_