#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include "step.h"
#include "mainwindow.h"
#include "../common/guiwrapper.h"

class GLWidget: public QOpenGLWidget, public BaseWidget, private Vipster::GuiWrapper
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget() override;
    void initializeGL(void) override;
    void paintGL(void) override;
    void resizeGL(int w, int h) override;
    void keyPressEvent(QKeyEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void updateWidget(uint8_t change) override;
public slots:
    void setMode(int i,bool t);
    void setMult(int i);
    void setCamera(int i);
private:
    void setStep(Vipster::StepProper* step);
    void setSel(Vipster::StepSelection* sel);
    // Input handling
    enum class MouseMode { Camera=-2, Select=-3, Modify=-4 };
    MouseMode mouseMode{MouseMode::Camera};
    QPoint mousePos, rectPos;
    Vipster::Vec shift;
    std::set<size_t> pickAtoms();
    void rotAtoms(QPoint delta);
    void shiftAtomsXY(QPoint delta);
    void shiftAtomsZ(QPoint delta);
};

#endif // GLWIDGET_
