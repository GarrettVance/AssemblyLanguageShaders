//
//
// ghv 20200311: removed all dependencies on NVIDIA nvparse; 
// ghv 20200320: replaced legacy pbuffer with fbo; 
//
//

#include "stdafx.h"


#define GLH_EXT_SINGLE_FILE
#include <glh_extensions/glh_extensions.h>


//   #include "nv_png.h"




// #include <GL/glu.h>


#include "V2Sprite.h"


#include "resource.h"







#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"






#define EXTENSIONS "GL_NV_texture_shader " \
                   "GL_NV_register_combiners " \
                   "WGL_ARB_pbuffer " \
                   "WGL_ARB_pixel_format "


const GLfloat XM_PI = 3.141592654f;

glh::vec3f masterShineLocations[NUMSHINES];





V2Sprite::V2Sprite() 
    :
    shineFactor(50.0),  // use e.g. 90.0 for minimum shine;
    megadepth_fbo(0),
    megadepth_depthbuffer(0),
    megadepth_img(0), 
    m_edge(512),
    dl_vertex_program_asm(0), 
    dl_dual_texture_combiner(0),
    dl_without_textures(0), 
    binit(false)
{}


V2Sprite::~V2Sprite()
{}


    






bool V2Sprite::gvConfirmExtensionAvailability()
{
    if (binit) {
        return 1;
    }

    free();

    if (!glh_init_extensions(EXTENSIONS))
    {
        printf("FAIL: program requires extensions \n");
        printf("   %s\n\n", glh_get_unsupported_extensions());
        return false;
    }
    if (!glh_init_extensions("WGL_ARB_render_texture"))
    {
        printf("FAIL: program requires extension WGL_ARB_render_texture");
        return false;
    }
    if (!glh_init_extensions("GL_ARB_multitexture"))
    {
        printf("FAIL: program requires extension GL_ARB_multitexture");
        return false;
    }
    if (!glh_init_extensions("GL_ARB_vertex_program"))
    {
        printf("FAIL: program requires extension GL_ARB_vertex_program");
        return false;
    }
    if (!glh_init_extensions("GL_ARB_fragment_program"))
    {
        printf("FAIL: program requires extension GL_ARB_fragment_program");
        return false;
    }

    // hook up the missing API methods: 

    glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)wglGetProcAddress("glGenProgramsARB");
    glIsProgramARB = (PFNGLISPROGRAMARBPROC)wglGetProcAddress("glIsProgramARB");
    glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)wglGetProcAddress("glProgramStringARB");
    glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)wglGetProcAddress("glBindProgramARB");
    glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)wglGetProcAddress("glDeleteProgramsARB");
    assert(glGenProgramsARB && glIsProgramARB && glProgramStringARB && glBindProgramARB && glDeleteProgramsARB);

    //
    //----> additional API methods: 
    //
    glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
    glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
    glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
    glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
    glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
    glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
    glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
    glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");

    return 1;
}





void V2Sprite::gvCreatePbuffer()
{
    //
    //----> create the pbuffer :
    //

    glGenFramebuffersEXT(1, &megadepth_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, megadepth_fbo);
    glGenRenderbuffersEXT(1, &megadepth_depthbuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, megadepth_depthbuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, m_edge, m_edge);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, megadepth_depthbuffer);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glGenTextures(1, &megadepth_img);
    glBindTexture(GL_TEXTURE_2D, megadepth_img);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,   //  was GL_RGB; 
        m_edge,
        m_edge,
        0,
        GL_RGBA,  // was GL_RGB; 
        GL_UNSIGNED_BYTE,
        NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, megadepth_img, 0);
    GLenum fboStatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

    _ASSERTE(fboStatus == GL_FRAMEBUFFER_COMPLETE_EXT);
}






