/************************************************************************
	 File:        TrainView.cpp

	 Author:
				  Michael Gleicher, gleicher@cs.wisc.edu

	 Modifier
				  Yu-Chi Lai, yu-chi@cs.wisc.edu

	 Comment:
						The TrainView is the window that actually shows the
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within
						a TrainWindow
						that is the outer window with all the widgets.
						The TrainView needs
						to be aware of the window - since it might need to
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know
						about it (beware circular references)

	 Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include<assimp/anim.h>
#include "GL/glu.h"

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"


#ifdef EXAMPLE_SOLUTION
#	include "TrainExample/TrainExample.H"
#endif


//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
TrainView::
TrainView(int x, int y, int w, int h, const char* l)
	: Fl_Gl_Window(x, y, w, h, l)
	//========================================================================
{
	mode(FL_RGB | FL_ALPHA | FL_DOUBLE | FL_STENCIL);

	resetArcball();
}

//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
int TrainView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value())
	{
		if (arcball.handle(event))
			return 1;
	}
	/*
	else if (tw->trainCam->value())
	{
		if (event)
		{
			if (Fl::event_x() >= 0 && Fl::event_y() >= 0 && event == FL_MOVE)
			{
				float or_x = this->tw->x() + this->x() + this->w() / 2;
				float or_y = this->tw->y() + this->y() + this->h() / 2;
				float dx = Fl::event_x() - this->w() / 2, dy = Fl::event_y() - this->h() / 2;

				glm::vec3 orientation = this->character.forward;

				dx *= 0.1;
				dy *= 0.1;
				orientation = glm::rotate(orientation, glm::radians(-dx), glm::normalize(this->character.up));
				orientation = glm::rotate(orientation, glm::radians(-dy), glm::normalize(this->character.right));

				this->character.forward = glm::normalize(orientation);
				this->character.right = glm::normalize(glm::cross(this->character.forward, this->character.up));

				SetCursorPos(or_x, or_y);
			}


			float speed = 0.3;
			//W
			if (GetAsyncKeyState(0x57))
				this->character.position += this->character.forward * speed;
			//A
			if (GetAsyncKeyState(0x41))
				this->character.position -= this->character.right * speed;
			//S
			if (GetAsyncKeyState(0x53))
				this->character.position -= this->character.forward * speed;
			//D
			if (GetAsyncKeyState(0x44))
				this->character.position += this->character.right * speed;
			//Space
			if (GetAsyncKeyState(VK_SPACE))
				this->character.position += this->character.up;
			//Q
			if (GetAsyncKeyState(0x51))
				this->character.position -= this->character.up;
		}
	}
	*/
	// remember what button was used
	static int last_push;

	switch (event) {
		// Mouse button being pushed event
	case FL_PUSH:
		last_push = Fl::event_button();
		// if the left button be pushed is left mouse button
		if (last_push == FL_LEFT_MOUSE) {
			doPick();
			damage(1);

			return 1;
		};
		break;

		// Mouse button release event
	case FL_RELEASE: // button release
		damage(1);
		last_push = 0;
		return 1;

		// Mouse button drag event
	case FL_DRAG:

		// Compute the new control point position
		if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
			ControlPoint* cp = &m_pTrack->points[selectedCube];

			double r1x, r1y, r1z, r2x, r2y, r2z;
			getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

			double rx, ry, rz;
			mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z,
				static_cast<double>(cp->pos.x),
				static_cast<double>(cp->pos.y),
				static_cast<double>(cp->pos.z),
				rx, ry, rz,
				(Fl::event_state() & FL_CTRL) != 0);

			cp->pos.x = (float)rx;
			cp->pos.y = (float)ry;
			cp->pos.z = (float)rz;

			change = true;
			damage(1);
		}
		break;

		// in order to get keyboard events, we need to accept focus
	case FL_FOCUS:
		return 1;

		// every time the mouse enters this window, aggressively take focus
	case FL_ENTER:
		focus(this);
		break;
	case FL_KEYBOARD:
		int k = Fl::event_key();
		int ks = Fl::event_state();
		if (k == 'p') {
			// Print out the selected control point information
			if (selectedCube >= 0)
				printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
					selectedCube,
					m_pTrack->points[selectedCube].pos.x,
					m_pTrack->points[selectedCube].pos.y,
					m_pTrack->points[selectedCube].pos.z,
					m_pTrack->points[selectedCube].orient.x,
					m_pTrack->points[selectedCube].orient.y,
					m_pTrack->points[selectedCube].orient.z);
			else
				printf("Nothing Selected\n");

			return 1;
		};
		break;
	}

	return Fl_Gl_Window::handle(event);
}

