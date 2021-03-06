#include "pybind11/pybind11.h"
#include "pybind11/operators.h"
#include "pybind11/stl_bind.h"

#include <sstream>
#include <molecule.h>
#include <iowrapper.h>

namespace py = pybind11;
using namespace py::literals;
using namespace Vipster;

template <typename Array, typename holder_type = std::unique_ptr<Array>, typename... Args>
py::class_<Array, holder_type> bind_array(py::module &m, std::string const &name, Args&&... args) {
    using Class_ = pybind11::class_<Array, holder_type>;
    using ValueType = typename Array::value_type;
    using SizeType = typename Array::size_type;

    Class_ cl(m, name.c_str(), std::forward<Args>(args)...);
    cl.def(py::init());
    cl.def("__getitem__",[](const Array &v, SizeType i)->const ValueType&{
        if(i<0 || i>= v.size())
            throw py::index_error();
        return v[i];
    }, py::return_value_policy::reference_internal);
    cl.def("__setitem__",[](Array &v, SizeType i, ValueType f){
        if(i<0 || i>= v.size())
            throw py::index_error();
        v[i] = f;
    });
    cl.def(py::init([](const py::iterable& it){
        Array arr;
        SizeType i=0;
        for(py::handle h:it){
            arr[i] = h.cast<ValueType>();
            i++;
        }
        return arr;
    }));
    cl.def(py::self == py::self);
    py::implicitly_convertible<py::iterable, Array>();
    cl.def("__repr__", [name](const Array &v){
        std::string repr = name + "[";
        for(size_t i=0; i<v.size()-1; ++i){
            repr += py::str(py::cast(v[i]).attr("__repr__")());
            repr += ", ";
        }
        repr += py::str(py::cast(v[v.size()-1]).attr("__repr__")());
        repr += "]";
        return repr;
    });
    return cl;
}

