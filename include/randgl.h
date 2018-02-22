
#ifdef __APPLE__
#include <OpenGLES/ES3/gl.h>
#else
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#include <stdio.h>
#include "rmath3d.h"

#include <vector>
#include <string>
using  std::string;


namespace random
{

template<typename gl_array_type>
class gl_buffer
{
private:
    const static GLenum enum_buffer_target = gl_array_type::enum_buffer_target;

public:
    gl_buffer(const void* data, GLsizeiptr _data_size, GLenum usage = GL_STATIC_DRAW)
    {
        glGenBuffers(1, &this->buffer_id);
        glBindBuffer(enum_buffer_target, this->buffer_id);
        glBufferData(enum_buffer_target, _data_size, data, usage);
        this->buffer_size = _data_size;
    }

    ~gl_buffer()
    {
        if(buffer_id != 0)
        {
            glDeleteBuffers(1, &buffer_id);
        }
        this->buffer_id = 0;
        this->buffer_size = 0;
    }

    GLuint        get_buff_id(){return buffer_id;}
    GLsizeiptr    get_buffer_size(){return buffer_size;}
    int           get_ref_count(){return ref_count;}
    int           inc_ref_count(){return ++ref_count;}
    int           dec_ref_count(){return --ref_count;}

private:
    GLuint        buffer_id;
    GLsizeiptr    buffer_size;
    int           ref_count;
};

template<typename gl_array_type>
class gl_array
{

public:
    const static GLenum enum_buffer_target = gl_array_type::enum_buffer_target;
    typedef gl_buffer<gl_array_type> gl_array_buff;
    gl_array()
        : use_buff(0)
        , use_offset(0)
        , use_size(0)
    {

    }

    ~gl_array()
    {
        unuse();
    }

    void    use(gl_array_buff* buff, GLsizeiptr offset, GLsizeiptr data_size)
    {
        this->unuse();
        use_buff = buff;
        buff->inc_ref_count();
        use_offset = offset;
        use_size = data_size;
    }

    void    use(gl_array_buff* buff)
    {
       this->use(buff, 0, buff->get_buffer_size());
    }

    void    unuse()
    {
        if(use_buff != 0)
        {
            use_buff->dec_ref_count();
        }
        use_buff = 0;
        use_offset = 0;
        use_size = 0;
    }

    GLsizeiptr update(const void * data, GLsizeiptr data_size)
    {
        if(!use_buff) return 0;
        if(data_size > use_size)
        {
            return 0;
        }
        glBindBuffer(enum_buffer_target, use_buff->buffer_id);
        glBufferSubData(enum_buffer_target, use_offset, data_size, data);
        return data_size;
    }

    GLvoid*   get(GLintptr offset, GLsizeiptr size, GLvoid *data)
    {
        if(!use_buff) return 0;
        glBindBuffer(enum_buffer_target, use_buff->buffer_id);
        glGetBufferSubData(enum_buffer_target, use_offset + offset, size, data);
        return data;
    }

    GLvoid*   open_map (GLenum access)
    {   
        GLvoid* ret;
        if(!use_buff) return 0;
        glBindBuffer(enum_buffer_target, use_buff->buffer_id);
        ret = glMapBuffer(enum_buffer_target, access);
        if(ret)
        {
            return (char*)ret + (size_t)use_offset;
        }
        return ret;
    }

    void       close_map ()
    {
        glUnmapBuffer(enum_buffer_target);
    }