//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void TrainView::draw()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	if (gladLoadGL())
	{
		if (this->shader == nullptr)
			this->shader = new Shader(
				"../assets/shaders/modelLoading.vert",
				nullptr, nullptr, nullptr,
				"../assets/shaders/modelLoading.frag");
		if (this->shadowShader == nullptr)
			this->shadowShader = new Shader(
				"../assets/shaders/modelLoading.vert",
				nullptr, nullptr, nullptr,
				"../assets/shaders/shadowShader.frag");
		if (this->waterShader == nullptr)
			this->waterShader = new Shader(
				"../assets/shaders/waterShader.vert",
				nullptr, nullptr, nullptr,
				"../assets/shaders/waterShader.frag");
		// Load models
		if (this->parkModel == nullptr)
			this->parkModel = new Model((GLchar*)"../assets/model/park/Mineways2Skfb.obj");

		if (this->trainModel == nullptr)
			this->trainModel = new Model((GLchar*)"../assets/model/train/11709_train_v1_L3.obj");

		if (this->waterPlan == nullptr)
			this->waterPlan = new Model((GLchar*)"../assets/model/waterPlan/waterPlan.obj");

		if (this->light == nullptr)
			this->light = new Model((GLchar*)"../assets/model/light/Light Pole.obj");
	}
	else
		throw std::runtime_error("Could not initialize GLAD!");

	// Set up the view port
	glViewport(0, 0, w(), h());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0, 0, .3f, 0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	//Calculate train position
	calcuTrainPos(this);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (tw->topCam->value()) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	}
	else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	if (this->change)
	{
		genTable(this);
		this->change = false;
	}
	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[] = { 0,1,1,0 }; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[] = { m_pTrack->points[1].pos.x, m_pTrack->points[1].pos.y, m_pTrack->points[1].pos.z, 0 };
	GLfloat lightPosition3[] = { 0, -1, 0, 0 };
	GLfloat yellowLight[] = { 0.5f, 0.5f, .0f, 1.0 };
	GLfloat whiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0 };
	GLfloat blueLight[] = { .1f,.1f,.3f,1.0 };
	GLfloat grayLight[] = { .3f, .3f, .3f, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, whiteLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);

	float noAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float diffuse[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float position[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	position[0] = this->m_pTrack->points[0].pos.x;
	position[1] = this->m_pTrack->points[0].pos.y;
	position[2] = this->m_pTrack->points[0].pos.z;

	//properties of the light
	glLightfv(GL_LIGHT2, GL_AMBIENT, noAmbient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, yellowLight);
	glLightfv(GL_LIGHT2, GL_POSITION, position);

	/*Spot properties*/
	//spot direction
	float direction[] = { -this->m_pTrack->points[0].orient.x, -this->m_pTrack->points[0].orient.y, -this->m_pTrack->points[0].orient.z };
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, direction);
	//angle of the cone light emitted by the spot : value between 0 to 180
	float spotCutOff = 45;
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, spotCutOff);

	//exponent propertie defines the concentration of the light
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 15.0f);
	//light attenuation (default values used here : no attenuation with the distance)
	glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.0f);
	glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.0f);


	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	glUseProgram(0);

	setupFloor();
	//glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT1);
	drawFloor(200, 10);
	glEnable(GL_LIGHT1);
	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();

	drawStuff();
	// this time drawing is for shadows (except for top view)
	if (!tw->topCam->value()) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}
}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value())
		arcball.setProjection(false);
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		}
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90, 1, 0, 0);
	}
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, 1.0, 0.01, 200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		float trainModelHeadPos[3] =
		{
			trainModel->position[0] + 9 * trainModel->forward[0] + 5 * trainModel->up[0],
			trainModel->position[1] + 9 * trainModel->forward[1] + 5 * trainModel->up[1],
			trainModel->position[2] + 9 * trainModel->forward[2] + 5 * trainModel->up[2]
		};


		gluLookAt(trainModelHeadPos[0], trainModelHeadPos[1], trainModelHeadPos[2],
			trainModelHeadPos[0] + trainModel->forward[0], trainModelHeadPos[1] + trainModel->forward[1], trainModelHeadPos[2] + trainModel->forward[2],
			trainModel->up[0], trainModel->up[1], trainModel->up[2]);
#ifdef EXAMPLE_SOLUTION
		trainCamView(this, aspect);
