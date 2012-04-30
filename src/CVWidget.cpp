#include "CVWidget.h"
#include "mainwindow.h"


// opencv C++ interface
using namespace cv;
using namespace std;

list<QGLWidget*> CVWidget::ms_shares;


CVWidget::CVWidget(QWidget *prnt,QGLWidget *shr):QGLWidget(prnt,shr),
m_vertices(NULL), m_vertexBuffer(0),
m_detectFaces(0),m_drawFPS(false),m_framesDrawn(0),m_lastFps(0)
{	
	// static shares, to have only one Gl-context for all widgets
    ms_shares.push_back(this);
    font.setPointSize(18);
}

CVWidget::CVWidget(const QGLFormat &format, QWidget *parent):
QGLWidget(format, parent), m_vertices(NULL), m_vertexBuffer(0),
m_detectFaces(0),m_drawFPS(false),m_framesDrawn(0),m_lastFps(0)
{
    // static shares, to have only one Gl-context for all widgets
    ms_shares.push_back(this);
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
	
	ms_shares.remove(this);
    
    if(m_vertices) delete [] m_vertices;
}

void CVWidget::setCVThread(const CVThreadPtr& cvt)
{
	if(m_cvThread)
		disconnect(m_cvThread.get(), SIGNAL(imageChanged()), this, SLOT(updateImage()) );
	
	m_cvThread = cvt;
    
	if(m_cvThread)
	{
		connect(m_cvThread.get(), SIGNAL(imageChanged()), this, SLOT(updateImage()),Qt::QueuedConnection );
	}
    
}

void CVWidget::initializeGL()
{	
	if(!doubleBuffer())
        throw runtime_error("no doublebuffering available ...");
	
	glClearColor(0.0, 0.0, 0.0, 1.0);

    glEnable(GL_TEXTURE_2D);
    
    try 
    {
        m_shader.loadFromFile("texShader.vsh", "texShader.fsh");
    } 
    catch (std::exception &e) 
    {
        printf("%s\n",e.what());
    }

    // display array and VBO for our canvas
	buildCanvasVBO() ;
    
	//FPS Timer
	if(m_drawFPS) startTimer(1000);
}

void CVWidget::paintGL()
{	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glLoadIdentity();
		
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
    
}

void CVWidget::buildCanvasVBO()
{
    //GL_T2F_V3F
    const GLfloat array[] ={0.0,0.0,0.0,0.0,0.0,
                            1.0,0.0,1.0,0.0,0.0,
                            1.0,1.0,1.0,1.0,0.0,
                            0.0,1.0,0.0,1.0,0.0};
    
    if (m_vertices) delete [] m_vertices;
    
    m_vertices = new GLfloat[sizeof(array) / sizeof(GLfloat)];
    std::memcpy(m_vertices, array, sizeof(array));
    
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(array), array, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	
	m_lastFps = m_framesDrawn ;
	m_framesDrawn = 0 ;
	
}

void CVWidget::drawTexture()
{	
    // Texture will be bound in this scope
    gl::scoped_bind<gl::Texture> texBind(m_texture);
    gl::scoped_bind<gl::Shader> shaderBind(m_shader);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    
    glm::mat4 projectionMatrix = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    glm::mat4 modelViewMatrix;
    
	m_shader.uniform("u_modelViewProjectionMatrix", 
                     projectionMatrix * modelViewMatrix);
    
    GLuint positionAttribLocation = m_shader.getAttribLocation("a_position");
    glEnableVertexAttribArray(positionAttribLocation);
    glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE,
                          5 * sizeof(GLfloat), BUFFER_OFFSET(2 * sizeof(GLfloat)));
    
    GLuint texCoordAttribLocation = m_shader.getAttribLocation("a_texCoord");
    glEnableVertexAttribArray(texCoordAttribLocation);
    glVertexAttribPointer(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE,
                          5 * sizeof(GLfloat), BUFFER_OFFSET(0));
    
    if(m_texture)
    {
        m_shader.uniform("u_textureMap", m_texture.getBoundTextureUnit());
        m_shader.uniform("u_textureMatrix", m_texture.getTextureMatrix());
    }
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CVWidget::updateImage()
{	
    const CVThread::FrameBundle &bundle = m_cvThread->getFrameBundle();

	updateGLTexture(bundle.m_result);
	
	updateGL();
}

void CVWidget::updateGLTexture(const Mat& img)
{	
    
	GLenum format=0;
	
	switch(img.channels()) 
	{
		case 1:
            format = GL_LUMINANCE;
			break;
		case 2:
            format = GL_LUMINANCE_ALPHA;
			break;
		case 3:
			format = GL_BGR;
			break;
        case 4:
			format = GL_BGRA;
		default:
			break;
	}
    
    m_texture.update(img.data, format, img.cols, img.rows, true);
}