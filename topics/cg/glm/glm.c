/*
   glm.c
   Nate Robins, 1997
   ndr@pobox.com, http://www.pobox.com/~ndr/

   Wavefront OBJ model file format reader/writer/manipulator.

   Includes routines for generating smooth normals with
   preservation of edges, welding redundant vertices & texture
   coordinate generation (spheremap and planar projections) + more.

*/
#include "glm.h"
#include <gl/glu.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define T(x) (model->triangles[(x)])

#ifdef _WIN32
#	pragma warning(disable:4305) // truncation from 'const double' to 'float'
#	pragma warning(disable:4244) // conversion from 'double' to 'float', possible loss of data
#	pragma warning(disable:4996) // This function or variable may be unsafe
#endif

#define LINE_BYTES(w, b)	((((w * b) + 31) & ~31) >> 3)

#define DEFAULT_TEX_ID 0xFFFF
#define DEFAULT_PIXEL_FORMAT GL_RGB
#define SINGLE_STRING_GROUP_NAMES 1

#ifdef _DEBUG
#define _printGLError() \
{\
	GLenum err;\
	while ( (err = glGetError()) != GL_NO_ERROR)\
	{\
	printf("GL error: %s in %s(%d)\n", gluErrorString(err), __FILE__,  __LINE__);\
	}\
}
#else
#define _printGLError()
#endif

/* _GLMnode: general purpose node
*/
typedef struct _GLMnode {
    GLuint           index;
    GLboolean        averaged;
    struct _GLMnode* next;
} GLMnode;

#ifndef _WIN32

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

