// ASMShadersCon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>



//
//
// ghv 20200311: removed all dependencies on NVIDIA nvparse; 
// ghv 20200320: replaced legacy pbuffer with fbo; 
//
//
#undef GLH_EXT_SINGLE_FILE
#include <glh_extensions/glh_extensions.h>
#include <glh_helpers/glh_glut.h>
#include "V2Sprite.h"

float g0_dolly_x_constant = -0.123f;
float g0_dolly_y_constant = -0.123f;
float g0_dolly_z_constant = -3.4321f;


glh::glut_callbacks cb_callback_object;

glh::glut_simple_mouse_interactor objectSpheres;


bool bOptions[256];
bool bInitialized = false;

GLfloat alpha = 0, beta = 0;

V2Sprite spriteSingleton;  // TODO: this is a singleton;  remove this and write some C++!




GLfloat spColor0[3] = { 0.0f, 0.0f, 0.0f };
GLfloat spColor1[3] = { 0.6f, 0.0f, 0.1f };
GLfloat spColor2[3] = { 0.0f, 0.2f, 0.5f };

// undo GLfloat light_pos[] = {5,5,10,1};
GLfloat light_pos[] = {-15,5,10,1};

GLfloat lightDiff[] = {1,1,1,1};
GLfloat lightSpec[] = {0.9,0.9,0.9,1};


//
//----> prototypes
//
//

// void __cdecl display();
// void __cdecl idle();
// void __cdecl key(unsigned char k, int x, int y);
// void __cdecl menu(int v);









//
//
//----> Overload glut_perspective_reshaper to catch window resizing for PBuffer:
//
//
class perspective_reshaper : public glh::glut_perspective_reshaper
{
public:
    perspective_reshaper(
        float in_fov_y = 60.f, 
        float in_zNear = 0.1f, 
        float in_zFar = 10.f
    ) 
        : 
        glut_perspective_reshaper(in_fov_y, in_zNear, in_zFar) // ghv: call the base class constructor;
    {
    }
    
    void __cdecl reshape(int w, int h)
    {
        glut_perspective_reshaper::reshape(w, h);
    }
};
perspective_reshaper subClassedReshaper;





void cleanExit(int exitval)
{
    spriteSingleton.free();
    exit(exitval);
}





void gvDrawFFSpheres(const GLfloat *colorA, const GLfloat *colorB)
{
    if (bOptions['s']) 
    {
        glColor3fv(colorA); 
        
        glutSolidSphere(1, 30, 30); 
        
        glPushMatrix(); 
        
        glTranslatef(1.5, 0, 0);

        glColor3fv(colorB); 
        
        glutSolidSphere(0.5, 30, 30); 
        
        glPopMatrix();
    }
}








//
//
//
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Section III
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//
//
void V2Sprite::gv_renderComplete_TwoPassRender()
{
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //          First Pass: 
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, megadepth_fbo);
    glViewport(0, 0, m_edge, m_edge);
    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    glDisable(GL_REGISTER_COMBINERS_NV); glDisable(GL_TEXTURE_SHADER_NV); glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST); // needed to render the spheres on the fixed function pipeline;
    gvDrawFFSpheres(spColor0, spColor0);
    glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_POLYGON_BIT); // Save state;
    glCallList(dl_vertex_program_asm);
    glCallList(dl_without_textures);  // paint each sprite white;
    gvBeginQuadsEndQuads(minSizePass1, maxSizePass1);
    glPopAttrib();
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);  // unbind the megadepth_fbo; 

    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //          Second Pass: 
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, bOptions['w'] ? GL_LINE : GL_FILL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, spriteSingleton.shineFactor);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST); // needed to render the spheres on the fixed function pipeline;
    gvDrawFFSpheres(spColor1, spColor2);
    glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_POLYGON_BIT); // Save state;
    glCallList(dl_vertex_program_asm);

    //  If the GL_BLEND isn't enabled here will see billboarding; 
    glEnable(GL_BLEND);  glBlendFunc(GL_ONE, GL_ONE); // comment this line to see billboarding;


    //  If the GL_DEPTH_TEST is enabled here will see SLICING; 
    glDisable(GL_DEPTH_TEST); // if this is commented will have SLICING;

    glCallList(dl_dual_texture_combiner); // pBuffer acts as Texture Image;



    glActiveTextureARB(GL_TEXTURE1_ARB); // undo ?
    glBindTexture(GL_TEXTURE_2D, megadepth_img); // on unit 1;



