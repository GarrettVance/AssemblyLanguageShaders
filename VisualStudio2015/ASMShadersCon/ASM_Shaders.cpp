//  
//  
// ghv 20200223 ASM_Shaders.cpp
//      
// ghv 20200311: removed all dependencies on NVIDIA nvparse; 
// ghv 20200320: replaced legacy pbuffer with fbo; 
//      

#include "V2Sprite.h"


void V2Sprite::Compile_ARBVertexProgram_ASM()
{
    fprintf(stdout, "\nASM Vertex Program (assembly language) \n"); 

    dl_vertex_program_asm = glGenLists(1); 
    glNewList(dl_vertex_program_asm, GL_COMPILE);
    GLuint vertexProgramNum;
    glGenProgramsARB(1, &vertexProgramNum);
    static const char bangbangVertexProgText[] = 
        "!!ARBvp1.0                                                     \n"			  
        "ATTRIB inputPosition = vertex.position;                        \n"
        "ATTRIB inputNormal = vertex.normal;                            \n"
        "ATTRIB inputSpriteSize = vertex.attrib[5];                     \n"
        "ATTRIB inputTexCoord = vertex.texcoord;                        \n"
        "PARAM modelViewMatrix[4] = { state.matrix.modelview };         \n"
        "PARAM modelViewIT[4] = { state.matrix.modelview.invtrans };    \n"
        "PARAM projectionMatrix[4] = { state.matrix.projection };       \n"
        "PARAM lightPosition = state.light[0].position;                 \n"
        "PARAM sinCosVal[30] = { {1.0000, 0.0000, 0.0, 0.0}, {1.0000, 0.0000, 0.0, 0.0}   \n"
            ", {1.0000, 0.0000, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0}  \n"
            ", {1.0000, 0.0000, 0.0, 0.0} , {0.4999, 0.8660, 0.0, 0.0} , {0.4383, 0.8987, 0.0, 0.0}  \n"
            ", {0.5735, 0.8191, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0}  \n"
            ", {1.0000, 0.0000, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0} , {0.9396, 0.3420, 0.0, 0.0}  \n"
            ", {1.0000, 0.0000, 0.0, 0.0} , {0.9396, 0.3420, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0}  \n"
            ", {1.0000, 0.0000, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0} , {0.8829, 0.4694, 0.0, 0.0}  \n"
            ", {0.8191, 0.5735, 0.0, 0.0} , {0.9396, 0.3420, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0}  \n"
            ", {1.0000, 0.0000, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0}  \n"
            ", {1.0000, 0.0000, 0.0, 0.0} , {0.9848, 0.1736, 0.0, 0.0} , {1.0000, 0.0000, 0.0, 0.0}  \n"
            ", {1.0000, 0.0000, 0.0, 0.0} };  \n"
        "ADDRESS sinCosAddress;                                         \n"
        "PARAM shininess = program.local[1];      \n"
        "OUTPUT HPosition = result.position;                            \n"
        "OUTPUT Color0 = result.color;                                  \n"
        "OUTPUT TexCoord0 = result.texcoord;                            \n"
        "OUTPUT TexCoord1 = result.texcoord[1];                         \n"
        "TEMP viewSpacePos;                                             \n"
        "DP4 viewSpacePos.x, inputPosition, modelViewMatrix[0];         \n"
        "DP4 viewSpacePos.y, inputPosition, modelViewMatrix[1];         \n"
        "DP4 viewSpacePos.z, inputPosition, modelViewMatrix[2];         \n"
        "DP4 viewSpacePos.w, inputPosition, modelViewMatrix[3];         \n"

        "TEMP abNormal;                                                 \n"
        "DP3 abNormal.x, inputNormal, modelViewIT[0];                   \n"
        "DP3 abNormal.y, inputNormal, modelViewIT[1];                   \n"
        "DP3 abNormal.z, inputNormal, modelViewIT[2];                   \n"

        "TEMP mvNormal;                                                 \n"
//         "NRM mvNormal, abNormal;                                        \n"
         "DP3 mvNormal.w, abNormal, abNormal;                            \n"
         "RSQ mvNormal.w, mvNormal.w;                                    \n"
         "MUL mvNormal.xyz, mvNormal.w, abNormal;                        \n"  // or abNormal.xyz;

        "TEMP eyeDir;                                                   \n"
        "DP3 eyeDir.w, viewSpacePos, viewSpacePos;                      \n"
        "RSQ eyeDir.w, eyeDir.w;                                        \n"
        "MUL eyeDir.xyz, -eyeDir.w, viewSpacePos;                       \n"  // or viewSpacePos.xyz;

        "TEMP diff;                                                     \n"
        "SUB diff, lightPosition, viewSpacePos;                         \n"
        "TEMP lightDir;                                                 \n"
        "DP3 lightDir.w, diff, diff;                                    \n"
        "RSQ lightDir.w, lightDir.w;                                    \n"
        "MUL lightDir.xyz, lightDir.w, diff;                            \n"   // or diff.xyz;

        "TEMP cross;                                                   \n"
        "TEMP mvpos2;                                                   \n"
        "XPD cross, lightDir, mvNormal;                                \n"
        "ADD mvpos2, cross, viewSpacePos;                              \n"
        "MOV mvpos2.w, { 0, 0, 0, 0 };                                 \n"

        "TEMP LPlusE;                                                   \n"  
        "ADD LPlusE, lightDir, eyeDir;                                  \n"
        "TEMP HalfV;                                                    \n"  
        "DP3 HalfV.w, LPlusE, LPlusE;                                   \n"
        "RSQ HalfV.w, HalfV.w;                                          \n"
        "MUL HalfV.xyz, HalfV.w, LPlusE;                                \n" 

        "TEMP NLCosSin;                                   \n"  
        "TEMP csIndex;                                    \n"  
        "DP3 NLCosSin.x, mvNormal, HalfV;                       \n"
        "MUL NLCosSin.z, NLCosSin.x, NLCosSin.x;                \n"  // Using z and w just for scratch calculations.
        "SUB NLCosSin.w, { 1, 1, 1, 1 }, NLCosSin.z;                     \n"
        "RSQ NLCosSin.w, NLCosSin.w;                            \n"
        "RCP NLCosSin.y, NLCosSin.w;                            \n"
        "MOV NLCosSin.zw, { 0, 0, 0, 0 };                                \n"  // Keep only the x and y components; 
        "MUL csIndex, NLCosSin.x, { 30.0, 30.0, 30.0, 30.0 };                     \n"   // Slices = 30; 
        "MAX csIndex, csIndex, { 4.0, 4.0, 4.0, 4.0 };                         \n"  
        "ARL sinCosAddress.x, csIndex.x;                        \n"
        "MUL NLCosSin, NLCosSin, sinCosVal[sinCosAddress.x + 0];    \n"  
        "TEMP NDotH;                                            \n"  
        "ADD NDotH, NLCosSin.x, NLCosSin.y;                     \n"  


        "TEMP preLit;                                       \n"  
        "MOV preLit.x, { 1, 1, 1, 1 };                               \n"  
        "MOV preLit.y, NDotH;                               \n"  
        "MOV preLit.w, shininess.w;                           \n"  
        "TEMP litCoefficients;                              \n"  
        "LIT litCoefficients, preLit;                       \n"  

        // projPos1 and projPos2:
        "TEMP projPos1;                                                 \n"
        "DP4 projPos1.x, projectionMatrix[0], viewSpacePos;             \n"
        "DP4 projPos1.y, projectionMatrix[1], viewSpacePos;             \n"
        "DP4 projPos1.z, projectionMatrix[2], viewSpacePos;             \n"
        "DP4 projPos1.w, projectionMatrix[3], viewSpacePos;             \n"
        "TEMP projPos2;                                             \n"
        "DP4 projPos2.x, projectionMatrix[0], mvpos2;               \n"
        "DP4 projPos2.y, projectionMatrix[1], mvpos2;               \n"
        "DP4 projPos2.z, projectionMatrix[2], mvpos2;               \n"
        "DP4 projPos2.w, projectionMatrix[3], mvpos2;               \n"
        "TEMP denom;                                          \n"
        "TEMP pos2d1;                                         \n"
        "RCP denom, projPos1.w;                               \n"
        "MUL pos2d1, projPos1, denom;                         \n"
        "TEMP pos2d2;                                         \n"
        "RCP denom, projPos2.w;                               \n"
        "MUL pos2d2, projPos2, denom;                         \n"
        "TEMP diffPos2;                                                 \n"
        "SUB diffPos2, pos2d2, pos2d1;                                  \n"
        "MOV diffPos2.zw, { 0, 0, 0, 0 };                                        \n"
        "TEMP dir2d;                                                    \n"
        "DP3 dir2d.w, diffPos2, diffPos2;                               \n"
        "RSQ dir2d.w, dir2d.w;                                          \n"
        "MUL dir2d.xyz, dir2d.w, diffPos2;                              \n"  // or diffPos2.xyz;

        "TEMP sVal;                                           \n"
        "MAD sVal, litCoefficients.zzzz, inputSpriteSize.zzzz, inputSpriteSize.xxxx;   \n"
        "MAD projPos1.xy, dir2d, sVal, projPos1;   \n"
        "MAD sVal, litCoefficients.zzzz, inputSpriteSize.wwww, inputSpriteSize.yyyy;   \n"
        "MAD projPos1.x, dir2d.y, sVal, projPos1.x;   \n"
        "MAD projPos1.y, -dir2d.x, sVal, projPos1.y;   \n"

        "MOV HPosition, projPos1;                                       \n"
        "MOV Color0, {1, 1, 1, 1};                                      \n"
        "MOV TexCoord0, inputTexCoord;                                  \n"
        "MOV TexCoord1.w, { 0, 0, 0, 1 };                                        \n"
        "MAD TexCoord1.xy, pos2d1, { 0.5, 0.5, 0.5, 0.5 }, { 0.5, 0.5, 0.5, 0.5 };  \n"
        "MOV TexCoord1.w, { 1, 1, 1, 1 };                                        \n"
        "END";
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertexProgramNum);

    glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, shineFactor, shineFactor, shineFactor, shineFactor);

    glProgramStringARB(
        GL_VERTEX_PROGRAM_ARB, 
        GL_PROGRAM_FORMAT_ASCII_ARB, 
        strlen(bangbangVertexProgText), 
        (const GLubyte *) bangbangVertexProgText
    );
    GLint errnoVert = glGetError();
    printf("...glGetError = %d\n", errnoVert);
    if (errnoVert != GL_NO_ERROR)
    {
        GLint errorpos; 
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorpos); 
        printf("errorpos: %d\n", errorpos); 
        printf("%s\n", (char *)glGetString(GL_PROGRAM_ERROR_STRING_ARB));
    }
    glEnable(GL_VERTEX_PROGRAM_ARB);
    glEndList();
}


