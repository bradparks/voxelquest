// f00380_gameworld.h
//

#include "f00380_gameworld.e"
#define LZZ_INLINE inline
GameWorld::GameWorld ()
        {

		// Notes
		// -----
		// Depth: 16 bits
		// Material: 16 bits
		// AO val: 8 bits, normal: 24 bits

	}
bool GameWorld::checkBounds (int i, int j, int k)
                                              {
		
		bool res = true;

		if (i < 0) {res = false;}
		if (j < 0) {res = false;}
		if (k < 0) {res = false;}
		if (i >= iDim.x) {res = false;}
		if (j >= iDim.y) {res = false;}
		if (k >= iDim.z) {res = false;}

		return res;
	}
void GameWorld::resetToState (E_STATES resState)
                                             {
		int i;
		for (i = 0; i < iVolumeSize; i++) {
			if (worldData[i]) {
				if( worldData[i]->curState > resState) {

					if (worldData[i]->fillState == E_FILL_STATE_PARTIAL) {
						worldData[i]->curState = resState;
					}

					
				}
			}
		}
	}
void GameWorld::init (iVector3 _iDim, Singleton * _singleton, int _renderMethod)
                                                                            {

		pageCount = 0;
		iRSize = 128;


		#ifdef DEBUG_MODE
		pushTrace("GameWorld init");
		#endif

		int i;
		int j;

		doDrawFBO = false;

		for (j = 0; j < E_RENDER_LENGTH; j++) {
			for (i = 0; i < E_STATE_LENGTH; i++) {
				diagrams[j][i] = E_STATE_WAIT;
			}
		}

		renderMethod = _renderMethod;
		singleton = _singleton;
		iDim = _iDim;

	    diagrams[E_RENDER_VOL][E_STATE_INIT_END] = E_STATE_CREATESIMPLEXNOISE_LAUNCH;
		diagrams[E_RENDER_VOL][E_STATE_CREATESIMPLEXNOISE_END] = E_STATE_COPYTOTEXTURE_LAUNCH;
	    diagrams[E_RENDER_VOL][E_STATE_COPYTOTEXTURE_END] = E_STATE_GENERATEVOLUME_LAUNCH;
	    diagrams[E_RENDER_VOL][E_STATE_GENERATEVOLUME_END] = E_STATE_LENGTH;

		curDiagram = diagrams[renderMethod];

		iPageSize = 4;
		loadRad = 2;
		loadRadZ = 2;
		
		renderRad = 12;

		iVolumeSize = iDim.x*iDim.y*iDim.z;
		
	    worldData = new GamePage*[iVolumeSize];
	    
	    
		

	    singleton->fCameraPos.x = loadRad*iRSize;
	    singleton->fCameraPos.y = loadRad*iRSize;
	    singleton->fCameraPos.z = loadRadZ*iRSize;
	    singleton->iCameraPos.x = loadRad*iRSize;
	    singleton->iCameraPos.y = loadRad*iRSize;
	    singleton->iCameraPos.z = loadRadZ*iRSize;
	    

		
		for (i = 0; i < iVolumeSize; i++) {
			worldData[i] = NULL;
		}


		origin.x = 0; origin.y = 0; origin.z = 0;

	    
	    #ifdef DEBUG_MODE
	    popTrace();
	    #endif
	}
