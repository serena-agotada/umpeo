#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

// ETIQUETADO --------------------------------------------------------

//!! AGREGAR a ofApp.h !!
// void dibujarEtiquetas(int x, int y, int w, int h)
// vector<Etiqueta> etiquetasVideos;
// RectEtiqueta grilla[97][10];
// ofColor colores_etiquetas[100]

// agregar clase Etiqueta(string nombre, float confianza, float posicion)
// agregar clase RectEtiqueta(string n, ofColor c)
// Colores etiquetas
		ofColor c1 = ofColor::fromHex(0x001f34);
		ofColor c2 = ofColor::fromHex(0x40c258);
		ofColor c3 = ofColor::fromHex(0x5261ee);
		ofColor c4 = ofColor::fromHex(0xd60062);


		for(int c=0; x<100; c++){
			if(c < 25){
				colores_etiquetas[c] = c1.ofLerp(c2, 0.04*c);
			}
			else if(c < 50){
				colores_etiquetas[c] = c2.ofLerp(c3, 0.04*(c-25));
			}
			else if(c < 75){
				colores_etiquetas[c] = c3.ofLerp(c4, 0.04*(c-50));
			}
		}

// creacion de variables de etiquetas

		float pos_prox_etiqueta = 0.0f;

		ofFile file("/home/sara/Desktop/etiqueras-umpeo/grilla.csv");
		
		ofBuffer buffer(file);

		int n_linea = 0;

		//Generar grilla de etiquetas
		for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
			string line = *it;
			//Split line into strings
			vector<string> e = ofSplitString(line, ",");

			//guardo cada nombre en el array y seteo el color a negro
			for(int x; x < 10; x++) {
				grilla[n_linea][x].nombre = e[x];
				grilla[n_linea][x].color = ofColor(0,0,0);
			}

			n_linea++;
		}	

	// Recorrer la lista de archivos y almacenarlos en etiquetasVideos
	for (int i = 0; i < cantVideos; i++) {
		//Load file
		ofFile file("/home/sara/Desktop/etiquetas-umpeo/Copia de labels - " + ofToString(i) +".csv");
		
		ofBuffer buffer(file);

		int cont = 0;

		//Read file line by line
		for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
			string line = *it;
			//Split line into strings
			vector<string> words = ofSplitString(line, ",");

			//Store strings into a custom container
			if (words.size()>=2) {
				Etiqueta e;
				e.nombre = words[0];
				e.confianza = words[1];
				e.posicion = ofMap(cont, 0, buffer.getLines().size(), 0, 1);

				//guardar etiqueta
				etiquetasVideos.push_back(e);

				cont++;
			}
		}	
	}

	
// VARIABLES DE SETTEO ---------------------------------------------------------
	cantVideos = 59;
	
	resizeVideo = 0.85;
	
	tam_dial = (int) ofGetWidth()/cantVideos/2.5;

// REPRODUCTOR ------------------------------------------------------------------
	
    videoPlayer.setUseTexture(true);

	shouldSetPosition = false;

	// Definir el directorio de los videos
	ofDirectory dir("/home/sara/Desktop/videos-umpeo");
	dir.allowExt("mp4");  // Filtra solo archivos con la extensi�n .mp4

	// Lista de archivos en el directorio
	dir.listDir();

	// Recorrer la lista de archivos y almacenarlos en videoFiles
	for (int i = 0; i < cantVideos; i++) {
		videoFiles.push_back("/home/sara/Desktop/videos-umpeo/" + ofToString(i) +".mp4");
		videoPositions.push_back(0.0f);
	}

	if (cantVideos > 0) {
		divisionSensor = 1024 / (cantVideos - 1);

		currentVideoIndex = 0;  // Iniciar con el primer video

		// Cargar el primer video
		videoPlayer.load(videoFiles[currentVideoIndex]);
		
		videoPlayer.setLoopState(OF_LOOP_NORMAL);
		videoPlayer.play();
		fbo.allocate(videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
		
	}
	else {
		ofLog() << "No se encontraron videos en el directorio!";
	}
	
//AUDIO ------------------------------------------------------------
	ofSoundStreamSettings settings;
    settings.setOutListener(this);
    settings.sampleRate = 44100;
    settings.numOutputChannels = 2;
    settings.numInputChannels = 0;
    settings.bufferSize = 256;
    settings.numBuffers = 4;
    soundStream.setup(settings);
	
	
// OSC -------------------------------------------------------------
	// listen on the given port
	ofLog() << "listening for osc messages on port " << PORT;
	receiver.setup(PORT);

	newVolume = 0;
	prevDist = 0;
	
	sensor = 0;
	distortionAmount = 0.0f;
	clip = 0;
	
	oscuridad = 0;
	
	textoDerecha = " ";
	
}