typedef struct tagBITMAPFILEHEADER {
	WORD    bfType; // must be "BM"
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;
/* constants for the biCompression field */
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#endif

#define BM 0x4D42 // "BM" DWORD

/* Load uncompressed 24-bit or 32-bit bmp file
*/
static void* glmLoadBitmapFile(const char* fileName, BITMAPINFOHEADER *bmInfoHeader)
{
    FILE *fp = 0;
    BITMAPFILEHEADER bmFileHeader;
    BYTE *pData, *pLine, *pTmp;
    BYTE tempRGB;  //our swap variable
    unsigned int nChannel;
    size_t	widthStep;
    long iy, ix;

    fp = fopen(fileName, "rb");
    if (fp == NULL)
        return NULL;

    fread(&bmFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);

    // verify the bitmap file header
    if (bmFileHeader.bfType != BM)
    {
        fclose(fp);
        return NULL;
    }

    // read the bitmap info header
    fread(bmInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    assert(bmInfoHeader->biCompression == BI_RGB);

    // move file point to the begin of bitmap data
    fseek(fp, bmFileHeader.bfOffBits, SEEK_SET);

    // allocate enough memory for the bit image data
    widthStep = LINE_BYTES(bmInfoHeader->biWidth, bmInfoHeader->biBitCount);
    pData = (unsigned char *)malloc(widthStep * bmInfoHeader->biHeight);
    if (!pData)
    {
        fclose(fp);
        return NULL;
    }

    // read the bitmap image data
	pTmp = pData;
    fread(pData, widthStep * bmInfoHeader->biHeight, 1, fp);
    if (!pData)
	{
		free(pTmp);
        fclose(fp);
        return NULL;
    }

    if (bmInfoHeader->biBitCount == 24 || 
        bmInfoHeader->biBitCount == 32)
    {
        nChannel = bmInfoHeader->biBitCount / 8;
        pLine = pData;
        //swap the r and b values to get RGB (bitmap is BGR)
        for (iy = 0; iy < bmInfoHeader->biHeight; ++iy, pLine+=widthStep)
        {
            pTmp = pLine;
            for (ix = 0; ix < bmInfoHeader->biWidth; ++ix, pTmp+=nChannel)
            {
                tempRGB = pTmp[0];
                pTmp[0] = pTmp[2];
                pTmp[2] = tempRGB;
            }
        }
    }

    return pData;
}

/* Save uncompressed 24-bit or 32-bit bmp file
*/
static GLboolean glmSaveBitmapFile(const char* fileName, unsigned char *pData,
                                   long nWidth, long nHeight, long nBitCount)
{
    FILE * fp = 0;
    DWORD dwTemp;
    BITMAPFILEHEADER bmFileHeader;
    BITMAPINFOHEADER bmInfoHeader;
    long nLineBytes, i;

    fp = fopen(fileName, "wb+");
    if (!fp)
        return GL_FALSE;

    nLineBytes = LINE_BYTES(nWidth, nBitCount);
    dwTemp = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    if (nBitCount == 16)
        dwTemp += 4*sizeof(unsigned long);
    else if (nBitCount == 8)
        dwTemp += 256*sizeof(RGBQUAD);
	
	// Set the bitmap file header
	memset(&bmFileHeader, 0, sizeof(BITMAPFILEHEADER));
	bmFileHeader.bfType = BM;
	bmFileHeader.bfSize = dwTemp + nLineBytes * nHeight;
	bmFileHeader.bfOffBits = dwTemp;

    memset(&bmInfoHeader, 0, sizeof(BITMAPINFOHEADER));
    bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmInfoHeader.biWidth = nWidth;
    bmInfoHeader.biHeight = nHeight;
    bmInfoHeader.biBitCount= (unsigned short)nBitCount;
    bmInfoHeader.biPlanes = 1;
    if (nBitCount == 16)
        bmInfoHeader.biCompression = BI_BITFIELDS;

	fwrite(&bmFileHeader,1, sizeof(BITMAPFILEHEADER),fp);
    fwrite(&bmInfoHeader,1, sizeof(BITMAPINFOHEADER),fp);

    if (nBitCount == 16)
    {
        unsigned long mask[4] = {0xF800, 0x7E0, 0x1F, 0};
        fwrite(mask, 4, sizeof(unsigned long), fp);
    }
    else if (nBitCount == 8) // index map
    {
        RGBQUAD rgb[256];
        for (i=0; i<256; i++)
        {
            rgb[i].rgbBlue	= (unsigned char)i;
            rgb[i].rgbGreen	= (unsigned char)i;
            rgb[i].rgbRed	= (unsigned char)i;
            rgb[i].rgbReserved	= 0;
        }
        fwrite(rgb, 256, sizeof(RGBQUAD), fp);
    }

    fwrite(pData,1, nLineBytes*nHeight,fp);

    fclose(fp);
    return GL_TRUE;
}

/* glmFileName: return the pure file name given a path
 *
 * path - file system path
 *
 * NOTE: the return string is part of the path
 */
static const char* glmFileName(const char* path)
{
    const char* s;

	s = strrchr(path, '/');
	if (s == NULL)
		s = strrchr(path, '\\');

	if (NULL==s)
		s = path;

    return s+1;
}

/* glmDirName: return the directory given a path
 *
 * dir	- file system directory
 * path - file system path
 *
 * NOTE: the dir must have enough space
 */
static char* glmDirName(char* dir, const char* path)
{
    char* s;

	strcpy(dir, path);
	s = strrchr(dir, '/');
	if (s == NULL)
		s = strrchr(dir, '\\');

    if (s)
        s[1] = '\0';
    else
        dir[0] = '\0';

    return dir;
}

/* glmNewPath: return the new path with new dir
 *
 * newpath	- new file system path
 * oldpath	- old file system path
 * refpath	- file system path that contains the target directory
 *
 * NOTE: the newpath must have enough space
 */
static char* glmNewPath(char* newpath, const char* oldpath, const char* refpath)
{
	char *snew;
	const char *sold;

	strcpy(newpath, refpath);

	snew = strrchr(newpath, '/');
	if (snew == NULL)
		snew = strrchr(newpath, '\\');

	sold = strrchr(oldpath, '/');
	if (sold == NULL)
		sold = strrchr(oldpath, '\\');

	if (snew && sold)
	{
		snew[1] = '\0';
		strcat(newpath, sold);
	}
	else
	{
		printf("glmNewPath() warning: invalid source or target path.\n");
		newpath[0] = '\0';
		return NULL;
	}
}


/* glmCopyFile: Copy file
*/
static GLboolean glmCopyFile(const char* from, const char* to)
{
    FILE *fp_from=NULL, *fp_to=NULL;
    char ch;

    fp_from = fopen(from, "rb");
    if (fp_from==NULL) 
	{
        fprintf(stderr, "glmCopyFile() failed: can't open file \"%s\" to read.\n", from);
        return GL_FALSE;
    }
    fp_to = fopen(to, "wb");
    if (fp_to==NULL) 
	{
        fprintf(stderr, "glmCopyFile() failed: can't open file \"%s\" to write.\n", to);
        goto failed;
    }
    while (!feof(fp_from)) 
	{
        ch = fgetc(fp_from);
		if (ferror(fp_from)) 
		{
			fprintf(stderr, "glmCopyFile() failed: can't read in file \"%s\".\n", from);
            clearerr(fp_from);
			goto failed;
        }
        fputc(ch, fp_to);
		if (ferror(fp_to)) 
		{
			fprintf(stderr, "glmCopyFile() failed: can't write to file \"%s\".\n", to);
			clearerr(fp_to);
			goto failed;
		}
    }
	return GL_TRUE;

failed:
    if (fp_from) fclose(fp_from);
	if (fp_to) fclose(fp_to);
    return GL_FALSE;
}

/* glmMax: returns the maximum of two floats */
static GLfloat glmMax(GLfloat a, GLfloat b)
{
    return (b > a) ? b : a;
}

/* glmAbs: returns the absolute value of a float */
static GLfloat glmAbs(GLfloat f)
{
    return (f < 0) ? -f : f;
}

/* glmDot: compute the dot product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
GLfloat glmDot(GLfloat* u, GLfloat* v)
{
    assert(u); assert(v);
    return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

/* glmCross: compute the cross product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 * n - array of 3 GLfloats (GLfloat n[3]) to return the cross product in
 */
GLvoid glmCross(GLfloat* u, GLfloat* v, GLfloat* n)
{
    assert(u); assert(v); assert(n);
    n[0] = u[1]*v[2] - u[2]*v[1];
    n[1] = u[2]*v[0] - u[0]*v[2];
    n[2] = u[0]*v[1] - u[1]*v[0];
}

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
GLvoid glmNormalize(GLfloat* v)
{
    GLfloat l;
    assert(v);

    l = (GLfloat)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}

/* glmEqual: compares two vectors and returns GL_TRUE if they are
 * equal (within a certain threshold) or GL_FALSE if not. An epsilon
 * that works fairly well is 0.000001.
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static GLboolean glmEqual(GLfloat* u, GLfloat* v, GLfloat epsilon)
{
    if (glmAbs(u[0] - v[0]) < epsilon &&
        glmAbs(u[1] - v[1]) < epsilon &&
        glmAbs(u[2] - v[2]) < epsilon)
    {
        return GL_TRUE;
    }
    return GL_FALSE;
}

/* glmWeldVectors: eliminate (weld) vectors that are within an
 * epsilon of each other.
 *
 * vectors    - array of GLfloat[3]'s to be welded
 * numvectors - number of GLfloat[3]'s in vectors
 * epsilon    - maximum difference between vectors
 *
 */
static GLuint glmWeldVectors(GLfloat* copies, GLfloat* vectors, GLuint* numvectors, GLfloat epsilon)
{
    GLuint   copied;
    GLuint   i, j;

    memcpy(copies, vectors, (sizeof(GLfloat) * 3 * (*numvectors + 1)));
    copied = 1;
    for (i = 1; i <= *numvectors; i++)
    {
        for (j = 1; j <= copied; j++)
        {
            if (glmEqual(&vectors[3 * i], &copies[3 * j], epsilon))
            {
                goto duplicate;
            }
        }

        // must not be any duplicates -- add to the copies array
        copies[3 * copied + 0] = vectors[3 * i + 0];
        copies[3 * copied + 1] = vectors[3 * i + 1];
        copies[3 * copied + 2] = vectors[3 * i + 2];
        j = copied;				// pass this along for below
        copied++;

duplicate:
        //set the first component of this vector to point at the correct index into the new copies array
        vectors[3 * i + 0] = (GLfloat)j;
    }

    *numvectors = copied-1;
    return copied;
}


/* glmFindGroup: Find a group in the model
*/
GLMgroup* glmFindGroup(GLMmodel* model, const char* name)
{
    GLint r;
    GLMgroup* group=NULL;

    assert(model);

    group = model->groups;
    while(group)
    {
        r = strcmp(group->name, name);
        if (r==0)
            break;
        group = group->next;
    }

    return group;
}

/* glmAddGroup: Add a group to the model
*/
GLMgroup* glmAddGroup(GLMmodel* model, const char* name)
{
    GLMgroup* group;

    group = glmFindGroup(model, name);
    if (!group)
    {
        group = (GLMgroup*)malloc(sizeof(GLMgroup));
        group->name = strdup(name);
        group->material = 0;
        group->numtriangles = 0;
        group->triangles = NULL;
        group->next = model->groups;
        model->groups = group;
        model->numgroups++;
    }

    return group;
}

/* glmFindGroup: Find a material in the model
*/
GLuint glmFindMaterial(GLMmodel* model, const char* name)
{
    GLuint i;

    // XXX doing a linear search on a string key'd list is pretty lame,
    //but it works and is fast enough for now.
    for (i = 0; i < model->nummaterials; i++)
    {
        if (!strcmp(model->materials[i].name, name))
            goto found;
    }

    // didn't find the name, so print a warning and return the default
    // material (0).
    printf("glmFindMaterial():  can't find material \"%s\".\n", name);
    i = 0;

found:
    return i;
}

/* glmReadMTL: read a wavefront material library file
 *
 * model - properly initialized GLMmodel structure
 * name  - name of the material library
 */
static GLvoid glmReadMTL(GLMmodel* model, const char* name)
{
    char buf[MAX_PATH];
    GLuint nummaterials, i;
    char filename[MAX_PATH];
    FILE* fd=NULL;

    if (NULL==model || name==NULL)
        return;

    glmDirName(filename, model->pathname);
    strcat(filename, name);

    fd = fopen(filename, "r");
    if (!fd)
    {
        fprintf(stderr, "glmReadMTL() failed: can't open material file \"%s\".\n", filename);
        exit(1);
    }

    // count the number of materials in the file
    nummaterials = 1;
    while(fscanf(fd, "%s", buf) != EOF)
    {
        switch(buf[0])
        {
        case '#':				// comment
            // eat up rest of line
            fgets(buf, sizeof(buf), fd);
            break;
        case 'n':				// newmtl
            fgets(buf, sizeof(buf), fd);
            nummaterials++;
            sscanf(buf, "%s %s", buf, buf);
            break;
        default:
            // eat up rest of line
            fgets(buf, sizeof(buf), fd);
            break;
        }
    }

    rewind(fd);

    model->materials = (GLMmaterial*)malloc(sizeof(GLMmaterial) * nummaterials);
    model->nummaterials = nummaterials;

    // set the default material
    for (i = 0; i < nummaterials; i++)
    {
        model->materials[i].name = NULL;
        model->materials[i].shininess = 65.0f;
        model->materials[i].diffuse[0] = 0.8f;
        model->materials[i].diffuse[1] = 0.8f;
        model->materials[i].diffuse[2] = 0.8f;
        model->materials[i].diffuse[3] = 1.0f;
        model->materials[i].ambient[0] = 0.2f;
        model->materials[i].ambient[1] = 0.2f;
        model->materials[i].ambient[2] = 0.2f;
        model->materials[i].ambient[3] = 1.0f;
        model->materials[i].specular[0] = 0.0f;
        model->materials[i].specular[1] = 0.0f;
        model->materials[i].specular[2] = 0.0f;
        model->materials[i].specular[3] = 1.0f;
        model->materials[i].kdMapName = NULL;
        model->materials[i].kdMapId = DEFAULT_TEX_ID;
		model->materials[i].format = DEFAULT_PIXEL_FORMAT;
    }
    model->materials[0].name = strdup("default");

    // now, read in the data
    nummaterials = 0;
    while(fscanf(fd, "%s", buf) != EOF)
    {
        switch(buf[0])
        {
        case '#':				// comment
            // eat up rest of line
            fgets(buf, sizeof(buf), fd);
            break;
        case 'n':				// newmtl
            fgets(buf, sizeof(buf), fd);
            sscanf(buf, "%s %s", buf, buf);
            nummaterials++;
            model->materials[nummaterials].name = strdup(buf);
            break;
        case 'N':
            fscanf(fd, "%f", &model->materials[nummaterials].shininess);
            // wavefront shininess is from [0, 1000], so scale for OpenGL
            model->materials[nummaterials].shininess /= 1000.0;
            model->materials[nummaterials].shininess *= 128.0;
            break;
        case 'K':
            switch(buf[1])
            {
            case 'd':
                fscanf(fd, "%f %f %f",
                       &model->materials[nummaterials].diffuse[0],
                       &model->materials[nummaterials].diffuse[1],
                       &model->materials[nummaterials].diffuse[2]);
                break;
            case 's':
                fscanf(fd, "%f %f %f",
                       &model->materials[nummaterials].specular[0],
                       &model->materials[nummaterials].specular[1],
                       &model->materials[nummaterials].specular[2]);
                break;
            case 'a':
                fscanf(fd, "%f %f %f",
                       &model->materials[nummaterials].ambient[0],
                       &model->materials[nummaterials].ambient[1],
                       &model->materials[nummaterials].ambient[2]);
                break;
            default:
                // eat up rest of line
                fgets(buf, sizeof(buf), fd);
                break;
            }
            break;
        case 'm':
            if (buf[4]=='K' && buf[5]=='d')
            {
                fgets(buf, sizeof(buf), fd);
                sscanf(buf, "%s %s", buf, buf);
                model->materials[nummaterials].kdMapName = strdup(buf);
            }
            else
                fgets(buf, sizeof(buf), fd);
            break;
        default:
            // eat up rest of line
            fgets(buf, sizeof(buf), fd);
            break;
        }
    }

    fclose(fd);
}

/* glmWriteMTL: write a wavefront material library file
 *
 * model      - properly initialized GLMmodel structure
 * modelpath  - pathname of the model being written
 * mtllibname - name of the material library to be written
 */
static GLvoid glmWriteMTL(GLMmodel* model, const char* modelpath, const char* mtllibname)
{
	FILE* file=NULL;
	char filename[MAX_PATH];
	char modeldir[MAX_PATH];	// e.g. d:/
	char modelname[MAX_PATH];	// e.g. new_model 
	char suffix[MAX_PATH];		// e.g. skin_hi.bmp
	char oldmodeldir[MAX_PATH];	// e.g. e:/
	char oldpath[MAX_PATH];		// e.g. e:/general_skin_hi.bmp
	char *s;
    GLMmaterial* material=NULL;
    GLuint i;
	int oldlen;
	
	oldlen = strlen(glmFileName(model->pathname));
    glmDirName(modeldir, modelpath);
	strcpy(modelname, glmFileName(modelpath));
	s = strrchr(modelname, '.');
	s[0] = '\0';
    sprintf(filename, "%s%s", modeldir, mtllibname);

	glmDirName(oldmodeldir, model->pathname);

    /* open the file */
    file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "glmWriteMTL() failed: can't open file \"%s\".\n",
                filename);
        exit(1);
    }

    /* spit out a header */
    fprintf(file, "#  \n");
    fprintf(file, "#  Wavefront MTL generated by GLM library\n");
    fprintf(file, "#  \n");
    fprintf(file, "#  GLM library\n");
    fprintf(file, "#  Nate Robins\n");
    fprintf(file, "#  ndr@pobox.com\n");
    fprintf(file, "#  http://www.pobox.com/~ndr\n");
    fprintf(file, "#  \n\n");

    for (i = 1; i < model->nummaterials; i++) {
        material = &model->materials[i];
        fprintf(file, "newmtl %s\n", material->name);
        fprintf(file, "Ka %f %f %f\n",
                material->ambient[0], material->ambient[1], material->ambient[2]);
        fprintf(file, "Kd %f %f %f\n",
                material->diffuse[0], material->diffuse[1], material->diffuse[2]);
        fprintf(file, "Ks %f %f %f\n",
                material->specular[0],material->specular[1],material->specular[2]);
        fprintf(file, "Ns %f\n", material->shininess / 128.0 * 1000.0);

        if(material->kdMapName!=NULL) {            
			strcpy(suffix, material->kdMapName+oldlen-3);
			sprintf(filename, "%s%s_%s", modeldir, modelname, suffix);
			fprintf(file, "map_Kd %s_%s\n", modelname, suffix);

			if (material->kdMapId == DEFAULT_TEX_ID)
			{
				sprintf(oldpath, "%s%s", oldmodeldir, material->kdMapName);
 				glmCopyFile(oldpath, filename);
			}
			else
            {
                int w, h;
                unsigned char *pTex = NULL;
				int nBitCount;
				GLenum format;

				glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, material->kdMapId);

                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
				
				nBitCount = (material->format == GL_RGBA) ? 32 : 24;
				format = (material->format == GL_RGBA) ? GL_BGRA_EXT : GL_BGR_EXT;

                pTex = (unsigned char *)malloc(LINE_BYTES(w, nBitCount)*h);
                glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, pTex);
				glDisable(GL_TEXTURE_2D);
				
				_printGLError();

                glmSaveBitmapFile(filename, pTex, w, h, nBitCount);
                if(pTex) {free(pTex); pTex=NULL;}
            }
        }

        fprintf(file, "\n");
    }

    fclose(file);
}