PYBIND11_MODULE(vipster, m) {
    m.doc() = "Vipster\n"
              "=======\n\n"
              "A molecular modeling framework with periodic structures in mind.\n"
              "Use readFile() and writeFile() to handle files.\n"
              "Please inspect Molecule and Step as the main data"
              "containers for more information.";

    /*
     * Step (Atoms, Bonds, Cell + PSE)
     */

    bind_array<Vec>(m, "Vec");
    bind_array<Mat>(m, "Mat");
    bind_array<ColVec>(m, "ColVec");
    py::bind_vector<std::vector<StepProper>>(m,"__StepVector__");
    py::bind_vector<std::vector<Bond>>(m,"__BondVector__");
    py::bind_vector<std::vector<std::string>>(m,"__StringVector__");
    py::bind_map<std::map<std::string,std::string>>(m,"__StrStrMap__");

    py::enum_<AtomFmt>(m, "AtomFmt")
        .value("Bohr", AtomFmt::Bohr)
        .value("Angstrom", AtomFmt::Angstrom)
        .value("Crystal", AtomFmt::Crystal)
        .value("Alat", AtomFmt::Alat)
    ;

    py::enum_<BondLevel>(m, "BondLevel")
        .value("None", BondLevel::None)
        .value("Molecule", BondLevel::Molecule)
        .value("Cell", BondLevel::Cell)
    ;

    py::enum_<BondFrequency>(m, "BondFrequency")
        .value("Never", BondFrequency::Never)
        .value("Once", BondFrequency::Once)
        .value("Always", BondFrequency::Always)
    ;

    py::enum_<CdmFmt>(m, "CellDimFmt")
        .value("Bohr", CdmFmt::Bohr)
        .value("Angstrom", CdmFmt::Angstrom)
    ;

    py::enum_<AtomFlag>(m, "AtomFlag")
        .value("FixX", AtomFlag::FixX)
        .value("FixY", AtomFlag::FixY)
        .value("FixZ", AtomFlag::FixZ)
        .value("Hidden", AtomFlag::Hidden)
    ;

    py::class_<AtomFlags>(m, "AtomFlags")
        .def("__getitem__",[](const AtomFlags &bs, AtomFlag ap){
            return bs[static_cast<uint8_t>(ap)];
        })
        .def("__setitem__",[](AtomFlags &bs, AtomFlag ap, bool val){
            bs[static_cast<uint8_t>(ap)] = val;
        })
    ;

    py::class_<Atom>(m, "Atom")
        .def_property("name", [](const Atom &a)->const std::string&{return a.name;},
                      [](Atom &a, std::string s){a.name = s;})
        .def_property("coord", [](const Atom &a)->const Vec&{return a.coord;},
                      [](Atom &a, Vec c){a.coord = c;})
        .def_property("properties", [](const Atom &a)->const AtomProperties&{return a.properties;},
                      [](Atom &a, AtomProperties bs){a.properties = bs;})
        .def(py::self == py::self)
        .def(py::self != py::self)
    ;

    py::class_<Bond>(m, "Bond")
        .def_readwrite("at1", &Bond::at1)
        .def_readwrite("at2", &Bond::at2)
        .def_readwrite("dist", &Bond::dist)
        .def_readwrite("xdiff", &Bond::xdiff)
        .def_readwrite("ydiff", &Bond::ydiff)
        .def_readwrite("zdiff", &Bond::zdiff)
    ;

    py::class_<PseMap, std::shared_ptr<PseMap>>(m, "PseMap")
        .def("__getitem__", &PseMap::operator [], py::return_value_policy::reference_internal)
        .def("__setitem__", [](PseMap &pse, std::string n, PseEntry& e){pse[n] = e;})
    ;

    py::class_<PseEntry>(m, "PseEntry")
        .def_readwrite("PWPP", &PseEntry::PWPP)
        .def_readwrite("CPPP", &PseEntry::CPPP)
        .def_readwrite("CPNL", &PseEntry::CPNL)
        .def_readwrite("Z", &PseEntry::Z)
        .def_readwrite("m", &PseEntry::m)
        .def_readwrite("bondcut", &PseEntry::bondcut)
        .def_readwrite("covr", &PseEntry::covr)
        .def_readwrite("vdwr", &PseEntry::vdwr)
        .def_readwrite("col", &PseEntry::col)
    ;

    py::class_<Step>(m, "Step")
        .def_readonly("pse", &Step::pse)
        .def_property("comment", &Step::getComment, &Step::setComment)
        .def("newAtom", [](Step& s){s.newAtom();})
        .def("newAtom", py::overload_cast<std::string, Vec, AtomProperties>(&StepProper::newAtom),
             "name"_a, "coord"_a=Vec{}, "properties"_a=AtomProperties{})
        .def("newAtom", py::overload_cast<const Atom&>(&StepProper::newAtom), "at"_a)
        .def("__getitem__", [](Step& s, int i){
                if (i<0){
                    i = i+static_cast<int>(s.getNat());
                }
                if ((i<0) || i>=static_cast<int>(s.getNat())){
                    throw py::index_error();
                }
                return s[static_cast<size_t>(i)];
            })
        .def("__setitem__", [](Step& s, int i, const Atom& at){
                if (i<0){
                    i = i+static_cast<int>(s.getNat());
                }
                if ((i<0) || i>=static_cast<int>(s.getNat())){
                    throw py::index_error();
                }
                s[static_cast<size_t>(i)] = at;
        })
        .def("__len__", &Step::getNat)
        .def_property_readonly("nat", &Step::getNat)
        .def("__iter__", [](const Step& s){return py::make_iterator(s.begin(), s.end());})
        .def("delAtom", &Step::delAtom, "i"_a)
        .def("getTypes", [](const Step& s){
            auto oldT = s.getTypes();
            return std::vector<std::string>(oldT.begin(), oldT.end());
        })
        .def_property_readonly("ntyp", &Step::getNtyp)
    //FMT
        .def("getFmt", &Step::getFmt)
        .def("asFmt", py::overload_cast<AtomFmt>(&Step::asFmt), "fmt"_a,
             py::return_value_policy::reference_internal)
    //CELL
        .def("enableCell", &Step::enableCell, "enable"_a)
        .def("getCellDim", &Step::getCellDim)
        .def("setCellDim", &Step::setCellDim, "cdm"_a, "fmt"_a, "scale"_a=false)
        .def("getCellVec", &Step::getCellVec)
        .def("setCellVec", &Step::setCellVec, "vec"_a, "scale"_a=false)
        .def("getCenter", &Step::getCenter, "fmt"_a, "com"_a=false)
    //BONDS
        .def("getBonds", py::overload_cast<BondLevel, BondFrequency>(&Step::getBonds, py::const_),
             "level"_a=BondLevel::Cell, "update"_a=BondFrequency::Always)
        .def("getBonds", py::overload_cast<float, BondLevel, BondFrequency>(&StepProper::getBonds, py::const_),
             "cutfac"_a, "level"_a=BondLevel::Cell, "update"_a=BondFrequency::Always)
        .def_property_readonly("nbond", &StepProper::getNbond)
    ;

    //TODO: allow construction?
    py::class_<StepProper, Step>(m, "StepProper")
        .def("setFmt", &StepProper::setFmt, "fmt"_a, "scale"_a=false);
    py::class_<StepFormatter, Step>(m, "StepFormatter");

    /*
     * K-Points
     */
    py::bind_vector<std::vector<DiscreteKPoint>>(m, "__KPointVector__");

    py::enum_<KPointFmt>(m, "KPointFmt")
        .value("Gamma",KPointFmt::Gamma)
        .value("MPG",KPointFmt::MPG)
        .value("Discrete",KPointFmt::Discrete)
    ;

    py::class_<KPoints> k(m, "KPoints");
    k.def(py::init())
        .def_readwrite("active", &KPoints::active)
        .def_readwrite("mpg", &KPoints::mpg)
        .def_readwrite("discrete", &KPoints::discrete)
    ;

    py::class_<KPoints::MPG>(k, "MPG")
        .def_readwrite("x",&KPoints::MPG::x)
        .def_readwrite("y",&KPoints::MPG::y)
        .def_readwrite("z",&KPoints::MPG::z)
        .def_readwrite("sx",&KPoints::MPG::sx)
        .def_readwrite("sy",&KPoints::MPG::sy)
        .def_readwrite("sz",&KPoints::MPG::sz)
    ;

    py::class_<KPoints::Discrete> disc(k, "Discrete");
    disc.def_readwrite("properties", &KPoints::Discrete::properties)
        .def_readwrite("kpoints", &KPoints::Discrete::kpoints)
    ;

    py::class_<DiscreteKPoint> dp(m, "DiscreteKPoint");
    dp.def(py::init([](const Vec& p, float w){return DiscreteKPoint{p,w};}))
        .def_readwrite("pos", &DiscreteKPoint::pos)
        .def_readwrite("weight", &DiscreteKPoint::weight)
    ;

    py::enum_<KPoints::Discrete::Properties>(dp, "Properties", py::arithmetic())
        .value("none", KPoints::Discrete::Properties::none)
        .value("crystal", KPoints::Discrete::Properties::crystal)
        .value("band", KPoints::Discrete::Properties::band)
        .value("contour", KPoints::Discrete::Properties::contour)
    ;

    /*
     * Molecule (Steps + KPoints)
     */
    py::class_<Molecule>(m, "Molecule")
        .def(py::init())
        .def_readonly("pse", &Molecule::pse)
        .def("newStep", [](Molecule& m){m.newStep();})
        .def("newStep", py::overload_cast<const StepProper&>(&Molecule::newStep), "step"_a)
        .def("getStep", py::overload_cast<size_t>(&Molecule::getStep), "i"_a, py::return_value_policy::reference_internal)
        .def("getSteps",py::overload_cast<>(&Molecule::getSteps), py::return_value_policy::reference_internal)
        .def_property_readonly("nstep", &Molecule::getNstep)
        .def_property("name", &Molecule::getName, &Molecule::setName)
        .def_property("kpoints", py::overload_cast<>(&Molecule::getKPoints), &Molecule::setKPoints)
    ;

    /*
     * IO
     */
    py::enum_<IOFmt>(m,"IOFmt")
        .value("XYZ", IOFmt::XYZ)
        .value("PWI", IOFmt::PWI)
        .value("PWO", IOFmt::PWO)
        .value("LMP", IOFmt::LMP)
        .value("DMP", IOFmt::DMP)
    ;

    auto io = m.def_submodule("IO");

    py::class_<BaseParam>(io, "BaseParam")
        .def_readwrite("name", &BaseParam::name)
    ;

    py::class_<BaseConfig>(io, "BaseConfig")
        .def_readwrite("name", &BaseConfig::name)
    ;

    py::class_<IO::PWParam, BaseParam>(io, "PWParam")
        .def_readwrite("control", &IO::PWParam::control)
        .def_readwrite("system", &IO::PWParam::system)
        .def_readwrite("electrons", &IO::PWParam::electrons)
        .def_readwrite("ions", &IO::PWParam::ions)
        .def_readwrite("cell", &IO::PWParam::cell)
    ;

    py::enum_<IO::LmpAtomStyle>(io, "LmpAtomStyle")
        .value("Angle", IO::LmpAtomStyle::Angle)
        .value("Atomic", IO::LmpAtomStyle::Atomic)
        .value("Bond", IO::LmpAtomStyle::Bond)
        .value("Charge", IO::LmpAtomStyle::Charge)
        .value("Full", IO::LmpAtomStyle::Full)
        .value("Molecular", IO::LmpAtomStyle::Molecular)
    ;

    py::class_<IO::LmpConfig, BaseConfig>(io, "LmpConfig")
        .def_readwrite("style", &IO::LmpConfig::style)
        .def_readwrite("angles", &IO::LmpConfig::angles)
        .def_readwrite("bonds", &IO::LmpConfig::bonds)
        .def_readwrite("dihedrals", &IO::LmpConfig::dihedrals)
        .def_readwrite("impropers", &IO::LmpConfig::impropers)
    ;

    py::class_<IO::State>(io, "State")
        .def_readwrite("index", &IO::State::index)
        .def_readwrite("atom_fmt", &IO::State::atom_fmt)
        .def_readwrite("cell_fmt", &IO::State::cell_fmt)
    ;

    m.def("readFile",[](std::string fn, IOFmt fmt){
        IO::Data data = readFile(fn,fmt);
        return py::make_tuple<py::return_value_policy::automatic>(data.mol, std::move(data.param));
    },"filename"_a,"format"_a);
    m.def("writeFile", &writeFile, "filename"_a, "format"_a, "molecule"_a,
          "param"_a=nullptr, "config"_a=nullptr, "state"_a=IO::State{});
}
