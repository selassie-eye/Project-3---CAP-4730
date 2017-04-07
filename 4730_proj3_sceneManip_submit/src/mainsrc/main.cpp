//--------------------------------------------------------
// Computer Graphics
// University of Florida
// Copyright 2015 Corey Toler-Franklin
//--------------------------------------------------------


//--------------------------------------------------------
// Included Files and Defs
//--------------------------------------------------------
// System
#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdexcept>
#include<map>
#include <stdlib.h>
#include <assert.h>
using namespace std;

// GLEW
#define GLEW_STATIC
#include "stglew.h"
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

 //stlib and rtlib
#include "STColor4ub.h"
#include "STImage.h"
#include "defs.h"
#include "Scene.h"
#include "utilities.h"
#include "STVector4.h"
#include "Triangle.h"
#include "STMatrix4.h"  // Added in Project 3




//--------------------------------------------------------
// forward declarations
//--------------------------------------------------------
void MouseMotionCallback(int x, int y);
void MouseCallback(int button, int state, int x, int y);
void ManipulationMode(int button, int state, int x, int y);
void KeyCallback(unsigned char key, int x, int y);
void SpecialKeyCallback(int key, int x, int y);
void ReshapeCallback(int w, int h);
void Setup(void);
void menu(int item);
void CleanUp (void);
void Spin (void);
void BoundingSphere(STPoint3 *center, float *radius);
void ProcessRotation(int mouseX, int mouseY);
void ProcessTranslation(void);
void drawskybox();
unsigned int skybox[6];





//---------------------------------
// globals
// a neccessary evil for most glut programs
//---------------------------------
int globalCount;




//--------------------------------------------------------
// Scenes
//--------------------------------------------------------
Scene       *pScene = NULL;         // scene geometry and lights





//----------------------------------------------------
// Rendering Modes
// TO DO: Proj3_scenemanip
// Note: Rendermode should always be NAVIGATION for this assignment
// No further Implementation required - just do not change the rendermode :)
//----------------------------------------------------
RenderMode startMode = NAVIGATION;  // this mode should not change for this assignment




//--------------------------------------------------------
// shaders
//--------------------------------------------------------
STShaderProgram *shader;
std::string vertexShader;
std::string fragmentShader;




//------------------------------------------------------
// Meshes
//------------------------------------------------------
typedef struct {
  int i1;
  int i2;
  int i3;
} TriangleIndices;

//mesh files
std::string meshOBJ;


// mesh params and modes
bool mesh = true; // draw mesh
bool smooth = true; // smooth/flat shading for mesh
bool normalMapping = false; // true=normalMapping, false=displacementMapping
bool proxyType=false; // false: use cylinder; true: use sphere
STPoint3 gMassCenter;
std::pair<STPoint3,STPoint3> gBoundingBox;
STTriangleMesh* gManualTriangleMesh = 0;
int TesselationDepth = 100;




//------------------------------------------------------
// Windowing UI and Mouse Input
//------------------------------------------------------
// Window size, kept for screenshots
static int gWindowSizeX = 800;
static int gWindowSizeY = 800;

// Stored mouse position for camera motions
int gPreviousMouseX = -1; // previous mouse x position
int gPreviousMouseY = -1; // previous mouse y position
int gMouseButton = -1; // current mouse position
bool bSpin = false; // true if in spinning mode
bool bMove = false; // true if in move mode
int beginx; // global var for start x position
int beginy; // global var for start y position
float curQMat[4]; // camera coordinate basis used for computing orientation for spin
float lastQMat[4]; // camera coordinate basis used for computing orientation for spin





//------------------------------------------------------------------
// Support for model geometry - meshes, surfaces etc...
//------------------------------------------------------------------
void BoundingSphere(STPoint3 *center, float *radius)
{
    STPoint3 topleft = gBoundingBox.first;
    STPoint3 bottomright = gBoundingBox.second;

    float c =  (topleft.x - bottomright.x) * (topleft.x - bottomright.x);
          c += (topleft.y - bottomright.y) * (topleft.y - bottomright.y);
          c += (topleft.z - bottomright.z) * (topleft.z - bottomright.z);

    *radius = fabs(sqrtf(c));
    *radius = (*radius)/2.0;
    *center = gMassCenter;
}