/* glmFirstPass: first pass at a Wavefront OBJ file that gets all the
 * statistics of the model (such as #vertices, #normals, etc)
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLvoid glmFirstPass(GLMmodel* model, FILE* file)
{
    GLuint    numvertices;		/* number of vertices in model */
    GLuint    numnormals;			/* number of normals in model */
    GLuint    numtexcoords;		/* number of texcoords in model */
    GLuint    numtriangles;		/* number of triangles in model */
    GLMgroup* group;			/* current group */
    unsigned  v, n, t;
    char      buf[128];

    /* make a default group */
    group = glmAddGroup(model, "default");

    numvertices = numnormals = numtexcoords = numtriangles = 0;
    while(fscanf(file, "%s", buf) != EOF) {
        switch(buf[0]) {
        case '#':				/* comment */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'v':				/* v, vn, vt */
            switch(buf[1]) {
            case '\0':			/* vertex */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numvertices++;
                break;
            case 'n':				/* normal */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numnormals++;
                break;
            case 't':				/* texcoord */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numtexcoords++;
                break;
            default:
                printf("glmFirstPass(): Unknown token \"%s\".\n", buf);
                exit(1);
                break;
            }
            break;
        case 'm':
            fgets(buf, sizeof(buf), file);
            sscanf(buf, "%s %s", buf, buf);
            model->mtllibname = strdup(buf);
            glmReadMTL(model, buf);
            break;
        case 'u':
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'g':				/* group */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
            sscanf(buf, "%s", buf);
#else
            buf[strlen(buf)-1] = '\0';	/* nuke '\n' */
#endif
            group = glmAddGroup(model, buf);
            break;
        case 'f':				/* face */
            v = n = t = 0;
            fscanf(file, "%s", buf);
            /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
            if (strstr(buf, "//")) {
                /* v//n */
                sscanf(buf, "%d//%d", &v, &n);
                fscanf(file, "%d//%d", &v, &n);
                fscanf(file, "%d//%d", &v, &n);
                numtriangles++;
                group->numtriangles++;
                while(fscanf(file, "%d//%d", &v, &n) > 0) {
                    numtriangles++;
                    group->numtriangles++;
                }
            } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
                /* v/t/n */
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                numtriangles++;
                group->numtriangles++;
                while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
                    numtriangles++;
                    group->numtriangles++;
                }
            } else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
                /* v/t */
                fscanf(file, "%d/%d", &v, &t);
                fscanf(file, "%d/%d", &v, &t);
                numtriangles++;
                group->numtriangles++;
                while(fscanf(file, "%d/%d", &v, &t) > 0) {
                    numtriangles++;
                    group->numtriangles++;
                }
            } else {
                /* v */
                fscanf(file, "%d", &v);
                fscanf(file, "%d", &v);
                numtriangles++;
                group->numtriangles++;
                while(fscanf(file, "%d", &v) > 0) {
                    numtriangles++;
                    group->numtriangles++;
                }
            }
            break;

        default:
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        }
    }

    /* set the stats in the model structure */
    model->numvertices  = numvertices;
    model->numnormals   = numnormals;
    model->numtexcoords = numtexcoords;
    model->numtriangles = numtriangles;

    /* allocate memory for the triangles in each group */
    group = model->groups;
    while(group) {
        group->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
        group->numtriangles = 0;
        group = group->next;
    }
}

