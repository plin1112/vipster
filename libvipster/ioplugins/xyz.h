#ifndef XYZ_H
#define XYZ_H

#include "../ioplugin.h"

namespace Vipster{
namespace IO{
extern const IO::Plugin XYZ;

struct XYZConfig: BaseConfig{
    enum class Data{None, Charge, Forces};
    enum class Mode{Step, Trajec, Cell};
    Mode filemode;
    Data atomdata;
    XYZConfig(std::string="", Mode=Mode::Step, Data=Data::None);
    std::unique_ptr<BaseConfig> copy() override;
};

const XYZConfig XYZConfigDefault{
    "default",
    XYZConfig::Mode::Step,
    XYZConfig::Data::None
};

}
}

#endif // XYZ_H
