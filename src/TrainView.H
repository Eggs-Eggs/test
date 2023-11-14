/************************************************************************
	 File:        TrainView.H

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

#pragma once

// Preclarify for preventing the compiler error
class TrainWindow;
class CTrack;


//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


// The TrainView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>
#pragma warning(pop)

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"
#include<map>
#include<vector>

#include"RenderUtilities/BufferObject.h"
#include"RenderUtilities/Shader.h"
#include"RenderUtilities/Texture.h"
#include"Model.h"

#define WIN32_LEAN_AND_MEAN
#include<winuser.h>
class Person
{
public:
	Person();
	~Person();

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 forward = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::vec3(0.0f, 0.0f, 1.0f);
private:

};


class TrainView : public Fl_Gl_Window
{
public:
	// note that we keep the "standard widget" constructor arguments
	TrainView(int x, int y, int w, int h, const char* l = 0);

	// overrides of important window things
	virtual int handle(int);
	virtual void draw();

	// all of the actual drawing happens in this routine
	// it has to be encapsulated, since we draw differently if
	// we're drawing shadows (no colors, for example)
	void drawStuff(bool doingShadows = false);

	// setup the projection - assuming that the projection stack has been
	// cleared for you
	void setProjection();

	// Reset the Arc ball control
	void resetArcball();

	// pick a point (for when the mouse goes down)
	void doPick();
public:
	ArcBallCam		arcball;			// keep an ArcBall for the UI
	int				selectedCube;  // simple - just remember which cube is selected

	TrainWindow* tw;				// The parent of this display window
	CTrack* m_pTrack;		// The track of the entire scene

	int countTrain = 0;
	float t_time = 0;
	float d_distance = 0;
	float water_time = 0;
	float trainTime = 0;
	bool change = true;
	float DIVIDE_LINE = 1000.0f;
	float totalDistant = 0;
	std::map <float, float> table;

	Shader* shader = nullptr;
	Shader* waterShader = nullptr;
	Shader* shadowShader = nullptr;
	Texture2D* texture = nullptr;
	VAO* plane = nullptr;
	UBO* commom_matrices = nullptr;
	Model* parkModel = nullptr;
	Model* trainModel = nullptr;
	Model* waterPlan = nullptr;
	Model* light = nullptr;
};

void genTable(TrainView* trainView);
void caulateGMT(float(*Gs)[3], float time, float* result, int type);
void caulateGMT(float(*Gs)[3], float time, float* result, TrainView* trainView);
void calcuTrainPos(TrainView* trainView);
void drawTrack(TrainView* trainView, bool doingShadows);
void drawSleeper(TrainView*, bool, float(*)[3], int);
void drawTrain(TrainView* trainView, bool doingShadows);
void drawLight(TrainView* trainView, bool doingShadows);
void drawPlan(TrainView* trainView);
void drawWaterPlan(TrainView* trainView);