//--------------------------------------------------------------
void ofApp::update(){
    
    videoPlayer.update();
    
    if(videoPlayer.isFrameNew() && videoTexture.isAllocated()) {videoTexture = videoPlayer.getTexture();
        
        fbo.begin();
        {
            applyGlitchEffect();
            
        }
        fbo.end();
    }

	// check for waiting messages
	while(receiver.hasWaitingMessages()){

		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage(m);

		
		if(m.getAddress() == "/video"){
			if(m.getArgAsInt32(0) < cantVideos){
				newIndex = m.getArgAsInt32(0);
				//cout << newIndex << " ";
			}
			else{
				cout << "Indice de video invalido ";
			}
		}
		else if(m.getAddress() == "/nitidez"){
			distortionAmount = m.getArgAsFloat(0);
			//cout << distortionAmount << " ";
		}
		else if(m.getAddress() == "/sVideo"){
			sensor = m.getArgAsInt32(0);
			//cout << sensor << "\n";
		}
		else if(m.getAddress() == "/oscuridad"){
			oscuridad = m.getArgAsInt32(0);
		}
	}
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0, 0, 0);

	if (ofGetFrameRate() >= 0){
		
		// solo hago update del video si cambie de indice
		if (newIndex != currentVideoIndex) {
			currentVideoIndex = newIndex;
			//videoPlayer.close();
			videoPlayer.load(videoFiles[currentVideoIndex]);
			
			// Espera a que el video se cargue
			if(videoPlayer.isLoaded()) {
				videoPlayer.update();
				videoPlayer.setPosition(videoPositions[currentVideoIndex]);
				videoPlayer.play();
				
				// Actualiza la textura inmediatamente
				videoTexture = videoPlayer.getTexture();
				
				// Reallocar el FBO con las nuevas dimensiones
				fbo.allocate(videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
			}
			
		}

		if (videoPlayer.isLoaded()) {
			
			// Actualiza la posici�n si no lo hizo todav�a
			if (shouldSetPosition && videoPlayer.getPosition() < videoPositions[currentVideoIndex]) {
				videoPlayer.setPosition(videoPositions[currentVideoIndex]);
				
				//Solo declaro que no hay que cambiar la posici�n si realmente se actualiz�
				if (videoPlayer.getPosition() == videoPositions[currentVideoIndex]) {
					shouldSetPosition = false;
				}
			}
			
// ---------- SONIDO --------------------------------------------------------------------------------------------			

			// el volumen varia con el valor de oscuridad
			if (oscuridad < 250){
				newVolume = ofMap(oscuridad, 0, 255, 0.99, 0.02, true);
				videoPlayer.setVolume( newVolume );
			}
			// el volumen var�a con el valor de los sensores
			else if (abs(prevDist - distSintVideo) > divisionSensor / 18) {
				prevDist = distSintVideo;
				newVolume = ofLerp(newVolume, ofMap(distSintVideo, 0, divisionSensor / 2, 0.95, 0, true), 0.5);
				videoPlayer.setVolume( newVolume );
				//cout << "volumen: " << newVolume << " | ";
			}
// ------------------------------------------------------------------------------------------------------------------				
			
			offsetVideoPosX = ( (float)(ofGetWidth()) - (float)videoPlayer.getWidth()*resizeVideo) / 2;
			offsetVideoPosY = ( (float)(ofGetHeight()) - (float)videoPlayer.getHeight()*resizeVideo) / 2;
			
			if(distortionAmount > 0.02){
				if(fbo.isAllocated() && videoTexture.isAllocated()) {
					
					fbo.draw(offsetVideoPosX, offsetVideoPosY, fbo.getWidth(), fbo.getHeight());
					ofDrawBitmapString("Memoria: FBO", 20, 80);
					
					// Guardar el frame actual como �ltimo v�lido
					fbo.readToPixels(pixels);
					lastValidFrame.loadData(pixels);
					
				} else if(lastValidFrame.isAllocated()) {
					// Si hay problemas, dibuja el �ltimo frame v�lido
					lastValidFrame.draw(offsetVideoPosX, offsetVideoPosY, lastValidFrame.getWidth()*resizeVideo, lastValidFrame.getHeight()*resizeVideo);
					ofDrawBitmapString("Memoria: UFV", 20, 80);
				}
				else{
					ofSetColor(255); // Resetear color a blanco
					videoPlayer.draw(offsetVideoPosX, offsetVideoPosY, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
					ofDrawBitmapString("Memoria: VO", 20, 80);
					
					// Guardar el frame actual como �ltimo v�lido
					lastValidFrame = videoPlayer.getTexture();
				}
			}
			else{
				ofSetColor(255); // Resetear color a blanco
				videoPlayer.draw(offsetVideoPosX, offsetVideoPosY, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
				ofDrawBitmapString("Memoria: VO", 20, 80);
				
				// Guardar el frame actual como �ltimo v�lido
				lastValidFrame = videoPlayer.getTexture();
			}
			
    
		}
		else {
			ofDrawBitmapString("CARGANDO MEMORIA", 20, 80);
		}
		

		// GRAFICOS DE REFERENCIA
		
		int xSensor = ofMap(sensor, 0, 1023, 0, ofGetWidth()); // valor traducido en x del sensor
		
		// fondo barra inferior
		ofSetColor(50, 60, 70);
		ofDrawRectangle(0, ofGetHeight()-20, ofGetWidth(), ofGetHeight());
	
		for (int i = 0; i < cantVideos - 1; i++) {
			
			float div = ((float)ofGetWidth() / (float)(cantVideos -2));

			int xSints = i * div;
			
			// marcas de sintonizacion
			if( abs(xSensor - xSints) < tam_dial/2 ){
				ofFill(); ofSetColor(190, 195, 200);
				ofDrawRectangle(xSints-(tam_dial*1.2/2), ofGetHeight()-24, tam_dial*1.2, ofGetHeight());
				
				ofDrawBitmapString("0x"+ ofToString(currentVideoIndex) + "aa8c" + ofToString((int) ofMap(distortionAmount, 0, 1, 100, 999)) , xSints-5, ofGetHeight()-30);
			}
			else{
				ofFill(); ofSetColor(150, 160, 180);
				ofDrawRectangle(xSints-(tam_dial/2), ofGetHeight()-20, tam_dial, ofGetHeight());
			}
		}
		
		// barra valor sensor
		ofFill(); ofSetColor(255);
		ofDrawRectangle(xSensor, ofGetHeight()-20, 2, ofGetHeight());
		
		if(oscuridad > 0){
			ofSetColor(0, oscuridad);
			ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		}

	}
	else{
		//ofSetColor(255, 255, 255, 180);
		lastValidFrame.draw(offsetVideoPosX, offsetVideoPosY, lastValidFrame.getWidth()*resizeVideo, lastValidFrame.getHeight()*resizeVideo);
		
		ofDrawBitmapString("CARGANDO MEMORIA", 20, 80);
	}
	
	
	ofSetColor(255);
	ofDrawBitmapString("Rendimiento: " + ofToString((int)ofGetFrameRate()) , 20, 20);
	ofDrawBitmapString("Recuperado: " + ofToString((int)ofMap(distortionAmount, 0, 1, 100, 0)) + "%", 20, 40);
	ofDrawBitmapString("Actividad: " + ofToString((int)ofMap(oscuridad, 0, 245, 100, 0)) + "%", 20, 60);
	ofDrawBitmapString("aTextura: " + string(videoTexture.isAllocated() ? "SI" : "NO"), 20, 100);
	ofDrawBitmapString("aFBO: " + string(fbo.isAllocated() ? "SI" : "NO"), 20, 120);
	ofDrawBitmapString("Memoria: " + string(videoPlayer.isLoaded() ? "SI" : "NO"), 20, 140);
	ofDrawBitmapString("Fragmento: " + ofToString(currentVideoIndex) , 20, 160);
	
	ofDrawBitmapString(textoDerecha, ofGetWidth()-offsetVideoPosX+20, 20); 
}

void ofApp::dibujarEtiquetas(int x, int y, int w, int h){
	int tam_etiqueta = ofMin(w/10, h/97);

	ofColor color_actual;

// !! agregar asignacion de indice de etiqueta cuando se cambia de video
	if(videoPlayer.getPosition() > = pos_prox_etiqueta){
		i_etiqueta++;
		pos_prox_etiqueta = etiquetasVideos[currentVideoIndex][i_etiqueta+1].posicion;
	}

	for(int y=0; y<10; y++){
		for(int x=0; x<97; x++){

			// me fijo si la etiqueta por dibujar es la actual
			if(grilla[y][x].nombre == colores_etiquetas[currentVideoIndex][i_etiqueta].nombre){
				ofColor color_conf = colores_etiquetas[etiqueta_actual.confianza]; // selecciono el color correspondiente al indice de confianza
				grilla[y][x].color = grilla[y][x].color.ofLerp(color_conf, 0.7); // cambio el color de esa etiqueta en la grilla
			}

			ofSetColor(grilla[y][x].color);
			ofDrawRectangle(x*tam_etiqueta, y*tam_etiqueta, tam_etiqueta, tam_etiqueta);
		}
	}
}

// --------------------------------------------------------------------------------------------------------
void ofApp::applyGlitchEffect() {
    ofClear(0, 0, 0, 255);
    ofSetColor(255); // Resetear color a blanco
	
    textoDerecha = "ERRORES\n";
    
    // Efecto base - cuanto mayor distortionAmount, m�s probable es que falle
    if(ofRandomuf() > distortionAmount * 0.0f) {
        videoTexture.draw(0, 0, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
        textoDerecha += "Base: 0\n";
    }
    else{
		 textoDerecha += "Base: 1\n";
	}

    // 1. Efecto de corrupci�n digital extrema cuando distortionAmount es alto
    if(distortionAmount > 0.5f) {
        float corruptionIntensity = ofMap(distortionAmount, 0.5f, 1.0f, 0.0f, 1.0f);
        
        // Distorsi�n de bloques (como compresi�n JPEG corrupta)
        if(ofRandomuf() < corruptionIntensity * 0.7f) {
            int blockSize = ofRandom(5, 55) * corruptionIntensity;
            int numBlocks = ofRandom(1, 25 + 60 * corruptionIntensity);
            
            for(int i = 0; i < numBlocks; i++) {
                int x = ofRandom(videoPlayer.getWidth()*resizeVideo);
                int y = ofRandom(videoPlayer.getHeight()*resizeVideo);
                int newX = x + ofRandom(-100, 100) * corruptionIntensity;
                int newY = y + ofRandom(-100, 100) * corruptionIntensity;
                
                ofSetColor(ofRandom(255), ofRandom(255), ofRandom(255));
                videoTexture.drawSubsection(
                    newX, newY, 
                    blockSize, blockSize, 
                    x, y, 
                    blockSize, blockSize
                );
            }
             textoDerecha += "Bloques: 1\n";
        }
        else  textoDerecha += "Bloques: 0\n";
        
        // L�neas de ruido digital (como errores de lectura)
        if(ofRandomuf() < corruptionIntensity * 0.5f) {
            int numLines = ofRandom(1, 15 + 60 * corruptionIntensity);
            for(int i = 0; i < numLines; i++) {
                int y = ofRandom(videoPlayer.getHeight()*resizeVideo);
                int height = ofRandom(1, 6 + 30 * corruptionIntensity);
                int xOffset = ofRandom(-55, 55) * corruptionIntensity;
                
                ofSetColor(ofRandom(150, 255), ofRandom(150, 255), ofRandom(150, 255));
                videoTexture.drawSubsection(
                    xOffset, y, 
                    videoPlayer.getWidth()*resizeVideo, height, 
                    0, y, 
                    videoPlayer.getWidth()*resizeVideo, height
                );
            }
            textoDerecha += "LRD: 1\n";
        }
        else  textoDerecha += "LRD: 0\n";
        
        // Congelamiento o salto de frames (simulando error de lectura)
        if(ofRandomuf() < corruptionIntensity * 0.05f) {
            ofSetColor(255);
            videoTexture.draw(0, 0, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo * ofRandom(0.6f, 1.0f));
            ofSetColor(100);
            ofDrawRectangle(0, videoPlayer.getHeight()*resizeVideo * ofRandom(0.6f, 1.0f), 
                          videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
                          
            textoDerecha += "Lectura: 1\n";
        }
        else  textoDerecha += "Lectura: 0\n"; 
    }
    else textoDerecha += "Bloques: 0\nLRD: 0\nLectura: 0\n";
    
    // 2. Efectos de color y desplazamiento
    if(distortionAmount > 0.1f) {
        // Desplazamiento RGB (m�s extremo cuando distortionAmount es alto)
        float rgbShift = 5 + 30 * powf(distortionAmount, 3);
        if(rgbShift > 1.0f) {
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            
            ofSetColor(255, 0, 0, ofMap(distortionAmount, 0, 1, 0, 100));
            videoTexture.draw(ofRandom(-rgbShift, 0), ofRandom(-rgbShift, 0));
            
            ofSetColor(0, 255, 0, ofMap(distortionAmount, 0, 1, 0, 100));
            videoTexture.draw(ofRandom(-rgbShift, 0), ofRandom(-rgbShift, 0));
            
            ofSetColor(0, 0, 255, ofMap(distortionAmount, 0, 1, 0, 100));
            videoTexture.draw(ofRandom(-rgbShift, 0), ofRandom(-rgbShift, 0));
            
            ofDisableBlendMode();
            
             textoDerecha += "dRGB: 1\n"; 
        }
        else  textoDerecha += "dRGB: 0\n";
        
    }
    else textoDerecha += "dRGB: 0\n";
}


void ofApp::audioOut(ofSoundBuffer &buffer) {
    // Limpiar el buffer
    buffer.set(0);
    
    // Solo procesar si hay video cargado
    if(videoPlayer.isLoaded() && distortionAmount > 0.01f) {
        // Obtener muestras de audio del video
        vector<float> tempBuffer(buffer.size());
        videoPlayer.getAudio(tempBuffer.data(), buffer.size(), 0);
        
        // Aplicar efectos de glitch
        for(size_t i = 0; i < buffer.size(); i++) {
            float sample = tempBuffer[i];
            
            // 1. Distorsi�n suave
            sample *= (1.0f + distortionAmount);
            
            // 2. Peque�as interrupciones aleatorias
            if(ofRandomuf() < distortionAmount * 0.2f) {
                sample *= ofRandom(0.0f, 1.0f);
            }
            
            // 3. Bit crushing (reducci�n de resoluci�n)
            if(ofRandomuf() < distortionAmount * 0.1f) {
                float steps = pow(2, 8 - int(distortionAmount * 5));
                sample = round(sample * steps) / steps;
            }
            
            // Limitar el rango y asignar al buffer
            buffer[i] = ofClamp(sample, -1.0f, 1.0f);
        }
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){

}

void ofApp::exit() {
	
}


//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
