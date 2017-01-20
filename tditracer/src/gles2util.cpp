#ifndef NOGLES2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <gles2util.h>

void Identity(float pMatrix[4][4]) {
  pMatrix[0][0] = 1.0f;
  pMatrix[0][1] = 0.0f;
  pMatrix[0][2] = 0.0f;
  pMatrix[0][3] = 0.0f;

  pMatrix[1][0] = 0.0f;
  pMatrix[1][1] = 1.0f;
  pMatrix[1][2] = 0.0f;
  pMatrix[1][3] = 0.0f;

  pMatrix[2][0] = 0.0f;
  pMatrix[2][1] = 0.0f;
  pMatrix[2][2] = 1.0f;
  pMatrix[2][3] = 0.0f;

  pMatrix[3][0] = 0.0f;
  pMatrix[3][1] = 0.0f;
  pMatrix[3][2] = 0.0f;
  pMatrix[3][3] = 1.0f;
}

static float Normalize(float afVout[3], float afVin[3]) {
  float fLen;

  fLen = afVin[0] * afVin[0] + afVin[1] * afVin[1] + afVin[2] * afVin[2];

  if (fLen <= 0.0f) {
    afVout[0] = 0.0f;
    afVout[1] = 0.0f;
    afVout[2] = 0.0f;
    return fLen;
  }

  if (fLen == 1.0F) {
    afVout[0] = afVin[0];
    afVout[1] = afVin[1];
    afVout[2] = afVin[2];
    return fLen;
  } else {
    fLen = ((float)1.0) / (float)sqrt((double)fLen);

    afVout[0] = afVin[0] * fLen;
    afVout[1] = afVin[1] * fLen;
    afVout[2] = afVin[2] * fLen;
    return fLen;
  }
}

void MultiplyMatrix(float psRes[4][4], float psSrcA[4][4], float psSrcB[4][4]) {
  float fB00, fB01, fB02, fB03;
  float fB10, fB11, fB12, fB13;
  float fB20, fB21, fB22, fB23;
  float fB30, fB31, fB32, fB33;
  int i;

  fB00 = psSrcB[0][0];
  fB01 = psSrcB[0][1];
  fB02 = psSrcB[0][2];
  fB03 = psSrcB[0][3];
  fB10 = psSrcB[1][0];
  fB11 = psSrcB[1][1];
  fB12 = psSrcB[1][2];
  fB13 = psSrcB[1][3];
  fB20 = psSrcB[2][0];
  fB21 = psSrcB[2][1];
  fB22 = psSrcB[2][2];
  fB23 = psSrcB[2][3];
  fB30 = psSrcB[3][0];
  fB31 = psSrcB[3][1];
  fB32 = psSrcB[3][2];
  fB33 = psSrcB[3][3];

  for (i = 0; i < 4; i++) {
    psRes[i][0] = psSrcA[i][0] * fB00 + psSrcA[i][1] * fB10 +
                  psSrcA[i][2] * fB20 + psSrcA[i][3] * fB30;
    psRes[i][1] = psSrcA[i][0] * fB01 + psSrcA[i][1] * fB11 +
                  psSrcA[i][2] * fB21 + psSrcA[i][3] * fB31;
    psRes[i][2] = psSrcA[i][0] * fB02 + psSrcA[i][1] * fB12 +
                  psSrcA[i][2] * fB22 + psSrcA[i][3] * fB32;
    psRes[i][3] = psSrcA[i][0] * fB03 + psSrcA[i][1] * fB13 +
                  psSrcA[i][2] * fB23 + psSrcA[i][3] * fB33;
  }
}

void Perspective(float pMatrix[4][4], float fovy, float aspect, float zNear,
                 float zFar) {
  float sine, cotangent, deltaZ;
  float radians;
  float m[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

  radians = fovy / 2 * PI / 180;

  deltaZ = zFar - zNear;
  sine = (float)sin(radians);
  if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
    return;
  }

  cotangent = (float)cos(radians) / sine;

  m[0][0] = cotangent / aspect;
  m[1][1] = cotangent;
  m[2][2] = -(zFar + zNear) / deltaZ;
  m[2][3] = -1;
  m[3][2] = -2 * zNear * zFar / deltaZ;
  m[3][3] = 0;

  MultiplyMatrix(pMatrix, m, pMatrix);
}

