#include "RectEtiqueta.h"

void RectEtiqueta::setup(string nombre, float x, float y, float w, float h, float conf, int ts) {
    name = nombre;
    width = w;
    height = h;
    left = x;
    top = y;
    confidence = conf;
    timestamp = ts;
}

void RectEtiqueta::actualizar() {
    
}

void RectEtiqueta::dibujar(float w, float h, int offX, int offY) {
    //ofLog() << width << " " << left << " " << w;
	ofNoFill();
	
	ofDrawRectangle(left*w+offX, top*h+offY, width*w, height*h);
	ofDrawBitmapString(ofToString(id) + name, left*w+offX+5, top*h+offY-10);
	ofDrawBitmapString((int)confidence, left*w+offX+5, top*h+offY+15);
}