/* glmSecondPass: second pass at a Wavefront OBJ file that gets all
 * the data.
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLvoid glmSecondPass(GLMmodel* model, FILE* file)
{
    GLuint    numvertices;		/* number of vertices in model */
    GLuint    numnormals;			/* number of normals in model */
    GLuint    numtexcoords;		/* number of texcoords in model */
    GLuint    numtriangles;		/* number of triangles in model */
    GLfloat*  vertices;			/* array of vertices  */
    GLfloat*  normals;			/* array of normals */
    GLfloat*  texcoords;			/* array of texture coordinates */
    GLMgroup* group;			/* current group pointer */
    GLuint    material;			/* current material */
    GLuint    v, n, t;
    char      buf[128];

    /* set the pointer shortcuts */
    vertices     = model->vertices;
    normals      = model->normals;
    texcoords    = model->texcoords;
    group        = model->groups;

    /* on the second pass through the file, read all the data into the
       allocated arrays */
    numvertices = numnormals = numtexcoords = 1;
    numtriangles = 0;
    material = 0;
    while(fscanf(file, "%s", buf) != EOF) {
        switch(buf[0]) {
        case '#':				/* comment */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'v':				/* v, vn, vt */
            switch(buf[1]) {
            case '\0':			/* vertex */
                fscanf(file, "%f %f %f",
                       &vertices[3 * numvertices + 0],
                       &vertices[3 * numvertices + 1],
                       &vertices[3 * numvertices + 2]);
                numvertices++;
                break;
            case 'n':				/* normal */
                fscanf(file, "%f %f %f",
                       &normals[3 * numnormals + 0],
                       &normals[3 * numnormals + 1],
                       &normals[3 * numnormals + 2]);
                numnormals++;
                break;
            case 't':				/* texcoord */
                fscanf(file, "%f %f",
                       &texcoords[2 * numtexcoords + 0],
                       &texcoords[2 * numtexcoords + 1]);
                numtexcoords++;
                break;
            }
            break;
        case 'u':
            fgets(buf, sizeof(buf), file);
            sscanf(buf, "%s %s", buf, buf);
            group->material = material = glmFindMaterial(model, buf);
            break;
        case 'g':				/* group */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
            sscanf(buf, "%s", buf);
#else
            buf[strlen(buf)-1] = '\0';	/* nuke '\n' */
#endif
            group = glmFindGroup(model, buf);
            group->material = material;
            break;
        case 'f':				/* face */
            v = n = t = 0;
            fscanf(file, "%s", buf);
            /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
            if (strstr(buf, "//")) {
                /* v//n */
                sscanf(buf, "%d//%d", &v, &n);
                T(numtriangles).vindices[0] = v;
                T(numtriangles).nindices[0] = n;
                fscanf(file, "%d//%d", &v, &n);
                T(numtriangles).vindices[1] = v;
                T(numtriangles).nindices[1] = n;
                fscanf(file, "%d//%d", &v, &n);
                T(numtriangles).vindices[2] = v;
                T(numtriangles).nindices[2] = n;
                group->triangles[group->numtriangles++] = numtriangles;
                numtriangles++;
                while(fscanf(file, "%d//%d", &v, &n) > 0) {
                    T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                    T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
                    T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                    T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).nindices[2] = n;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                }
            } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
                /* v/t/n */
                T(numtriangles).vindices[0] = v;
                T(numtriangles).tindices[0] = t;
                T(numtriangles).nindices[0] = n;
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                T(numtriangles).vindices[1] = v;
                T(numtriangles).tindices[1] = t;
                T(numtriangles).nindices[1] = n;
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                T(numtriangles).vindices[2] = v;
                T(numtriangles).tindices[2] = t;
                T(numtriangles).nindices[2] = n;
                group->triangles[group->numtriangles++] = numtriangles;
                numtriangles++;
                while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
                    T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                    T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
                    T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
                    T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                    T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
                    T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).tindices[2] = t;
                    T(numtriangles).nindices[2] = n;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                }
            } else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
                /* v/t */
                T(numtriangles).vindices[0] = v;
                T(numtriangles).tindices[0] = t;
                fscanf(file, "%d/%d", &v, &t);
                T(numtriangles).vindices[1] = v;
                T(numtriangles).tindices[1] = t;
                fscanf(file, "%d/%d", &v, &t);
                T(numtriangles).vindices[2] = v;
                T(numtriangles).tindices[2] = t;
                group->triangles[group->numtriangles++] = numtriangles;
                numtriangles++;
                while(fscanf(file, "%d/%d", &v, &t) > 0) {
                    T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                    T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
                    T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                    T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).tindices[2] = t;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                }
            } else {
                /* v */
                sscanf(buf, "%d", &v);
                T(numtriangles).vindices[0] = v;
                fscanf(file, "%d", &v);
                T(numtriangles).vindices[1] = v;
                fscanf(file, "%d", &v);
                T(numtriangles).vindices[2] = v;
                group->triangles[group->numtriangles++] = numtriangles;
                numtriangles++;
                while(fscanf(file, "%d", &v) > 0) {
                    T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                    T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                    T(numtriangles).vindices[2] = v;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                }
            }
            break;

        default:
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        }
    }

#if 0
    /* announce the memory requirements */
    printf(" Memory: %d bytes\n",
           numvertices  * 3*sizeof(GLfloat) +
           numnormals   * 3*sizeof(GLfloat) * (numnormals ? 1 : 0) +
           numtexcoords * 3*sizeof(GLfloat) * (numtexcoords ? 1 : 0) +
           numtriangles * sizeof(GLMtriangle));
#endif
}


/* public functions */


/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure
 */
GLfloat glmUnitize(GLMmodel* model)
{
    GLuint  i;
    GLfloat maxx, minx, maxy, miny, maxz, minz;
    GLfloat cx, cy, cz, w, h, d;
    GLfloat scale;

    assert(model);
    assert(model->vertices);

    /* get the max/mins */
    maxx = minx = model->vertices[3 + 0];
    maxy = miny = model->vertices[3 + 1];
    maxz = minz = model->vertices[3 + 2];
    for (i = 1; i <= model->numvertices; i++) {
        if (maxx < model->vertices[3 * i + 0])
            maxx = model->vertices[3 * i + 0];
        if (minx > model->vertices[3 * i + 0])
            minx = model->vertices[3 * i + 0];

        if (maxy < model->vertices[3 * i + 1])
            maxy = model->vertices[3 * i + 1];
        if (miny > model->vertices[3 * i + 1])
            miny = model->vertices[3 * i + 1];

        if (maxz < model->vertices[3 * i + 2])
            maxz = model->vertices[3 * i + 2];
        if (minz > model->vertices[3 * i + 2])
            minz = model->vertices[3 * i + 2];
    }

    /* calculate model width, height, and depth */
    w = glmAbs(maxx-minx);
    h = glmAbs(maxy-miny);
    d = glmAbs(maxz-minz);

    /* calculate center of the model */
    cx = (maxx + minx) / 2.0;
    cy = (maxy + miny) / 2.0;
    cz = (maxz + minz) / 2.0;

    /* calculate unitizing scale factor */
    scale = 2.0 / glmMax(glmMax(w, h), d);

    /* translate around center then scale */
    for (i = 1; i <= model->numvertices; i++) {
        model->vertices[3 * i + 0] -= cx;
        model->vertices[3 * i + 1] -= cy;
        model->vertices[3 * i + 2] -= cz;
        model->vertices[3 * i + 0] *= scale;
        model->vertices[3 * i + 1] *= scale;
        model->vertices[3 * i + 2] *= scale;
    }

#if 0
    model->center[0] = 0.0f;
    model->center[1] = 0.0f;
    model->center[2] = 0.0f;
#endif

    return scale;
}

/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model      - initialized GLMmodel structure
 * dimensions - array of 3 GLfloats (GLfloat dimensions[3])
 */
