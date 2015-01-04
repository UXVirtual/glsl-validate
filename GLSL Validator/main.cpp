//
//  main.cpp
//  GLSL Validator
//
//  Created by Sherief Farouk on 3/15/13.
//  Copyright (c) 2013 Sherief Farouk. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <cassert>

#include "twisted_works/cli_parser.hpp"

#include "opengl_context.hpp"

using namespace std;

bool program_is_valid(GLuint Program)
{
    GLint LinkStatus;
    glGetProgramiv(Program, GL_LINK_STATUS, &LinkStatus);
    return (LinkStatus == GL_TRUE);
}

std::string read_file(const std::string& Path)
{
    ifstream InputStream(Path.c_str());
    string FileContents((std::istreambuf_iterator<char>(InputStream)),
                      std::istreambuf_iterator<char>());
    return FileContents;
}

bool validate_command_line_parameters(const twisted_works::cli_parser& CommandLine)
{
    //Exactly one input file must be provided.
    if(CommandLine.anonymous_argument_count() == 1)
    {
        //The 'profile' argument must be provided.
        if(CommandLine.is_named_argument("profile"))
        {
            const std::string ProfileString = CommandLine.argument("profile");
            //The 'profile' argument must be either 'vertex' or 'fragment'.
            if((ProfileString == "vertex") || (ProfileString == "geometry") || (ProfileString == "tessellation-control") || (ProfileString == "tessellation-evaluation") || (ProfileString == "fragment"))
            {
                return true;
            }
        }
    }
    return false;
}

void show_help()
{
    cout << "Usage: glsl-validate <input file> +profile=[vertex|tessellation-control|tessellation-evaluation|fragment]\n";
}

class validation_parameters
{
public:
    validation_parameters();
    validation_parameters(const twisted_works::cli_parser& CommandLine);
    std::string file_path;
    GLenum profile;
};

validation_parameters::validation_parameters(const twisted_works::cli_parser& CommandLine)
{
    assert(CommandLine.anonymous_argument_count() > 0);
    file_path = CommandLine.anonymous_argument(0);
    profile = 0;
    const std::string ProfileString = CommandLine.argument("profile");
    if(ProfileString == "vertex")
    {
        profile = GL_VERTEX_SHADER;
    }
    if(ProfileString == "geometry")
    {
        profile = GL_GEOMETRY_SHADER;
    }
    if(ProfileString == "tessellation-control")
    {
        profile = GL_TESS_CONTROL_SHADER;
    }
    if(ProfileString == "tessellation-evaluation")
    {
        profile = GL_TESS_EVALUATION_SHADER;
    }
    if(ProfileString == "fragment")
    {
        profile = GL_FRAGMENT_SHADER;
    }
    assert(profile != 0);
}

int main(int argc, char ** argv)
{
    twisted_works::cli_parser CommandLine(argc, argv);
    if(!validate_command_line_parameters(CommandLine))
    {
        show_help();
        return -1;
    }
    validation_parameters Parameters(CommandLine);
    create_opengl_context();
    string ProgramTextString = read_file(Parameters.file_path);
    GLuint Program;
    const char* const ProgramText = ProgramTextString.c_str();
    Program = glCreateShaderProgramv(Parameters.profile, 1, &ProgramText);
    int ReturnValue = 0;
    if(!program_is_valid(Program))
    {
        cout << "FAIL\n";
        ReturnValue = 1;
        const unsigned int MaximumInfoLogLength = 4096;
        char InfoLog[MaximumInfoLogLength] = {};
        GLsizei ActualInfoLogLength = 0;
        glGetProgramInfoLog(Program, MaximumInfoLogLength, &ActualInfoLogLength, InfoLog);
        cout << "Info Log:\n";
        cout << InfoLog;
    }
    else
    {
        cout << "PASS\n";
    }
    destory_opengl_context();
    return ReturnValue;
}
