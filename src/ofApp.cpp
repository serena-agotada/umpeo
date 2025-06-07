#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
// VARIABLES DE SETTEO ---------------------------------------------------------
	
	cantVideos = 59;
	
	resizeVideo = 0.85;
	
	tam_dial = cantVideos * 30;
	
	dist_dial = (int) ofGetWidth()/cantVideos/2.5;
	
	videoFiles.reserve(cantVideos);

// VIDEO
	
    videoPlayer.setUseTexture(true);

	shouldSetPosition = false;

	// Definir el directorio de los videos
	ofDirectory dir("/home/sara/Desktop/videos-umpeo");
	dir.allowExt("mp4");  // Filtra solo archivos con la extensión .mp4

	// Lista de archivos en el directorio
	dir.listDir();

	// Recorrer la lista de archivos y almacenarlos en videoFiles
	for (int i = 0; i < cantVideos; i++) {
		videoFiles.push_back("/home/sara/Desktop/videos-umpeo/" + ofToString(i) +".mp4");
		videoPositions.push_back(0.0f);
	}

	if (cantVideos > 0) {
		divisionSensor = tam_dial / (cantVideos - 1);

		currentVideoIndex = 0;  // Iniciar con el primer video

		// Cargar el primer video
		videoPlayer.load(videoFiles[currentVideoIndex]);
		
		videoPlayer.setLoopState(OF_LOOP_NORMAL);
		videoPlayer.play();
		fbo.clear();
		fbo.allocate(videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
		
		ofLog() << "Videos cargados";
		
	}
	else {
		ofLog() << "No se encontraron videos en el directorio!";
	}
    
// ETIQUETAS ------------------------------------------------------------------------------	

		// Colores etiquetas
		ofColor c1 = ofColor::fromHex(0x001f34);
		ofColor c2 = ofColor::fromHex(0x40c258);
		ofColor c3 = ofColor::fromHex(0x00FFFF);
		ofColor c4 = ofColor::fromHex(0x5261ee);
		ofColor c5 = ofColor::fromHex(0xd60062);


		for(int c=0; c<100; c++){
			if(c < 25){
				colores_etiquetas[c] = c1.lerp(c2, 0.04*c);
			}
			else if(c < 50){
				colores_etiquetas[c] = c2.lerp(c3, 0.04*(c-25));
			}
			else if(c < 75){
				colores_etiquetas[c] = c3.lerp(c4, 0.04*(c-50));
			}
			else{
				colores_etiquetas[c] = c4.lerp(c5, 0.04*(c-75));
			}
		}
		
		// creacion de variables de etiquetas

		float pos_prox_etiqueta = 0.0f;

		ofFile file("/home/sara/Desktop/etiquetas-umpeo/grilla.csv");
		
		ofBuffer buffer(file);

		int n_linea = 0;

		//Generar grilla de etiquetas
		for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
			string line = *it;
			//Split line into strings
			vector<string> et = ofSplitString(line, ",");

			//guardo cada nombre en el array y seteo el color a negro
			for(int x=0; x < 18; x++) {
				grilla[n_linea][x].nombre = et[x];
				grilla[n_linea][x].color = ofColor(0);
			}

			n_linea++;
		}
		ofLog() << "Grilla de etiquetas cargada";

	// Recorrer la lista de archivos y almacenarlos en etiquetasVideos
	for (int i = 0; i < cantVideos; i++) {
		//Load file
		ofFile file("/home/sara/Desktop/etiquetas-umpeo/Copia de labels - " + ofToString(i) +".csv");
		
		ofBuffer buffer(file);
		
		ofBuffer::Lines lines = buffer.getLines(); // Obtener las líneas como iterable
		int numLines = std::distance(lines.begin(), lines.end()); // Cuenta las líneas eficientemente

		int cont = 0;

		//Read file line by line
		for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
			string line = *it;
			//Split line into strings
			vector<string> words = ofSplitString(line, ",");

			//Store strings into a custom container
			if (words.size()>=3) {
				Etiqueta e;
				e.nombre = words[1];
				e.confianza = (float)stoi(words[2]);
				e.posicion = ofMap(cont, 0, numLines, 0, 1);

				//guardar etiqueta
				etiquetasVideos[i].push_back(e);

				cont++;
			}
		}	
	}
	ofLog() << "Etiquetas cargadas";
	
	
// OSC -------------------------------------------------------------------------
	ofLog() << "listening for osc messages on port " << PORT;
	receiver.setup(PORT);
	
	