void CreateYourOwnMesh()
{
    float leftX   = -2.0f;
    float rightX  = -leftX;
    float nearZ   = -2.0f;
    float farZ    = -nearZ;

    gManualTriangleMesh= new STTriangleMesh();
    for (int i = 0; i < TesselationDepth+1; i++){
        for (int j = 0; j < TesselationDepth+1; j++) {
            float s0 = (float) i / (float) TesselationDepth;
            float x0 =  s0 * (rightX - leftX) + leftX;
            float t0 = (float) j / (float) TesselationDepth;
            float z0 = t0 * (farZ - nearZ) + nearZ;

            gManualTriangleMesh->AddVertex(x0,(x0*x0+z0*z0)*0.0f,z0,s0,t0);
        }
    }
    for (int i = 0; i < TesselationDepth; i++){
        for (int j = 0; j < TesselationDepth; j++) {
            unsigned int id0=i*(TesselationDepth+1)+j;
            unsigned int id1=(i+1)*(TesselationDepth+1)+j;
            unsigned int id2=(i+1)*(TesselationDepth+1)+j+1;
            unsigned int id3=i*(TesselationDepth+1)+j+1;
            gManualTriangleMesh->AddFace(id0,id2,id1);
            gManualTriangleMesh->AddFace(id0,id3,id2);
        }
    }
    gManualTriangleMesh->Build();
    gManualTriangleMesh->mMaterialAmbient[0]=0.2f;
    gManualTriangleMesh->mMaterialAmbient[1]=0.2f;
    gManualTriangleMesh->mMaterialAmbient[2]=0.6f;
    gManualTriangleMesh->mMaterialDiffuse[0]=0.2f;
    gManualTriangleMesh->mMaterialDiffuse[1]=0.2f;
    gManualTriangleMesh->mMaterialDiffuse[2]=0.6f;
    gManualTriangleMesh->mMaterialSpecular[0]=0.6f;
    gManualTriangleMesh->mMaterialSpecular[1]=0.6f;
    gManualTriangleMesh->mMaterialSpecular[2]=0.6f;
    gManualTriangleMesh->mShininess=8.0f;
}

// clean up
void CleanUp()
{
    if(pScene)
        delete(pScene);
}






//----------------------------------------------------------------------
// Modes for the manipulators
//----------------------------------------------------------------------
void ManipulationMode(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    bSpin = false;
    glutIdleFunc(NULL);
    bMove = true;
    beginx = x;
    beginy = y;
  }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    bMove = false;
  }
}




//-------------------------------------------------------------------
// Manipulation movements are updated by these functions
//-------------------------------------------------------------------

//------------------------------------------------------------------------
// TO DO: Proj3_scenemanip
// called every time the rotation manipulator is dragged
// complete the function Scene::Rotate (raystart, raysend)
//-------------------------------------------------------------------------
void ProcessRotation(int mouseX, int mouseY)
{

    if(!pScene->HasManipulator())
        return;

    double matModelView[16], matProjection[16];
    int viewport[4];


    glGetDoublev(GL_MODELVIEW_MATRIX, matModelView);
    glGetDoublev(GL_PROJECTION_MATRIX, matProjection);
    glGetIntegerv(GL_VIEWPORT, viewport );
    double winX = (double)mouseX;
    double winY = viewport[3] - (double)mouseY;

    GLdouble   m_startX, m_startY, m_startZ;
    GLdouble   m_endX,   m_endY,   m_endZ;

    gluUnProject(winX, winY, 0.0, matModelView, matProjection,
                 viewport, &m_startX, &m_startY, &m_startZ);
    gluUnProject(winX, winY, 1.0, matModelView, matProjection,
                 viewport, &m_endX, &m_endY, &m_endZ);


    double t = 0.0;
    STVector3 closestPt;
    bool bHit = false;
    STVector3 raystart(m_startX, m_startY, m_startZ);
    STVector3 raysend(m_endX, m_endY, m_endZ);


    //------------------------------------------------------------------------------
    // TO DO: Proj3_scenemanip
    // Update the matrix transformation for the manipulator geometry
    // Update current model matrix
    // Use this information to call  pScene->Rotate();
    //
    //------------------------------------------------------------------------------
    pScene->Rotate(raystart, raysend);
    //---------------------------------------------------------------------------------
}

