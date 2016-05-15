#ifndef OPENGL_H
#define OPENGL_H

#include "../graphic/shader.h"
#include "../graphic/camera.h"
#include "../game/object.h"

#include <QtWidgets/QOpenGLWidget>
#include <QtCore/QBasicTimer.h>

#define graphic_fov 1.0f
#define graphic_znear 0.01f
#define graphic_zfar 100.0f

class Opengl : public QOpenGLWidget {
    public:
        Opengl( QWidget *parent = NULL );
        virtual ~Opengl();

        void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;
        void paintGL() Q_DECL_OVERRIDE;

        void initializeGL() Q_DECL_OVERRIDE;
        void initShaders();
    protected:
    private:
        QBasicTimer m_timer;
        Shader* m_shader;
        Camera *m_camera;
        Object *m_obj;
};

#endif // OPENGL_H
