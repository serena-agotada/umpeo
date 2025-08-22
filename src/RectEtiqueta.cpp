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

void RectEtiqueta::dibujar(ofTrueTypeFont font, float w, float h, int offX, int offY) {
    //ofLog() << width << " " << left << " " << w;
	
	ofColor col = ofColor(0);
	col.setHsb(ofMap(confidence, 45, 100, 0, 150), ofMap(confidence, 10, 100, 0, 255), ofMap(confidence, 10, 100, 0, 250));
	
	ofSetLineWidth(5);
	ofSetColor(col);
	ofNoFill();
	ofDrawRectangle(left*w+offX, top*h+offY, width*w, height*h);
	
	font.drawString(name, left*w+offX+5, top*h+offY-10);
	//font.drawString(ofToString((int)confidence), left*w+offX+5, top*h+offY+15);
}