GLvoid glmDimensions(GLMmodel* model, GLfloat* dimensions)
{
    GLuint i;
    GLfloat maxx, minx, maxy, miny, maxz, minz;

    assert(model);
    assert(model->vertices);
    assert(dimensions);

    /* get the max/mins */
    maxx = minx = model->vertices[3 + 0];
    maxy = miny = model->vertices[3 + 1];
    maxz = minz = model->vertices[3 + 2];
    for (i = 1; i <= model->numvertices; i++) {
        if (maxx < model->vertices[3 * i + 0])
            maxx = model->vertices[3 * i + 0];
        if (minx > model->vertices[3 * i + 0])
            minx = model->vertices[3 * i + 0];

        if (maxy < model->vertices[3 * i + 1])
            maxy = model->vertices[3 * i + 1];
        if (miny > model->vertices[3 * i + 1])
            miny = model->vertices[3 * i + 1];

        if (maxz < model->vertices[3 * i + 2])
            maxz = model->vertices[3 * i + 2];
        if (minz > model->vertices[3 * i + 2])
            minz = model->vertices[3 * i + 2];
    }

    /* calculate model width, height, and depth */
    dimensions[0] = glmAbs(maxx - minx);
    dimensions[1] = glmAbs(maxy - miny);
    dimensions[2] = glmAbs(maxz - minz);
}

/* glmScale: Scales a model by a given amount.
 *
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid glmScale(GLMmodel* model, GLfloat scale)
{
    GLuint i;

    for (i = 1; i <= model->numvertices; i++) {
        model->vertices[3 * i + 0] *= scale;
        model->vertices[3 * i + 1] *= scale;
        model->vertices[3 * i + 2] *= scale;
    }
}

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.  Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 *
 * model - properly initialized GLMmodel structure
 */
GLvoid glmReverseWinding(GLMmodel* model)
{
    GLuint i, swap;

    assert(model);

    for (i = 0; i < model->numtriangles; i++) {
        swap = T(i).vindices[0];
        T(i).vindices[0] = T(i).vindices[2];
        T(i).vindices[2] = swap;

        if (model->numnormals) {
            swap = T(i).nindices[0];
            T(i).nindices[0] = T(i).nindices[2];
            T(i).nindices[2] = swap;
        }

        if (model->numtexcoords) {
            swap = T(i).tindices[0];
            T(i).tindices[0] = T(i).tindices[2];
            T(i).tindices[2] = swap;
        }
    }

    /* reverse facet normals */
    for (i = 1; i <= model->numfacetnorms; i++) {
        model->facetnorms[3 * i + 0] = -model->facetnorms[3 * i + 0];
        model->facetnorms[3 * i + 1] = -model->facetnorms[3 * i + 1];
        model->facetnorms[3 * i + 2] = -model->facetnorms[3 * i + 2];
    }

    /* reverse vertex normals */
    for (i = 1; i <= model->numnormals; i++) {
        model->normals[3 * i + 0] = -model->normals[3 * i + 0];
        model->normals[3 * i + 1] = -model->normals[3 * i + 1];
        model->normals[3 * i + 2] = -model->normals[3 * i + 2];
    }
}

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid glmFacetNormals(GLMmodel* model)
{
    GLuint  i;
    GLfloat u[3];
    GLfloat v[3];

    assert(model);
    assert(model->vertices);

    // clobber any old facet normals
    if (model->numfacetnorms && model->numfacetnorms != model->numtriangles) {
        free(model->facetnorms);
        model->facetnorms = NULL;
    }
    if (NULL == model->facetnorms) {
        // allocate memory for the new facet normals
        model->numfacetnorms = model->numtriangles;
        model->facetnorms = (GLfloat*)malloc(sizeof(GLfloat) *
                                             3 * (model->numfacetnorms + 1));
    }

    for (i = 0; i < model->numtriangles; i++) {
        model->triangles[i].findex = i+1;

        u[0] = model->vertices[3 * T(i).vindices[1] + 0] -
            model->vertices[3 * T(i).vindices[0] + 0];
        u[1] = model->vertices[3 * T(i).vindices[1] + 1] -
            model->vertices[3 * T(i).vindices[0] + 1];
        u[2] = model->vertices[3 * T(i).vindices[1] + 2] -
            model->vertices[3 * T(i).vindices[0] + 2];

        v[0] = model->vertices[3 * T(i).vindices[2] + 0] -
            model->vertices[3 * T(i).vindices[0] + 0];
        v[1] = model->vertices[3 * T(i).vindices[2] + 1] -
            model->vertices[3 * T(i).vindices[0] + 1];
        v[2] = model->vertices[3 * T(i).vindices[2] + 2] -
            model->vertices[3 * T(i).vindices[0] + 2];

        glmCross(u, v, &model->facetnorms[3 * (i+1)]);
        glmNormalize(&model->facetnorms[3 * (i+1)]);
    }
}

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
GLvoid glmVertexNormals(GLMmodel* model, GLfloat angle)
{
    GLMnode*  node =NULL;
    GLMnode*  tail=NULL;
    GLMnode** members=NULL;
    GLfloat*  normals=NULL;
    GLuint    numnormals;
    GLfloat   average[3];
    GLfloat   dot, cos_angle;
    GLuint    i, avg;

    assert(model);
    assert(model->facetnorms);

    // calculate the cosine of the angle (in degrees)
    cos_angle = cos(angle * M_PI / 180.0);

    // nuke any previous normals
    if (model->normals)
    {
        free(model->normals);
        model->normals = NULL;
    }

    // allocate space for new normals
    model->numnormals = model->numtriangles * 3; // 3 normals per triangle
    //model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));
    normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));


    // allocate a structure that will hold a linked list of triangle indices for each vertex
    members = (GLMnode**)malloc(sizeof(GLMnode*) * (model->numvertices + 1));
    for (i = 1; i <= model->numvertices; i++)
        members[i] = NULL;

    // for every triangle, create a node for each vertex in it
    for (i = 0; i < model->numtriangles; i++)
    {
        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[0]];
        members[T(i).vindices[0]] = node;

        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[1]];
        members[T(i).vindices[1]] = node;

        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[2]];
        members[T(i).vindices[2]] = node;
    }

    // calculate the average normal for each vertex
    numnormals = 1;
    for (i = 1; i <= model->numvertices; i++)
    {
        // calculate an average normal for this vertex by averaging the
        // facet normal of every triangle this vertex is in
        node = members[i];

        if (!node)
            fprintf(stderr, "glmVertexNormals(): vertex w/o a triangle\n");

        average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
        avg = 0;
        while (node)
        {
            // only average if the dot product of the angle between the two
            // facet normals is greater than the cosine of the threshold
            // angle -- or, said another way, the angle between the two
            // facet normals is less than (or equal to) the threshold angle
            dot = glmDot(&model->facetnorms[3 * T(node->index).findex],
                         &model->facetnorms[3 * T(members[i]->index).findex]);

            if (dot > cos_angle)
            {
                node->averaged = GL_TRUE;
                average[0] += model->facetnorms[3 * T(node->index).findex + 0];
                average[1] += model->facetnorms[3 * T(node->index).findex + 1];
                average[2] += model->facetnorms[3 * T(node->index).findex + 2];
                avg = 1;			// we averaged at least one normal!
            }
            else
            {
                node->averaged = GL_FALSE;
            }
            node = node->next;
        }

        if (avg)
        {
            // normalize the averaged normal
            glmNormalize(average);

            // add the normal to the vertex normals list
            normals[3 * numnormals + 0] = average[0];
            normals[3 * numnormals + 1] = average[1];
            normals[3 * numnormals + 2] = average[2];
            avg = numnormals;
            numnormals++;
        }

        // set the normal of this vertex in each triangle it is in
        node = members[i];
        while (node)
        {
            if (node->averaged)
            {
                // if this node was averaged, use the average normal
                if (T(node->index).vindices[0] == i)
                    T(node->index).nindices[0] = avg;
                else if (T(node->index).vindices[1] == i)
                    T(node->index).nindices[1] = avg;
                else if (T(node->index).vindices[2] == i)
                    T(node->index).nindices[2] = avg;
            }
            else
            {
                // if this node wasn't averaged, use the facet normal
                normals[3 * numnormals + 0] = model->facetnorms[3 * T(node->index).findex + 0];
                normals[3 * numnormals + 1] = model->facetnorms[3 * T(node->index).findex + 1];
                normals[3 * numnormals + 2] = model->facetnorms[3 * T(node->index).findex + 2];
                if (T(node->index).vindices[0] == i)
                    T(node->index).nindices[0] = numnormals;
                else if (T(node->index).vindices[1] == i)
                    T(node->index).nindices[1] = numnormals;
                else if (T(node->index).vindices[2] == i)
                    T(node->index).nindices[2] = numnormals;
                numnormals++;
            }
            node = node->next;
        }
    }

    model->numnormals = numnormals - 1;

    // free the member information
    for (i = 1; i <= model->numvertices; i++)
    {
        node = members[i];
        while (node)
        {
            tail = node;
            node = node->next;
            free(tail);
        }
    }
    free(members);

    // pack the normals array (we previously allocated the maximum
    //   number of normals that could possibly be created (numtriangles *
    //   3), so get rid of some of them (usually alot unless none of the
    //   facet normals were averaged))
    //normals = model->normals;
    model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));
    for (i = 1; i <= model->numnormals; i++) {
        model->normals[3 * i + 0] = normals[3 * i + 0];
        model->normals[3 * i + 1] = normals[3 * i + 1];
        model->normals[3 * i + 2] = normals[3 * i + 2];
    }
    free(normals);
    normals = NULL;
}


