#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QMenu>
#include <QMouseEvent>

#include "CVThread.h"
#include "Texture.h"

class CVWidget : public QGLWidget
{
    Q_OBJECT
	
public:
    
    CVWidget(QWidget *parent=NULL,QGLWidget *shardWdgt=ms_shares.empty() ? NULL:ms_shares.front());
    virtual ~CVWidget();
	
	//-- QGLWidget Callbacks
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
	
	const gl::Texture &getTexture() const {return m_texture;};
	void setTexture(const gl::Texture &theTexture){m_texture = theTexture;};
	
	void setCVThread(const CVThreadPtr& cvt);
	CVThreadPtr getCVThread() const {return m_cvThread;};
    
public slots:
	
	virtual void updateImage();
	
	void setDrawFPS(bool b){m_drawFPS=b;};
	
private:
	
    static std::list<QGLWidget*> ms_shares;
	
    QFont font;
    int pressedButton;
	
	//-- OpenCV
    
	//-- Thread for processing CV-Tasks in the background
    CVThreadPtr m_cvThread;
	
    gl::Texture m_texture;
    
 	//-- OpenGL VBO for drawing the canvas
    GLfloat *m_vertices;
    GLuint m_canvasVBO ;
	
	bool m_detectFaces;
	
	bool m_drawFPS;
	int m_framesDrawn ;
	int m_lastFps ;
	
	void buildCanvasVBO();
	
protected:
	
    // mouse-events
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
	
	//timer-event
	void timerEvent(QTimerEvent* e);
	
	//-- Create a drawing texture of an OpenCV Mat
	void updateGLTexture(const cv::Mat& img);
	
	void drawTexture();
	
};

#endif // MYGLWIDGET_H