void Orthographic(float pMatrix[4][4], float left, float right, float bottom,
                  float top, float zNear, float zFar) {
  float deltaX, deltaY, deltaZ;
  float m[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

  deltaX = right - left;
  deltaY = top - bottom;
  deltaZ = zFar - zNear;

  if ((deltaZ == 0) || (deltaX == 0) || (deltaY == 0)) {
    return;
  }

  m[0][0] = 2 / deltaX;
  m[1][1] = 2 / deltaY;
  m[2][2] = -2 / deltaZ;

  m[3][0] = -(right + left) / (right - left);
  m[3][1] = -(top + bottom) / (top - bottom);
  m[3][2] = -(zFar + zNear) / (zFar - zNear);

  MultiplyMatrix(pMatrix, m, pMatrix);
}

void Frustum(float pMatrix[4][4], float left, float right, float bottom,
             float top, float zNear, float zFar) {
  float deltaX, deltaY, deltaZ;
  float m[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

  deltaX = right - left;
  deltaY = top - bottom;
  deltaZ = zFar - zNear;

  if ((deltaZ == 0) || (deltaX == 0) || (deltaY == 0)) {
    return;
  }

  m[0][0] = 2 * zNear / deltaX;
  m[1][1] = 2 * zNear / deltaY;
  m[2][0] = (left + right) / deltaX;
  m[2][1] = (top + bottom) / deltaY;
  m[2][2] = -(zFar + zNear) / deltaZ;
  m[2][3] = -1;
  m[3][2] = -2 * zNear * zFar / deltaZ;
  m[3][3] = 0;

  MultiplyMatrix(pMatrix, m, pMatrix);
}

void Scale(float pMatrix[4][4], float fX, float fY, float fZ) {
  float fM0, fM1, fM2, fM3;

  fM0 = fX * pMatrix[0][0];
  fM1 = fX * pMatrix[0][1];
  fM2 = fX * pMatrix[0][2];
  fM3 = fX * pMatrix[0][3];
  pMatrix[0][0] = fM0;
  pMatrix[0][1] = fM1;
  pMatrix[0][2] = fM2;
  pMatrix[0][3] = fM3;

  fM0 = fY * pMatrix[1][0];
  fM1 = fY * pMatrix[1][1];
  fM2 = fY * pMatrix[1][2];
  fM3 = fY * pMatrix[1][3];
  pMatrix[1][0] = fM0;
  pMatrix[1][1] = fM1;
  pMatrix[1][2] = fM2;
  pMatrix[1][3] = fM3;

  fM0 = fZ * pMatrix[2][0];
  fM1 = fZ * pMatrix[2][1];
  fM2 = fZ * pMatrix[2][2];
  fM3 = fZ * pMatrix[2][3];
  pMatrix[2][0] = fM0;
  pMatrix[2][1] = fM1;
  pMatrix[2][2] = fM2;
  pMatrix[2][3] = fM3;
}

void Translate(float pMatrix[4][4], float fX, float fY, float fZ) {
  float fM30, fM31, fM32, fM33;

  fM30 = fX * pMatrix[0][0] + fY * pMatrix[1][0] + fZ * pMatrix[2][0] +
         pMatrix[3][0];
  fM31 = fX * pMatrix[0][1] + fY * pMatrix[1][1] + fZ * pMatrix[2][1] +
         pMatrix[3][1];
  fM32 = fX * pMatrix[0][2] + fY * pMatrix[1][2] + fZ * pMatrix[2][2] +
         pMatrix[3][2];
  fM33 = fX * pMatrix[0][3] + fY * pMatrix[1][3] + fZ * pMatrix[2][3] +
         pMatrix[3][3];

  pMatrix[3][0] = fM30;
  pMatrix[3][1] = fM31;
  pMatrix[3][2] = fM32;
  pMatrix[3][3] = fM33;
}

void Rotate(float pMatrix[4][4], float fX, float fY, float fZ, float fAngle) {
  float fRadians, fSine, fCosine, fAB, fBC, fCA, fT;
  float afAv[4], afAxis[4];
  float afMatrix[4][4];

  afAv[0] = fX;
  afAv[1] = fY;
  afAv[2] = fZ;
  afAv[3] = 0;

  Normalize(afAxis, afAv);

  fRadians = fAngle * DegreesToRadians;
  fSine = (float)sin(fRadians);
  fCosine = (float)cos(fRadians);

  fAB = afAxis[0] * afAxis[1] * (1 - fCosine);
  fBC = afAxis[1] * afAxis[2] * (1 - fCosine);
  fCA = afAxis[2] * afAxis[0] * (1 - fCosine);

  Identity(afMatrix);

  fT = afAxis[0] * afAxis[0];
  afMatrix[0][0] = fT + fCosine * (1 - fT);
  afMatrix[2][1] = fBC - afAxis[0] * fSine;
  afMatrix[1][2] = fBC + afAxis[0] * fSine;

  fT = afAxis[1] * afAxis[1];
  afMatrix[1][1] = fT + fCosine * (1 - fT);
  afMatrix[2][0] = fCA + afAxis[1] * fSine;
  afMatrix[0][2] = fCA - afAxis[1] * fSine;

  fT = afAxis[2] * afAxis[2];
  afMatrix[2][2] = fT + fCosine * (1 - fT);
  afMatrix[1][0] = fAB - afAxis[2] * fSine;
  afMatrix[0][1] = fAB + afAxis[2] * fSine;

  MultiplyMatrix(pMatrix, afMatrix, pMatrix);
}

static int create_program(const char *v, const char *f) {
    char pszInfoLog[1024];
    int nShaderStatus, nInfoLogLength;

    //
    int vertshaderhandle = glCreateShader(GL_VERTEX_SHADER);
    int fragshaderhandle = glCreateShader(GL_FRAGMENT_SHADER);
    int programhandle = glCreateProgram();

    //
    char *pszProgramString0 = (char *)v;
    int nProgramLength0 = strlen(v);
    glShaderSource(vertshaderhandle, 1, (const char **)&pszProgramString0,
                   &nProgramLength0);
    glCompileShader(vertshaderhandle);
    glGetShaderiv(vertshaderhandle, GL_COMPILE_STATUS, &nShaderStatus);
    if (nShaderStatus != GL_TRUE) {
        printf("Error: Failed to compile GLSL shader\n");
        glGetShaderInfoLog(vertshaderhandle, 1024, &nInfoLogLength, pszInfoLog);
        printf("%s", pszInfoLog);
    }
    glAttachShader(programhandle, vertshaderhandle);

    //
    char *pszProgramString1 = (char *)f;
    int nProgramLength1 = strlen(f);
    glShaderSource(fragshaderhandle, 1, (const char **)&pszProgramString1,
                   &nProgramLength1);
    glCompileShader(fragshaderhandle);
    glGetShaderiv(fragshaderhandle, GL_COMPILE_STATUS, &nShaderStatus);
    if (nShaderStatus != GL_TRUE) {
        printf("Error: Failed to compile GLSL shader\n");
        glGetShaderInfoLog(fragshaderhandle, 1024, &nInfoLogLength, pszInfoLog);
        printf("%s", pszInfoLog);
    }
    glAttachShader(programhandle, fragshaderhandle);

    glLinkProgram(programhandle);
    glGetProgramiv(programhandle, GL_LINK_STATUS, &nShaderStatus);
    if (nShaderStatus != GL_TRUE) {
        printf("Error: Failed to link GLSL program\n");
        glGetProgramInfoLog(programhandle, 1024, &nInfoLogLength, pszInfoLog);
        printf("%s", pszInfoLog);
    }

    glValidateProgram(programhandle);
    glGetProgramiv(programhandle, GL_VALIDATE_STATUS, &nShaderStatus);
    if (nShaderStatus != GL_TRUE) {
        printf("Error: Failed to validate GLSL program\n");
        glGetProgramInfoLog(programhandle, 1024, &nInfoLogLength, pszInfoLog);
        printf("%s", pszInfoLog);
    }

    return programhandle;
}

#endif
