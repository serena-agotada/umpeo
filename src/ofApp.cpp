#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
// VARIABLES DE SETTEO ---------------------------------------------------------

	ofSetWindowTitle("run file_analysis()");
	
	resizeVideo = 0.95;
	
	texto.load("UbuntuMono-Regular.ttf", 14, true, true);
	
	ofSetWindowShape(1600, 900);
	
	lastFrameTime = 0;

// VIDEO
	
	videoPlayer.setUseTexture(true);

	shouldSetPosition = false;

	// Definir el directorio de los videos
	ofDirectory dir("/home/sara/Desktop/una-maquina-para-el-olvido/Videos/");
	dir.allowExt("mp4");  // Filtra solo archivos con la extensión .mp4

	// Lista de archivos en el directorio
	dir.listDir();
	cantVideos = 10;
	ofLog() << cantVideos;
	tam_dial = (cantVideos+1) * 15;
	
	videoFiles.reserve(cantVideos);
	deteccionesEtiquetas.resize(cantVideos);
	etiquetasVideos.resize(cantVideos);

	// Recorrer la lista de archivos y almacenarlos en videoFiles
	for (int i = 0; i < cantVideos; i++) {
		videoFiles.push_back("/home/sara/Desktop/una-maquina-para-el-olvido/Videos/" + ofToString(i) +".mp4");
		videoPositions.push_back(0.0f);
	}

	if (cantVideos > 0) {
		divisionSensor = tam_dial / (cantVideos - 1);

		currentVideoIndex = 0;  // Iniciar con el primer video

		// Cargar el primer video
		videoPlayer.load(videoFiles[currentVideoIndex]);
		
		//videoPlayer.setLoopState(OF_LOOP_NORMAL);
		videoPlayer.setLoopState(OF_LOOP_NONE);
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
ofLog() << "Cargando etiquetas...";
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
		

	// Recorrer la lista de archivos y almacenarlos en etiquetasVideos
	for (int i = 0; i < cantVideos; i++) {
	// -- deteccion de objetos
			ofFile file("/home/sara/Desktop/una-maquina-para-el-olvido/Etiquetas/" + ofToString(i) +".json");
			if(file.exists()){
				int id = 0;
				
				file >> js; //Reads the JSON data from the file into a ofJson object (variable js)
				if (js.contains("VideoMetadata") && !js["VideoMetadata"].is_null()){
					int duracion = js["VideoMetadata"]["DurationMillis"];
					
					// Access the "Labels" array
					for (auto& labelItem : js["Labels"]) {
					    // Access Label object
					    if (labelItem.contains("Label") && !labelItem["Label"].is_null()){
						    
						    ofJson label = labelItem["Label"];
						    if (label.contains("Instances") && !label["Instances"].is_null() && label["Instances"].size() > 0){
							    
								float timestamp = ofMap(labelItem["Timestamp"], 0, duracion, 0, 1);
								string name = label["Name"];
								float confidence = label["Confidence"];
								
								if(!label["Instances"].is_null()){
								    // Access Instances array
									for (auto& instance : label["Instances"]) {
										
											RectEtiqueta e;
											e.id = id;
											e.name = name;
											e.timestamp = timestamp;
											timestamp+=0.015;
											e.dibujable = true;
										
											// Access BoundingBox
											ofJson bbox = instance["BoundingBox"];
													
											e.width = bbox["Width"];
											e.height = bbox["Height"];
											e.left = bbox["Left"];
											e.top = bbox["Top"];
										
											// Access Instance Confidence
											e.confidence = instance["Confidence"];
											
											// si no se superpone con la etiqueta anterior
											bool resuelto = false;
											if(!deteccionesEtiquetas[i].empty()){
												for(int j = deteccionesEtiquetas[i].size()-1; j >= 0; j--){
													
													if(deteccionesEtiquetas[i][j].timestamp >= e.timestamp-0.07) {
													    // encuentra un objeto en el mismo lugar
														if(deteccionesEtiquetas[i][j].left == e.left && deteccionesEtiquetas[i][j].top == e.top){
															//lo reemplaza si confidence es mayor
															if(deteccionesEtiquetas[i][j].confidence < e.confidence) deteccionesEtiquetas[i][j] = e;
															
															// si no, deja el objeto anterior pero no guarda el nuevo
															resuelto = true;
														}
													}
													else break;
													
													
												}
											}
											
											if(!resuelto) deteccionesEtiquetas[i].push_back(e);
											//else ofLog() << "resuelto";
											
									}
								}
								else if(confidence <= 100){
									RectEtiqueta e;
									e.id = id;
									e.name = name;
									e.timestamp = timestamp;
									e.confidence = confidence;
									e.width = ofRandom(0.2);
									e.height = ofRandom(0.2);
									e.left = ofRandom(1-e.width);
									e.top = ofRandom(1-e.height);
									e.dibujable = true;
								}
								else{
									RectEtiqueta e;
									e.id = id;
									e.name = name;
									e.timestamp = timestamp;
									e.confidence = confidence;
									e.dibujable = false;
								}
						    }
						}
						else ofLog() << "El archivo no tiene etiquetas!";
						
						id++;
					}
					
				}
				else ofLog() << "El archivo no tiene duracion!";
				
			}
			else ofLog() << "El archivo de etiquetas " << i << " no existe!";
	
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
	
	ultimoRectEtiqueta = 0;
}

//--------------------------------------------------------------
void ofApp::update(){
    
    videoPlayer.update();
    
    if (videoPlayer.getIsMovieDone()) {
	    videoPlayer.stop();
	    videoPlayer.setPosition(0);
	    videoPlayer.play();
    }

    
    if(videoPlayer.isFrameNew()) {
	    lastFrameTime = ofGetElapsedTimef();
	    
	    if(videoPlayer.getTexture().isAllocated()){
		videoTexture = videoPlayer.getTexture();
		
		fbo.begin();
		{
		    applyGlitchEffect();
		    
		}
		fbo.end();
	    }
    }
    
        if(videoPlayer.isPlaying() && (ofGetElapsedTimef() - lastFrameTime > 1.0f)){
	    // No hubo frames nuevos en más de 1 segundo -> resync
	    //ofLogWarning() << "Reiniciando video por falta de frames";
	    ofSetColor(0);
	    ofDrawRectangle(offsetVideoPosX, offsetVideoPosY, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
	    ofSetColor(255);
	    ofDrawCircle(offsetVideoPosX+videoPlayer.getWidth()*resizeVideo/2, offsetVideoPosY+videoPlayer.getHeight()*resizeVideo/2, 50);
	    ofDrawBitmapString("CARGANDO", offsetVideoPosX + videoPlayer.getWidth()*resizeVideo/2, offsetVideoPosY+videoPlayer.getHeight()*resizeVideo/2+50);
	    float pos = getPosicionSegura();
	    videoPlayer.stop();
	    videoPlayer.setPosition(pos);
	    videoPlayer.play();
	    ofSleepMillis(100); // breve pausa
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
	
	offsetVideoPosX = ( (float)(ofGetWidth()) - (float)videoPlayer.getWidth()*resizeVideo);
	offsetVideoPosY = 0;
	
	dist_dial = (int) (ofGetWidth()-offsetVideoPosX)/cantVideos;

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0, 0, 0);

	if (ofGetFrameRate() >= 0){
		
// ACTUALIZACION DE VIDEO -----------------------------------------------------------------------
		// solo hago update del video si cambie de indice
		if (newIndex != currentVideoIndex) {
			 // Cierra y libera el video anterior completamente
			videoPlayer.stop();
			videoPlayer.close();
			ofSleepMillis(50); // breve pausa para liberar el pipeline
    
			//ofLog() << "nuevo indice: " << newIndex;
			currentVideoIndex = newIndex;
			
			videoPlayer.load(videoFiles[currentVideoIndex]);
			
			// Espera a que el video se cargue
			if(videoPlayer.isLoaded()) {
				videoPlayer.update();
				videoPlayer.setPosition(videoPositions[currentVideoIndex]);
				videoPlayer.play();
				
				// Actualiza el indice de etiqueta
				i_etiqueta = floor( ofMap( (float)videoPositions[currentVideoIndex], 0, 1, 0, (int)etiquetasVideos[currentVideoIndex].size() ) );
				
				if( i_etiqueta < (int)etiquetasVideos[currentVideoIndex].size() ){
					pos_prox_etiqueta = etiquetasVideos[currentVideoIndex][i_etiqueta+1].posicion;
				}
				
				// Actualiza la textura inmediatamente
				videoTexture = videoPlayer.getTexture();
				
				// Reallocar el FBO con las nuevas dimensiones
				fbo.clear();
				fbo.allocate(videoPlayer.getWidth(), videoPlayer.getHeight());
			}
			ultimoRectEtiqueta = 0;
			
		}

		if (videoPlayer.isLoaded()) {
			// Actualiza la posición si no lo hizo todavía
			if (shouldSetPosition && getPosicionSegura() < videoPositions[currentVideoIndex]) {
				videoPlayer.setPosition(videoPositions[currentVideoIndex]);
				
				//Solo declaro que no hay que cambiar la posición si realmente se actualizó
				if (getPosicionSegura() == videoPositions[currentVideoIndex]) {
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
			if(distortionAmount > 0.02f){

				if (distortionAmount > 0.8f && ofRandomuf() < (ofGetFrameNum()%10)*0.1f*distortionAmount){
					ofDrawBitmapString("ERROR DE MEMORIA", offsetVideoPosX + videoPlayer.getWidth()*resizeVideo/2.5, offsetVideoPosY+videoPlayer.getHeight()*resizeVideo/2);
					ofSleepMillis(100);
				}
				else if(fbo.isAllocated() && videoTexture.isAllocated()) {
					
					fbo.draw(offsetVideoPosX, offsetVideoPosY, fbo.getWidth()*resizeVideo, fbo.getHeight()*resizeVideo);

					// Guardar el frame actual como último válido
					fbo.readToPixels(pixels);
					lastValidFrame.loadData(pixels);

					
				} else if(lastValidFrame.isAllocated()) {
					// Si hay problemas, dibuja el último frame válido
					lastValidFrame.draw(offsetVideoPosX, offsetVideoPosY, lastValidFrame.getWidth()*resizeVideo, lastValidFrame.getHeight()*resizeVideo);

				}
				else{
					videoPlayer.draw(offsetVideoPosX, offsetVideoPosY, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);

					// Guardar el frame actual como último válido
					lastValidFrame = videoPlayer.getTexture();
				}
			}
			else{
				videoPlayer.draw(offsetVideoPosX, offsetVideoPosY, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo);
				
				// Guardar el frame actual como último válido
				lastValidFrame = videoPlayer.getTexture();
			}
			
			dibujarDeteccion();
			
    
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
		
		dibujarGraficoEtiquetas(0, ofGetHeight()*0.19, offsetVideoPosX, ofGetHeight());
		
	}
	else{
		//ofSetColor(255, 255, 255, 180);
		lastValidFrame.draw(offsetVideoPosX, offsetVideoPosY, lastValidFrame.getWidth()*resizeVideo, lastValidFrame.getHeight()*resizeVideo);
		
		ofDrawBitmapString("CARGANDO MEMORIA", 20, offsetVideoPosY);
	}
	
	int margen = offsetVideoPosX/22;
	
	ofSetColor(50);
	ofDrawRectangle(0, 0, offsetVideoPosX, 105+margen*2);
	
	ofSetColor(255);
	texto.drawString("Actividad:" + ofToString(ofMap(oscuridad, 0, 240, 100, 0, true)) + "%" , 20, 40);
	dibujarBarraProgreso(0, 45, offsetVideoPosX, ofMap(oscuridad, 0, 240, 100, 0, true));
	
	ofSetColor(255);
	texto.drawString("Fidelidad:" + ofToString((int)ofMap(distortionAmount, 0, 1, 100, 0, true)) + "%", 20, 100);
	dibujarBarraProgreso(0, 105, offsetVideoPosX, ofMap(distortionAmount, 0, 1, 100, 0, true));
	
	dibujarBarraRecorrido();

}

void ofApp::dibujarDeteccion(){
	if(distortionAmount < 0.9){
		frame_ids_detectados.clear();
		
		float min_conf = ofMap(distortionAmount, 1, 0, 0, 75, true);
		float amp_conf = ofMap(distortionAmount, 1, 0, 10, 40);
		
		for(int i = deteccionesEtiquetas[currentVideoIndex].size()-1; i >= 0 && deteccionesEtiquetas[currentVideoIndex][i].timestamp > getPosicionSegura()-0.35; i--){
			
			RectEtiqueta e = deteccionesEtiquetas[currentVideoIndex][i];
			
			if(e.confidence > min_conf && e.confidence < min_conf+amp_conf && e.timestamp < getPosicionSegura()+0.03 && e.timestamp > getPosicionSegura()-0.03){
				
				if(deteccionesEtiquetas[currentVideoIndex][i].dibujable){
						deteccionesEtiquetas[currentVideoIndex][i].dibujar(texto, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo, offsetVideoPosX, offsetVideoPosY);
						ultimoRectEtiqueta = i;
				}
				
				/*if(frame_ids_detectados.empty() || find(frame_ids_detectados.begin(), frame_ids_detectados.end(), e.id) == frame_ids_detectados.end()){
					if(deteccionesEtiquetas[currentVideoIndex][i].dibujable){
						deteccionesEtiquetas[currentVideoIndex][i].dibujar(texto, videoPlayer.getWidth()*resizeVideo, videoPlayer.getHeight()*resizeVideo, offsetVideoPosX, offsetVideoPosY);
						ultimoRectEtiqueta = i;
					
						frame_ids_detectados.push_back(e.id);
					}
				}*/
				
				bool etiquetaYaGuardada = false;
			
				for(int i=0; i < etiquetasDetectadas.size(); i++){
					if(etiquetasDetectadas[i].name == e.name){
						etiquetasDetectadas[i].timestamp++;
						etiquetasDetectadas[i].confidence = (etiquetasDetectadas[i].confidence/etiquetasDetectadas[i].timestamp)*(etiquetasDetectadas[i].timestamp-1) + e.confidence/etiquetasDetectadas[i].timestamp;
						etiquetaYaGuardada = true;
						break;
					}

				}
				if(!etiquetaYaGuardada){
					RectEtiqueta nEtiqueta;
					nEtiqueta.name = e.name;
					nEtiqueta.confidence = e.confidence;
					nEtiqueta.timestamp = 1;
					
					etiquetasDetectadas.push_back(nEtiqueta);
				}
			}
			
		}
		
	}
}

//--------------------------------------------------------------
void ofApp::dibujarGraficoEtiquetas(int xx, int yy, int ww, int hh){
	int margen = ww/22;
	
	//fondo
	ofSetColor(50);
	ofDrawRectangle(0, yy-margen*2, ww, hh);
	
	ofSetColor(255);
	texto.drawString("Objetos encontrados:", xx + margen*2, yy);
	
	if(!etiquetasDetectadas.empty()){
		ofSort(etiquetasDetectadas, &compararPorConfidence);
		
		ww = margen*18;
		xx = xx + margen*2;
		
		int ancho_barra = hh/32;
		
		int cant_et = hh/ancho_barra - 2;
		
		int alto_refe = (hh - cant_et * (ancho_barra+1.5)) / cant_et;
		
		
		for(int i = 0; i < etiquetasDetectadas.size() && i < cant_et; i++){
			// barra grafico
			ofSetColor((int)ofMap(etiquetasDetectadas[i].confidence, 0, 60, 255, 0), (int)ofMap(etiquetasDetectadas[i].confidence, 60, 100, 0, 255), (int)ofMap(etiquetasDetectadas[i].confidence, 30, 85, 255, 0));
			ofDrawRectangle(xx, yy+(i+1)*ancho_barra, ofMap(etiquetasDetectadas[i].confidence, 0, 100, 0, ww, true), ancho_barra);
			ofSetColor(255);
			texto.drawString(etiquetasDetectadas[i].name + " " + ofToString((int)etiquetasDetectadas[i].confidence) + "%",xx+margen, yy + (i+1)*ancho_barra + ancho_barra*0.77);
			
			// referencias
			//ofDrawRectangle(				xx, 		yy + ancho_barra*(cant_et+1) + alto_refe*i, ancho_barra, ancho_barra);
			//ofSetColor(255);
			//texto.drawString(etiquetasDetectadas[i].name,	 xx*2.5, 	yy + ancho_barra*(cant_et+1.8) + alto_refe*i);
		}
	}
}

// --------------------------------------------------------------------------------------------------------
void ofApp::dibujarEtiquetas(int xx, int yy, int w, int h){
	int tam_etiqueta = (w-10-17*2)/18;

	ofColor color_actual;

	// Setteo el indice de etiqueta en 0 si loopeo el video
	if(getPosicionSegura() >= 1 || getPosicionSegura() <= 0){
		i_etiqueta = 0;
		pos_prox_etiqueta = etiquetasVideos[currentVideoIndex][i_etiqueta+1].posicion;
	}
	// Actualizo el indice de etiqueta si ya tendría que pasar al siguiente
	else if(getPosicionSegura() >= pos_prox_etiqueta && i_etiqueta < (int)etiquetasVideos[currentVideoIndex].size()){
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
		if((int)displayEtiquetas.size()*20 + tam_etiqueta*52 + offsetVideoPosY >= ofGetHeight()-offsetVideoPosY){
			displayEtiquetas.erase( displayEtiquetas.begin() );
		}
	}
	ofSetColor(255);
	for(int de=0; de < (int)displayEtiquetas.size(); de++){
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

    if(distortionAmount > 0.25f && ofRandomuf() < distortionAmount*0.05f) {
	// Saltos de frames
	float salto = getPosicionSegura() + ofRandom(-0.2, 0.2) * distortionAmount;

	if (salto < 0.0f || salto > 1.0f || isnan(salto)) {
	    salto = ofClamp(salto, 0.0f, 1.0f);
	}

	videoPlayer.setPosition(salto);

	
    } 
    videoTexture.draw(0, 0, videoPlayer.getWidth(), videoPlayer.getHeight());

    if(distortionAmount > 0.1f && oscuridad < 200) {

        if(ofRandomuf() < distortionAmount*2 || distortionAmount > 0.7f) {
		
		if(ofGetFrameNum() % (int)ofRandom(20*distortionAmount) == 0){
			glitchFbo.begin();
			ofClear(0, 0, 0, 0);
			
			glitchTexture = videoPlayer.getTexture();
			int sizeX = 0;
			int sizeY = 0;
			int x = 0;
			int alto_linea = videoPlayer.getHeight() * distortionAmount;
			if(ofRandom(1) > 0.98*distortionAmount){
				inicio_linea_corrupta = ofRandom(videoPlayer.getHeight() - alto_linea - ofRandom(0, videoPlayer.getHeight() - alto_linea));
			}
			int y = ofRandom(inicio_linea_corrupta/5);
			
			// ELECCION DE COLOR
			int ran_color = ofRandom(3);
			ofColor color_corrupcion;
			
			if(distortionAmount > 0.1){
				if(ran_color <= 1){
					color_corrupcion = ofColor(
						ofRandom(245, 255), 
						ofRandom(0, 10), 
						ofRandom(245, 255),
						ofMap(distortionAmount, 0, 1, 200, 255)
					    );
				}
				else if(ran_color <= 2){
					color_corrupcion = ofColor(
						ofRandom(0, 10), 
						ofRandom(245, 255), 
						ofRandom(245, 255),
						ofMap(distortionAmount, 0, 1, 200, 255)
					);
				}
				else if(ran_color <= 3){
					color_corrupcion = ofColor(
						ofRandom(245, 255), 
						ofRandom(245, 255), 
						ofRandom(0, 10),
						ofMap(distortionAmount, 0, 1, 200, 255)
					    );
				}
			}
			    
			    
			    // Declara ofPixels
			    ofPixels texturePixels;
			    texturePixels.allocate(glitchTexture.getWidth(), glitchTexture.getHeight(), OF_PIXELS_RGBA);
			    glitchTexture.readToPixels(texturePixels);
				
			for(int i = 0; y < videoPlayer.getHeight() - 10; i++) {
				 
				// FUERA DE LINEA CORRUPTA
				if(y < inicio_linea_corrupta || y > inicio_linea_corrupta+alto_linea){
					// TAMAÑO DE BLOQUE
					sizeY = 2 * (int)ofRandom(2, 10);
					sizeX = sizeY * (int)ofRandom(3);
					
					if( x >= videoPlayer.getWidth() - sizeX ){
						if(videoPlayer.getWidth()/(99*distortionAmount) > x && i % (sizeX*20) < 5){
							y = y + ofRandom(sizeY, videoPlayer.getHeight()/(99*distortionAmount) - y);
						}
						else{
							y = y + ofRandom(sizeY, videoPlayer.getHeight()-y);
						}
						x = 0;
					}
					
					else if(videoPlayer.getWidth()/(99*distortionAmount) > x && i % 20 < ofRandom(15)){
						x = x + ofRandom(sizeX, videoPlayer.getWidth()/(99*distortionAmount)-x);
					}
					else{
						x = x + ofRandom(sizeX, videoPlayer.getWidth()-x);
					}
				}
				// DENTRO DE LINEA CORRUPTA
				else{
					// PRIMERAS LINEAS CORRUPTAS
					if(y < inicio_linea_corrupta + alto_linea/4 && ofRandom(inicio_linea_corrupta + alto_linea/2) > y){
						if( x >= videoPlayer.getWidth() - sizeX){
							y = ofClamp(y + sizeY, 0, videoPlayer.getHeight());
							x = 0;
						}
						else{
							x = ofClamp(x + sizeX * (i % (sizeX * 3)), 0, videoPlayer.getWidth());
						}
					}
					// SIGUIENTES LINEAS
					else{
						
						if( x >= videoPlayer.getWidth() - sizeX){
							y = ofClamp(y + sizeY, 0, videoPlayer.getHeight());
							x = 0;
						}
						else{
							x = ofClamp(x + sizeX, 0, videoPlayer.getWidth());
						}
					}
					
					// TAMAÑO DE BLOQUE
					if(distortionAmount >= 0.9 && ofRandom(1) < 0.25){
						sizeY = 4 * (3 + distortionAmount*6);
						sizeX = sizeY * (int)ofRandom(3);
					}
					else{
						sizeY = 2 * (3 + distortionAmount*6);
						sizeX = sizeY * (int)ofRandom(2);
					}
				}

				// CALCULO COLOR DE PIXEL
				ofColor pixelColor = texturePixels.getColor(x, y);
				
				float  ran = ofRandom(1);
				
			// BLOQUES NEGROS - DISTORSION MUY GRANDE
				if(distortionAmount >= 0.85 && sizeX > 20){
					ofSetColor(0, 0, 0, 255);
							
					ofDrawRectangle(
						x, y,
						sizeX, sizeY
					);
				}
			// CORRUPCION MAS FUERTE - COLOR
				else if(((distortionAmount >= 0.8 && ran < 0.8)) || (pixelColor.getBrightness() > 50 && ofRandom(1.5) < distortionAmount)){
					
					if(pixelColor.getBrightness() > ofRandom(10, 100*distortionAmount)){
						
						// DESPLAZAMIENTO DE BLOQUES + COLOREADO (PARA PIXELES NO TAN CLAROS)
						if(pixelColor.getBrightness() < 200 || x % (int)ofRandom(10, 30) < 10){
							int dx = x + ofRandom(-5, 5);
							int dy = y + ofRandom(-5, 5);
							
							
							ofSetColor(color_corrupcion);
								
							glitchTexture.drawSubsection(
								x+sizeX*(int)ofRandom(-1*distortionAmount, 1*distortionAmount), y,
								sizeX, sizeY,
								dx, dy,
								sizeX, sizeY
							);
							
						}
						else{
							if(ofRandom(0.9) > distortionAmount){
								// SI EL PIXEL NO ES CLARO PINTO TODO EL BLOQUE DEL COLOR CORRUPTO
								if(pixelColor.getBrightness() < 200 ){
									ofSetColor(color_corrupcion.r, color_corrupcion.g, color_corrupcion.b, 255);
								}
								// SI EL PIXEL ES CLARO LE SUBO MUCHO EL BRILLO
								else if(pixelColor.getBrightness() > 200){
									ofColor color_corrupcion_brillo = color_corrupcion;
									//color_corrupcion_brillo.setBrightness(255);
									color_corrupcion_brillo.setHsb(color_corrupcion_brillo.getHue(), 255, 252);
									ofSetColor(color_corrupcion_brillo);
								}
							}
							// BLOQUE NEGRO	
							else{
								ofSetColor(0, 0, 0, 255);
							}
							
							ofDrawRectangle(
								x, y,
								sizeX, sizeY
							);
						}
					}
				}
				
				// PIXELADO
				else if(distortionAmount >= 0.1 && (x*5) % (int)ofRandom(1, 5*distortionAmount) < 1){
					
					/*if(distortionAmount >= 0.3 && ((i*15) % sizeX*25) < 1){
						ofSetColor(color_corrupcion);
					}
					else{
						ofSetColor(pixelColor);
					}*/
					ofSetColor(pixelColor);
					ofDrawRectangle(
						x+ofRandom(2), y+ofRandom(2),
						sizeX, sizeY
					);
				}
			}
			// BLOQUES NEGROS
			if(ofRandom(1) > 0.7){
				sizeX = ofRandom(8, 15)*distortionAmount;
				sizeY = ofRandom(8, 15)*distortionAmount;
					
				ofSetColor(0, 0, 0, 255);
				ofDrawRectangle(
					x, y,
					sizeX, sizeY
				);
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
void ofApp::dibujarBarraProgreso(int xx, int yy, int ww, float porcentaje){
	int m = ww/22;
	ww = m*18;
	xx = xx + m*2;
	
	ofSetColor(10);
	ofDrawRectangle(xx, yy, ww, 20);
	
	int ancho_progreso = ofMap(porcentaje, 0, 100, 0, ww, true);
	ofSetColor(10, 255, 100);
	ofDrawRectangle(xx, yy, ancho_progreso, 20);
}

//--------------------------------------------------------------
void ofApp::dibujarBarraRecorrido(){
	int ancho_barra = ofGetWidth()-offsetVideoPosX;
	int alto_barra = ofGetHeight()-videoPlayer.getHeight()*resizeVideo;
	int xSensor = ofMap(sensor, 0, tam_dial, offsetVideoPosX, ancho_barra); // valor traducido en x del sensor
	float div = ((float)ancho_barra / (float)(cantVideos - 1));
		
	// fondo barra inferior
	ofSetColor(0);
	ofDrawRectangle(offsetVideoPosX, ofGetHeight()-alto_barra, ofGetWidth(), alto_barra);

	for (int i = 0; i < cantVideos; i++) {
		
		int xSints = offsetVideoPosX + (i * div);
			
		// marcas de sintonizacion
		if( abs(xSensor - xSints) < div*0.25 ){
			// verde de sintonizacion
			ofFill(); ofSetColor(5, 255, 100);
			ofDrawRectangle(xSints-div*0.25, ofGetHeight()-alto_barra, div*0.5, alto_barra);
				
			texto.drawString("0x"+ ofToString(currentVideoIndex) + "ae" + ofToString(distSintVideo) + "c" + ofToString((int) ofMap(distortionAmount, 0, 1, 100, 999)) , xSints-5, ofGetHeight()-alto_barra-10);
		}
		else{
			// puntos no sintonizados
			ofFill(); ofSetColor(180);
			ofDrawRectangle(xSints-div*0.15, ofGetHeight()-alto_barra, div*0.3, alto_barra);
		}
	}
	// barra valor sensor
	ofFill(); ofSetColor(255, 0, 0, (int)ofMap(oscuridad, 0, 240, 255, 5));
	ofDrawRectangle(xSensor, ofGetHeight()-alto_barra, 3, alto_barra);
		
		
}


bool ofApp::compararPorConfidence(RectEtiqueta &a, RectEtiqueta &b) {
    return a.confidence > b.confidence;
}

//--------------------------------------------------------------
float ofApp::getPosicionSegura(){
	if(videoPlayer.getPosition() > 0 && videoPlayer.getPosition() < 1){
		return videoPlayer.getPosition();
	} 
	else{
		return ofMap(videoPlayer.getPosition(), -1.0f, -9.0f, 0.0f, 1.0f, true);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){

}

void ofApp::exit() {
	videoPlayer.stop();
	videoPlayer.close();
	
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
