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

void RectEtiqueta::dibujar(ofTrueTypeFont font, float w, float h, int offX, int offY) {
    //ofLog() << width << " " << left << " " << w;
	
	ofColor col = ofColor(0);
	col.setHsb(ofMap(confidence, 45, 100, 0, 150), ofMap(confidence, 15, 100, 0, 255), ofMap(confidence, 15, 100, 0, 250));
	
	ofSetLineWidth(5);
	ofSetColor(col);
	ofNoFill();
	ofDrawRectangle(left*w+offX, top*h+offY, width*w, height*h);
	
	ofRectangle rect = font.getStringBoundingBox(name, 0,0);
	ofFill();
	//ofNoStroke();
	ofDrawRectangle(left*w+offX, top*h+offY-rect.height+12, rect.width+10, rect.height+10);
	
	ofSetColor(255);
	font.drawString(name, left*w+offX+5, top*h+offY+12);
	//font.drawString(ofToString((int)confidence), left*w+offX+5, top*h+offY+15);
}