    gl_array_buff*    get_use_buff(){return use_buff;}

protected:
    gl_array_buff*                       use_buff;
    GLsizeiptr                           use_offset;
    GLsizeiptr                           use_size;
};

// 这个结合点可以用来保存glVertexAttribPointer()设置的顶尖数组。在实际工程中这一目标可能是最为常用的
class gl_attrib
    : public gl_array<gl_attrib>
{
public:
    const static GLenum enum_buffer_target = GL_ARRAY_BUFFER;

    gl_attrib(int pos)
        : gl_array<gl_attrib>()
        , attribute_pos(pos) 
    {

    }
    
    ~gl_attrib(){}

    void       attrib_pointer (GLint glsize, GLenum gltype, GLboolean normalized = GL_FALSE, GLsizei glstride = 0)
    {
        glVertexAttribPointer(attribute_pos, glsize, gltype, normalized, glstride,(GLvoid *)use_offset);
        glEnableVertexAttribArray(attribute_pos);
    }

    void        divisor(GLuint value)
    {
        glVertexAttribDivisor(attribute_pos, value);
    }

    int         get_attribute_pos(){return attribute_pos;}

private:
    int                 attribute_pos;
};

class gl_element
    : public gl_array<gl_element>
{
public:
    const static GLenum enum_buffer_target = GL_ELEMENT_ARRAY_BUFFER;

    gl_element(GLuint etype)
        : gl_array<gl_element>()
    {
        this->element_type = etype;
    }

    ~gl_element(){}

    GLuint              get_element_type(){return element_type;}

    GLuint              get_type_size()
    {
        GLuint element_size = 0;
        switch(element_type)
        {
        case GL_UNSIGNED_BYTE:
            element_size = sizeof(GLubyte);
            break;
        case GL_UNSIGNED_SHORT:
            element_size = sizeof(GLushort);
            break;
        case GL_UNSIGNED_INT:
             element_size = sizeof(GLuint);
             break;
        default:
            return 0;
        }
        return element_size;
    }

private:
    GLuint              element_type;
    
};


struct shader_node
{
    std::string     source;
    GLuint          shader_id;
    GLenum          shader_type;
};

class gl_shader
{
public:
    gl_shader(){}
    ~gl_shader()
    {
        for(int i = 0; i < shaders.size(); ++i)
        {
            if(shaders[i].shader_id > 0)
            {
                glDeleteShader(shaders[i].shader_id);
            }
        }
        shaders.clear();
    }

    int       load_file(GLenum type, const char* filename)
    {
    #ifdef _MSC_VER
        FILE* infile;
        fopen_s( &infile, filename, "rb" );
    #else
        FILE* infile = fopen( filename, "rb" );
    #endif // WIN32

        if ( !infile ) {
    #ifdef _DEBUG
            fprintf(stderr, "Unable to open file: %s %s \n", filename, log);
    #endif /* DEBUG */
            return -1;
        }

        fseek( infile, 0, SEEK_END );
        int len = ftell( infile );
        fseek( infile, 0, SEEK_SET );

        struct shader_node node;
        node.shader_id = 0;
        node.shader_type = type;
        shaders.push_back(node);
        std::string& s = shaders[shaders.size() - 1].source;
        s.resize(len, 0);
        fread( (char*)s.data(), 1, len, infile );
        fclose( infile );
        return compile(shaders[shaders.size() - 1]);
    }

    int       load_source(GLenum type, const char* source)
    {
        struct shader_node node;
        node.shader_id = 0;
        node.shader_type = type;
        shaders.push_back(node);
        shaders[shaders.size() - 1].source.assign(source);
        return compile(shaders[shaders.size() - 1]);
    }

    GLuint     link(GLuint program_id)
    {
        for(int i = 0; i < shaders.size(); ++i)
        {
            glAttachShader( program_id, shaders[i].shader_id);
        }
        glLinkProgram( program_id );
        for(int i = 0; i < shaders.size(); ++i)
        {
            glDetachShader( program_id, shaders[i].shader_id);
        }
        GLint linked;
        glGetProgramiv( program_id, GL_LINK_STATUS, &linked );
        if ( !linked ) {
    #ifdef _DEBUG
            GLsizei len;
            glGetProgramiv( program_id, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetProgramInfoLog( program_id, len, &len, log );
            fprintf(stderr, "Shader linking failed: %s \n", log);
            delete [] log;
    #endif /* DEBUG */
            return 0;
        }
        return program_id;
    }


private:
    int     compile(struct shader_node& node)
    {
 
        GLuint shader = glCreateShader( node.shader_type );
        node.shader_id = shader;
        const char * source = node.source.c_str();
        glShaderSource( shader, 1, &source, NULL );
        glCompileShader( shader );

        GLint compiled;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
    #ifdef _DEBUG
            GLsizei len;
            glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetShaderInfoLog( shader, len, &len, log );
            fprintf(stderr, "RglShaderFile compilation failed: %s \n %s", log, node.source.c_str());
            delete [] log;
    #endif /* DEBUG */
            return -1;
        }
        
        return 0;
    }

private:
    std::vector<shader_node>    shaders;
 
};


class gl_program
{
public:
    gl_program()
    {
        program_id = 0;
    }
    ~gl_program(){}

