
#define PI ((float)3.14159265358979323846)
#define DegreesToRadians (PI / (float)180.0)

void Identity(float pMatrix[4][4]);
float Normalize(float afVout[3], float afVin[3]);
void MultiplyMatrix(float psRes[4][4], float psSrcA[4][4], float psSrcB[4][4]);
void Perspective(float pMatrix[4][4], float fovy, float aspect, float zNear,
                 float zFar);
void Orthographic(float pMatrix[4][4], float left, float right, float bottom,
                  float top, float zNear, float zFar);
void Frustum(float pMatrix[4][4], float left, float right, float bottom,
             float top, float zNear, float zFar);
void Scale(float pMatrix[4][4], float fX, float fY, float fZ);
void Translate(float pMatrix[4][4], float fX, float fY, float fZ);
void Rotate(float pMatrix[4][4], float fX, float fY, float fZ, float fAngle);
int create_program(const char *v, const char *f);