#ifdef GHV_OPTION_USE_GLH_TEX_OBJ

    texture2D_unit_0_shine.bind(); // texture_0 is the sprite image;

#else

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, texture2D_unit_0_shine);


#endif




    
#ifdef GHV_OPTION_USE_SHARK_COMPILER
    //  When using the result of the Shark compiler, need: 
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_2D);
#endif

    gvBeginQuadsEndQuads(minSizePass2, maxSizePass2);

    glPopAttrib();
}
//
//
//
void __cdecl gv_glut_callback_for_display()
{
    if (!bInitialized) 
    {
        return;
    }

    glPushMatrix(); // Save state before rotating scene; 

   
    objectSpheres.apply_transform();


    glRotatef(alpha, 0,1,0);
    glRotatef(beta, 1,0,0);

    float debug_ModelViewMatrix[16], debug_ProjectionMatrix[16]; // just to peek inside. 
    glGetFloatv(GL_MODELVIEW_MATRIX, debug_ModelViewMatrix);
    glGetFloatv(GL_PROJECTION_MATRIX, debug_ProjectionMatrix);

    spriteSingleton.gv_renderComplete_TwoPassRender();  

    glPopMatrix(); // Restore to the state before scene rotation; 

    if(bOptions[' '])  
    {
        alpha += 0.6; // if animating, then
        beta += 0.2234; //  increment angles; 
    }
    glutSwapBuffers();
}
//
//
//
void gv_glut_callback_for_menu(int v)
{
    bOptions[v] = !bOptions[v];
    switch (v)
    {
    case '+':
        spriteSingleton.shineFactor += 1;
        if(spriteSingleton.shineFactor > 128) spriteSingleton.shineFactor = 128;
        break;
    case '-':
        spriteSingleton.shineFactor -= 1;
        if(spriteSingleton.shineFactor <= 0) spriteSingleton.shineFactor = 1;
        break;
    case 27:
        cleanExit(0);
    }
    glutPostRedisplay();
}
//
//
//
void gv_glut_callback_for_key(unsigned char k, int x, int y)
{
    gv_glut_callback_for_menu((int)k);
}
//
//
//
void __cdecl gv_glut_callback_for_idle()
{
    if (bInitialized) glutPostRedisplay();
}
//
//
//
int __cdecl main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );


    glutInitWindowSize(spriteSingleton.m_edge, spriteSingleton.m_edge);


    glutCreateWindow("retro assembly language shaders");

    bOptions['d'] = false; // mode is NOT debug mode; 
    bOptions[' '] = true; // animate;
    bOptions['w'] = false; // wireframe;
    bOptions['s'] = true; // show spheres;



    glClearColor(0.5, 0.0, 1.0, 1.0); 
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, lightSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, spriteSingleton.shineFactor);
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);





    bInitialized = spriteSingleton.gvConfirmExtensionAvailability(); 
    if (!bInitialized) 
    {
        printf("\n   Initialisation failed. \n\n");
        cleanExit(0);
    }

    spriteSingleton.gvCreatePbuffer(); 

    spriteSingleton.gvCreateShineTexture();

    spriteSingleton.gvCreateDisplayLists();

    spriteSingleton.setRandomShinePositions();








    glh::glut_helpers_initialize();

    cb_callback_object.keyboard_function = gv_glut_callback_for_key;


    objectSpheres.configure_buttons(1);
    objectSpheres.dolly.dolly[0] = g0_dolly_x_constant;
    objectSpheres.dolly.dolly[1] = g0_dolly_y_constant;
    objectSpheres.dolly.dolly[2] = g0_dolly_z_constant;
    objectSpheres.trackball.r.set_value(glh::vec3f(1, 0, 0), 0);
    glh::glut_add_interactor(&objectSpheres);


    glh::glut_add_interactor(&cb_callback_object);


    glh::glut_add_interactor(&subClassedReshaper);


    glutCreateMenu(gv_glut_callback_for_menu);
    glutAddMenuEntry( "wireframe [w]", 'w' );
    glutAddMenuEntry( "anim [ ]", ' ' );
    glutAddMenuEntry( "increase shininess [+]", '+' );
    glutAddMenuEntry( "decrease shininess [-]", '-' );
    glutAddMenuEntry( "view sphere [s]", 's' );
    glutAddMenuEntry( "debug mode [d]", 'd' );
    glutAttachMenu( GLUT_RIGHT_BUTTON );


    glutIdleFunc(gv_glut_callback_for_idle);
    glutDisplayFunc(gv_glut_callback_for_display);

    glutMainLoop();
    return 0;
}