void GameWorld::update (bool changesMade, bool bufferInvalid, int maxH)
                                                                    {


		bool procResult = processPages();
		if (procResult || changesMade) {
			renderPages(maxH);
			
			if ( !(singleton->animateGrass) ) {
				renderGrass();
				renderGeom();
				combineBuffers();
			}

			
		}

		if (singleton->animateGrass) {
			renderGrass();
			renderGeom();
			combineBuffers();
			bufferInvalid = true;
		}


		if (procResult || changesMade || bufferInvalid) {

			glClearColor(0.6,0.6,0.7,0.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			postProcess();
			
			glutSwapBuffers();
			glFlush();
			
		}


		
		
		
	}
bool GameWorld::processPages ()
                            {


		

		int i, j, k;
		int res;
		int ind;

		int iw = iDim.x;
	    int jw = iDim.y;
	    int kw = iDim.z;

	    int ii;
	    int jj;
	    int kk;

	    bool cmade = false;


	    iVector3 curPos;
	    iVector3 camPagePos;

	    camPagePos.x = singleton->iCameraPos.x/iRSize;
	    camPagePos.y = singleton->iCameraPos.y/iRSize;
	    camPagePos.z = singleton->iCameraPos.z/iRSize;

		minWithinLR.x = camPagePos.x - loadRad;
		minWithinLR.y = camPagePos.y - loadRad;
		minWithinLR.z = camPagePos.z - loadRadZ;

		maxWithinLR.x = camPagePos.x + loadRad;
		maxWithinLR.y = camPagePos.y + loadRad;
		maxWithinLR.z = camPagePos.z + loadRadZ;

	    int curInd = 0;
		

	    int m;
	    E_STATES nState;


	    int changeCount = 0;
	    int maxChanges = 32;

	    int extraRad = singleton->extraRad;

	    if (singleton->lbDown) {
	    	extraRad = 0;
	    	maxChanges = 1;
	    }

	    if ( singleton->rbDown) {

	    }
	    

		for (k = -(loadRadZ+extraRad); k <= (loadRadZ+extraRad); k++) {
			for (j = -(loadRad+extraRad); j <= (loadRad+extraRad); j++) {
				for (i = -(loadRad+extraRad); i <= (loadRad+extraRad); i++) {

					
					/*
					if (threadpool.allocated() >= 16) {
						threadpool.joinAll();
					}
					*/

					ii = i+camPagePos.x;
					jj = j+camPagePos.y;
					kk = k+camPagePos.z;


					if ( checkBounds(ii,jj,kk) ) {



						ind = kk*jw*iw + jj*iw + ii;

						curPos.x = ii*iPageSize;
						curPos.y = jj*iPageSize;
						curPos.z = kk*iPageSize;

						


						if (worldData[ind] == NULL) {

							#ifdef DEBUG_MODE
							doTrace("E_STATE_INIT_LAUNCH");
							#endif

							worldData[ind] = new GamePage();
							worldData[ind]->init(singleton, iPageSize, curPos, iRSize*2);

							pageCount++;
							changeCount++;


						}
						else {

							nState = (E_STATES)curDiagram[worldData[ind]->curState];

							switch(nState) {
	                            case E_STATE_CREATESIMPLEXNOISE_LAUNCH:

	                            	#ifdef DEBUG_MODE
	                            	doTrace("E_STATE_CREATESIMPLEXNOISE_LAUNCH");
	                            	#endif

	                            	worldData[ind]->nextState = nState;

	                            	if ( threadpool.available() > 1 ) {

	                            		try {
	                            			threadpool.start(*worldData[ind]);
	                            		}
	                            		catch (SystemException & exc) {
	                            			doTrace("MEM EXCEPTION");
	                            		}

	                            		

	                            		changeCount++;
	                            	}
	                            	else {
	                            		return cmade;
	                            	}
	                            	

	                            	//worldData[ind]->run();
									
									
								break;
								case E_STATE_COPYTOTEXTURE_LAUNCH:

									#ifdef DEBUG_MODE
									doTrace("E_STATE_COPYTOTEXTURE_LAUNCH");
									#endif

									
									worldData[ind]->nextState = nState;
									worldData[ind]->copyToTexture();
									cmade = true;
									
									changeCount++;

									//return cmade;
								break;


								case E_STATE_GENERATEVOLUME_LAUNCH:

									#ifdef DEBUG_MODE
									doTrace("E_STATE_GENERATE_VOLUME_LAUNCH");
									#endif

									
									worldData[ind]->nextState = nState;
									worldData[ind]->generateVolume();
									cmade = true;

									changeCount++;
									
									//return cmade;
								break;

								
								case E_STATE_WAIT:

									//do nothing
								break;
								case E_STATE_LENGTH:
	                            	
								break;
								default:
	                                
								break;

							}

							if (changeCount >= maxChanges) {
								return cmade;
							}
							
						}

					}

					curInd++;
				}
			}
		}



		
		return cmade;
	}
void GameWorld::renderPages (int maxH)
                                   {

		int i, j, k;
		int res;
		int ind;

		int iw = iDim.x;
	    int jw = iDim.y;
	    int kw = iDim.z;

	    int ii;
	    int jj;
	    int kk;

	    iVector3 camPagePos;

	    camPagePos.x = singleton->iCameraPos.x/iRSize;
	    camPagePos.y = singleton->iCameraPos.y/iRSize;
	    camPagePos.z = singleton->iCameraPos.z/iRSize;

	    int curInd = 0;
		

	    int m;


	    singleton->bindShader("BlitShader");
	    singleton->bindFBO("pagesFBO");

	    

	    maxH = std::max(std::min(renderRad,maxH), -renderRad);

		for (k = -renderRad; k <= renderRad; k++) {
			for (j = -renderRad; j <= renderRad; j++) {
				for (i = -renderRad; i <= renderRad; i++) {

					ii = i+camPagePos.x;
					jj = j+camPagePos.y;
					kk = k+camPagePos.z;

					if ( checkBounds(ii,jj,kk) ) {



						ind = kk*jw*iw + jj*iw + ii;

						if (worldData[ind] == NULL) {

						}
						else {

							if (worldData[ind]->fillState = E_FILL_STATE_PARTIAL) {
								switch(curDiagram[worldData[ind]->curState]) {
									case E_STATE_LENGTH:
										drawPage(worldData[ind], ii, jj, kk);
									break;
								}
							}

							
							
						}

					}

					curInd++;
				}
			}
		}

		singleton->unbindShader();
		singleton->unbindFBO();
	    

		//doTrace( "POSSIBLE ERROR: " , i__s(glGetError()) , "\n" );

	}
void GameWorld::drawPage (GamePage * gp, int dx, int dy, int dz)
                                                            {

		int pitchSrc = gp->iRenderSize;
		int pitchSrc2 = gp->iRenderSize/2;

		int dxmod = dx*pitchSrc2 - singleton->iCameraPos.x;
		int dymod = dy*pitchSrc2 - singleton->iCameraPos.y;
		int dzmod = dz*pitchSrc2 - singleton->iCameraPos.z;


		int x1 = (dxmod-dymod) - pitchSrc2;
		int y1 = (-(dxmod/2) + -(dymod/2) + dzmod) - pitchSrc2;
		int x2 = x1 + pitchSrc;
		int y2 = y1 + pitchSrc;

		float fx1 = x1;
		float fy1 = y1;
		float fx2 = x2;
		float fy2 = y2;

		fVector2 fScreenDim;
		fScreenDim.x = (float)singleton->baseW;
		fScreenDim.y = (float)singleton->baseH;

		float myZoom = std::min(1.0f,singleton->cameraZoom);


		fx1 = fx1*2.0f*(myZoom)/fScreenDim.x;
		fy1 = fy1*2.0f*(myZoom)/fScreenDim.y;
		fx2 = fx2*2.0f*(myZoom)/fScreenDim.x;
		fy2 = fy2*2.0f*(myZoom)/fScreenDim.y;



	    singleton->sampleFBODirect(gp->fboSet);


	    glColor4f(1, 1, 1, 1);
	    glBegin(GL_QUADS);
	    glNormal3f(0, 0, 1);
	    
	    
	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(fx1,fy1,0.0f);
	    
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f(fx2,fy1,0.0f);
	    
	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f(fx2,fy2,0.0f);
	    
	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(fx1,fy2,0.0f);
	    
	    glEnd();

	    singleton->unsampleFBODirect(gp->fboSet);



	}
void GameWorld::combineBuffers ()
                              {

		singleton->bindShader("CombineShader");

		singleton->bindFBO("combineFBO");
		singleton->sampleFBO("pagesFBO",0);
		singleton->sampleFBO("grassFBO",2);
		singleton->sampleFBO("geomFBO",4);


		singleton->setShaderFloat("cameraZoom", singleton->cameraZoom);
		
		singleton->drawFSQuad(1.0f);


		singleton->unsampleFBO("pagesFBO",0);
		singleton->unsampleFBO("grassFBO",2);
		singleton->unsampleFBO("geomFBO",4);
		singleton->unbindFBO();
		singleton->unbindShader();
	}
void GameWorld::renderGeom ()
                          {


		
		//glEnable(GL_DEPTH_TEST);

		singleton->bindShader("GeomShader");
		
		singleton->setShaderFloat("curTime", singleton->curTime);
		singleton->setShaderFloat("cameraZoom", singleton->cameraZoom);
		singleton->setShaderfVec3("cameraPos", &(singleton->fCameraPos));
		singleton->setShaderFloat("bufferWidth",singleton->fBufferWidth);
		
		singleton->bindFBO("geomFBO");
		//singleton->sampleFBO("pagesFBO");


		glEnable(GL_DEPTH_TEST);
		singleton->drawCubeCentered(singleton->fLightPos,64.0);
		glDisable(GL_DEPTH_TEST);

		

		//singleton->unsampleFBO("pagesFBO");
		singleton->unbindFBO();
		singleton->unbindShader();

		//glDisable(GL_DEPTH_TEST);

		
	}
void GameWorld::renderGrass ()
                           {


		
		//glEnable(GL_DEPTH_TEST);

		singleton->bindShader("GrassShader");
		
		singleton->setShaderFloat("curTime", singleton->curTime);
		singleton->setShaderFloat("cameraZoom", singleton->cameraZoom);
		singleton->setShaderfVec3("cameraPos", &(singleton->fCameraPos));
		
		singleton->bindFBO("grassFBO");
		singleton->sampleFBO("pagesFBO");

		if (singleton->grassOn) {
			glCallList(singleton->grassTris);
		}

		

		singleton->unsampleFBO("pagesFBO");
		singleton->unbindFBO();
		singleton->unbindShader();

		//glDisable(GL_DEPTH_TEST);

		
	}
void GameWorld::postProcess ()
                           {
		


		int dxmod = singleton->iLightPos.x - singleton->iCameraPos.x;
		int dymod = singleton->iLightPos.y - singleton->iCameraPos.y;
		int dzmod = singleton->iLightPos.z - singleton->iCameraPos.z;

		int x = (dxmod-dymod);
		int y = (-(dxmod/2) + -(dymod/2) + dzmod);
		int z = singleton->iLightPos.z;

		

		float newZoom = std::max(1.0f,singleton->cameraZoom);


		float fx = ((float)x)*2.0f*(newZoom)/(singleton->fBufferWidth);
		float fy = ((float)y)*2.0f*(newZoom)/(singleton->fBufferWidth);
		float fz = ((float)z);


		singleton->bindShader("LightingShader");
		singleton->setShaderVec2("mouseCoords",singleton->mouseX,singleton->mouseY);
		singleton->setShaderfVec3("cameraPos", &(singleton->fCameraPos));
		singleton->setShaderfVec3("lightPosWS", &(singleton->fLightPos));

		singleton->setShaderVec3("lightPosSS", fx, fy, fz);
		singleton->setShaderFloat("cameraZoom",newZoom);
		singleton->setShaderFloat("bufferWidth",singleton->fBufferWidth);


		
		singleton->bindFBO("resultFBO");
		singleton->sampleFBO("combineFBO");

		//MUST BE CALLED AFTER FBO IS BOUND
		singleton->setShaderVec2("resolution",singleton->currentFBOResolutionX, singleton->currentFBOResolutionY);

		singleton->drawFSQuad(1.0f);
		singleton->unsampleFBO("combineFBO");
		singleton->unbindFBO();
		singleton->unbindShader();

		singleton->drawFBO("resultFBO", 0, newZoom );
		
	}
GameWorld::~ GameWorld ()
                     {


		threadpool.stopAll();

		int i;

		for (i = 0; i < iVolumeSize; i++) {
			if (worldData[i]) {
				delete worldData[i];
			}
			
		}
		
	}
#undef LZZ_INLINE
 