#include "surfacerenderable.h"
#include "openglwindow.h"
#include <cmath>
#include <fstream>
#include <sstream>

SurfaceRenderable::SurfaceRenderable()
    : m_vertices(new QVector<QVector3D>()),
      m_colors(new QVector<QVector4D>()),
      m_normals(new QVector<QVector3D>()),
      m_indices(new QVector<unsigned int>())
{
  m_lightPos = QVector3D(0.0, -4.0, -10);
}

SurfaceRenderable::~SurfaceRenderable()
{
}

void SurfaceRenderable::init(OpenGLWindow *openGLWindow)
{
  createBuffers(openGLWindow->glFunctions());
  createShader(openGLWindow);
}

void SurfaceRenderable::createShader(QObject *obj)
{
  m_shaderProgram = new QOpenGLShaderProgram(obj);
  m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "../Renderable/shaders/vertshader.glsl");
  m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "../Renderable/shaders/fragshader.glsl");
  m_shaderProgram->link();

  m_projecionMatrixUniform = m_shaderProgram->uniformLocation("projectionMatrix");
  m_modelViewMatrixUniform = m_shaderProgram->uniformLocation("modelViewMatrix");
  m_readNormalsUniform     = m_shaderProgram->uniformLocation("readNormals");
  m_lightPosUniform        = m_shaderProgram->uniformLocation("lightPos");
}

void SurfaceRenderable::render(QOpenGLFunctions_4_1_Core *glFunctions, QMatrix4x4 &projectionMatrix)
{
  glFunctions->glBindVertexArray(m_vao);

  m_shaderProgram->bind();
  m_modelViewMatrix.setToIdentity();
  m_shaderProgram->setUniformValue(m_modelViewMatrixUniform, m_modelViewMatrix);
  m_shaderProgram->setUniformValue(m_projecionMatrixUniform, projectionMatrix);
  m_shaderProgram->setUniformValue(m_readNormalsUniform, m_state.readNormals);
  m_shaderProgram->setUniformValue(m_lightPosUniform, m_lightPos);

  updateBuffers(glFunctions);
  //  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  //  glFunctions->glDrawElements(GL_TRIANGLES, m_indices->size(), GL_UNSIGNED_INT, 0);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glFunctions->glDrawElements(GL_TRIANGLES, m_indices->size(), GL_UNSIGNED_INT, 0);

  m_shaderProgram->release();
}

void SurfaceRenderable::createBuffers(QOpenGLFunctions_4_1_Core *glFunctions)
{
  glFunctions->glGenVertexArrays(1, &m_vao);
  glFunctions->glBindVertexArray(m_vao);

  glFunctions->glGenBuffers(1, &m_coordsBO);
  glFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_coordsBO);
  glFunctions->glEnableVertexAttribArray(0);
  glFunctions->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glFunctions->glGenBuffers(1, &m_colourBO);
  glFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_colourBO);
  glFunctions->glEnableVertexAttribArray(1);
  glFunctions->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

  glFunctions->glGenBuffers(1, &m_normalBO);
  glFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_normalBO);
  glFunctions->glEnableVertexAttribArray(2);
  glFunctions->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glFunctions->glGenBuffers(1, &m_indicesBO);
  glFunctions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBO);

  glFunctions->glBindVertexArray(0);
}

void SurfaceRenderable::updateBuffers(QOpenGLFunctions_4_1_Core *glFunctions)
{
  glFunctions->glBindVertexArray(m_vao);

  glFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_coordsBO);
  glFunctions->glBufferData(
      GL_ARRAY_BUFFER, sizeof(QVector3D) * m_vertices->size(), m_vertices->data(), GL_DYNAMIC_DRAW);

  glFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_colourBO);
  glFunctions->glBufferData(
      GL_ARRAY_BUFFER, sizeof(QVector4D) * m_colors->size(), m_colors->data(), GL_DYNAMIC_DRAW);

  if (m_state.readNormals)
  {
    glFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_normalBO);
    glFunctions->glBufferData(
        GL_ARRAY_BUFFER, sizeof(QVector3D) * m_normals->size(), m_normals->data(), GL_DYNAMIC_DRAW);
  }

  glFunctions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBO);
  glFunctions->glBufferData(
      GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_indices->size(), m_indices->data(), GL_DYNAMIC_DRAW);
}

void SurfaceRenderable::load_obj(const char *filename)
{
  ifstream in(filename, ios::in);
  if (!in)
  {
    qDebug() << "Cannot open " << filename << endl;
    exit(1);
  }

  string line;
  while (getline(in, line))
  {
    if (line.substr(0, 2) == "v ")
    {
      istringstream s(line.substr(2));
      QVector3D v;
      s >> v[0];
      s >> v[1];
      s >> v[2];
      m_vertices->append(v);
      m_colors->append(QVector4D(0.6, 0.6, 0.6, 0.5f));
    }
    else if (line.substr(0, 2) == "f ")
    {
      istringstream s(line.substr(2));
      int a;
      QVector<unsigned int> t_indices;
      while (not s.eof())
      {
        s >> a;
        t_indices.append(a - 1);
      }
      for (size_t i = 2; i < t_indices.size(); ++i)
      {
        m_indices->append(t_indices[0]);
        m_indices->append(t_indices[i - 1]);
        m_indices->append(t_indices[i]);
      }
    }
    else if (line.substr(0, 3) == "vn ")
    {
      m_state.readNormals = true;
    }
    else if (line[0] == '#')
    {
      /* ignoring this line */
    }
    else
    {
      /* ignoring this line */
    }
  }
}
