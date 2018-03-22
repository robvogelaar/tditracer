enum { PARENT, SUB, RENDER };

void texturecapture_captexture(unsigned int name, int ttype, int frame,
                               int xoffset, int yoffset, int width, int height,
                               int format, int type, const void* pixels);
void texturecapture_writepngtextures(void);
void texturecapture_deletetextures(void);