/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid glmLinearTexture(GLMmodel* model)
{
    GLMgroup *group;
    GLfloat dimensions[3];
    GLfloat x, y, scalefactor;
    GLuint i;

    assert(model);

    if (model->texcoords)
        free(model->texcoords);
    model->numtexcoords = model->numvertices;
    model->texcoords=(GLfloat*)malloc(sizeof(GLfloat)*2*(model->numtexcoords+1));

    glmDimensions(model, dimensions);
    scalefactor = 2.0 /
        glmAbs(glmMax(glmMax(dimensions[0], dimensions[1]), dimensions[2]));

    /* do the calculations */
    for(i = 1; i <= model->numvertices; i++) {
        x = model->vertices[3 * i + 0] * scalefactor;
        y = model->vertices[3 * i + 2] * scalefactor;
        model->texcoords[2 * i + 0] = (x + 1.0) / 2.0;
        model->texcoords[2 * i + 1] = (y + 1.0) / 2.0;
    }

    /* go through and put texture coordinate indices in all the triangles */
    group = model->groups;
    while(group) {
        for(i = 0; i < group->numtriangles; i++) {
            T(group->triangles[i]).tindices[0] = T(group->triangles[i]).vindices[0];
            T(group->triangles[i]).tindices[1] = T(group->triangles[i]).vindices[1];
            T(group->triangles[i]).tindices[2] = T(group->triangles[i]).vindices[2];
        }
        group = group->next;
    }

#if 0
    printf("glmLinearTexture(): generated %d linear texture coordinates\n",
           model->numtexcoords);
#endif
}