//------------------------------------------------------------
// TO DO: Proj3_scenemanip
// Called every time the translation manipulator is dragged.
// Read through this function.
// Then implement Scene::Translate
//------------------------------------------------------------
void ProcessTranslation(int mouseX, int mouseY)
{
    // check for the manipulatotr
    if(!pScene->HasManipulator())
        return;

    // declare vars
    double matModelView[16], matProjection[16];
    int viewport[4];
    double t = 0.0;
    STVector3 closestPt;
    bool bHit = false;

    // get the model and projection matrices
    glGetDoublev(GL_MODELVIEW_MATRIX, matModelView );
    glGetDoublev(GL_PROJECTION_MATRIX, matProjection );
    glGetIntegerv(GL_VIEWPORT, viewport );
    double winX = (double)mouseX;
    double winY = viewport[3] - (double)mouseY;


    // mouse positions and z coords of near and far plane
    GLdouble   m_startX, m_startY, m_startZ;
    GLdouble   m_endX,   m_endY,   m_endZ;


    // Projects the mouse position in window coordinates
    // into points in world space. The projected points
    // and the near and far planes returned are used to
    // create a ray in the scene along which the object
    // will be tranaslated forward and backward.
    gluUnProject(winX, winY, 0.0, matModelView, matProjection,
                 viewport, &m_startX, &m_startY, &m_startZ);
    gluUnProject(winX, winY, 1.0, matModelView, matProjection,
                 viewport, &m_endX, &m_endY, &m_endZ);

    STVector3 raystart(m_startX, m_startY, m_startZ);
    STVector3 raysend(m_endX, m_endY, m_endZ);

    //------------------------------------------------------------------------------
    // TO DO: Proj3_scenemanip
    // Complete the function Scene::Translate which is called here
    //------------------------------------------------------------------------------
    pScene->Translate(raystart, raysend);
    //---------------------------------------------------------------------------------


}



//-----------------------------------------------------------
// TO DO: Proj3_scenemanip for your first test, change the value
// of the transform that initializes the group transformation node below
// When you propogate the transform to the children the cube should move in the scene
// This function should also be updated at the end of the assignment.
// Setting up the scene adds geometry - a triangle mesh.
//-----------------------------------------------------------
void initializeScene(void)
{


    //----------------------------------------------------
    // TO DO: Proj3_scenemanip
    // For the final step in the project (after everything is complete)
    // you will replace this with a more complex model.
    // See options for doing this in your intructions.
    //-----------------------------------------------------
    cout << "Adding nodes" << endl;

    // load a cube model
    std::vector<STTriangleMesh*> gTriangleMeshes;
    meshOBJ                         = std::string("../../data/meshes/cube.obj");
    STTriangleMesh::LoadObj(gTriangleMeshes,meshOBJ);

    // add the group transform for these triangle meshes here
    // TO DO: Proj3_scenemanip for your first test, change the value
    // of the transform that initializes the transformation node below
    // When you propogate the transform to the children the cube should move in the scene
    STMatrix4 *translation = new STMatrix4();    // New matrix initialization for Projec 3 pt 1
    translation->EncodeT(2.f, 2.f, 2.f);
    TransformNode *pNode =  pScene->AddTransform(*translation, pScene->GetRoot());

    // add the triangle meshes
    for(int i = 0; i < (int)gTriangleMeshes.size(); ++i) {
        pScene->AddTriangleMesh(gTriangleMeshes[i], (SceneNode*)pNode);
    }
    pScene->PropogateTransforms(pScene->GetRoot());

    // update the bounding box
    pScene->GetBBox(&gMassCenter, &gBoundingBox);

    cout << "Adding nodes done" << endl;
    //----------------------------------------------------------------------------
}




