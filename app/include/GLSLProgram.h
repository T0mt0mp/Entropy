/**
 * @file app/GLSLProgram.h
 * @author Tomas Polasek
 * @brief Abstraction around OpenGL shaders and programs.
 */

#ifndef SIMPLE_GAME_GLSLPROGRAM_H
#define SIMPLE_GAME_GLSLPROGRAM_H

#include "Types.h"

/// Information about a shader to be loaded.
class GLSLShaderInfo
{
public:
    /**
     * Get shader source from given file.
     * @param shaderType Type of the shader (e.g. GL_VERTEX_SHADER)
     * @param filename File, where the shader source is located.
     */
    GLSLShaderInfo(GLenum shaderType, const std::string &filename) :
        mShaderType{shaderType},
        mFilename{filename}
    {
        std::ifstream file(mFilename);
        if (!file.is_open())
        {
            throw std::runtime_error(std::string("Error, unable to open shader file \"") + mFilename + "\"");
        }

        // Reserve the string size upfront.
        file.seekg(0, std::ios::end);
        mSource.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        // Read the whole file into the string.
        mSource.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    /**
     * Get shader source from supplied string.
     * @param shaderType Type of the shader (e.g. GL_VERTEX_SHADER).
     * @param src Source of the shader.
     */
    GLSLShaderInfo(GLenum shaderType, const char *src) :
        mShaderType{shaderType}, mFilename{"SourceShader"}, mSource{src} { }

    /**
     * Shader type getter.
     * @return Returns shader type (e.g. GL_VERTEX_SHADER).
     */
    GLenum type() const
    { return mShaderType; }

    /**
     * Shader source code getter.
     * @return Returns shader code.
     */
    const std::string &source() const
    { return mSource; }

    /**
     * Filename getter.
     * @return Returns filename, from which the shader has benn loaded.
     */
    const std::string &filename() const
    { return mFilename; }
private:
    /// Type of the shader (e.g. GL_VERTEX_SHADER).
    GLenum mShaderType;
    /// Shader filename, if it exists.
    std::string mFilename;
    /// Source for the shader.
    std::string mSource;
protected:
}; // class GLSLShaderInfo

/// OpenGL shader compilation.
class GLSLShader
{
public:
    /**
     * Create a shader from given information.
     * @param info Information about the requested shader.
     */
    GLSLShader(const GLSLShaderInfo &info) :
        mShaderId{glCreateShader(info.type())}
    {
        GLint result{GL_FALSE};
        GLint logLength{0};
        const char *src{info.source().c_str()};

        if (!mShaderId)
        {
            throw std::runtime_error("Unable to glCreateShader!");
        }

        // So far only 1 source file for each shader is supported.
        glShaderSource(mShaderId, 1, &src, nullptr);
        glCompileShader(mShaderId);

        // Get information about the compilation.
        glGetShaderiv(mShaderId, GL_COMPILE_STATUS, &result);
        glGetShaderiv(mShaderId, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0)
        {
            // Reserve space for the message (+1 for terminating null character).
            std::vector<char> errorMessage(logLength + 1);
            glGetShaderInfoLog(mShaderId, logLength, nullptr, &errorMessage[0]);
            deleteShader();
            throw std::runtime_error(info.filename() + ": \n" + std::string(&errorMessage[0]));
        }
    }

    /// Copying is not allowed.
    GLSLShader(const GLSLShader &rhs) = delete;

    /// Move shader.
    GLSLShader(GLSLShader &&rhs) :
        mShaderId{0}
    { swap(rhs); }

    /// Copying is not allowed.
    GLSLShader &operator=(const GLSLShader &rhs) = delete;

    /// Move shader.
    GLSLShader &operator=(GLSLShader &&rhs)
    { swap(rhs); return *this; }

    /// Shader ID getter.
    GLuint id() const
    { return mShaderId; }

    /// Destructor DOES delete the shader object!
    ~GLSLShader()
    { deleteShader(); }
private:
    /// Delete the shader inside.
    void deleteShader()
    {
        if (mShaderId)
        {
            glDeleteShader(mShaderId);
            mShaderId = 0;
        }
    }

    /// Swap 2 shaders.
    void swap(GLSLShader &other)
    { std::swap(mShaderId, other.mShaderId); }

    /// ID of the shader.
    GLuint mShaderId;
protected:
}; // class GLSLShader

/// OpenGL shader program compilation.
class GLSLProgram
{
public:
    /// Default constructor.
    GLSLProgram() :
        mProgramId{0}
    { }

    /**
     * Create GLSL program, from given list specifying the shaders.
     * @param list List specifying the shaders (e.g. {GL_VERTEX_SHADER, "shader.vert"}).
     */
    GLSLProgram(std::initializer_list<GLSLShaderInfo> list) :
        mProgramId{glCreateProgram()}
    {
        GLint result{GL_FALSE};
        GLint logLength{0};
        std::vector<GLSLShader> shaders;

        // Prepare the shaders.
        try
        {
            for (auto &info : list)
            {
                GLSLShader shader(info);
                glAttachShader(mProgramId, shader.id());
                shaders.emplace_back(std::move(shader));
            }
        } catch (std::runtime_error &e)
        {
            glDeleteProgram(mProgramId);
            throw;
        }

        // Link the program.
        glLinkProgram(mProgramId);

        // Get information about program compilation.
        glGetShaderiv(mProgramId, GL_COMPILE_STATUS, &result);
        glGetShaderiv(mProgramId, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0)
        {
            // Reserve space for the message (+1 for terminating null character).
            std::vector<char> errorMessage(logLength + 1);
            glGetShaderInfoLog(mProgramId, logLength, nullptr, &errorMessage[0]);
            glDeleteProgram(mProgramId);
            throw std::runtime_error(std::string(&errorMessage[0]));
        }

        // Detach the shaders.
        for (auto &shader : shaders)
        {
            glDetachShader(mProgramId, shader.id());
        }

        // Auto-delete shaders.
    }

    /**
     * TODO - optimize multiple calls for same name.
     * Get location for uniform with given name.
     * @param name Name of the uniform.
     * @return Location of the uniform for this program.
     */
    GLint getUniformLocation(const char *name) const
    { return glGetUniformLocation(mProgramId, name); }

    /// Destructor DOES delete the program!
    ~GLSLProgram()
    { deleteProgram(); }

    /// Use this program.
    void use()
    { glUseProgram(mProgramId); }

    /// Copying is forbidden.
    GLSLProgram(const GLSLProgram &rhs) = delete;

    /// Move constructor.
    GLSLProgram(GLSLProgram &&rhs) :
        mProgramId{0}
    { swap(rhs); }

    /// Copying is forbidden.
    GLSLProgram &operator=(const GLSLProgram &rhs) = delete;

    /// Move-assign operator.
    GLSLProgram &operator=(GLSLProgram &&rhs)
    { swap(rhs); return *this; }

private:
    /// Delete the program.
    void deleteProgram()
    {
        if (mProgramId)
        {
            glDeleteProgram(mProgramId);
            mProgramId = 0;
        }
    }

    /// Swap 2 GLSL programs.
    void swap(GLSLProgram &other)
    { std::swap(mProgramId, other.mProgramId); }

    /// ID of the shader program.
    GLuint mProgramId;
protected:
}; // class GLSLProgram

#endif //SIMPLE_GAME_GLSLPROGRAM_H
