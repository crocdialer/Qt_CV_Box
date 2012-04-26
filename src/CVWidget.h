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
	
	GLuint getGLTextureIndex() const {return m_GLTextureIndex;};
	void setGLTextureIndex(GLuint i){m_GLTextureIndex=i;};
	
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
    
	//-- OpenGL texture index
	GLuint m_GLTextureIndex ;
	
	//-- OpenGL display list for drawing the canvas
	GLuint m_canvasList ;
	
	bool m_detectFaces;
	
	bool m_drawFPS;
	int m_framesDrawn ;
	int m_lastFps ;
	
	GLuint buildCanvasList();
	
    //void fetchGlobalAxis(Vector& x,Vector& y,Vector& z);
	
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
	void createGLTexture(const cv::Mat& img);
	
	void drawTexture();
	
};

#endif // MYGLWIDGET_H
