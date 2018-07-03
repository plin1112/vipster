#ifndef CPPARAM_H
#define CPPARAM_H

#include <QWidget>
#include "../paramwidget.h"

namespace Ui {
class CPParam;
}

class CPParam : public ParamBase
{
    Q_OBJECT

public:
    explicit CPParam(QWidget *parent = nullptr);
    ~CPParam() override;
    void setParam(Vipster::BaseParam *p) override;

private slots:
    void on_comboBox_currentIndexChanged(const QString &arg1);

private:
    void focusOutEvent(QFocusEvent *e) override;
    void fillText();
    void saveText();
    Ui::CPParam *ui;
    Vipster::IO::CPParam* curParam{nullptr};
    Vipster::IO::CPParam::Section Vipster::IO::CPParam::* curSection{nullptr};
};

#endif // CPPARAM_H
