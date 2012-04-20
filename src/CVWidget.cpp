#include "CVWidget.h"
#include "mainwindow.h"


// opencv C++ interface
using namespace cv;
using namespace std;
using boost::shared_ptr;

list<QGLWidget*> CVWidget::ms_shares;


CVWidget::CVWidget(QWidget *prnt,QGLWidget *shr):QGLWidget(prnt,shr),
m_type(WIDGET_RESULT),
m_GLTextureIndex(0),m_detectFaces(0),m_drawFPS(false),m_framesDrawn(0),m_lastFps(0)
{	
	// static shares, to have only one Gl-context for all widgets
    ms_shares.push_back(this);
	
	//setFormat(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer));
	m_canvasList = 0;
	
    font.setPointSize(18);
}

CVWidget::~CVWidget()
{	
	if(m_cvThread.use_count() == 1)
	{
		//stop processing thread and wait until it finishes
		m_cvThread->stop();
		m_cvThread->wait();
		
	}
	
	glDeleteTextures(1, &m_GLTextureIndex);
	
	ms_shares.remove(this);
}

void CVWidget::setCVThread(const shared_ptr<CVThread>& cvt)
{
	if(m_cvThread.get())
		disconnect(m_cvThread.get(), SIGNAL(imageChanged()), this, SLOT(updateImage()) );
	
	m_cvThread = cvt;
    
	if(m_cvThread.get())
	{
		connect(m_cvThread.get(), SIGNAL(imageChanged()), this, SLOT(updateImage()),Qt::QueuedConnection );
	}
    
}

void CVWidget::initializeGL()
{	
	assert(doubleBuffer());
	
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	
	// generate texture-index for drawingTexture
	glGenTextures(1, &m_GLTextureIndex);
	assert(m_GLTextureIndex);
	
	//printf("textureIndex: %d\n",m_GLTextureIndex);
	
	// displaylist for our canvas
	m_canvasList = buildCanvasList() ;
	
	//FPS Timer
	if(m_drawFPS) startTimer(1000);
}

void CVWidget::paintGL()
{	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
		
	//drawing with texture
	drawTexture();
	
	if(m_drawFPS)
	{
		char fpsString [25];
		sprintf(fpsString, "fps: %d \n",m_lastFps);
		
		//render fps
		renderText(10, 20, fpsString, font);
		
		// 1 more frame
		m_framesDrawn++;
	}
	//swapBuffers();
}

void CVWidget::resizeGL(int newWidth, int newHeight)
{	
	//float aspectRatio = ( (float) newWidth / (float)(newHeight?newHeight:1) );
	
	glViewport(0, 0, newWidth, newHeight);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(0, 1.0, 0, 1.0, 0.0, 1.0);
    
	//gluOrtho2D(0, newWidth, 0, newHeight);
	
	glMatrixMode(GL_MODELVIEW);
}

GLuint CVWidget::buildCanvasList()
{	
	GLuint list = glGenLists(1);
	assert(list);
	
    //GL_T2F_V3F
    const GLfloat array[] ={0.0,0.0,0.0,0.0,0.0,
                            1.0,0.0,1.0,0.0,0.0,
                            1.0,1.0,1.0,1.0,0.0,
                            0.0,1.0,0.0,1.0,0.0};
    
    
    
    glNewList(list, GL_COMPILE);
	
    glInterleavedArrays(GL_T2F_V3F, 0, array);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    /*
	glBegin( GL_QUADS );
	
	glTexCoord2f(0, 0);
	glVertex2f(0.0, 0.0);
	
	glTexCoord2f(1.0, 0.0);
	glVertex2f(1.0, 0.0);
	
	glTexCoord2f(1.0, 1.0);
	glVertex2f(1.0, 1.0);
	
	glTexCoord2f(0, 1.0);
	glVertex2f(0, 1.0);
	
	glEnd();
	*/
    
	glEndList();
	
	return list ;
}

void CVWidget::mousePressEvent(QMouseEvent *event)
{	
	setFocus(Qt::MouseFocusReason);
	pressedButton=event->button();
	
}

void CVWidget::mouseMoveEvent(QMouseEvent *event)
{	
	event->ignore();
}

void CVWidget::mouseReleaseEvent(QMouseEvent *event)
{
	event->ignore();
	
}


void CVWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	// event will be passed through
	//(otherwise it would just trigger a release-event)
	event->ignore();
	
}


void CVWidget::wheelEvent(QWheelEvent *event)
{
	event->ignore();
}

void CVWidget::timerEvent(QTimerEvent* e) 
{
	e->accept();
	//printf("%d fps\n",m_framesDrawn);
	
	m_lastFps = m_framesDrawn ;
	m_framesDrawn = 0 ;
	
}

void CVWidget::drawTexture()
{	
	
	
	glBindTexture(GL_TEXTURE_2D, m_GLTextureIndex);
	
	//draw renderTexture
	glCallList(m_canvasList);
	
}

void CVWidget::updateImage()
{	
	Mat img;
    const CVThread::FrameBundle &bundle = m_cvThread->getFrameBundle();
    
    switch (m_type) {
        case WIDGET_INPUT:
            img = bundle.m_inFrame;
            break;
        case WIDGET_DEPTH:
            img = bundle.m_depthMap;
            break;
        case WIDGET_RESULT:
            img = bundle.m_result;
            break;
            
        default:
            break;
    }
    
    
	createGLTexture(img);
	
	updateGL();
}

void CVWidget::createGLTexture(const Mat& img)
{	
	//m_image=img;
    
	GLenum format=0,internalFormat=0;
	
	switch(img.channels()) 
	{
		case 1:
			format = internalFormat = GL_LUMINANCE;
			break;
		case 2:
			format = internalFormat = GL_LUMINANCE_ALPHA;
			break;
		case 3:
			format = GL_BGR;
			//format = GL_RGB;
			internalFormat = GL_RGB ;
			break;
		default:
			break;
	}
    // Vertical flip
	glPushMatrix();
    glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef(1.f,-1.f,1.f);
    
	/* Create linear filtered Texture */
	glBindTexture(GL_TEXTURE_2D, m_GLTextureIndex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, img.cols, img.rows, 0, format, GL_UNSIGNED_BYTE,img.data);
    
    glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}