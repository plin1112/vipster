#ifndef PARAMWIDGET_H
#define PARAMWIDGET_H

#include <QWidget>
#include "mainwindow.h"

namespace Ui {
class ParamWidget;
}

class ParamBase: public QWidget
{
    Q_OBJECT

public:
    explicit ParamBase(QWidget *parent = nullptr);
    virtual ~ParamBase() = default;
    virtual void setParam(Vipster::BaseParam *p)=0;
};

class ParamWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParamWidget(QWidget *parent = nullptr);
    ~ParamWidget() override;
    std::vector<std::pair< Vipster::IOFmt, std::unique_ptr<Vipster::BaseParam>>> params;
    void registerParam(Vipster::IOFmt fmt,
                       std::unique_ptr<Vipster::BaseParam>&& data);
    void clearParams();
    Vipster::BaseParam *curParam;

private slots:
    void on_paramSel_currentIndexChanged(int index);

private:
    Ui::ParamWidget *ui;
};

#endif // PARAMWIDGET_H