#ifdef GHV_OPTION_USE_SHARK_COMPILER
void V2Sprite::Compile_RegisterCombiners_Shark_Dual()
{
    //   
    //  ghv: the key to reversing the RC1.0 Register Combiner program 
    //  is the set of NV_register_combiners Combiner State Queries which  
    //  will be found on pages 56 and 57 of the NVIDIA whitepaper 
    //  "Texture Compositing With Register Combiners" 
    //  by John Spitzer. 
    //   

    fprintf(stdout, "\n\n++++> Using result of SHARK Shader Assembly Compiler \n\n"); 

    dl_dual_texture_combiner = glGenLists(1);
    glNewList(dl_dual_texture_combiner, GL_COMPILE);
    glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
    //  
    //  Generated by Shark (Jun 27 2004, 16:49:59) from: rc.psh
    //  
    //  cf    https://community.khronos.org/t/converting-from-nvparse-rc1-0/36270
    //  
    //      
    //  General Combiner Inputs: 
    //  ========================
    glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE0_ARB, GL_SIGNED_IDENTITY_NV, GL_RGB);
    glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_TEXTURE0_ARB, GL_SIGNED_IDENTITY_NV, GL_ALPHA);
    glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_TEXTURE1_ARB, GL_SIGNED_IDENTITY_NV, GL_RGB);
    glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_TEXTURE1_ARB, GL_SIGNED_IDENTITY_NV, GL_ALPHA);
    //      
    //      
    //  General Combiner Outputs: 
    //  =========================
    // typedef void (APIENTRYP PFNGLCOMBINEROUTPUTNVPROC) (
    //      GLenum stage, GLenum portion, 
    //      GLenum abOutput, GLenum cdOutput, GLenum sumOutput, 
    //      GLenum scale, GLenum bias, 
    //      GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum
    //  );
    // 
    //---------------------------------------------- a*b output,---- c*d output,---- sum output -----------------------------------------------------   
    glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB,     GL_SPARE0_NV,   GL_DISCARD_NV,  GL_DISCARD_NV,    GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE);
    glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA,   GL_SPARE0_NV,   GL_DISCARD_NV,  GL_DISCARD_NV,    GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE);
    //      
    //      
    //  Final Combiner Inputs: 
    //  ======================
    glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_ZERO,           GL_UNSIGNED_INVERT_NV,      GL_RGB);
    glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_SPARE0_NV,      GL_UNSIGNED_IDENTITY_NV,    GL_RGB);
    glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO,           GL_UNSIGNED_IDENTITY_NV,    GL_RGB);
    glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_ZERO,           GL_UNSIGNED_IDENTITY_NV,    GL_RGB);
    glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_SPARE0_NV,      GL_UNSIGNED_IDENTITY_NV,    GL_ALPHA);

    glEnable(GL_REGISTER_COMBINERS_NV);
    glEnable(GL_TEXTURE_SHADER_NV);

    glEnable(GL_BLEND);  glBlendFunc(GL_ONE, GL_ONE);    // Blending is enabled!!!

    glDepthMask(GL_FALSE); glDisable(GL_DEPTH_TEST);

    glEndList();
}
#else
void V2Sprite::Compile_RegisterCombiners_Dual_Texture_Combiner()
{
    fprintf(stdout, "\nASM Fragment Program (dual textures) \n"); 
    //
    // create the dual-texture combiner display-list
    //
    // ghv 20200311: 
    // Replaced two calls to nvparse (one using RC1.0 register combiner script 
    // and the other using Texture Shader script) with a single 
    // assembly language fragment program. 
    //

    dl_dual_texture_combiner = glGenLists(1);
    glNewList(dl_dual_texture_combiner, GL_COMPILE);
    GLuint fragprognum;
    glGenProgramsARB(1, &fragprognum);
    static const char bangbangFragmentProgText[] = 
        "!!ARBfp1.0                             \n"			  
        "TEMP temp0;                            \n"
        "ATTRIB tex0 = fragment.texcoord;       \n"
        "TEX temp0, tex0, texture[0], 2D;       \n"
        "TEMP temp1;                            \n"
        "ATTRIB tex1 = fragment.texcoord[1];    \n"
        "TEX temp1, tex1, texture[1], 2D;       \n"
        "TEMP tempZ;                            \n"
        "MUL tempZ, temp1, temp0;               \n"  // for LUMINANCE get temp1 = depth rrr1;
        "MOV tempZ.a, temp0.a;                  \n"
        "MOV result.color, tempZ;               \n"
        "END";
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragprognum);
    glProgramStringARB(
        GL_FRAGMENT_PROGRAM_ARB, 
        GL_PROGRAM_FORMAT_ASCII_ARB, 
        strlen(bangbangFragmentProgText), 
        (const GLubyte *) bangbangFragmentProgText
    );
    GLint errnoFrag = glGetError();
    printf("...glGetError = %d\n", errnoFrag);
    if (errnoFrag != GL_NO_ERROR)
    {
        GLint errorpos; 
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorpos); 
        printf("errorpos: %d\n", errorpos); 
        printf("%s\n", (char *)glGetString(GL_PROGRAM_ERROR_STRING_ARB));
    }
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    glEnable(GL_REGISTER_COMBINERS_NV);
    glEnable(GL_TEXTURE_SHADER_NV);
    glEndList();    // display list ends with depthmask false and depth test disabled; 
}
#endif


