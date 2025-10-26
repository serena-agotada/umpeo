#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "Etiqueta.h"
#include "RectEtiqueta.h"
#include "CuadrillaEtiqueta.h"

// send host (aka ip address)
#define HOST "localhost"

/// send port
#define PORT 12345

class ofApp : public ofBaseApp{
	private:
		static bool compararPorConfidence(RectEtiqueta &a, RectEtiqueta &b);

	public:

		void setup();
		void update();
		void draw();
		void exit();
		
		void applyGlitchEffect();
		void dibujarEtiquetas(int x, int y, int w, int h);
		void dibujarBarraProgreso(int xx, int yy, int ww, float porcentaje);
		void dibujarDeteccion();
		void dibujarBarraRecorrido();
		void dibujarGraficoEtiquetas(int xx, int yy, int ww, int hh);
		
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
		float getPosicionSegura();

		// VARIABLES PARA REPRODUCTOR
		int clip;
		int sensor;
		int randomize; // controla cuanto cambia de clip
		int cantVideos;
		vector<string>		videoFiles;  // Declaración del vector para los nombres de los videos
		vector<float> videoPositions; // Ultima posición de cada video
		ofVideoPlayer		videoPlayer;  // Objeto de ofVideoPlayer para reproducir los videos
		int currentVideoIndex;      // Índice para saber qué video se está reproduciendo
		int newIndex; // Indice para calcular nuevo video
		bool shouldSetPosition;
		int divisionSensor; // distancia entre cada posición de sintonía
		int sintVideoIndex; // video en sintonía más cercano
		float distSintVideo; // distancia del sensor al video sintonizado
		float newVolume; // para calcular el volumen
		float prevDist;
		ofPixels pixels;
		ofTexture videoTexture, glitchTexture, lastValidFrame;
		float distortionAmount;
		ofFbo fbo, glitchFbo;
		int oscuridad;
		float resizeVideo;
		string textoDerecha;
		int offsetVideoPosX;
		int offsetVideoPosY;
		int dist_dial;
		int tam_dial;
		vector<vector<Etiqueta>> etiquetasVideos;
		CuadrillaEtiqueta grilla[52][18];
		int i_etiqueta;
		float pos_prox_etiqueta;
		ofColor colores_etiquetas[100];
		vector<string> displayEtiquetas;
		int inicio_linea_corrupta;
		vector<vector<RectEtiqueta>> deteccionesEtiquetas;
		vector<RectEtiqueta> etiquetasDetectadas;
		ofJson js;
		int ultimoRectEtiqueta;
		vector<int> frame_ids_detectados;
		ofTrueTypeFont texto;
		float lastFrameTime;

		ofxOscReceiver receiver;
};