/* glmSpheremapTexture: Generates texture coordinates according to a
 * spherical projection of the texture map.  Sometimes referred to as
 * spheremap, or reflection map texture coordinates.  It generates
 * these by using the normal to calculate where that vertex would map
 * onto a sphere.  Since it is impossible to map something flat
 * perfectly onto something spherical, there is distortion at the
 * poles.  This particular implementation causes the poles along the X
 * axis to be distorted.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid glmSpheremapTexture(GLMmodel* model)
{
    GLMgroup* group;
    GLfloat theta, phi, rho, x, y, z, r;
    GLuint i;

    assert(model);
    assert(model->normals);

    if (model->texcoords)
        free(model->texcoords);
    model->numtexcoords = model->numnormals;
    model->texcoords=(GLfloat*)malloc(sizeof(GLfloat)*2*(model->numtexcoords+1));

    for (i = 1; i <= model->numnormals; i++) {
        z = model->normals[3 * i + 0];	/* re-arrange for pole distortion */
        y = model->normals[3 * i + 1];
        x = model->normals[3 * i + 2];
        r = sqrt((x * x) + (y * y));
        rho = sqrt((r * r) + (z * z));

        if(r == 0.0) {
            theta = 0.0;
            phi = 0.0;
        } else {
            if(z == 0.0)
                phi = 3.14159265 / 2.0;
            else
                phi = acos(z / rho);

            if(y == 0.0)
                theta = 3.141592365 / 2.0;
            else
                theta = asin(y / r) + (3.14159265 / 2.0);
        }

        model->texcoords[2 * i + 0] = theta / 3.14159265;
        model->texcoords[2 * i + 1] = phi / 3.14159265;
    }

    /* go through and put texcoord indices in all the triangles */
    group = model->groups;
    while(group) {
        for (i = 0; i < group->numtriangles; i++) {
            T(group->triangles[i]).tindices[0] = T(group->triangles[i]).nindices[0];
            T(group->triangles[i]).tindices[1] = T(group->triangles[i]).nindices[1];
            T(group->triangles[i]).tindices[2] = T(group->triangles[i]).nindices[2];
        }
        group = group->next;
    }
}

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
GLvoid glmDelete(GLMmodel* model)
{
    GLMgroup* group=NULL;
    GLuint i;

    assert(model);

    if (model->pathname)   {free(model->pathname); model->pathname =NULL;}
    if (model->mtllibname) {free(model->mtllibname); model->mtllibname =NULL;}
    if (model->vertices)   {free(model->vertices); model->vertices =NULL;}
    if (model->normals)    {free(model->normals); model->normals = NULL;}
    if (model->texcoords)  {free(model->texcoords); model->texcoords = NULL;}
    if (model->facetnorms) {free(model->facetnorms); model->facetnorms = NULL;}
    if (model->triangles)  {free(model->triangles); model->triangles = NULL;}
    if (model->materials)
    {
        for (i = 0; i < model->nummaterials; i++)
        {
            free(model->materials[i].name);
            model->materials[i].name = NULL;

			if (model->materials[i].kdMapId != DEFAULT_TEX_ID)
				glDeleteTextures(1, &(model->materials[i].kdMapId));
			
			_printGLError();

            if (model->materials[i].kdMapName !=NULL)
            {
                free(model->materials[i].kdMapName);
                model->materials[i].kdMapName = NULL;
            }
            //
        }
    }
    free(model->materials);
    model->materials = NULL;

    while(model->groups) {
        group = model->groups;
        model->groups = model->groups->next;
        free(group->name);
        group->name = NULL;

        free(group->triangles);
        group->triangles = NULL;

        free(group);
        group = NULL;
    }

    free(model);
    model = NULL;
}

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be freed with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 */
GLMmodel* glmReadOBJ(const char* filename)
{
    GLMmodel *model=NULL;
    FILE *file=NULL;

    // open the file
    file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "glmReadOBJ() failed: can't open data file \"%s\".\n", filename);
        return NULL;
    }

    // allocate a new model
    model = (GLMmodel*)malloc(sizeof(GLMmodel));
    model->pathname      = strdup(filename);

    model->mtllibname    = NULL;
    model->numvertices   = 0;
    model->vertices      = NULL;
    model->numnormals    = 0;
    model->normals       = NULL;
    model->numtexcoords  = 0;
    model->texcoords     = NULL;
    model->numfacetnorms = 0;
    model->facetnorms    = NULL;
    model->numtriangles  = 0;
    model->triangles     = NULL;
    model->nummaterials  = 0;
    model->materials     = NULL;
    model->numgroups     = 0;
    model->groups        = NULL;

    // make a first pass through the file to get a count of the number
    // of vertices, normals, texcoords & triangles
    glmFirstPass(model, file);

    // allocate memory
    model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
                                       3 * (model->numvertices + 1));
    model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
                                            model->numtriangles);
    if (model->numnormals)
    {
        model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
                                          3 * (model->numnormals + 1));
    }
    if (model->numtexcoords)
    {
        model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
                                            2 * (model->numtexcoords + 1));
    }

    // rewind to beginning of file and read in the data this pass
    rewind(file);
    glmSecondPass(model, file);

    // close the file
    fclose(file);

    return model;
}

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model    - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode     - a bitwise or of values describing what is written to the file
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid glmWriteOBJ(GLMmodel* model, const char* filename, GLuint mode)
{
    GLuint		i;
    FILE*		file=NULL;
    GLMgroup*	group=NULL;
	
    assert(model);

    /* do a bit of warning */
    if (mode & GLM_FLAT && !model->facetnorms) {
        printf("glmWriteOBJ() warning: flat normal output requested "
               "with no facet normals defined.\n");
        mode &= ~GLM_FLAT;
    }
    if (mode & GLM_SMOOTH && !model->normals) {
        printf("glmWriteOBJ() warning: smooth normal output requested "
               "with no normals defined.\n");
        mode &= ~GLM_SMOOTH;
    }
    if (mode & GLM_TEXTURE && !model->texcoords) {
        printf("glmWriteOBJ() warning: texture coordinate output requested "
               "with no texture coordinates defined.\n");
        mode &= ~GLM_TEXTURE;
    }
    if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
        printf("glmWriteOBJ() warning: flat normal output requested "
               "and smooth normal output requested (using smooth).\n");
        mode &= ~GLM_FLAT;
    }
    if (mode & GLM_COLOR && !model->materials) {
        printf("glmWriteOBJ() warning: color output requested "
               "with no colors (materials) defined.\n");
        mode &= ~GLM_COLOR;
    }
    if (mode & GLM_MATERIAL && !model->materials) {
        printf("glmWriteOBJ() warning: material output requested "
               "with no materials defined.\n");
        mode &= ~GLM_MATERIAL;
    }
    if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
        printf("glmWriteOBJ() warning: color and material output requested "
               "outputting only materials.\n");
        mode &= ~GLM_COLOR;
    }

    /* open the file */
    file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "glmWriteOBJ() failed: can't open file \"%s\" to write.\n",
                filename);
        exit(1);
    }

    /* spit out a header */
    fprintf(file, "#  \n");
    fprintf(file, "#  Wavefront OBJ generated by GLM library\n");
    fprintf(file, "#  \n");
    fprintf(file, "#  GLM library\n");
    fprintf(file, "#  \n");
		
    if (mode & GLM_MATERIAL && model->mtllibname) {
		char mtlfilename[MAX_PATH];
		int len;

		strcpy(mtlfilename, glmFileName(filename));
		len = strlen(mtlfilename);
		mtlfilename[len-3] = 'm';
		mtlfilename[len-2] = 't';
		mtlfilename[len-1] = 'l';
		fprintf(file, "\nmtllib %s\n\n", mtlfilename);
        glmWriteMTL(model, filename, mtlfilename);
    }

    /* spit out the vertices */
    fprintf(file, "\n");
    fprintf(file, "# %d vertices\n", model->numvertices);
    for (i = 1; i <= model->numvertices; i++) {
        fprintf(file, "v %f %f %f\n",
                model->vertices[3 * i + 0],
                model->vertices[3 * i + 1],
                model->vertices[3 * i + 2]);
    }

    /* spit out the smooth/flat normals */
    if (mode & GLM_SMOOTH) {
        fprintf(file, "\n");
        fprintf(file, "# %d normals\n", model->numnormals);
        for (i = 1; i <= model->numnormals; i++) {
            fprintf(file, "vn %f %f %f\n",
                    model->normals[3 * i + 0],
                    model->normals[3 * i + 1],
                    model->normals[3 * i + 2]);
        }
    } else if (mode & GLM_FLAT) {
        fprintf(file, "\n");
        fprintf(file, "# %d normals\n", model->numfacetnorms);
        for (i = 1; i <= model->numnormals; i++) {
            fprintf(file, "vn %f %f %f\n",
                    model->facetnorms[3 * i + 0],
                    model->facetnorms[3 * i + 1],
                    model->facetnorms[3 * i + 2]);
        }
    }

    /* spit out the texture coordinates */
    if (mode & GLM_TEXTURE) {
        fprintf(file, "\n");
        fprintf(file, "# %f texcoords\n", model->texcoords);
        for (i = 1; i <= model->numtexcoords; i++) {
            fprintf(file, "vt %f %f\n",
                    model->texcoords[2 * i + 0],
                    model->texcoords[2 * i + 1]);
        }
    }

    fprintf(file, "\n");
    fprintf(file, "# %d groups\n", model->numgroups);
    fprintf(file, "# %d faces (triangles)\n", model->numtriangles);
    fprintf(file, "\n");

    group = model->groups;
    while(group) {
        fprintf(file, "g %s\n", group->name);
        if (mode & GLM_MATERIAL)
            fprintf(file, "usemtl %s\n", model->materials[group->material].name);
        for (i = 0; i < group->numtriangles; i++) {
            if (mode & GLM_SMOOTH && mode & GLM_TEXTURE) {
                fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                        T(group->triangles[i]).vindices[0],
                        T(group->triangles[i]).nindices[0],
                        T(group->triangles[i]).tindices[0],
                        T(group->triangles[i]).vindices[1],
                        T(group->triangles[i]).nindices[1],
                        T(group->triangles[i]).tindices[1],
                        T(group->triangles[i]).vindices[2],
                        T(group->triangles[i]).nindices[2],
                        T(group->triangles[i]).tindices[2]);
            } else if (mode & GLM_FLAT && mode & GLM_TEXTURE) {
                fprintf(file, "f %d/%d %d/%d %d/%d\n",
                        T(group->triangles[i]).vindices[0],
                        T(group->triangles[i]).findex,
                        T(group->triangles[i]).vindices[1],
                        T(group->triangles[i]).findex,
                        T(group->triangles[i]).vindices[2],
                        T(group->triangles[i]).findex);
            } else if (mode & GLM_TEXTURE) {
                fprintf(file, "f %d/%d %d/%d %d/%d\n",
                        T(group->triangles[i]).vindices[0],
                        T(group->triangles[i]).tindices[0],
                        T(group->triangles[i]).vindices[1],
                        T(group->triangles[i]).tindices[1],
                        T(group->triangles[i]).vindices[2],
                        T(group->triangles[i]).tindices[2]);
            } else if (mode & GLM_SMOOTH) {
                fprintf(file, "f %d//%d %d//%d %d//%d\n",
                        T(group->triangles[i]).vindices[0],
                        T(group->triangles[i]).nindices[0],
                        T(group->triangles[i]).vindices[1],
                        T(group->triangles[i]).nindices[1],
                        T(group->triangles[i]).vindices[2],
                        T(group->triangles[i]).nindices[2]);
            } else if (mode & GLM_FLAT) {
                fprintf(file, "f %d//%d %d//%d %d//%d\n",
                        T(group->triangles[i]).vindices[0],
                        T(group->triangles[i]).findex,
                        T(group->triangles[i]).vindices[1],
                        T(group->triangles[i]).findex,
                        T(group->triangles[i]).vindices[2],
                        T(group->triangles[i]).findex);
            } else {
                fprintf(file, "f %d %d %d\n",
                        T(group->triangles[i]).vindices[0],
                        T(group->triangles[i]).vindices[1],
                        T(group->triangles[i]).vindices[2]);
            }
        }
        fprintf(file, "\n");
        group = group->next;
    }

    fclose(file);
}