#endif
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (!tw->trainCam->value()) {
		for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if (((int)i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}

	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################

	float noAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float whiteDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float position[] = { 1.0f, 1.0f, 1.0f,0.0f };

	drawTrack(this, doingShadows);
	drawTrain(this, doingShadows);
	drawLight(this, doingShadows);
	//drawPlan(this);
	if (doingShadows == false)
		drawWaterPlan(this);

#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train

	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();

	// where is the mouse?
	int mx = Fl::event_x();
	int my = Fl::event_y();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix((double)mx, (double)(viewport[3] - my),
		5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100, buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
		glLoadName((GLuint)(i + 1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3] - 1;
	}
	else // nothing hit, nothing selected
		selectedCube = -1;

	printf("Selected Cube %d\n", selectedCube);
}

Person::Person()
{
}

Person::~Person()
{

}

void genTable(TrainView* trainView)
{
	trainView->table.clear();
	float countDistant = 0;

	if (trainView->tw->splineBrowser->value() == LINEAR_TRACK)
	{
		for (size_t i = 0; i < trainView->m_pTrack->points.size(); ++i)
		{
			// pos
			Pnt3f cp_pos_p1 = trainView->m_pTrack->points[i].pos;
			Pnt3f cp_pos_p2 = trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].pos;
			float percent = 1.0f / trainView->DIVIDE_LINE;
			float t = 0;
			Pnt3f qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
			Pnt3f qt0, qt1 = qt;

			for (size_t j = 0; j < trainView->DIVIDE_LINE; j++)
			{
				trainView->table[countDistant] = t + i;
				qt0 = qt1;
				t += percent;
				qt1 = (1 - t) * cp_pos_p1 + t * cp_pos_p2;

				countDistant += Pnt3f::distant(qt0, qt1);
			}
		}
	}
	else if (trainView->tw->splineBrowser->value() == B_SPLINE)
	{
		for (size_t i = 0; i < trainView->m_pTrack->points.size(); ++i)
		{
			// pos
			Pnt3f cp_pos[4] =
			{
				trainView->m_pTrack->points[(i + 0) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 2) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 3) % trainView->m_pTrack->points.size()].pos,
			};

			float cp_pos_arr[4][3]
			{
				{cp_pos[0].x,cp_pos[0].y,cp_pos[0].z},
				{cp_pos[1].x,cp_pos[1].y,cp_pos[1].z},
				{cp_pos[2].x,cp_pos[2].y,cp_pos[2].z},
				{cp_pos[3].x,cp_pos[3].y,cp_pos[3].z},
			};
			float getResu[3];
			float percent = 1.0f / trainView->DIVIDE_LINE;
			float t = 0;

			caulateGMT(cp_pos_arr, t, getResu, B_SPLINE);

			Pnt3f qt(getResu[0], getResu[1], getResu[2]);
			Pnt3f qt0, qt1 = qt;

			for (size_t j = 0; j < trainView->DIVIDE_LINE; j++)
			{
				trainView->table[countDistant] = t + i;
				qt0 = qt1;
				t += percent;
				caulateGMT(cp_pos_arr, t, getResu, B_SPLINE);
				qt1.x = getResu[0];
				qt1.y = getResu[1];
				qt1.z = getResu[2];

				countDistant += Pnt3f::distant(qt0, qt1);
			}
		}
	}
	else if (trainView->tw->splineBrowser->value() == CARDINAL)
	{
		for (size_t i = 0; i < trainView->m_pTrack->points.size(); ++i)
		{
			// pos
			Pnt3f cp_pos[4] =
			{
				trainView->m_pTrack->points[(i + 0) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 2) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 3) % trainView->m_pTrack->points.size()].pos,
			};

			float cp_pos_arr[4][3]
			{
				{cp_pos[0].x,cp_pos[0].y,cp_pos[0].z},
				{cp_pos[1].x,cp_pos[1].y,cp_pos[1].z},
				{cp_pos[2].x,cp_pos[2].y,cp_pos[2].z},
				{cp_pos[3].x,cp_pos[3].y,cp_pos[3].z},
			};
			float getResu[3];
			float percent = 1.0f / trainView->DIVIDE_LINE;
			float t = 0;

			caulateGMT(cp_pos_arr, t, getResu, trainView);

			Pnt3f qt(getResu[0], getResu[1], getResu[2]);
			Pnt3f qt0, qt1 = qt;

			for (size_t j = 0; j < trainView->DIVIDE_LINE; j++)
			{
				trainView->table[countDistant] = t + i;
				qt0 = qt1;
				t += percent;
				caulateGMT(cp_pos_arr, t, getResu, trainView);
				qt1.x = getResu[0];
				qt1.y = getResu[1];
				qt1.z = getResu[2];

				countDistant += Pnt3f::distant(qt0, qt1);
			}
		}
	}

	trainView->totalDistant = countDistant;
}
void caulateGMT(float(*Gs)[3], float time, float* result, int type)
{

	float Bmat[][4] =
	{
		{-1.0 / 6, 3.0 / 6,-3.0 / 6,1.0 / 6},
		{ 3.0 / 6,-6.0 / 6, 0.0 / 6,4.0 / 6},
		{-3.0 / 6, 3.0 / 6, 3.0 / 6,1.0 / 6},
		{ 1.0 / 6, 0.0 / 6, 0.0 / 6,0.0 / 6}
	};
	float Cmat[][4] =
	{
		{-1.0 / 2, 2.0 / 2,-1.0 / 2,0.0 / 2},
		{ 3.0 / 2,-5.0 / 2, 0.0 / 2,2.0 / 2},
		{-3.0 / 2, 4.0 / 2, 1.0 / 2,0.0 / 2},
		{ 1.0 / 2,-1.0 / 2, 0.0 / 2,0.0 / 2}
	};
	float resuVec[4] = { 0 };
	float Tvec[] = { pow(time,3),pow(time,2),time,1 };
	Pnt3f resultPnt3f, GsPnt3f[4];

	for (int i = 0; i < 4; i++)
	{
		GsPnt3f[i].x = Gs[i][0];
		GsPnt3f[i].y = Gs[i][1];
		GsPnt3f[i].z = Gs[i][2];
	}

	if (type == B_SPLINE)
		for (int i = 0; i < 4; i++)
		{
			float sum = 0;
			sum = Bmat[i][0] * Tvec[0]
				+ Bmat[i][1] * Tvec[1]
				+ Bmat[i][2] * Tvec[2]
				+ Bmat[i][3] * Tvec[3];

			resuVec[i] = sum;
		}
	else if (type == CARDINAL)
		for (int i = 0; i < 4; i++)
		{
			float sum = 0;
			sum = Cmat[i][0] * Tvec[0]
				+ Cmat[i][1] * Tvec[1]
				+ Cmat[i][2] * Tvec[2]
				+ Cmat[i][3] * Tvec[3];

			resuVec[i] = sum;
		}

	resultPnt3f = GsPnt3f[0] * resuVec[0]
		+ GsPnt3f[1] * resuVec[1]
		+ GsPnt3f[2] * resuVec[2]
		+ GsPnt3f[3] * resuVec[3];

	result[0] = resultPnt3f.x;
	result[1] = resultPnt3f.y;
	result[2] = resultPnt3f.z;
}
void caulateGMT(float(*Gs)[3], float time, float* result, TrainView* trainView)
{
	float scale = trainView->tw->scale->value();
	float Cmat[][4] =
	{
		{	 -scale,    2 * scale,-scale, 0},
		{ 2 - scale,    scale - 3,	   0, 1},
		{scale - 2,3 - 2 * scale, scale, 0},
		{	  scale,	   -scale,	   0, 0}
	};
	float resuVec[4] = { 0 };
	float Tvec[] = { pow(time,3),pow(time,2),time,1 };
	Pnt3f resultPnt3f, GsPnt3f[4];

	for (int i = 0; i < 4; i++)
	{
		GsPnt3f[i].x = Gs[i][0];
		GsPnt3f[i].y = Gs[i][1];
		GsPnt3f[i].z = Gs[i][2];
	}

	for (int i = 0; i < 4; i++)
	{
		float sum = 0;
		sum = Cmat[i][0] * Tvec[0]
			+ Cmat[i][1] * Tvec[1]
			+ Cmat[i][2] * Tvec[2]
			+ Cmat[i][3] * Tvec[3];

		resuVec[i] = sum;
	}

	resultPnt3f = GsPnt3f[0] * resuVec[0]
		+ GsPnt3f[1] * resuVec[1]
		+ GsPnt3f[2] * resuVec[2]
		+ GsPnt3f[3] * resuVec[3];

	result[0] = resultPnt3f.x;
	result[1] = resultPnt3f.y;
	result[2] = resultPnt3f.z;
}
void calcuTrainPos(TrainView* trainView)
{
	Pnt3f dir, up;
	Pnt3f cross_t;
	float percent = 1.0f / trainView->DIVIDE_LINE;

	float t = trainView->t_time;

	if (trainView->tw->arcLength->value())
	{
		for (auto i = trainView->table.begin(); i != trainView->table.end(); i++)
		{
			if ((*i).first >= trainView->d_distance)
			{
				trainView->countTrain = (*i).second;
				t = (*i).second - trainView->countTrain;
				break;
			}
		}
	}

	//set trainTime
	trainView->trainTime = t;

	if (trainView->tw->splineBrowser->value() == LINEAR_TRACK)
	{
		size_t i = trainView->countTrain;
		// pos
		Pnt3f cp_pos_p1 = trainView->m_pTrack->points[i].pos;
		Pnt3f cp_pos_p2 = trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].pos;
		// orient
		Pnt3f cp_orient_p1 = trainView->m_pTrack->points[i].orient;
		Pnt3f cp_orient_p2 = trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].orient;

		Pnt3f qt0, qt1, orient_t;
		qt0 = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
		t += percent;
		qt1 = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
		trainView->trainModel->position[0] = qt0.x;
		trainView->trainModel->position[1] = qt0.y;
		trainView->trainModel->position[2] = qt0.z;

		orient_t = (1 - t) * cp_orient_p1 + t * cp_orient_p2;
		cross_t = (qt1 - qt0) * orient_t;
		cross_t.normalize();

		dir = qt1 - qt0;
		dir.normalize();
		up = orient_t;
	}
	else if (trainView->tw->splineBrowser->value() == B_SPLINE)
	{
		size_t i = trainView->countTrain;
		// pos
		Pnt3f cp_pos[4] =
		{
			trainView->m_pTrack->points[(i + 0) % trainView->m_pTrack->points.size()].pos,
			trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].pos,
			trainView->m_pTrack->points[(i + 2) % trainView->m_pTrack->points.size()].pos,
			trainView->m_pTrack->points[(i + 3) % trainView->m_pTrack->points.size()].pos,
		};
		// orient
		Pnt3f cp_orient[4] =
		{
			trainView->m_pTrack->points[(i + 0) % trainView->m_pTrack->points.size()].orient,
			trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].orient,
			trainView->m_pTrack->points[(i + 2) % trainView->m_pTrack->points.size()].orient,
			trainView->m_pTrack->points[(i + 3) % trainView->m_pTrack->points.size()].orient,
		};

		float cp_pos_arr[4][3]
		{
			{cp_pos[0].x,cp_pos[0].y,cp_pos[0].z},
			{cp_pos[1].x,cp_pos[1].y,cp_pos[1].z},
			{cp_pos[2].x,cp_pos[2].y,cp_pos[2].z},
			{cp_pos[3].x,cp_pos[3].y,cp_pos[3].z},
		};
		float cp_orient_arr[4][3]
		{
			{cp_orient[0].x,cp_orient[0].y,cp_orient[0].z},
			{cp_orient[1].x,cp_orient[1].y,cp_orient[1].z},
			{cp_orient[2].x,cp_orient[2].y,cp_orient[2].z},
			{cp_orient[3].x,cp_orient[3].y,cp_orient[3].z},
		};
		float getResu[3] = { 0 };

		caulateGMT(cp_pos_arr, t, getResu, B_SPLINE);
		Pnt3f qt0(getResu[0], getResu[1], getResu[2]);
		caulateGMT(cp_orient_arr, t, getResu, B_SPLINE);
		Pnt3f orient_t(getResu[0], getResu[1], getResu[2]);

		t += percent;
		caulateGMT(cp_pos_arr, t, getResu, B_SPLINE);
		Pnt3f qt1(getResu[0], getResu[1], getResu[2]);

		orient_t.normalize();
		cross_t = (qt1 - qt0) * orient_t;
		cross_t.normalize();

		trainView->trainModel->position[0] = qt0.x;
		trainView->trainModel->position[1] = qt0.y;
		trainView->trainModel->position[2] = qt0.z;

		//Set dir
		dir = qt1 - qt0;
		dir.normalize();
		up = orient_t;
	}
	else if (trainView->tw->splineBrowser->value() == CARDINAL)
	{
		size_t i = trainView->countTrain;
		// pos
		Pnt3f cp_pos[4] =
		{
			trainView->m_pTrack->points[(i + 0) % trainView->m_pTrack->points.size()].pos,
			trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].pos,
			trainView->m_pTrack->points[(i + 2) % trainView->m_pTrack->points.size()].pos,
			trainView->m_pTrack->points[(i + 3) % trainView->m_pTrack->points.size()].pos,
		};
		// orient
		Pnt3f cp_orient[4] =
		{
			trainView->m_pTrack->points[(i + 0) % trainView->m_pTrack->points.size()].orient,
			trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].orient,
			trainView->m_pTrack->points[(i + 2) % trainView->m_pTrack->points.size()].orient,
			trainView->m_pTrack->points[(i + 3) % trainView->m_pTrack->points.size()].orient,
		};

		float cp_pos_arr[4][3]
		{
			{cp_pos[0].x,cp_pos[0].y,cp_pos[0].z},
			{cp_pos[1].x,cp_pos[1].y,cp_pos[1].z},
			{cp_pos[2].x,cp_pos[2].y,cp_pos[2].z},
			{cp_pos[3].x,cp_pos[3].y,cp_pos[3].z},
		};
		float cp_orient_arr[4][3]
		{
			{cp_orient[0].x,cp_orient[0].y,cp_orient[0].z},
			{cp_orient[1].x,cp_orient[1].y,cp_orient[1].z},
			{cp_orient[2].x,cp_orient[2].y,cp_orient[2].z},
			{cp_orient[3].x,cp_orient[3].y,cp_orient[3].z},
		};
		float getResu[3] = { 0 };

		caulateGMT(cp_pos_arr, t, getResu, trainView);
		Pnt3f qt0(getResu[0], getResu[1], getResu[2]);
		caulateGMT(cp_orient_arr, t, getResu, trainView);
		Pnt3f orient_t(getResu[0], getResu[1], getResu[2]);

		t += percent;
		caulateGMT(cp_pos_arr, t, getResu, trainView);
		Pnt3f qt1(getResu[0], getResu[1], getResu[2]);

		orient_t.normalize();
		cross_t = (qt1 - qt0) * orient_t;
		cross_t.normalize();

		trainView->trainModel->position[0] = qt0.x;
		trainView->trainModel->position[1] = qt0.y;
		trainView->trainModel->position[2] = qt0.z;

		//Set dir
		dir = qt1 - qt0;
		dir.normalize();
		up = orient_t;
	}

	glm::vec3 glmUp, glmRight, glmDir;

	glmDir.x = dir.x;
	glmDir.y = dir.y;
	glmDir.z = dir.z;

	glmRight.x = cross_t.x;
	glmRight.y = cross_t.y;
	glmRight.z = cross_t.z;
	//caculate
	glmUp = glm::cross(glmRight, glmDir);

	glm::normalize(glmRight);
	glm::normalize(glmUp);
	glm::normalize(glmDir);

	trainView->trainModel->forward = glmDir;
	trainView->trainModel->up = glmUp;
	trainView->trainModel->right = glmRight;
}
void drawTrack(TrainView* trainView, bool doingShadows)
{
	const float sleeperDistant = 10;
	float countSleeperDistant = 0;

	if (trainView->tw->splineBrowser->value() == LINEAR_TRACK)
	{
		for (size_t i = 0; i < trainView->m_pTrack->points.size(); ++i)
		{
			// pos
			Pnt3f cp_pos_p1 = trainView->m_pTrack->points[i].pos;
			Pnt3f cp_pos_p2 = trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].pos;
			// orient
			Pnt3f cp_orient_p1 = trainView->m_pTrack->points[i].orient;
			Pnt3f cp_orient_p2 = trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].orient;
			float percent = 1.0f / trainView->DIVIDE_LINE;
			float t = 0;
			Pnt3f qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
			Pnt3f qt0, qt1 = qt;

			for (size_t j = 0; j < trainView->DIVIDE_LINE; j++)
			{
				qt0 = qt1;
				t += percent;
				qt1 = (1 - t) * cp_pos_p1 + t * cp_pos_p2;

				Pnt3f orient_t = (1 - t) * cp_orient_p1 + t * cp_orient_p2;
				orient_t.normalize();
				Pnt3f cross_t = (qt1 - qt0) * orient_t;
				cross_t.normalize();
				cross_t = cross_t * 2.5f;
				glLineWidth(1);
				glBegin(GL_LINES);
				glVertex3f(qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z);
				glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);
				glVertex3f(qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z);
				glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
				glEnd();

				countSleeperDistant += Pnt3f::distant(qt0, qt1);
				if (countSleeperDistant >= sleeperDistant)
				{
					float sleeperWidth = 5;
					float sleeperLength = 7;
					Pnt3f vec = qt0 - qt1, qt3;
					vec.normalize();
					vec = sleeperWidth * vec;
					qt3 = vec + qt1;
					cross_t.normalize();
					cross_t = cross_t * sleeperLength;

					float vertex[4][3] =
					{
						{qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z},
						{qt3.x + cross_t.x, qt3.y + cross_t.y, qt3.z + cross_t.z},
						{qt3.x - cross_t.x, qt3.y - cross_t.y, qt3.z - cross_t.z},
						{qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z},
					};

					countSleeperDistant = 0;
					drawSleeper(trainView, doingShadows, vertex, 4);
				}
			}
		}
	}
	else if (trainView->tw->splineBrowser->value() == B_SPLINE)
	{
		for (size_t i = 0; i < trainView->m_pTrack->points.size(); ++i)
		{
			// pos
			Pnt3f cp_pos[4] =
			{
				trainView->m_pTrack->points[(i + 0) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 2) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 3) % trainView->m_pTrack->points.size()].pos,
			};
			// orient
			Pnt3f cp_orient[4] =
			{
				trainView->m_pTrack->points[(i + 0) % trainView->m_pTrack->points.size()].orient,
				trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].orient,
				trainView->m_pTrack->points[(i + 2) % trainView->m_pTrack->points.size()].orient,
				trainView->m_pTrack->points[(i + 3) % trainView->m_pTrack->points.size()].orient,
			};

			float cp_pos_arr[4][3]
			{
				{cp_pos[0].x,cp_pos[0].y,cp_pos[0].z},
				{cp_pos[1].x,cp_pos[1].y,cp_pos[1].z},
				{cp_pos[2].x,cp_pos[2].y,cp_pos[2].z},
				{cp_pos[3].x,cp_pos[3].y,cp_pos[3].z},
			};
			float cp_orient_arr[4][3]
			{
				{cp_orient[0].x,cp_orient[0].y,cp_orient[0].z},
				{cp_orient[1].x,cp_orient[1].y,cp_orient[1].z},
				{cp_orient[2].x,cp_orient[2].y,cp_orient[2].z},
				{cp_orient[3].x,cp_orient[3].y,cp_orient[3].z},
			};
			float getResu[3];
			float percent = 1.0f / trainView->DIVIDE_LINE;
			float t = 0;

			caulateGMT(cp_pos_arr, t, getResu, B_SPLINE);

			Pnt3f qt(getResu[0], getResu[1], getResu[2]);
			Pnt3f qt0, qt1 = qt;

			for (size_t j = 0; j < trainView->DIVIDE_LINE; j++)
			{
				qt0 = qt1;
				t += percent;
				caulateGMT(cp_pos_arr, t, getResu, B_SPLINE);
				qt1.x = getResu[0];
				qt1.y = getResu[1];
				qt1.z = getResu[2];
				glLineWidth(3);
				glBegin(GL_LINES);
				if (!doingShadows)
					glColor3ub(32, 32, 64);
				glVertex3f(qt0.x, qt0.y, qt0.z);
				glVertex3f(qt1.x, qt1.y, qt1.z);
				glEnd();

				caulateGMT(cp_orient_arr, t, getResu, B_SPLINE);
				Pnt3f orient_t(getResu[0], getResu[1], getResu[2]);
				orient_t.normalize();
				Pnt3f cross_t = (qt1 - qt0) * orient_t;
				cross_t.normalize();
				cross_t = cross_t * 2.5f;
				glLineWidth(1);
				glBegin(GL_LINES);
				glVertex3f(qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z);
				glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);
				glVertex3f(qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z);
				glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
				glEnd();

				countSleeperDistant += Pnt3f::distant(qt0, qt1);
				if (countSleeperDistant >= sleeperDistant)
				{
					float sleeperWidth = 5;
					float sleeperLength = 7;
					Pnt3f vec = qt0 - qt1, qt3;
					vec.normalize();
					vec = sleeperWidth * vec;
					qt3 = vec + qt1;
					cross_t.normalize();
					cross_t = cross_t * sleeperLength;

					float vertex[4][3] =
					{
						{qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z},
						{qt3.x + cross_t.x, qt3.y + cross_t.y, qt3.z + cross_t.z},
						{qt3.x - cross_t.x, qt3.y - cross_t.y, qt3.z - cross_t.z},
						{qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z},
					};

					countSleeperDistant = 0;
					drawSleeper(trainView, doingShadows, vertex, 4);
				}
			}
		}
	}
	else if (trainView->tw->splineBrowser->value() == CARDINAL)
	{
		for (size_t i = 0; i < trainView->m_pTrack->points.size(); ++i)
		{
			// pos
			Pnt3f cp_pos[4] =
			{
				trainView->m_pTrack->points[(i + 0) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 2) % trainView->m_pTrack->points.size()].pos,
				trainView->m_pTrack->points[(i + 3) % trainView->m_pTrack->points.size()].pos,
			};
			// orient
			Pnt3f cp_orient[4] =
			{
				trainView->m_pTrack->points[(i + 0) % trainView->m_pTrack->points.size()].orient,
				trainView->m_pTrack->points[(i + 1) % trainView->m_pTrack->points.size()].orient,
				trainView->m_pTrack->points[(i + 2) % trainView->m_pTrack->points.size()].orient,
				trainView->m_pTrack->points[(i + 3) % trainView->m_pTrack->points.size()].orient,
			};

			float cp_pos_arr[4][3]
			{
				{cp_pos[0].x,cp_pos[0].y,cp_pos[0].z},
				{cp_pos[1].x,cp_pos[1].y,cp_pos[1].z},
				{cp_pos[2].x,cp_pos[2].y,cp_pos[2].z},
				{cp_pos[3].x,cp_pos[3].y,cp_pos[3].z},
			};
			float cp_orient_arr[4][3]
			{
				{cp_orient[0].x,cp_orient[0].y,cp_orient[0].z},
				{cp_orient[1].x,cp_orient[1].y,cp_orient[1].z},
				{cp_orient[2].x,cp_orient[2].y,cp_orient[2].z},
				{cp_orient[3].x,cp_orient[3].y,cp_orient[3].z},
			};
			float getResu[3];
			float percent = 1.0f / trainView->DIVIDE_LINE;
			float t = 0;

			caulateGMT(cp_pos_arr, t, getResu, trainView);

			Pnt3f qt(getResu[0], getResu[1], getResu[2]);
			Pnt3f qt0, qt1 = qt;

			for (size_t j = 0; j < trainView->DIVIDE_LINE; j++)
			{
				qt0 = qt1;
				t += percent;
				caulateGMT(cp_pos_arr, t, getResu, trainView);
				qt1.x = getResu[0];
				qt1.y = getResu[1];
				qt1.z = getResu[2];
				glLineWidth(3);
				glBegin(GL_LINES);
				if (!doingShadows)
					glColor3ub(32, 32, 64);
				glVertex3f(qt0.x, qt0.y, qt0.z);
				glVertex3f(qt1.x, qt1.y, qt1.z);
				glEnd();

				caulateGMT(cp_orient_arr, t, getResu, trainView);
				Pnt3f orient_t(getResu[0], getResu[1], getResu[2]);
				orient_t.normalize();
				Pnt3f cross_t = (qt1 - qt0) * orient_t;
				cross_t.normalize();
				cross_t = cross_t * 2.5f;
				glLineWidth(1);
				glBegin(GL_LINES);
				glVertex3f(qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z);
				glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);
				glVertex3f(qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z);
				glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
				glEnd();

				countSleeperDistant += Pnt3f::distant(qt0, qt1);
				if (countSleeperDistant >= sleeperDistant)
				{
					float sleeperWidth = 5;
					float sleeperLength = 7;
					Pnt3f vec = qt0 - qt1, qt3;
					vec.normalize();
					vec = sleeperWidth * vec;
					qt3 = vec + qt1;
					cross_t.normalize();
					cross_t = cross_t * sleeperLength;

					float vertex[4][3] =
					{
						{qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z},
						{qt3.x + cross_t.x, qt3.y + cross_t.y, qt3.z + cross_t.z},
						{qt3.x - cross_t.x, qt3.y - cross_t.y, qt3.z - cross_t.z},
						{qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z},
					};

					countSleeperDistant = 0;
					glNormal3d(orient_t.x, orient_t.y, orient_t.z);
					drawSleeper(trainView, doingShadows, vertex, 4);
				}
			}
		}
	}
}
void drawSleeper(TrainView* trainView, bool doingShadows, float(*vertex)[3], int vertexSize)
{
	glBegin(GL_QUADS);
	if (!doingShadows)
		glColor3ub(120, 120, 64);
	glVertex3f(vertex[0][0], vertex[0][1], vertex[0][2]);
	glVertex3f(vertex[1][0], vertex[1][1], vertex[1][2]);
	glVertex3f(vertex[2][0], vertex[2][1], vertex[2][2]);
	glVertex3f(vertex[3][0], vertex[3][1], vertex[3][2]);
	glEnd();
}
void drawTrain(TrainView* trainView, bool doingShadows)
{
	if (doingShadows == false)
		trainView->shader->Use();
	else
		trainView->shadowShader->Use();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(trainView->shader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(glGetUniformLocation(trainView->shader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));

	// Draw the loaded model
	glm::mat4 model(1);
	model = glm::translate(model, trainView->trainModel->position);
	model[0][0] = trainView->trainModel->right[0];
	model[0][1] = trainView->trainModel->right[1];
	model[0][2] = trainView->trainModel->right[2];
	model[1][0] = trainView->trainModel->up[0];
	model[1][1] = trainView->trainModel->up[1];
	model[1][2] = trainView->trainModel->up[2];
	model[2][0] = trainView->trainModel->forward[0];
	model[2][1] = trainView->trainModel->forward[1];
	model[2][2] = trainView->trainModel->forward[2];

	// Translate it down a bit so it's at the center of the scene
	//model = glm::lookAt(glm::vec3(0), trainView->trainModel->forward, trainView->trainModel->up);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1, 0, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, -1));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
	glUniformMatrix4fv(glGetUniformLocation(trainView->shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	trainView->trainModel->Draw(*trainView->shader);

	Shader::Unuse();
}
void drawLight(TrainView* trainView, bool doingShadows)
{
	trainView->shader->Use();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(trainView->shader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(glGetUniformLocation(trainView->shader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));

	// Draw the loaded model
	glm::mat4 model(1);
	model = glm::translate(model, glm::vec3(0.0f, 10.f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
	glUniformMatrix4fv(glGetUniformLocation(trainView->shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	trainView->light->Draw(*trainView->shader);

	Shader::Unuse();
}
void drawPlan(TrainView* trainView)
{
	trainView->shader->Use();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(trainView->shader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(glGetUniformLocation(trainView->shader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));

	// Draw the loaded model
	glm::mat4 model(1);
	model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));
	glUniformMatrix4fv(glGetUniformLocation(trainView->shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	trainView->parkModel->Draw(*trainView->shader);

	Shader::Unuse();
}
void drawWaterPlan(TrainView* trainView)
{
	trainView->waterShader->Use();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(trainView->waterShader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(glGetUniformLocation(trainView->waterShader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));

	// Draw the loaded model
	glm::mat4 model(1);
	model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
	model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));
	glUniformMatrix4fv(glGetUniformLocation(trainView->waterShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	trainView->waterPlan->Draw(*trainView->waterShader);

	glm::mat4 view_matrix_inverse = glm::inverse(view_matrix);

	glUniform1f(glGetUniformLocation(trainView->waterShader->Program, "water_time"), trainView->water_time);
	glUniform3fv(glGetUniformLocation(trainView->waterShader->Program, "viewPos"), 1
		, glm::value_ptr(glm::vec3(view_matrix_inverse[3][0], view_matrix_inverse[3][1], view_matrix_inverse[3][2])));

	Shader::Unuse();
}