//------------------------------------------------------------------
// TO DO: Proj3_scenemanip
// Initial set up of the scene, OpenGL and the application
// No further implementation required in this function.
//------------------------------------------------------------------
void Setup()
{

    // reset the mass center
    gMassCenter = STPoint3(0,0,0);

    // clear the color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


    // create scene object and set the rendermode
    // rendermode should always be NAVIGATION for this assignment
    // TO DO: Proj3_scenemanip
    // NOTE: the RenderMode must stay at NAVIGATION for this assignment
    // No further implementation required here
    if(pScene == NULL) {
        pScene = new Scene();
        pScene->SetRenderMode(startMode);
    }

    // initialize the scene
    initializeScene();

    // Set up lighting variables in OpenGL
    // Once we do this, we will be able to access them as built-in
    // attributes in the shader (see examples of this in normalmap.frag)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_SPECULAR,  pScene->specularLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT,   pScene->ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,   pScene->diffuseLight);


    // load the vertes and fragment shaders
    shader = new STShaderProgram();
    shader->LoadVertexShader(vertexShader);
    shader->LoadFragmentShader(fragmentShader);


    //--------------------------------------------------------------------
    // TO DO: Proj3_scenemanip
    // No Further implementation required here.
    // This code resets the camera and then sets it to a position in front of the model
    // with the  lookat vector pointing toward the model.
    //-----------------------------------------------------------------------
    pScene->GetCamera()->Reset();
    pScene->GetCamera()->SetLookAt(STVector3(0.f,0.f,0.f));
    pScene->GetCamera()->SetPosition(STVector3(0.f,5.f,15.f));
    pScene->GetCamera()->SetUp(STVector3(0.f,1.f,0.f));
    //------------------------------------------------------------------------

    // clear color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // enable depth buffer
    glEnable(GL_DEPTH_TEST);

}



//----------------------------------------------------
// displaying information
//----------------------------------------------------


// This function shows examples of how to bind textures
// to the phong shader
// The ReflectanceMapping function is where you will
// set parameters to your shaders
// No additional implementation required here
//-----------------------------------------------------
void defaultMode(void)
{
    // clear buffers - color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the model view to the identity matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    //------------------------------------------------------------
    // TO DO: Proj3_scenemanip
    // No new implementation required.
    // This sets the default camera position.
    // for the first view.
    //-------------------------------------------------------------

    // sets the local camera parameters
    pScene->GetCamera()->SetUpAndRight();
    STVector3 mPosition = pScene->GetCamera()->Position();
    STVector3 mLookAt = pScene->GetCamera()->LookAt();
    STVector3 mUp = pScene->GetCamera()->Up();

    // creates a view matrix
    gluLookAt(mPosition.x,mPosition.y,mPosition.z,
              mLookAt.x,mLookAt.y,mLookAt.z,
              mUp.x,mUp.y,mUp.z);
    //-------------------------------------------------------------

    // set the light position
    float thelightposition[4];
    pScene->GetLightPosition(thelightposition);
    glLightfv(GL_LIGHT0, GL_POSITION, thelightposition);

    if(mesh)
    {
        //----------------------------------------------------
        // TO DO: Proj3_scenemanip
        // No Further implementation required here. The scene
        // is drawn here. A new bounding box is also created around
        // all group nodes. This is required so that a sphere
        // bounding box can be used for selecting objects
        // (ray sphere intersection). If the bounding box
        // is not up to date and in world space coordinates,
        // Scene::SelectObjects will not work.
        //----------------------------------------------------
        pScene->Draw();
        pScene->GetBBox(&gMassCenter, &gBoundingBox);
        //--------------------------------------------------------------------------------

        // Set shader params. Note, no support for normal or displacement mapping implemented
        shader->SetUniform("normalMapping", -1.0);
        shader->SetUniform("displacementMapping", -1.0);
        shader->SetUniform("colorMapping", 1.0);
    }

    // unbind shares
    shader->UnBind();

    // done with front frame buffer so swap to
    // the back frame buffer that was rendering
    // in the background
    glutSwapBuffers();
}

