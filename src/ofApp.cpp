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
		fbo.allocate(videoPlayer.getWidth(), videoPlayer.getHeight());
		glitchFbo.allocate(videoPlayer.getWidth(), videoPlayer.getHeight());
		
		glitchTexture.allocate(videoPlayer.getWidth(), videoPlayer.getHeight(), OF_PIXELS_RGBA);
		
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
	
	inicio_linea_corrupta = 0;
	
	
}

//--------------------------------------------------------------
void ofApp::update(){
    
    videoPlayer.update();
    
    if(videoPlayer.isFrameNew() && videoTexture.isAllocated()) {
	    
	videoTexture = videoPlayer.getTexture();
        
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
				fbo.allocate(videoPlayer.getWidth(), videoPlayer.getHeight());
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
					
					fbo.draw(offsetVideoPosX, offsetVideoPosY, fbo.getWidth()*resizeVideo, fbo.getHeight()*resizeVideo);
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
	ofDrawBitmapString("Actividad:" + ofToString(ofMap(oscuridad, 0, 240, 100, 0, true)) + "%" , 20, offsetVideoPosY+20);
	
	//dibujarBarraProgreso(20, offsetVideoPosY+25, ofMap(oscuridad, 0, 240, 100, 0, true));
	
	ofSetColor(255);
	ofDrawBitmapString("Archivo recuperado:" + ofToString(ofMap(distortionAmount, 0, 1, 100, 0, true)) + "%", 20, offsetVideoPosY+60);
	
	//dibujarBarraProgreso(20, offsetVideoPosY+65, ofMap(distortionAmount, 0, 1, 100, 0, true));
	
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
    ofSetColor(255);
	
    textoDerecha = "ERRORES\n";

    // Dibujar frame base con una pequeña chance de fallar
    if(ofRandomuf() > distortionAmount * 0.05f) {
        videoTexture.draw(0, 0, videoPlayer.getWidth(), videoPlayer.getHeight());
        textoDerecha += "Base: 0\n";
    } 
    else {
	videoTexture.draw(0, 0, videoPlayer.getWidth(), videoPlayer.getHeight());   
        textoDerecha += "Base: 1\n";
    }

    if(distortionAmount > 0.1f) {

        // --- 2. Bloques glitch con color alterado ---
        if(ofRandomuf() < distortionAmount*2 || distortionAmount > 0.7) {
		
		if(ofGetFrameNum() % (int)ofRandom(20*distortionAmount) == 0){
			glitchFbo.begin();
			ofClear(0, 0, 0, 0);
			glitchTexture = videoPlayer.getTexture();
			int sizeX = 0;
			int sizeY = 0;
			int x = 0;
			int alto_linea = videoPlayer.getHeight()/1.7 * distortionAmount;
			if(ofRandom(1) > 0.95){
				inicio_linea_corrupta = ofRandom(videoPlayer.getHeight() - alto_linea);
			}
			int y = ofRandom(inicio_linea_corrupta);
			
			// ELECCION DE COLOR
			int ran_color = ofRandom(3);
			ofColor color_corrupcion;
			
			if(distortionAmount > 0.8){
				if(ran_color <= 1){
					color_corrupcion = ofColor(
						ofRandom(245, 255), 
						ofRandom(0, 10), 
						ofRandom(245, 255),
						ofMap(distortionAmount, 0, 1, 10, 255)
					    );
				}
				else if(ran_color <= 2){
					color_corrupcion = ofColor(
						ofRandom(0, 10), 
						ofRandom(245, 255), 
						ofRandom(245, 255),
						ofMap(distortionAmount, 0, 1, 10, 255)
					);
				}
				else if(ran_color <= 3){
					color_corrupcion = ofColor(
						ofRandom(245, 255), 
						ofRandom(245, 255), 
						ofRandom(0, 10),
						ofMap(distortionAmount, 0, 1, 10, 255)
					    );
				}
			}
			    
			    
			    // 1. Declara ofPixels
			    ofPixels texturePixels;

			    // 2. Obtén los píxeles de la textura (cuando necesites leer)
			    texturePixels.allocate(glitchTexture.getWidth(), glitchTexture.getHeight(), OF_PIXELS_RGBA);
			    glitchTexture.readToPixels(texturePixels);
				
			    for(int i = 0; y < videoPlayer.getHeight() - 10; i++) {
				 
				// FUERA DE LINEA CORRUPTA
				if(y < inicio_linea_corrupta || y > inicio_linea_corrupta+alto_linea){
					if( x >= videoPlayer.getWidth() - 10){
						y = y + ofRandom(5, videoPlayer.getHeight()-y);
						x = 0;
					}
					x = x + ofRandom(5, videoPlayer.getWidth()-x);
				}
				// DENTRO DE LINEA CORRUPTA
				else{
					if( x >= videoPlayer.getWidth() - 10){
						y = ofClamp(y + ofRandom(5, 10), 0, videoPlayer.getHeight());
						x = 0;
					}
					x = ofClamp(x + ofRandom(3, 7), 0, videoPlayer.getWidth());
				}

				// CALCULO COLOR DE PIXEL
				ofColor pixelColor = texturePixels.getColor(x, y);
				
				// TAMAÑO DE BLOQUE
				sizeX = ofRandom(8, 10);
				sizeY = ofRandom(8, 10);
				
				// CORRUPCION MAS FUERTE - COLOR
				if(distortionAmount > 0.8){
					if(pixelColor.getBrightness() > 80){
						
						// DESPLAZAMIENTO DE BLOQUES + COLOREADO (PARA PIXELES NO TAN CLAROS)
						if(pixelColor.getBrightness() < 220 || i % (int)ofRandom(10, 30) < 10){
							int dx = x + ofRandom(-5, 5);
							int dy = y + ofRandom(-5, 5);
							
							ofSetColor(color_corrupcion);
								
							glitchTexture.drawSubsection(
								dx, dy,
								sizeX, sizeY,
								x, y,
								sizeX, sizeY
							);
							
						}
						else{
							// SI EL PIXEL NO ES CLARO PINTO TODO EL BLOQUE DEL COLOR CORRUPTO
							if(pixelColor.getBrightness() < 220){
								ofSetColor(color_corrupcion.r, color_corrupcion.g, color_corrupcion.b, 255);
							}
							// SI EL PIXEL ES CLARO LE SUBO MUCHO EL BRILLO
							else{
								ofColor color_corrupcion_brillo = color_corrupcion;
								color_corrupcion_brillo.setBrightness(252);
								ofSetColor(color_corrupcion_brillo);
							}
							
							ofDrawRectangle(
								x, y,
								sizeX, sizeY
							);
						}
					}
					// BLOQUES NEGROS
					else if(ofRandom(1) > 0.9){
						ofSetColor(0, 0, 0, 255);
						ofDrawRectangle(
							x, y,
							sizeX, sizeY
						);
					}
				}
				
				// PIXELADO
				else if(distortionAmount > 0.1 && i % (int)ofRandom(2, 10) < 5){
					
					ofSetColor(pixelColor);
					ofDrawRectangle(
						x, y,
						sizeX, sizeY
					);
				}
			}
			
			glitchFbo.end();
		    
	    }
	    //ofEnableBlendMode(OF_BLENDMODE_SCREEN);
	    glitchFbo.draw(0, 0, videoPlayer.getWidth(), videoPlayer.getHeight());
	    //ofDisableBlendMode();
	    
            textoDerecha += "Bloques: 1\n";
        } else textoDerecha += "Bloques: 0\n";
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
