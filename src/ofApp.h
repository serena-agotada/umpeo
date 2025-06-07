#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

// send host (aka ip address)
#define HOST "localhost"

/// send port
#define PORT 12345

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();
		
		void applyGlitchEffect();
		void audioOut(ofSoundBuffer &buffer);
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		

		// VARIABLES PARA REPRODUCTOR
		int clip;
		int sensor;
		int randomize; // controla cuanto cambia de clip
		int cantVideos;
		vector<string>		videoFiles;  // Declaraci�n del vector para los nombres de los videos
		vector<float> videoPositions; // Ultima posici�n de cada video
		ofVideoPlayer		videoPlayer;  // Objeto de ofVideoPlayer para reproducir los videos
		ofSoundStream soundStream;
		int currentVideoIndex;      // �ndice para saber qu� video se est� reproduciendo
		int newIndex; // Indice para calcular nuevo video
		bool shouldSetPosition;
		int divisionSensor; // distancia entre cada posici�n de sinton�a
		int sintVideoIndex; // video en sinton�a m�s cercano
		float distSintVideo; // distancia del sensor al video sintonizado
		float newVolume; // para calcular el volumen
		float prevDist;
		ofPixels pixels;
		ofTexture videoTexture, lastValidFrame;
		float distortionAmount;
		ofFbo fbo;
		int oscuridad;
		float resizeVideo;
		string textoDerecha;
		int offsetVideoPosX;
		int offsetVideoPosY;
		int tam_dial;
		
		// VARIABLES PARA REPRODUCTOR
		ofxOscReceiver receiver;


};

