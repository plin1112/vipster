#ifndef PWIPARAM_H
#define PWIPARAM_H

#include <QWidget>
#include "ioplugins/pwinput.h"
#include "../paramwidget.h"

namespace Ui {
class PWParam;
}

class PWParam : public ParamBase
{
    Q_OBJECT

public:
    explicit PWParam(QWidget *parent = nullptr);
    ~PWParam();
    void setParam(Vipster::BaseParam *p);

private:
    Ui::PWParam *ui;
    Vipster::IO::PWParam *curParam{nullptr};
};

#endif // PWIPARAM_H