void V2Sprite::Compile_RegisterCombiners_Without_Textures()
{
    fprintf(stdout, "\nASM Fragment Program (no textures) \n"); 
    //
    // ghv 20200311: Replaced nvparse of !!RC1.0 register combiner script 
    // with assembly language fragment program. 
    // 
    // This display-list uses no textures. 
    //
    dl_without_textures = glGenLists(1);
    glNewList(dl_without_textures, GL_COMPILE);
    GLuint fragprognum;
    glGenProgramsARB(1, &fragprognum);
    static const char bangbangFragmentProgText[] = 
        "!!ARBfp1.0                             \n"			  
        "MOV result.color, fragment.color;      \n"
        "END";
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragprognum);
    glProgramStringARB(
        GL_FRAGMENT_PROGRAM_ARB, 
        GL_PROGRAM_FORMAT_ASCII_ARB, 
        strlen(bangbangFragmentProgText), 
        (const GLubyte *) bangbangFragmentProgText
    );
    GLint errnoFrag = glGetError();
    printf("...glGetError = %d\n", errnoFrag);
    if (errnoFrag != GL_NO_ERROR)
    {
        GLint errorpos; 
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorpos); 
        printf("errorpos: %d\n", errorpos); 
        printf("%s\n", (char *)glGetString(GL_PROGRAM_ERROR_STRING_ARB));
    }
    glEnable(GL_FRAGMENT_PROGRAM_ARB); 
    glEnable(GL_REGISTER_COMBINERS_NV);
    glDisable(GL_TEXTURE_SHADER_NV);
    glEndList();    // display list ends with depthmask true and depth test ENABLED; 
}