void V2Sprite::gvCreateShineTexture()
{
    //
    // load the shiny png image as texture unit 0: 
    //
    glActiveTextureARB(GL_TEXTURE0_ARB);


#ifdef GHV_OPTION_USE_GLH_TEX_OBJ
    texture2D_unit_0_shine.bind();
    set_png_module_handle(
        (unsigned long)GetModuleHandle(NULL)
    );
    set_png_module_restypename("PNG");
    glh::array2<glh::vec3ub> imgBytes;
    read_png_rgb("shine_2007_original.png", imgBytes);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        imgBytes.get_width(),
        imgBytes.get_height(),
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        (const void *)imgBytes.get_pointer()
    );

    texture2D_unit_0_shine.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture2D_unit_0_shine.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#else

    glGenTextures(1, &texture2D_unit_0_shine);
    glBindTexture(GL_TEXTURE_2D, texture2D_unit_0_shine);

    HMODULE hExe = GetModuleHandle(NULL); 
    HRSRC hResInfo = ::FindResource(hExe, MAKEINTRESOURCE(IDB_PNG1), L"PNG");
    HGLOBAL resourceHandle = ::LoadResource(NULL, hResInfo);
    unsigned char *resBytes = (unsigned char *)::LockResource(resourceHandle);

    // Need to know the size of the resource (the image): 

    DWORD bufLength = SizeofResource(hExe, hResInfo); 

    int imgWidth; 
    int imgHeight;
    int imgNumComponents;

    unsigned char * imgUC = stbi_load_from_memory(
        resBytes, 
        bufLength,
        &imgWidth, 
        &imgHeight,
        &imgNumComponents, 
        3
    );


    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        imgWidth,
        imgHeight,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        (const void *)imgUC
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#endif
}





void V2Sprite::gvCreateDisplayLists()
{
    Compile_ARBVertexProgram_ASM();

#ifdef GHV_OPTION_USE_SHARK_COMPILER
    Compile_RegisterCombiners_Shark_Dual();
#else
    Compile_RegisterCombiners_Dual_Texture_Combiner();
#endif
    Compile_RegisterCombiners_Without_Textures();

}





    
void V2Sprite::setRandomShinePositions()
{
    float const magicRadius = 1.01f;

    for(int i = 0; i < NUMSHINES; i++)
    {
        float be = XM_PI * (float)(rand() & 255) / 255.0;

        float al = 2 * XM_PI * (float)(rand() & 255) / 255.0;

        masterShineLocations[i].set_value(
            magicRadius * cos(al) * sin(be), magicRadius * sin(al), magicRadius * cos(al) * cos(be)
        );
    }
}
















void V2Sprite::gvVertexAttribute(GLfloat pa, GLfloat pb, GLfloat pc, GLfloat pd)
{
    GLint index_of_spriteSize = 5; 
    glVertexAttrib4fARB(index_of_spriteSize, pa, pb, pc, pd);
}


void V2Sprite::draw_sprite( glh::vec3f &pos, glh::vec3f &norm, GLfloat MinSz, GLfloat MaxSz)
{
    gvVertexAttribute(MinSz, MinSz, MaxSz, MaxSz);
    glTexCoord2f(1, 1);
    glNormal3fv(&(norm[0]));
    glVertex3fv(&(pos[0]));

    gvVertexAttribute(-MinSz, MinSz, -MaxSz, MaxSz);
    glTexCoord2f(0, 1);
    glNormal3fv(&(norm[0]));
    glVertex3fv(&(pos[0]));

    gvVertexAttribute(-MinSz, -MinSz, -MaxSz, -MaxSz);
    glTexCoord2f(0, 0);
    glNormal3fv(&(norm[0]));
    glVertex3fv(&(pos[0]));

    gvVertexAttribute(MinSz, -MinSz, MaxSz, -MaxSz);
    glTexCoord2f(1, 0);
    glNormal3fv(&(norm[0]));
    glVertex3fv(&(pos[0]));
}





void V2Sprite::gvBeginQuadsEndQuads(GLfloat pSizeMin, GLfloat pSizeMax)
{
    glBegin(GL_QUADS);

    for (int idx = 0; idx < NUMSHINES; idx++)
    {
        draw_sprite(masterShineLocations[idx], masterShineLocations[idx], pSizeMin, pSizeMax);

        glh::vec3f pos(masterShineLocations[idx]); 
        pos *= 0.5; 
        pos[0] += 1.5;

        draw_sprite(pos, masterShineLocations[idx], pSizeMin, pSizeMax);
    }

    glEnd(); // matches glBegin(GL_QUADS);
}















void V2Sprite::free()
{
    // TODO: 
}
