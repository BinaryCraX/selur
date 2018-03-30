#ifndef OBJECT_H
#define OBJECT_H

#include <dirent.h>
#include <algorithm>

#include <q3.h>

#include "../xml/tinyxml2.h"

#include "../graphic/graphic.h"
#include "../graphic/texture.h"

#define DEFINITION_FILE "definition.xml"

class object_type {
    public:
        object_type();
        ~object_type();

        void init(  Transform *transform);

        bool load_type( config *config, std::string l_path, std::string l_name);
        bool load_file( std::string file);

        void updateVbo();
        void updateVao();

        void draw( glm::mat4 model, Shader* shader, glm::mat4 viewprojection);

//        btRigidBody *makeBulletMesh();

        std::string getName() { return p_name; };
        glm::vec3 getScale() { return p_size; }
    private:
        std::string p_name;
        std::string p_file;
        std::string p_texture_file;
        texture *p_texture;

        std::vector<glm::vec3> p_vertices;
        std::vector< glm::vec2 > p_texcoords;
        std::vector<glm::vec3> p_normal;
        std::vector<unsigned int> p_indices;

        glm::vec3 p_size;

        GLuint p_vao;
        GLuint p_vbo_vertices;
        GLuint p_vbo_normal;
        GLuint p_vbo_index;
        GLuint p_vbo_texture;

        // physic
        std::vector<q3BoxDef> p_boxDef;
};

class object {
    public:
        object();
        ~object();

        void init();
        void draw( Shader* shader, glm::mat4 viewprojection);

        void update_model();

        void setPosition( glm::vec3 pos);
        void setRotation( glm::vec3 rot);
        void setType( object_type *type);
        object_type *getType() { return p_type; }
    protected:
    public:
        bool p_model_change;
        glm::mat4 p_model;
        glm::vec3 p_pos;
        glm::vec3 p_rot;
        glm::vec3 p_scale;

        object_type *p_type;
};

class object_handle
{
    public:
        object_handle();
        ~object_handle();

        bool load( config *config);
        bool load_folder( std::string folder, config *config);

        object_type *get( std::string name);
    private:

        std::vector<object_type *> p_types;

};

#endif // OBJECT_H