// INICIALIZACION DE VARIABLES
	currentVideoIndex = 0;
	newVolume = 0;
	prevDist = 0;
	
	sensor = 0;
	distortionAmount = 0.0f;
	clip = 0;
	
	oscuridad = 0;
	
	i_etiqueta = 0;
	pos_prox_etiqueta = etiquetasVideos[currentVideoIndex][1].posicion;
	
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
		else if(m.getAddress() == "/distSint"){
			distSintVideo = m.getArgAsInt32(0);
		}
	}
	
	offsetVideoPosX = ( (float)(ofGetWidth()) - (float)videoPlayer.getWidth()*resizeVideo) / 2;
	offsetVideoPosY = ( (float)(ofGetHeight()) - (float)videoPlayer.getHeight()*resizeVideo) / 2 - 10;
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0, 0, 0);

	if (ofGetFrameRate() >= 0){
		
// ACTUALIZACION DE VIDEO -----------------------------------------------------------------------
		// solo hago update del video si cambie de indice
		if (newIndex != currentVideoIndex) {
			//ofLog() << "nuevo indice: " << newIndex;
			currentVideoIndex = newIndex;
			//videoPlayer.close();
			videoPlayer.load(videoFiles[currentVideoIndex]);
			
			// Espera a que el video se cargue
			if(videoPlayer.isLoaded()) {
				videoPlayer.update();
				videoPlayer.setPosition(videoPositions[currentVideoIndex]);
				videoPlayer.play();
				
				// Actualiza el indice de etiqueta
				i_etiqueta = floor( ofMap( (float)videoPositions[currentVideoIndex], 0, 1, 0, (int)etiquetasVideos[currentVideoIndex].size() ) );
				
				if( i_etiqueta < etiquetasVideos[currentVideoIndex].size() ){
					pos_prox_etiqueta = etiquetasVideos[currentVideoIndex][i_etiqueta+1].posicion;
				}
				
				// Actualiza la textura inmediatamente
				videoTexture = videoPlayer.getTexture();
				
				// Reallocar el FBO con las nuevas dimensiones
				fbo.clear();
				fbo.allocate(videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
			}
			
		}

		if (videoPlayer.isLoaded()) {
			
			// Actualiza la posición si no lo hizo todavía
			if (shouldSetPosition && videoPlayer.getPosition() < videoPositions[currentVideoIndex]) {
				videoPlayer.setPosition(videoPositions[currentVideoIndex]);
				
				//Solo declaro que no hay que cambiar la posición si realmente se actualizó
				if (videoPlayer.getPosition() == videoPositions[currentVideoIndex]) {
					shouldSetPosition = false;
				}
			}
			
// SONIDO --------------------------------------------------------------------------------------------			

			// el volumen varia con el valor de oscuridad
			if (oscuridad < 240){
				newVolume = ofMap(oscuridad, 0, 240, 0.99, 0.02, true);
				videoPlayer.setVolume( newVolume );
			}
			// el volumen varía con el valor de los sensores
			else if (abs(prevDist - distSintVideo) > divisionSensor / 18) {
				prevDist = distSintVideo;
				newVolume = ofLerp(newVolume, ofMap(distSintVideo, 0, divisionSensor / 2, 0.95, 0, true), 0.5);
				videoPlayer.setVolume( newVolume );
				//cout << "volumen: " << newVolume << " | ";
			}
// REPRODUCTOR ------------------------------------------------------------------------------------------------------------------				
			
			
			ofSetColor(255);
			if(distortionAmount > 0.02){
				if(fbo.isAllocated() && videoTexture.isAllocated()) {
					
					fbo.draw(offsetVideoPosX, offsetVideoPosY, fbo.getWidth(), fbo.getHeight());
					//ofDrawBitmapString("Memoria: FBO", 20, offsetVideoPosY+60);
					
					// Guardar el frame actual como último válido
					fbo.readToPixels(pixels);
					lastValidFrame.loadData(pixels);
					
				} else if(lastValidFrame.isAllocated()) {
					// Si hay problemas, dibuja el último frame válido
					lastValidFrame.draw(offsetVideoPosX, offsetVideoPosY, lastValidFrame.getWidth()*resizeVideo, lastValidFrame.getHeight()*resizeVideo);
					//ofDrawBitmapString("Memoria: UFV", 20, offsetVideoPosY+60);
				}
				else{
					videoPlayer.draw(offsetVideoPosX, offsetVideoPosY, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
					//ofDrawBitmapString("Memoria: VO", 20, offsetVideoPosY+60);
					
					// Guardar el frame actual como último válido
					lastValidFrame = videoPlayer.getTexture();
				}
			}
			else{
				videoPlayer.draw(offsetVideoPosX, offsetVideoPosY, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
				//ofDrawBitmapString("Memoria: VO", 20, offsetVideoPosY+60);
				
				// Guardar el frame actual como último válido
				lastValidFrame = videoPlayer.getTexture();
			}
			
    
		}
		else {
			ofDrawBitmapString("CARGANDO MEMORIA", 20, offsetVideoPosY+60);
		}
		
// GRAFICOS ---------------------------------------------------------------------------------------------------

		//oscuresco
		if(oscuridad > 0){
			ofSetColor(0, oscuridad);
			ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		}
		
		int xSensor = ofMap(sensor, 0, tam_dial, 0, ofGetWidth()); // valor traducido en x del sensor
		
		// fondo barra inferior
		ofSetColor(50, 60, 70);
		ofDrawRectangle(0, ofGetHeight()-20, ofGetWidth(), ofGetHeight());
	
		for (int i = 0; i < cantVideos - 1; i++) {
			
			float div = ((float)ofGetWidth() / (float)(cantVideos -2));

			int xSints = i * div;
			
			// marcas de sintonizacion
			if( abs(xSensor - xSints) < dist_dial/2 ){
				ofFill(); ofSetColor(190, 195, 200);
				ofDrawRectangle(xSints-(dist_dial*1.2/2), ofGetHeight()-24, dist_dial*1.2, ofGetHeight());
				
				ofDrawBitmapString("0x"+ ofToString(currentVideoIndex) + "aa" + ofToString(distSintVideo) + "c" + ofToString((int) ofMap(distortionAmount, 0, 1, 100, 999)) , xSints-5, ofGetHeight()-30);
			}
			else{
				ofFill(); ofSetColor(150, 160, 180);
				ofDrawRectangle(xSints-(dist_dial/2), ofGetHeight()-20, dist_dial, ofGetHeight());
			}
		}
		
		// barra valor sensor
		ofFill(); ofSetColor(255, (int)ofMap(oscuridad, 0, 240, 255, 5));
		ofDrawRectangle(xSensor, ofGetHeight()-20, 2, ofGetHeight());
		

	}
	else{
		//ofSetColor(255, 255, 255, 180);
		lastValidFrame.draw(offsetVideoPosX, offsetVideoPosY, lastValidFrame.getWidth()*resizeVideo, lastValidFrame.getHeight()*resizeVideo);
		
		ofDrawBitmapString("CARGANDO MEMORIA", 20, offsetVideoPosY);
	}
	
	
	ofSetColor(255);
	ofDrawBitmapString("Reproduccion: " + ofToString((int)ofGetFrameRate()) , 20, offsetVideoPosY);
	ofDrawBitmapString("Actividad:", 20, offsetVideoPosY+20);
	
	dibujarBarraProgreso(20, offsetVideoPosY+25, ofMap(oscuridad, 0, 240, 100, 0, true));
	
	ofSetColor(255);
	ofDrawBitmapString("Archivo recuperado:", 20, offsetVideoPosY+60);
	
	dibujarBarraProgreso(20, offsetVideoPosY+65, ofMap(distortionAmount, 0, 1, 100, 0, true));
	
	ofSetColor(255);
	//ofDrawBitmapString("Memoria recuperada: " + string(videoPlayer.isLoaded() ? "SI" : "NO"), 20, offsetVideoPosY+100);
	//ofDrawBitmapString("Indice: " + ofToString(currentVideoIndex) , 20, offsetVideoPosY+120);
	//ofDrawBitmapString("aTextura: " + string(videoTexture.isAllocated() ? "SI" : "NO"), 20, offsetVideoPosY+80);
	//ofDrawBitmapString("aFBO: " + string(fbo.isAllocated() ? "SI" : "NO"), 20, offsetVideoPosY+100);
	
	//ofDrawBitmapString(textoDerecha, 20, offsetVideoPosY+160); 
	
	dibujarEtiquetas( ofGetWidth()-offsetVideoPosX+10, offsetVideoPosY, offsetVideoPosX, (ofGetHeight()/3)*2-offsetVideoPosY*2) ;
}

// --------------------------------------------------------------------------------------------------------
void ofApp::dibujarEtiquetas(int xx, int yy, int w, int h){
	int tam_etiqueta = (w-10-17*2)/18;

	ofColor color_actual;

	// Setteo el indice de etiqueta en 0 si loopeo el video
	if(videoPlayer.getPosition() >= 1 || videoPlayer.getPosition() <= 0){
		i_etiqueta = 0;
		pos_prox_etiqueta = etiquetasVideos[currentVideoIndex][i_etiqueta+1].posicion;
	}
	// Actualizo el indice de etiqueta si ya tendría que pasar al siguiente
	else if(videoPlayer.getPosition() >= pos_prox_etiqueta && i_etiqueta < etiquetasVideos[currentVideoIndex].size()){
		i_etiqueta++;
		pos_prox_etiqueta = etiquetasVideos[currentVideoIndex][i_etiqueta+1].posicion;
	}
	
	Etiqueta etiqueta_actual = etiquetasVideos[currentVideoIndex][i_etiqueta];
	
	//muestro las etiquetas
	if(ofGetFrameNum() % 8 == 0){
		vector<string>::iterator ultima = displayEtiquetas.end();
		if(displayEtiquetas.size() == 0 || *ultima != etiqueta_actual.nombre){
			displayEtiquetas.push_back(etiqueta_actual.nombre);
		}
		if(displayEtiquetas.size()*20 + tam_etiqueta*52 + offsetVideoPosY >= ofGetHeight()-offsetVideoPosY){
			displayEtiquetas.erase( displayEtiquetas.begin() );
		}
	}
	ofSetColor(255);
	for(int de=0; de < displayEtiquetas.size(); de++){
		ofDrawBitmapString(displayEtiquetas[de], xx, tam_etiqueta*52 + offsetVideoPosY*2 + de*20);
	}

	for(int y=0; y<52; y++){
		for(int x=0; x<18; x++){
			
			// me fijo si la etiqueta por dibujar es la actual
			if(grilla[y][x].nombre == etiqueta_actual.nombre){
				ofColor color_conf = colores_etiquetas[(int)etiqueta_actual.confianza-1]; // selecciono el color correspondiente al indice de confianza
				grilla[y][x].color = grilla[y][x].color.lerp(color_conf, 0.5); // cambio el color de esa etiqueta en la grilla
				
				color_conf = ofColor(0, 20, 50).lerp(ofColor(250, 10, 20), etiqueta_actual.confianza*0.01); // heatmap
				grilla[y][x].color = grilla[y][x].color.lerp(ofColor(0, 250, 50), etiqueta_actual.confianza*0.01); // heatmap
				
				ofSetColor(color_conf);
				ofDrawRectangle(xx+x*tam_etiqueta-2, yy+y*tam_etiqueta-2, tam_etiqueta+2, tam_etiqueta+2);
			}
			else{
				grilla[y][x].color = grilla[y][x].color.lerp(colores_etiquetas[0], 0.0000001);
				ofSetColor(grilla[y][x].color);
				ofDrawRectangle(xx+x*tam_etiqueta, yy+y*tam_etiqueta, tam_etiqueta-2, tam_etiqueta-2);
			}
		}
	}
	
	ofSetColor(255);
}

// --------------------------------------------------------------------------------------------------------
void ofApp::applyGlitchEffect() {
    ofClear(0, 0, 0, 255);
    ofSetColor(255); // Resetear color a blanco
	
    textoDerecha = "ERRORES\n";
    
    // Efecto base - cuanto mayor distortionAmount, más probable es que falle
    if(ofRandomuf() > distortionAmount * 0.0f) {
        videoTexture.draw(0, 0, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
        textoDerecha += "Base: 0\n";
    }
    else{
		 textoDerecha += "Base: 1\n";
	}

    // 1. Efecto de corrupción digital extrema cuando distortionAmount es alto
    if(distortionAmount > 0.3f) {
        float corruptionIntensity = ofMap(distortionAmount, 0.3f, 1.0f, 0.0f, 1.0f);
        
        // Distorsión de bloques (como compresión JPEG corrupta)
        if(ofRandomuf() < corruptionIntensity * 0.4f) {
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
        
        // Líneas de ruido digital (como errores de lectura)
        if(ofRandomuf() < corruptionIntensity * 0.3f) {
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
        if(ofRandomuf() < corruptionIntensity * 0.1f) {
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
        // Desplazamiento RGB (más extremo cuando distortionAmount es alto)
        float rgbShift = 3 + 30 * powf(distortionAmount, 3);
        
        if(rgbShift > 1.0f) {
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            
            ofSetColor(255, 0, 0, 100);
            videoTexture.draw(ofRandom(-rgbShift, 0), ofRandom(-rgbShift, 0), videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
            ofSetColor(0, 255, 0, 100);
            videoTexture.draw(ofRandom(0, rgbShift), ofRandom(-rgbShift, 0), videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
            ofSetColor(0, 0, 255, 100);
            videoTexture.draw(ofRandom(-rgbShift, rgbShift), ofRandom(0, rgbShift), videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
            
            ofDisableBlendMode();
            
             textoDerecha += "dRGB: 1\n"; 
        }
        else  textoDerecha += "dRGB: 0\n";
        
    }
    else textoDerecha += "dRGB: 0\n";
}

void ofApp::dibujarBarraProgreso(int xx, int yy, float porcentaje){
	
	int ancho_barra = offsetVideoPosX-20-xx;
	int ancho_completado = (int)ofMap(porcentaje, 0, 100, 0, ancho_barra);
	
	ofSetColor(255, 100);
	ofDrawRectangle(xx, yy, ancho_barra, 15);
	
	ofSetColor(5, 180, 80);
	ofDrawRectangle(xx, yy, ancho_completado, 15);
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
