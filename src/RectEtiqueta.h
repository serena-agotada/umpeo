#include "ofMain.h"

struct RectEtiqueta {
    void setup(string nombre, float x, float y, float w, float h, float conf, int ts);
    void dibujar(ofTrueTypeFont font, float w, float h, int offX, int offY);
    
    int id;
    string name;
    float width, height, left, top, confidence, timestamp;
    ofColor color;
    
    bool dibujable;
};