// Display the output image from our vertex and fragment shaders
void DisplayCallback()
{
    defaultMode();
}




//------------------------------------------------------------
// Windowing and Mouse callbacks
//------------------------------------------------------------
//
// Reshape the window and record the size so
// that we can use it for screenshots.
//
void ReshapeCallback(int w, int h)
{


    gWindowSizeX = w;
    gWindowSizeY = h;

    glViewport(0, 0, gWindowSizeX, gWindowSizeY);

   if (pScene->GetRenderMode() == ENVIRONMENTMAP)
       return;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    // Set up a perspective projection
    float aspectRatio = (float) gWindowSizeX / (float) gWindowSizeY;
    gluPerspective(30.0f,aspectRatio, .1f, 10000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void SpecialKeyCallback(int key, int x, int y)
{
    switch(key) {
        case GLUT_KEY_LEFT:
            pScene->GetCamera()->Strafe(10,0);
            break;
        case GLUT_KEY_RIGHT:
            pScene->GetCamera()->Strafe(-10,0);
            break;
        case GLUT_KEY_DOWN:
            pScene->GetCamera()->Strafe(0,-10);
            break;
        case GLUT_KEY_UP:
            pScene->GetCamera()->Strafe(0,10);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void KeyCallback(unsigned char key, int x, int y)
{


    switch(key) {

        case 'a': {
        }
        break;

        case 'c': {
            // TO DO: Proj3_scenemanip
            // No new implementation required, this has been added for you
            // 1. key press event 'c' toggles the camera
            // between Fly and Orbit mode
            // 2. Use Camera::ToggleRotationMode
            //------------------------------------------------

            pScene->GetCamera()->ToggleRotationMode();

            //------------------------------------------------
        }
        break;


        case 'e': {
            //---------------------------------------------------
            // TO DO: Proj3_scenemanip
            // No new implementation required. This has been done for you.
            // key press event 'e' toggles the manipulator
            // geometry. This makes it easier to select
            // parts of the manipulator. It toggles between:
            //  - AXIS_ALL, AXIS_TRANSXY, ROTATIONALL and RemoveManip() -
            //------------------------------------------------
            fprintf(stderr, "toggle manip mode\n");
            pScene->ToggleManipGeometry();
            //------------------------------------------------
        }
        break;

        case 'f': {// switch between smooth shading and flat shading
            smooth = !smooth;
        }
        break;


        case 'k': {
            //------------------------------------------------
            // TO DO: Proj3_scenemanip
            // No new implementatin required. This has been completed for you.
            // 'k' toggles between LOCAL, PARENT, and NONE modes
            //------------------------------------------------
            pScene->ToggleManipMode();
            //------------------------------------------------
        }
        break;


        case 'r': {
            pScene->GetCamera()->Reset();
        }
        break;


        case 's': {
                //
                // Take a screenshot, and save as screenshot.jpg
                //
                STImage* screenshot = new STImage(gWindowSizeX, gWindowSizeY);
                screenshot->Read(0,0);
                screenshot->Save("../../data/images/screenshot.jpg");
                delete screenshot;
         }
        break;

        case 'm': {// switch between the mesh you create and the mesh from file
            mesh = !mesh;
        }
        break;

        case 'n': { // switch between normalMapping and displacementMapping
            normalMapping = !normalMapping;
        }
        break;

        case 'u': {
            pScene->GetCamera()->ResetUp();
        }
        break;

        case 'w': {
       }
       break;

        case 'q': {
            exit(0);
        }

        default:
            break;
    }

    glutPostRedisplay();
}

// Mouse event handler
//---------------------------------------------
void MouseCallback(int button, int state, int x, int y)
{

    if(pScene->GetRenderMode() == ENVIRONMENTMAP) {
        ManipulationMode(button, state, x, y);
        return;
    }


    if (button == GLUT_LEFT_BUTTON
        || button == GLUT_RIGHT_BUTTON
        || button == GLUT_MIDDLE_BUTTON){
        gMouseButton = button;
    }
    else {
        gMouseButton = -1;
    }

    if (state == GLUT_UP)
    {
        gPreviousMouseX = -1;
        gPreviousMouseY = -1;
    }

    if(  (button == GLUT_LEFT_BUTTON)
      && (state == GLUT_DOWN)
      && (pScene->GetRenderMode() == NAVIGATION)
      && (pScene->CurrentManipMode() == LOCAL || pScene->CurrentManipMode() == PARENT)) {



         if(pScene->HasManipulator())
            fprintf(stderr, "has manip!\n");
         else
             fprintf(stderr, "NO manip!\n");

        //-----------------------------------------------------------------------------
        // TO DO: Proj3_scenemanip
        // No new implementation required. This is implemented for you.
        // If there is a manupulator in the scene, shoot a ray from the
        // mouse position in screen coordinates to the the 3D geometry in
        // world space to see if it hits the manipulator. If there is a hit, make the
        // selected  part active, set the manipulation motion mode and return.
         //---------------------------------------------------------------------------
        if(pScene->HasManipulator()) {
            pScene->SelectManipulator(x, y, gWindowSizeX, gWindowSizeY);
            return;
        }
        //-----------------------------------------------------------------------------

        STPoint3 center;
        float radius;
        BoundingSphere(&center, &radius);


        //--------------------------------------------------------------------
        // TO DO: Proj3_scenemanip
        // No new implementation required.
        // This code selects objects.
        // If you are in a manipulation mode LOCAL or PARENT, a manipulator will be added to the object.
        // Note: selections occur as long as you click in the bounding sphere of the object group
        // (so technically, you can select an object group if the mouse is not on the object but close to it).
        // Use the 'e' keypress event to toggle between - manipulator geometry and no manipuator geometry.
        // The manipulator geometry states are - AXIS_ALL, AXIS_TRANSXYZ, AXIS_ROTATIONALL and RemoveManipulator.
        //---------------------------------------------------------------------------------
        if(pScene->SelectObject(x, y, gWindowSizeX, gWindowSizeY, center, radius)) {
            fprintf(stderr, "SELECTED!\n");

            if(!pScene->HasManipulator())
                pScene->AddManipulator();
        }
        else {
            fprintf(stderr, "No selection found!\n");
            pScene->RemoveManip();
        }
        //------------------------------------------------------------------------------------------
    }
}


// spin
void Spin (void)
{
    pScene->Spin(lastQMat, curQMat, curQMat);
}


//------------------------------------------------------------------
// Mouse active motion callback (when button is pressed)
// rotatation  and translation callbacks
// update mouse positionsmkm
//------------------------------------------------------------------
void MouseMotionCallback(int x, int y)
{


    // translate
    if (pScene->CurrentManipMotion() == TRANS_X ||
        pScene->CurrentManipMotion() == TRANS_Y ||
        pScene->CurrentManipMotion() == TRANS_Z){
        ProcessTranslation(x, y);
        return;
    }

    // rotate
    if (pScene->CurrentManipMotion() == ROTATE_X ||
        pScene->CurrentManipMotion() == ROTATE_Y ||
        pScene->CurrentManipMotion() == ROTATE_Z){
        ProcessRotation(x, y);
        return;
    }


    // compute the change in x and y
    if (gPreviousMouseX >= 0 && gPreviousMouseY >= 0)
    {
        //compute delta
        float deltaX = x-gPreviousMouseX;
        float deltaY = y-gPreviousMouseY;
        gPreviousMouseX = x;
        gPreviousMouseY = y;



        if (gMouseButton == GLUT_LEFT_BUTTON)
        {
            //-----------------------------------------------------------
            // TO DO: Proj3_scenemanip
            // Orbits the camera IF there is no current manipulation mode
            // There are two functions in Camera::Orbit that must be implemented
            // to complete fly and orbit mode
            //-----------------------------------------
            float axis[4];
            pScene->GetCamera()->Orbit(axis, x-deltaX, y-deltaY, x, y);
            //-------------------------------------------
        }
        else if (gMouseButton == GLUT_MIDDLE_BUTTON)
        {
            pScene->GetCamera()->Strafe(deltaX, deltaY);
        }
        else if (gMouseButton == GLUT_RIGHT_BUTTON)
        {
           pScene->GetCamera()->Zoom(deltaY);
        }

    }
    else {
        gPreviousMouseX = x;
        gPreviousMouseY = y;
    }

}




//------------------------------------------------------------------
// context menu implementation
//------------------------------------------------------------------
void menu(int item)
{
    //------------------------------------------------------------------------------------
    // TO DO: Proj3_scenemanip
    // "Optional"
    // On third mouse button down, CreateContextMenu()
    // alows you to add a context menu
    // The menu toggles modes
    // implement this menu() function to complete this
    // This is an optional assignment and not required for grading
    //-------------------------------------------------------------------------------------
    switch (item) {
    default:
        assert(0);
    }
    glutPostRedisplay();
}




//------------------------------------------------------------------
// user feedback on input args
// currently empty because there is no user input param
//-----------------------------------------------------------------
void usage()
{
}




//------------------------------------------------------------------------------------
// TO DO: Proj3_scenemanip
// "Optional"
// On third mouse button down, this function
// alows you to add a context menu
// The menu toggles modes
// implement the menu() function to complete this
// This is an optional  and not required for grading
//-------------------------------------------------------------------------------------
void CreateContextMenu()
{
  glutCreateMenu(menu);
  glutAddMenuEntry("Local", LOCAL);
  glutAddMenuEntry("Parent", PARENT);
  glutAddMenuEntry("Selection", NONE);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
//--------------------------------------
}




//-------------------------------------------------
// main program loop
//-------------------------------------------------
int main(int argc, char** argv)
{

    // vertex abd fragment shaders loaded here
    vertexShader   = std::string("kernels/default.vert");
    fragmentShader = std::string("kernels/phong.frag");

    // Initialize GLUT.
    glutInitWindowSize(800, 800);
    glutInit(&argc, argv);


    // initialize display mode
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Program 3_scenemanip");


    //
    // Initialize GLEW.
    //
#ifndef __APPLE__
    glewInit();
    if(!GLEW_VERSION_2_0) {
        printf("Your graphics card or graphics driver does\n"
            "\tnot support OpenGL 2.0, trying ARB extensions\n");

        if(!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader) {
            printf("ARB extensions don't work either.\n");
            printf("\tYou can try updating your graphics drivers.\n"
                "\tIf that does not work, you will have to find\n");
            printf("\ta machine with a newer graphics card.\n");
            exit(1);
        }
    }
#endif

    // callback functions
    glutDisplayFunc(DisplayCallback);
    glutReshapeFunc(ReshapeCallback);
    glutSpecialFunc(SpecialKeyCallback);
    glutKeyboardFunc(KeyCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MouseMotionCallback);
    glutIdleFunc(DisplayCallback);


    // initial scene geometry
    Setup();


    //------------------------------------------------------------------------------------
    // TO DO: Proj3_scenemanip
    // "Optional" enable context menu, CreateContextMenu()
    // This is not required for grading
    //------------------------------------------------------------------------------------

    //------------------------------------------------------------------------------------


    // run the main glut loop
    glutMainLoop();


    // Cleanup code should be called here.
    CleanUp();

    // return
    return 0;
}
