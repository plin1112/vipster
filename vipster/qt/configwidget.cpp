#include "configwidget.h"
#include "ui_configwidget.h"
#include "iowrapper.h"

using namespace Vipster;

ConfigBase::ConfigBase(QWidget *parent):
    QWidget{parent}
{}

ConfigWidget::ConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigWidget)
{
    ui->setupUi(this);
}

ConfigWidget::~ConfigWidget()
{
    delete ui;
}

void ConfigWidget::clearConfigs()
{
    configs.clear();
    ui->configSel->clear();
}

void ConfigWidget::registerConfig(Vipster::IOFmt fmt,
                                  std::unique_ptr<Vipster::BaseConfig>&& data)
{
    configs.emplace_back(fmt, std::move(data));
    ui->configSel->addItem(QString::fromStdString(
                               "(" + IOPlugins.at(fmt)->command +
                               ") " + configs.back().second->name
                               ));
}

void ConfigWidget::on_configSel_currentIndexChanged(int index)
{
    if(index<0){
        ui->configStack->setCurrentWidget(ui->NoCWidget);
        curConfig = nullptr;
        return;
    }
    if(static_cast<size_t>(index) >= configs.size()){
        throw Error("Invalid configuration preset selected");
    }
    const auto& pair = configs.at(static_cast<size_t>(index));
    curConfig = pair.second.get();
    switch (pair.first) {
    case IOFmt::PWI:
        ui->configStack->setCurrentWidget(ui->PWWidget);
        ui->PWWidget->setConfig(curConfig);
        break;
    case IOFmt::LMP:
        ui->configStack->setCurrentWidget(ui->LmpWidget);
        ui->LmpWidget->setConfig(curConfig);
        break;
    default:
        throw Error("Invalid config format");
    }
}