    void        open(gl_shader* shader)
    {
        program_id = glCreateProgram();
        shader->link(program_id);
        if(program_id == 0)
        {
            printf("ERROR: program is 0 \n");
            return;
        }
        GLsizei length = 0;
        GLint  ssize = 0;
        GLenum etype = 0;
        int pos = 0;
        char nname[64];
        nname[0] = 0;

        int index = 0;
        glGetActiveAttrib(program_id, index, 64, &length, &ssize, &etype, nname);
        while(etype)
        {
            pos = glGetAttribLocation(program_id, nname);
            printf("open %d attr %d %d %d %d %s\n", index, length, ssize, etype, pos, nname);
            nname[0] = 0;
            etype = 0;
            ++index;
            glGetActiveAttrib(program_id, index, 64, &length, &ssize, &etype, nname);
        }
        if(0 == vertex_index)
        {
            glGenVertexArrays(1, &vertex_index);
        }
        attributes_size = index;
        if(attributes_size > 0)
        {
            attributes = (gl_attrib*)operator new ( sizeof( gl_attrib ) * attributes_size);
            for(int i = 0; i < attributes_size; ++i)
            {
                new (&attributes[i]) gl_attrib(i);
            }
        }
        glUseProgram(program_id);
        glBindVertexArray(vertex_index);
    }

    void        close()
    {
        if(program_id)
        {
            glDeleteProgram(program_id);
            program_id = 0;
        }
    }
    
    void        begin()
    {
        glUseProgram(program_id);
        glBindVertexArray(vertex_index);
    }

    void        end()
    {
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void        feed_back(GLsizei count, const GLchar* const *varyings, GLenum bufferMode)
    {
        glTransformFeedbackVaryings(program_id, count, varyings, bufferMode);
    }

    GLint       get_attrib_loc(const GLchar* name)
    {
        GLint err;
        err =  glGetAttribLocation(program_id, name);
        if(err == -1)
        {
            printf("ERROR: get attrib %s location error : %d  %s\n", name,  glGetError(), glewGetErrorString(glGetError()));
        }
        return err;
    }

    GLint       get_uniform_loc(const GLchar* name)
    {
        GLint err;
        err =  glGetUniformLocation(program_id, name);
        if(err == -1)
        {
            printf("ERROR: get uniform %s location error : %d  %s\n", name,  glGetError(), glewGetErrorString(glGetError()));
        }
        return err;
    }

    void        draw_array(GLenum mode, GLint first, GLsizei count, GLuint instance = 0)
    {
        if(!instance)
        {
            glDrawArrays(mode, first, count);
        }
        else
        {
            glDrawArraysInstanced(mode, first, count, instance);
        }
    }

    int         get_attributes_size() {return attributes_size;}

    void        draw_elements(gl_element* element, GLenum mode, GLint first, GLsizei count, GLuint instance = 0)
    {
        GLuint eid = element->get_use_buff()->get_buff_id();
        if(!eid) return;

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eid);
        if(!instance)
        {
            glDrawElements(mode, count, element->get_element_type(), (const GLvoid*)(first * element->get_type_size()));
        }
        else
        {
            glDrawElementsInstanced(mode, count, element->get_element_type(), (const GLvoid*)(first * element->get_type_size()), instance);
        }
    }

    GLuint      get_program_id(){return program_id;}
    void        get_uniform(GLuint loc, GLfloat* data){glGetUniformfv(program_id, loc, data);}
    void        get_uniform(GLuint loc, GLdouble* data){glGetUniformdv(program_id, loc, data);}
    void        get_uniform(GLuint loc, GLuint* data){glGetUniformuiv(program_id, loc, data);}
    void        get_uniform(GLuint loc, GLint* data){glGetUniformiv(program_id, loc, data);}
    void        set_uniform(GLint location, GLint v0) {glUniform1i(location, v0);}
    void        set_uniform(GLint location, GLint v0, GLint v1){glUniform2i(location, v0, v1);}
    void        set_uniform(GLint location, GLint v0, GLint v1, GLint v2){glUniform3i(location, v0, v1, v2);}
    void        set_uniform(GLint location, GLint v0, GLint v1, GLint v2, GLint v3){glUniform4i(location, v0, v1, v2, v3);}
    void        set_uniform(GLint location, GLfloat v0){glUniform1f(location, v0);}
    void        set_uniform(GLint location, GLfloat v0, GLfloat v1){glUniform2f(location, v0, v1);}
    void        set_uniform(GLint location, GLfloat v0, GLfloat v1, GLfloat v2){glUniform3f(location, v0, v1, v2);}
    void        set_uniform(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3){glUniform4f(location, v0, v1, v2, v3);}
    void        set_uniform(GLint location, GLboolean transpose, const mat<GLfloat, 2, 2>& m, size_t num = 1){glUniformMatrix2fv(location, num, transpose, &m[0][0]);}
    void        set_uniform(GLint location, GLboolean transpose, const mat<GLfloat, 3, 3>& m, size_t num = 1){glUniformMatrix3fv(location, num, transpose, &m[0][0]);}
    void        set_uniform(GLint location, GLboolean transpose, const mat<GLfloat, 4, 4>& m, size_t num = 1){glUniformMatrix4fv(location, num, transpose, &m[0][0]);}

