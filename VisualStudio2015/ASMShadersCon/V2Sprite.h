#ifndef __V2Sprite__
#define __V2Sprite__




// remove  #include <glh_helpers/glh_obs.h>

#include "windows.h"

#include <shiny_GL/GL/gl.h>  // ghv: TODO:  ambiguous...


#include <shiny_GL/GL/glext.h>



#include <glh_extensions/glh_extensions.h>
#include <glh_helpers/glh_linear.h>  // ghv: glh_linear.h was crashing due to macro named "equivalent"; 


#include "ShinyOptions.h"




#include <string>

class V2Sprite
{
public:
    GLfloat                 shineFactor;
    GLuint                  megadepth_fbo;
    GLuint                  megadepth_depthbuffer;
    GLuint                  megadepth_img;  
    GLuint                  m_edge; 
private:


    GLfloat minSizePass1 = 0.02f; // Use 0.01 or 0.02; 
    GLfloat maxSizePass1 = 0.02f; // Use 0.01 or 0.02; 

    GLfloat minSizePass2 = 0.2f; // Use 0.2f;
    GLfloat maxSizePass2 = 0.9f; // Use 0.7f;






    std::string             m_msg; 
    bool                    binit;
    GLuint                  dl_vertex_program_asm;
    GLuint                  dl_dual_texture_combiner;
    GLuint                  dl_without_textures;



#ifdef GHV_OPTION_USE_GLH_TEX_OBJ
    glh::tex_object_2D      texture2D_unit_0_shine;
#else
    GLuint                  texture2D_unit_0_shine;
#endif



public:

    V2Sprite();

    ~V2Sprite();

    void gvVertexAttribute(GLfloat pa, GLfloat pb, GLfloat pc, GLfloat pd);

    void Compile_ARBVertexProgram_ASM();


#ifdef GHV_OPTION_USE_SHARK_COMPILER

    void Compile_RegisterCombiners_Shark_Dual();

#else

    void Compile_RegisterCombiners_Dual_Texture_Combiner();

#endif


    void Compile_RegisterCombiners_Without_Textures();


    void setRandomShinePositions();


    bool gvConfirmExtensionAvailability();

    void gvCreatePbuffer(); 

    void gvCreateShineTexture();

    void gvCreateDisplayLists();


    void draw_sprite(glh::vec3f &pos, glh::vec3f &norm, GLfloat MinSz, GLfloat MaxSz);


    void gv_renderComplete_TwoPassRender();


    void V2Sprite::gvBeginQuadsEndQuads(GLfloat pSizeMin, GLfloat pSizeMax);


    void free();
};

#endif