/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid glmDraw(GLMmodel* model, GLuint mode)
{
    GLuint i;
    GLMgroup* group;
    GLMtriangle* triangle;
    GLMmaterial* material;
	int passid;
	GLenum curformat;

    assert(model);
    assert(model->vertices);

    /* do a bit of warning */
    if (mode & GLM_FLAT && !model->facetnorms) {
        printf("glmDraw() warning: flat render mode requested "
               "with no facet normals defined.\n");
        mode &= ~GLM_FLAT;
    }
    if (mode & GLM_SMOOTH && !model->normals) {
        printf("glmDraw() warning: smooth render mode requested "
               "with no normals defined.\n");
        mode &= ~GLM_SMOOTH;
    }
    if (mode & GLM_TEXTURE && !model->texcoords) {
        printf("glmDraw() warning: texture render mode requested "
               "with no texture coordinates defined.\n");
        mode &= ~GLM_TEXTURE;
    }
    if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
        printf("glmDraw() warning: flat render mode requested "
               "and smooth render mode requested (using smooth).\n");
        mode &= ~GLM_FLAT;
    }
    if (mode & GLM_COLOR && !model->materials) {
        printf("glmDraw() warning: color render mode requested "
               "with no materials defined.\n");
        mode &= ~GLM_COLOR;
    }
    if (mode & GLM_MATERIAL && !model->materials) {
        printf("glmDraw() warning: material render mode requested "
               "with no materials defined.\n");
        mode &= ~GLM_MATERIAL;
    }
    if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
        printf("glmDraw() warning: color and material render mode requested "
               "using only material mode.\n");
        mode &= ~GLM_COLOR;
    }
    if (mode & GLM_COLOR)
        glEnable(GL_COLOR_MATERIAL);
    else if (mode & GLM_MATERIAL)
        glDisable(GL_COLOR_MATERIAL);

    /* perhaps this loop should be unrolled into material, color, flat,
       smooth, etc. loops?  since most cpu's have good branch prediction
       schemes (and these branches will always go one way), probably
       wouldn't gain too much?  */
	
	for (passid=0; passid<2; ++passid)
	{
		// two pass rendering: first opaque objects, second transparent objects
		curformat = (passid==0) ? GL_RGB : GL_RGBA;
	    group = model->groups;
	    while (group) {
	        material = &model->materials[group->material];
			if (material->format != curformat) continue;
	        if (mode & GLM_MATERIAL) {
	
	            // NOTE: brighter
	            material->ambient[0] = 0.3f;
	            material->ambient[1] = 0.3f;
	            material->ambient[2] = 0.3f;
	
	            material->diffuse[0] = 0.9f;
	            material->diffuse[1] = 0.9f;
	            material->diffuse[2] = 0.9f;
	
	            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
	            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
	            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
	            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
	        }
	
	        if ( (mode & GLM_TEXTURE) && (material->kdMapName != NULL) ) {
	            if (material->kdMapId == DEFAULT_TEX_ID)
	                glmBuildTexture(model, material);
	
	            glEnable(GL_TEXTURE_2D);
	            glBindTexture(GL_TEXTURE_2D, material->kdMapId);
	        } else {
	            glDisable(GL_TEXTURE_2D);
	        }
	
	        if (mode & GLM_COLOR) {
	            glColor3fv(material->diffuse);
	        }
	
	        if (mode == GLM_NONE)
	            glBegin(GL_POINTS);
	        else
	            glBegin(GL_TRIANGLES);
	
	        for (i = 0; i < group->numtriangles; i++)
	        {
	            triangle = &T(group->triangles[i]);
	
	            if (mode & GLM_FLAT)
	                glNormal3fv(&model->facetnorms[3 * triangle->findex]);
	
	            if (mode & GLM_SMOOTH)
	                glNormal3fv(&model->normals[3 * triangle->nindices[0]]);
	            if (mode & GLM_TEXTURE)
	                glTexCoord2fv(&model->texcoords[2 * triangle->tindices[0]]);
	            glVertex3fv(&model->vertices[3 * triangle->vindices[0]]);
	
	            if (mode & GLM_SMOOTH)
	                glNormal3fv(&model->normals[3 * triangle->nindices[1]]);
	            if (mode & GLM_TEXTURE)
	                glTexCoord2fv(&model->texcoords[2 * triangle->tindices[1]]);
	            glVertex3fv(&model->vertices[3 * triangle->vindices[1]]);
	
	            if (mode & GLM_SMOOTH)
	                glNormal3fv(&model->normals[3 * triangle->nindices[2]]);
	            if (mode & GLM_TEXTURE)
	                glTexCoord2fv(&model->texcoords[2 * triangle->tindices[2]]);
	            glVertex3fv(&model->vertices[3 * triangle->vindices[2]]);
	        }
	        glEnd();
	
	        if (glIsEnabled(GL_TEXTURE_2D))
	            glDisable(GL_TEXTURE_2D);
	
	        group = group->next;
	    }
    }

	_printGLError();
}

///////////////// ´´½¨ÎÆÀí //////////////////////////////////
GLboolean glmBuildTexture(GLMmodel *model, GLMmaterial* material)
{
    char	pathname[MAX_PATH];				// Full Path To Picture
    long	lWidthPixels;					// Width In Pixels
    long	lHeightPixels;					// Height In Pixels
    GLint	glMaxTexDim;					// Holds Maximum Texture Size
    void	*pBits;
    GLenum	format;
    BITMAPINFOHEADER bmInfo;

	assert(material);

	glmDirName(pathname, model->pathname);
	strcat(pathname, material->kdMapName);

    pBits = glmLoadBitmapFile(pathname, &bmInfo);
    if (pBits == NULL)
        return GL_FALSE;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);
    lWidthPixels = bmInfo.biWidth;
    lHeightPixels = bmInfo.biHeight;

    // Resize Image To Closest Power Of Two
    if (lWidthPixels <= glMaxTexDim) // Is Image Width Less Than Or Equal To Cards Limit
        lWidthPixels = 1 << (int)floor((log((double)lWidthPixels)/log(2.0f)) + 0.5f);
    else // Otherwise Set Width To "Max Power Of Two" That The Card Can Handle
        lWidthPixels = glMaxTexDim;

    if (lHeightPixels <= glMaxTexDim) // Is Image Height Greater Than Cards Limit
        lHeightPixels = 1 << (int)floor((log((double)lHeightPixels)/log(2.0f)) + 0.5f);
    else // Otherwise Set Height To "Max Power Of Two" That The Card Can Handle
        lHeightPixels = glMaxTexDim;

    if (material->kdMapId == DEFAULT_TEX_ID || !glIsTexture(material->kdMapId))
        glGenTextures(1, &material->kdMapId);                      // Create The Texture

    format = (bmInfo.biBitCount == 32) ? GL_RGBA : GL_RGB;
	material->format = format;

    // Typical Texture Generation Using Data From The Bitmap
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, material->kdMapId);								// Bind To The Texture ID
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);			// (Modify This For The Type Of Filtering You Want)
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);			// (Modify This For The Type Of Filtering You Want)
#if 0
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR );  // (Modify This For The Type Of Filtering You Want)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );				// (Modify This For The Type Of Filtering You Want)
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, lWidthPixels, lHeightPixels, format, GL_UNSIGNED_BYTE, pBits);   // (Modify This If You Want Mipmaps)
#else
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );  // NOTE: GL_LINEAR_MIPMAP_LINEAR does not work
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );				// (Modify This For The Type Of Filtering You Want)
    glTexImage2D(GL_TEXTURE_2D, 0, format, lWidthPixels, lHeightPixels, 0, format, GL_UNSIGNED_BYTE, pBits);
#endif
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

    free(pBits);

	_printGLError();


    return GL_TRUE;              // Return True (All Good)
}

GLvoid glmBuildTexture2(int width, int heigh, GLenum format, GLenum type, const void* pData, GLuint* texid)
{
	if (format != GL_RGB && format != GL_BGR_EXT)
	{
		printf("glmBuildTexture2() warning: can only build texture with RGB or BGR format.\n");
		return;
	}

    if ((*texid == DEFAULT_TEX_ID) || !glIsTexture(*texid))
		glGenTextures(1, texid);

	_printGLError();
	
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, *texid);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);// NOTE: GL_LINEAR_MIPMAP_LINEAR does not work
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, heigh, 0, format, type, pData);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	_printGLError();
}


/* glmList: Generates and returns a display list for the model using
 * the mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.
 * GLM_FLAT and GLM_SMOOTH should not both be specified.  */
GLuint glmList(GLMmodel* model, GLuint mode)
{
    GLuint list;

    list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glmDraw(model, mode);
    glEndList();

	_printGLError();

    return list;
}

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model      - initialized GLMmodel structure
 * epsilon    - maximum difference between vertices
 *              ( 0.00001 is a good start for a unitized model)
 *
 */
GLvoid glmWeld(GLMmodel* model, GLfloat epsilon)
{
    GLfloat* vectors=NULL;
    GLfloat* copies=NULL;
    GLuint   numvectors;
    GLuint   i;
    GLuint	res;

    /* vertices */
    numvectors = model->numvertices;
    vectors    = model->vertices;

    //copies = glmWeldVectors(vectors, &numvectors, epsilon);
    copies = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (numvectors + 1));
    res = glmWeldVectors(copies, vectors, &numvectors, epsilon);
    if(res!=0)
    {
        if(copies) {free(copies); copies=NULL;}
        return;
    }

    printf("glmWeld(): %d redundant vertices.\n", model->numvertices - numvectors - 1);

    for (i = 0; i < model->numtriangles; i++)
    {
        T(i).vindices[0] = (GLuint)vectors[3 * T(i).vindices[0] + 0];
        T(i).vindices[1] = (GLuint)vectors[3 * T(i).vindices[1] + 0];
        T(i).vindices[2] = (GLuint)vectors[3 * T(i).vindices[2] + 0];
    }

    /* free space for old vertices */
    free(vectors);

    /* allocate space for the new vertices */
    model->numvertices = numvectors;
    model->vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (model->numvertices + 1));

    /* copy the optimized vertices into the actual vertex list */
    for (i = 1; i <= model->numvertices; i++)
    {
        model->vertices[3 * i + 0] = copies[3 * i + 0];
        model->vertices[3 * i + 1] = copies[3 * i + 1];
        model->vertices[3 * i + 2] = copies[3 * i + 2];
    }

    if(copies) {free(copies); copies=NULL;}
}