    gl_attrib*  attrib(unsigned int index){if(index >= attributes_size) return 0; return &(attributes[index]);}

private:
    GLuint      program_id;
    GLuint      vertex_index;
    int         attributes_size;
    gl_attrib * attributes;

};




// 这两个目标是一对相互匹配的结合点，用于拷贝缓存之间的数据，并且不会引起OpenGL状态的变化，也不会产生任何特殊形式的OpenGL调用
// struct gl_copy_read_buffer
//     : gl_base_buffer<gl_copy_read_buffer>
// {
//     const GLenum enum_buffer_target = GL_COPY_READ_BUFFER;
// };

// struct gl_copy_write_buffer
//     : gl_base_buffer<gl_copy_write_buffer>
// {
//     const GLenum enum_buffer_target = GL_COPY_WRITE_BUFFER;
// };

// // 如果采取间接绘制(indirect drawing)的方法，那么这个缓存目标用于存储绘制命令的参数
// struct gl_draw_indirect_buffer
//     : gl_base_buffer<gl_draw_indirect_buffer>
// {
//     const GLenum enum_buffer_target = GL_DRAW_INDIRECT_BUFFER;
// };

// // 绑定到这个目标的缓存中可以包含顶点索引数据，以使用与glDrawElements()等索引形式的绘制命令
// struct gl_element_array_buffer
//     : gl_base_buffer<gl_element_array_buffer>
// {
//     const GLenum enum_buffer_target = GL_ELEMENT_ARRAY_BUFFER;
// };

// // 这一缓存目标用于从图像对象中读取数据，例如纹理和帧缓存数据。相关的OpenGL命令包括glGetTexImage()和glReadPixels()等
// struct gl_pixel_pack_buffer
//     : gl_base_buffer<gl_pixel_pack_buffer>
// {
//     const GLenum enum_buffer_target = GL_PIXEL_PACK_BUFFER;
// };

// // 这一缓存目标与之前的GL_PIXCEL_PACK_BUFFER相反，他可以作为glTexImage2D()命令的数据源使用
// struct gl_pixel_unpack_buffer
//     : gl_base_buffer<gl_pixel_unpack_buffer>
// {
//     const GLenum enum_buffer_target = GL_PIXEL_UNPACK_BUFFER;
// };

// // 纹理缓存也就是直接绑定到位对象的缓存，这一就可以直接在着色器中读取他们的数据信息。GL_TEXTURE_BUFFER可以提供一个控制此类缓存的目标，但是我们还需要将缓存关联到纹理，才能确保他们在着色器中可用
// struct gl_texture_buffer
//     : gl_base_buffer<gl_texture_buffer>
// {
//     const GLenum enum_buffer_target = GL_TEXTURE_BUFFER;
// };

// // transform feedback 是OpenGL提供的一种便捷方案，他可以在管线的顶点处理部分结束时(即经过了顶点着色，可能还有几何着色阶段)，将经过变换的顶点重新捕获，并且将部分属性写入到缓存对象中。这一目标就提供了这样的结合点，可以建立专门的缓存来记录这些属性数组。
// struct gl_transform_feedback_buffer
//     : gl_base_buffer<gl_transform_feedback_buffer>
// {
//     const GLenum enum_buffer_target = GL_TRANSFORM_FEEDBACK_BUFFER;
// };

// // 这个目标可以用于创建uniform缓存对象（uniform buffer object)的缓存对象
// struct gl_uniform_buffer
//     : gl_base_buffer<gl_uniform_buffer>
// {
//     const GLenum enum_buffer_target = GL_TRANSFORM_FEEDBACK_BUFFER;
// };


}
